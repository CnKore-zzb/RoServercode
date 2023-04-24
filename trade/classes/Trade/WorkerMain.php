<?php
namespace RO\Trade;

use RO\Booth\BoothOrderService;
use RO\Booth\BoothPlayerLock;
use RO\Cmd\ETradeType;
use RO\Trade\Dao\Goods;
use RO\Trade\Dao\Prohibition;

class WorkerMain extends \MyQEE\Server\WorkerTCP
{
    /**
     * @var BoothPlayerLock
     */
    public static $boothLock;

    /**
     * Worker 进程数
     *
     * @var int
     */
    protected $workerNum;

    const SERVER_TYPE_NULL    = 0;
    const SERVER_TYPE_SUPER   = 1;
    const SERVER_TYPE_SESSION = 2;
    const SERVER_TYPE_SCENE   = 3;
    const SERVER_TYPE_GATE    = 4;
    const SERVER_TYPE_RECORD  = 5;
    const SERVER_TYPE_PROXY   = 6;
    const SERVER_TYPE_STAT    = 6;
    const SERVER_TYPE_TRADE   = 8;
    const SERVER_TYPE_GLOBAL  = 9;
    const SERVER_TYPE_TEAM    = 10;
    const SERVER_TYPE_GUILD   = 11;
    const SERVER_TYPE_SOCIAL  = 12;
    const SERVER_TYPE_GZONE   = 13;

    const RET_DELETE_CHAR_REGCMD = 115;

    public function onStop()
    {
        if (null !== Server::$mysqlMaster)Server::$mysqlMaster->close();
        if (null !== Server::$redis)Server::$redis->close();
    }

    public function onStart()
    {
        $this->workerNum = $this->server->setting['worker_num'];
        $isOpen          = file_get_contents(Server::$configPath . 'isTradeOpen.txt') == '0' ? false : true;
        if (Server::$isOpen != $isOpen)
        {
            Server::$isOpen = $isOpen;
        }

        if (file_get_contents(Server::$configPath .'isTradeOpen.txt') != '1')
        {
            Server::$isOpen = false;
        }

        # 加载策划表内容
        Server::loadTableConfig();

        Server::initConnection();

        # 初始化需要禁止上架的物品
        Prohibition::initProhibitItem();

        if ($this->id == 0)
        {
            # 5 点整更新配置
            $time = strtotime(date('Y-m-d 05:00:00', time() + (intval(date('H')) > 5 ? 86400 : 0)));
            $diff = max(1, $time - time());

            // 加一个在 5 点整执行的定时器
            swoole_timer_after($diff * 1000, function()
            {
                // 加定时器, 循环执行
                swoole_timer_tick(86400 * 1000, function()
                {
                    $this->reloadConfig();
                });

                // 执行
                $this->reloadConfig();
            });
        }

        // 对 mt_rand 重新播种 see https://wiki.swoole.com/wiki/page/732.html
        mt_srand();

        // 初始化摆摊玩家进程锁
        self::$boothLock = new BoothPlayerLock();
    }

    public function onReceive($server, $fd, $fromId, $data)
    {
        # 结构体: flags len data，其中 data 由 cmd param proto 组成
        # flags: 标记位，1字节，0-无，1-压缩，2-加密
        # len:   data 长度，2字节
        # data内结构
        # cmd:   命令号，1字节
        # param: 参数，1字节
        # proto: 内容体，长度为 len - 2

        if (false === ($header = @unpack('Cflags/Slen', substr($data, 0, 3))))
        {
            $this->warn('warning pack: '. self::hexString($data));
            return;
        }

        if ($header['flags'] === 0)
        {
            # 没有任何压缩或加密，全部直接解包
            $buf = @unpack('Cflags/Slen/Ccmd/Cparam/a*proto', $data);
        }
        else
        {
            $buf = substr($data, 3);
            //if ($header['flags'] & \RO\ZoneForward::PACKET_FLAG_ENCRYPT)
            //{
            //    # 加密的，解密数据
            //    $buf = ro_decrypt($buf);
            //}
            if ($header['flags'] & \RO\ZoneForward::PACKET_FLAG_COMPRESS == \RO\ZoneForward::PACKET_FLAG_COMPRESS)
            {
                # 压缩的，处理解压
                $buf = @gzuncompress($buf);
                if (false === $buf)
                {
                    $this->warn('uncompress data error: '. self::hexString($data));
                    return;
                }
            }

            $buf = @unpack('Ccmd/Cparam/a*proto', $buf);
        }

        if (false === $buf)
        {
            # 解包数据失败
            $this->warn('unpack data fail: '. self::hexString($data));
            return;
        }

        $cmd   = $buf['cmd'];
        $param = $buf['param'];
        $proto = $buf['proto'];
        unset($buf, $tmp);

        switch ($cmd)
        {
            case \RO\Cmd\Command::TRADE_PROTOCMD:
                # 发来到交易服务的请求
                switch ($param)
                {
                    case \RO\Cmd\RecordServerTradeParam::SESSION_FORWARD_USERCMD_TRADE:
                        # 用户的请求
                        if (IS_DEBUG)
                        {
                            $msg = json_encode(new \RO\Cmd\SessionForwardUsercmdTrade($proto));
                            if (false !== $msg)
                            {
                                $this->debug('收到用户信息：' . $msg);
                            }
                        }

                        # 交易所没有开启
                        if (false === Server::isOpen())return;

                        try
                        {
                            $time = microtime(true);
                            $obj  = new \RO\Cmd\SessionForwardUsercmdTrade($proto);
                            $msg  = new ZoneForwardUser($fd, $fromId, $obj);
                            $msg->run();
                            if (($useTime = microtime(true) - $time) > 1)
                            {
                                $this->warn('来自玩家的请求执行时间过长 cmd: '. $msg->cmd . ', param: '. $msg->param .', 耗时: '. $useTime);
                            }
                        }
                        catch (\Exception $e)
                        {
                            self::$Server->warn($e->getMessage());
                        }
                        break;

                    case \RO\Cmd\RecordServerTradeParam::SESSION_FORWARD_SCENECMD_TRADE:
                        try
                        {
                            # 场景服务器回消息
                            $time = microtime(true);
                            $obj  = new \RO\Cmd\SessionForwardScenecmdTrade($proto);
                            $msg  = new ZoneForwardScene($fd, $fromId, $obj);
                            $msg->run();
                            if (($useTime = microtime(true) - $time) > 1)
                            {
                                $this->warn('来自场景的请求执行时间过长 cmd: '. $msg->cmd . ', param: '. $msg->param .', 耗时: '. $useTime);
                            }
                        }
                        catch (\Exception $e)
                        {
                            self::$Server->warn($e->getMessage());
                        }
                        break;

                    default:
                        $this->warn('No support trade_protocmd cmd: '. $cmd . ', param: '. $param);
                        break;
                }
                break;

            case \RO\Cmd\Command::SOCIAL_PROTOCMD:
                # 来自场景服务器的请求
                switch ($param)
                {
                    case \RO\Cmd\SocialParam::SOCIALPARAM_ONLINESTATUS:
                        # 玩家上线、下线发送一个请求包, see SessionUser.cpp line 47 and 80
                        # 之前的ro场景服务器是用来缓存玩家信息的（charid, zoneid, name）
                        $msg = new \RO\Cmd\OnlineStatusSocialCmd($proto);
                        if (IS_DEBUG)
                        {
                            $this->debug("[玩家上线/下线] msg:" . json_encode($msg));
                        }

                        if ($msg->online)
                        {
                            Player::setZoneId($msg->user->charid, $msg->user->zoneid);
                        }
                        else
                        {
                            Server::$players->del($msg->user->charid);
                        }
                        break;
                    default:
                        $this->warn('No support social_protocmd cmd: '. $cmd . ', param: '. $param);
                        break;
                }
                break;

            case \RO\Cmd\Command::SYSTEM_PROTOCMD:
                # 注册服务器
                switch ($param)
                {
                    case \RO\Cmd\SystemParam::REGIST_REGION_SYSCMD:
                        # 注册服务器
                        $this->register($fd, $fromId, new \RO\Cmd\RegistRegionSystemCmd($proto));
                        break;

                    case \RO\Cmd\SystemParam::COMMON_RELOAD_SYSCMD:
                        # 重新加载配置

                        // 20171214 该协议负责加载CommonConfig, 避免重新加载所有配置文件。
//                        $this->info("[重新加载配置] 收到一个重新加载配置的请求");
//                        $this->reloadConfig();

                        break;

                    default:
                        $this->warn('No support system_protocmd cmd: '. $cmd . ', param: '. $param);
                        break;
                }
                break;

            case \RO\Cmd\Command::REG_CMD:
                switch ($param)
                {
                    case self::RET_DELETE_CHAR_REGCMD:
                        # 玩家删号
                        break;
                    default:
                        $this->warn('No support REG_CMD cmd: '. $cmd . ', param: '. $param);
                        break;
                }
                break;

            default:
                $this->warn('No support cmd: '. $cmd . ', param: '. $param);
                return;
        }
    }

    public function onPipeMessage($server, $fromWorkerId, $message, $fromServerId = -1)
    {
        if ($message instanceof Command)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[管道消息] 接收到命令, command:" . json_encode($message));
            }

            switch ($message->cmd)
            {
                case Command::PROHIBIT_ITEM_CMD:
                    Prohibition::prohibitItem($message->tradeType, $message->itemId, $message->refineLv, $message->enchantId, $message->enchantBuffId);
                    break;
                case Command::PROHIBIT_ENCHANT_CMD:
                    $isBuff = false;
                    $enchantId = $message->enchantId;
                    if(isset($message->enchantBuffId) && $message->enchantBuffId > 0)
                    {
                        $enchantId = $message->enchantBuffId;
                        $isBuff    = true;
                    }

                    Prohibition::prohibitEnchant($message->tradeType, $enchantId, $isBuff);
                    break;
                case Command::PERMIT_ITEM_CMD:
                    Prohibition::permitItem($message->tradeType, $message->itemId, $message->refineLv, $message->enchantId, $message->enchantBuffId);
                    break;
                case Command::PERMIT_ENCHANT_CMD:
                    $isBuff = false;
                    $enchantId = $message->enchantId;
                    if(isset($message->enchantBuffId) && $message->enchantBuffId > 0)
                    {
                        $enchantId = $message->enchantBuffId;
                        $isBuff    = true;
                    }

                    Prohibition::permitEnchant($message->tradeType, $enchantId, $isBuff);
                    break;
            }
            return;
        }
        else
        {
            switch ($message)
            {
                case 'reloadConfig':
                    # 重新加载配置
                    Server::loadTableConfig();
                    Server::$instance->info("[管道消息] 接收到重新加载配置消息, 当前WorkerId:" . Server::$instance->server->worker_id);
                    break;

                case 'open':
                    # 开启交易所
                    Server::$isOpen = true;
                    Server::$instance->info("[管道消息] 接收到开启消息, 当前WorkerId:" . Server::$instance->server->worker_id);
                    break;

                case 'close':
                    # 关闭交易所
                    Server::$isOpen = false;
                    Server::$instance->info("[管道消息] 接收到关闭消息, 当前WorkerId:" . Server::$instance->server->worker_id);
                    break;

                default:
                    $this->warn('未知 onPipeMessage 数据');
                    print_r($message);
                    break;
            }
        }
    }

    /**
     * 禁止物品上架
     *
     * @param Prohibition $prohibition
     * @param bool $isCancelOff 是否下架商品
     */
    public function prohibitItem($prohibition, $isCancelOff = true)
    {
        $command = new Command();
        if ($prohibition->type == Prohibition::ENCHANT_TYPE)
        {
            $command->cmd = Command::PERMIT_ENCHANT_CMD;
            $command->enchantId = $prohibition->enchantId;
            $command->enchantBuffId = $prohibition->enchantBuffId;
            $command->tradeType = $prohibition->tradeType;
        }
        else
        {
            $command->cmd = Command::PROHIBIT_ITEM_CMD;
            $command->itemId = $prohibition->itemId;
            $command->refineLv = $prohibition->refineLv;
            $command->enchantId = $prohibition->enchantId;
            $command->enchantBuffId = $prohibition->enchantBuffId;
            $command->tradeType = $prohibition->tradeType;
        }
        $this->sendMessageToAllWorker($command, \MyQEE\Server\Message::SEND_MESSAGE_TYPE_WORKER);

        if ($isCancelOff)
        {
            if ($command->tradeType === ETradeType::ETRADETYPE_TRADE || $command->tradeType === ETradeType::ETRADETYPE_ALL)
            {
                if ($prohibition->type == Prohibition::ENCHANT_TYPE)
                {
                    ActSell::cancelOffByEnchant($prohibition->enchantId, $prohibition->enchantBuffId);
                }
                else
                {
                    ActSell::cancelOffByItem($prohibition->itemId, $prohibition->refineLv, $prohibition->enchantId, $prohibition->enchantBuffId);
                }
            }

            if ($command->tradeType === ETradeType::ETRADETYPE_BOOTH || $command->tradeType === ETradeType::ETRADETYPE_ALL)
            {
                BoothOrderService::cancelOffByCondition($prohibition);
            }
        }

        Server::$instance->info("[安全指令] 禁止操作成功! 交易类型:{$prohibition->tradeType}, 物品:{$prohibition->itemId}, 精炼等级:{$prohibition->refineLv}, 附魔id:{$prohibition->enchantId}, 特殊附魔id:{$prohibition->enchantBuffId}");
    }

    /**
     * 允许物品上架
     *
     * @param Prohibition $prohibition
     */
    public function permitItem($prohibition)
    {
        $command = new Command();
        $command->tradeType = $prohibition->tradeType;
        if ($prohibition->type == Prohibition::ENCHANT_TYPE)
        {
            $command->cmd = Command::PERMIT_ENCHANT_CMD;
            $command->enchantId = $prohibition->enchantId;
            $command->enchantBuffId = $prohibition->enchantBuffId;
        }
        else
        {
            $command->cmd = Command::PERMIT_ITEM_CMD;
            $command->itemId = $prohibition->itemId;
            $command->refineLv = $prohibition->refineLv;
            $command->enchantId = $prohibition->enchantId;
            $command->enchantBuffId = $prohibition->enchantBuffId;
        }
        $this->sendMessageToAllWorker($command, \MyQEE\Server\Message::SEND_MESSAGE_TYPE_WORKER);
        Server::$instance->info("[安全指令] 许可操作成功! 交易类型:{$prohibition->tradeType}, 物品:{$prohibition->itemId}, 精炼等级:{$prohibition->refineLv}, 附魔id:{$prohibition->enchantId}, 特殊附魔id:{$prohibition->enchantBuffId}");
    }

    /**
     * 交易安全指令
     *
     *
     * @param Command $cmd
     *          禁止物品参数构造如下:
     *          $cmd = new Command();
     *          $cmd->type = Prohibition::ITEM_TYPE;
     *          $cmd->itemId = 40007;
     *          $cmd->refineLv = 0;
     *          $cmd->enchantId = 0;
     *          $cmd->enchantBuffId = 0;
     *          $cmd->tradeType = ETradeType::ETRADETYPE_TRADE // 根据这个判断是交易所还是摆摊
     *
     *          禁止某项附魔参数构造如下:
     *          $cmd = new Command();   #普通附魔
     *          $cmd->type = Prohibition::ENCHANT_TYPE;
     *          $cmd->enchantId = 0;
     *
     *          $cmd = new Command();   #特殊附魔
     *          $cmd->type = Prohibition::ENCHANT_TYPE;
     *          $cmd->enchantBuffId = 0;
     * @throws \Exception
     * @return bool
     */
    public function tradeSecurityCommand($cmd)
    {
        if (!$cmd instanceof Command)
        {
            throw new \InvalidArgumentException('必须是\RO\Trade\Command类型');
        }

        $cmd->type = isset($cmd->type) ? $cmd->type : Prohibition::ITEM_TYPE;
        $cmd->itemId = isset($cmd->itemId) ? $cmd->itemId : 0;
        $cmd->refineLv = isset($cmd->refineLv) ? $cmd->refineLv : 0;
        $cmd->enchantId = isset($cmd->enchantId) ? $cmd->enchantId : 0;
        $cmd->enchantBuffId = isset($cmd->enchantBuffId) ? $cmd->enchantBuffId : 0;
        $cmd->tradeType = isset($cmd->tradeType) ? $cmd->tradeType : ETradeType::ETRADETYPE_TRADE;

        switch ($cmd->type)
        {
            case Prohibition::ITEM_TYPE:
                if (!\RO\Trade\Item::get($cmd->itemId))
                    throw new \Exception('物品不存在');

                if ($cmd->refineLv > 15)
                    throw new \Exception('精练值不能大于15');

                if ($cmd->enchantId && !RoleDataConfig::getRoleData($cmd->enchantId))
                {
                    throw new \Exception('附魔属性id不存在!');
                }

                if ($cmd->enchantBuffId)
                {
                    $isExist = false;
                    foreach (EnchantConfig::getConfigs() as $cfg)
                    {
                        if ($cfg->getEnchantAttrByBuffId($cmd->enchantBuffId))
                        {
                            $isExist = true;
                            break;
                        }
                    }

                    if (!$isExist)
                    {
                        throw new \Exception('特殊附魔id不存在!');
                    }
                }
                break;
            case Prohibition::ENCHANT_TYPE:
                if ($cmd->enchantBuffId)
                {
                    $isExist = false;
                    foreach (EnchantConfig::getConfigs() as $cfg)
                    {
                        if ($cfg->getEnchantAttrByBuffId($cmd->enchantBuffId))
                        {
                            $isExist = true;
                            break;
                        }
                    }

                    if (!$isExist)
                    {
                        throw new \Exception('特殊附魔id不存在!');
                    }
                }
                else
                {
                    if (!RoleDataConfig::getRoleData($cmd->enchantId))
                    {
                        throw new \Exception('附魔属性id不存在!');
                    }
                }
                break;
            default:
                return false;
        }

        $prohibit = Prohibition::getByUniqueId(Prohibition::generateUniqueId($cmd->tradeType, $cmd->type, [
            $cmd->itemId, $cmd->refineLv, $cmd->enchantId, $cmd->enchantBuffId
        ]));

        try
        {
            if ($prohibit)
            {
                if ($prohibit->status == Prohibition::STATUS_ACTIVE)
                {
                    $prohibit->status = Prohibition::STATUS_DELETED;
                    if (false !== $prohibit->update())
                    {
                        $this->permitItem($prohibit);
                        return true;
                    }
                    else
                    {
                        throw new \Exception("更新禁止命令失败");
                    }
                }
                else
                {
                    $prohibit->status = Prohibition::STATUS_ACTIVE;
                    if (false !== $prohibit->update())
                    {
                        $this->prohibitItem($prohibit);
                        return true;
                    }
                    else
                    {
                        throw new \Exception("更新禁止命令失败");
                    }
                }
            }
            else
            {
                $prohibit = new Prohibition();
                $prohibit->itemId = $cmd->itemId;
                $prohibit->refineLv = isset($cmd->refineLv) ? (int) $cmd->refineLv : 0;
                $prohibit->enchantId = isset($cmd->enchantId) ? (int) $cmd->enchantId : 0;
                $prohibit->enchantBuffId = isset($cmd->enchantBuffId) ? (int) $cmd->enchantBuffId : 0;
                $prohibit->type = $cmd->type;
                $prohibit->status = Prohibition::STATUS_ACTIVE;
                $prohibit->tradeType = $cmd->tradeType;

                if ($prohibit->insert())
                {
                    $this->prohibitItem($prohibit);
                    return true;
                }
                else
                {
                    throw new \Exception("插入禁止命令失败");
                }
            }

        }
        catch (\Exception $e)
        {
            Server::$instance->warn('[安全指令] 错误信息:'. $e->getMessage());
            throw $e;
        }
    }

    /**
     * 重新加载配置
     *
     * 如果配置关闭交易所,重新加载配置后交易所则不会接收新的请求
     */
    public function reloadConfig()
    {
        global $options;
        try
        {
            if (isset($options['local']))
            {
                Server::formatLua2json(true);
            }
            else
            {
                # 重新生成配置文件
                Server::formatLua2json();
            }
        }
        catch (\Exception $e)
        {
            $this->warn("[重新加载配置] lua2json 失败 " . $e->getMessage());

            return;
        }

        if (true === Server::isOpen())
        {
            # 交易所开着的，执行关闭
            Server::closeTrade();

            # 10秒后执行
            $this->info("[重新加载配置] 10秒后开始执行配置加载, WorkerId:{$this->server->worker_id}");
            $this->server->after(10000, [$this, 'doReloadConfig']);
        }
        else
        {
            $this->doReloadConfig();
        }
    }

    function doReloadConfig()
    {
        do
        {
            $this->info("[重新加载配置] 开始执行配置加载, WorkerId:{$this->server->worker_id}");

            $exchange = Server::loadConfig('Exchange');
            if (!$exchange)
            {
                $this->warn('[重新加载配置] 读取 Exchange.json 配置失败');
                break;
            }
            $item = Server::loadConfig('Item');
            if (!$exchange)
            {
                $this->warn('[重新加载配置] 读取 Item.json 配置失败');
                break;
            }
            $equip = Server::loadConfig('Equip');
            if (!$equip)
            {
                $this->warn('[重新加载配置] 读取 Equip.json 配置失败');
                break;
            }

            $equipComposeConfig = Server::loadConfig('EquipCompose');

            # 更新所有 item 信息
            $changedOverlap  = [];
            $changedCategory = [];
            $oldItem = [];
            foreach (Server::$item as $k => $v)
            {
                $oldItem[$k] = $v;
            }
            $branchKey   = Server::$branchKeys[Server::$branchID];
            foreach ($exchange as $itemId => $itemExc)
            {
                if (!isset($item[$itemId]))
                {
                    $this->warn("[重新加载配置] Exchange 策划表配置异常，{$itemId} 在 Item 中不存在");
                }

                if ($branchKey === 'TF') {
                    $tradeTime   = isset($itemExc['TFTradeTime']) ? strtotime($itemExc['TFTradeTime']) ?: 0 : 0;
                    $unTradeTime = isset($itemExc['TFUnTradeTime']) ? strtotime($itemExc['TFUnTradeTime']) ?: 0 : 0;
                } else {
                    $tradeTime   = isset($itemExc['TradeTime']) ? strtotime($itemExc['TradeTime']) ?: 0 : 0;
                    $unTradeTime = isset($itemExc['UnTradeTime']) ? strtotime($itemExc['UnTradeTime']) ?: 0 : 0;
                }

                $minPriceType = $itemExc['MinPrice']['type'] ?? ExchangeItemNode::MIN_PRICE_TYPE_SELF;
                $mainEquipId  = 0;
                if ($minPriceType === ExchangeItemNode::MIN_PRICE_TYPE_EQUIP_NEW_COMPOSE)
                {
                    $id = $itemExc['MinPrice']['equipcomposeid'] ?? 0;
                    $mainEquipId = $equipComposeConfig[$id]['Material']['1']['id'] ?? 0;
                }

                $data = [
                    'isTrade'           => isset($itemExc['Trade']) ? (int)$itemExc['Trade'] : 0,
                    'isOverlap'         => isset($itemExc['Overlap']) ? (int)$itemExc['Overlap'] : 0,
                    'name'              => $itemExc['NameZh'],
                    'exchangeNum'       => isset($itemExc['ExchangeNum']) ? (int)$itemExc['ExchangeNum'] : 0,
                    'equipType'         => isset($equip[$itemId]['EquipType']) ? (int)$equip[$itemId]['EquipType'] : 0,
                    'minPriceType'      => $minPriceType,
                    'mainEquipId'       => $mainEquipId,
                    'equipUpgradeId'    => isset($itemExc['MinPrice']) ? isset($itemExc['MinPrice']['equip_upgrade_id']) ? (int) $itemExc['MinPrice']['equip_upgrade_id'] : 0 : 0,
                    'itemType'          => isset($item[$itemId]['Type']) ? (int)$item[$itemId]['Type'] : 0,
                    'publicityShowTime' => isset($itemExc['ShowTime']) ? (int)$itemExc['ShowTime'] : 0,
                    'category'          => isset($itemExc['Category']) ? (int)$itemExc['Category'] : 0,
                    'fashionType'       => isset($itemExc['FashionType']) ? (int)$itemExc['FashionType'] : 0,
                    'moneyType'         => isset($itemExc['MoneyType']) ? (int)$itemExc['MoneyType'] : 131,
                    'tradeTime'         => $tradeTime,
                    'unTradeTime'       => $unTradeTime,
                ];

                $old = isset($oldItem[$itemId]) ? $oldItem[$itemId] : false;
                if (false === $old)
                {
                    $diff = $data;
                    if (1 == $data['isTrade'])
                    {
                        # 先在数据库里查询下
                        $rs = Server::$mysqlMaster->query("SELECT * FROM `trade_item_info` WHERE `itemid` = '" . $itemId . "'");
                        if ($rs)
                        {
                            if ($rs->num_rows > 0)
                            {
                                $row  = $rs->fetch_assoc();
                                $diff = array_merge($row, $diff);
                            }
                            elseif (!Server::$mysqlMaster->query("INSERT INTO `trade_item_info` (`itemid`) VALUES ('" . $itemId . "')"))
                            {
                                # 更新到数据库
                                $this->warn("SQL: " . Server::$mysqlMaster->last_query . ", Error: " . Server::$mysqlMaster->error);
                            }

                            $rs->free();
                        }
                    }
                    $this->info("[重新加载配置] 新增 ItemId: {$itemId}, ". json_encode($data, JSON_UNESCAPED_UNICODE));
                }
                else
                {
                    unset($oldItem[$itemId]);
                    $diff = array_diff_assoc($data, $old);
                    if (!$diff)continue;        # 没有修改过内容

                    $before = [];
                    foreach ($diff as $k => $v)
                    {
                        $before[$k] = $old[$k];
                    }

                    if (isset($diff['isOverlap']))
                    {
                        $changedOverlap[$itemId] = $diff['isOverlap'];
                    }

                    if (isset($diff['category']))
                    {
                        $changedCategory[$itemId] = $diff['category'];
                    }

                    # 通知玩家原物品转为不能交易的物品
                    if (isset($diff['isTrade']) && $diff['isTrade'] == 0)
                    {
                        Goods::noticePlayerItemCannotTrade($itemId);
                    }

                    $this->info("[重新加载配置] 更新 ItemId: {$itemId}, before: ". json_encode($before, JSON_UNESCAPED_UNICODE) .'. after: '. json_encode($diff, JSON_UNESCAPED_UNICODE));
                }
                Server::$item->set($itemId, $diff);
            }

            if (count($oldItem))
            {
                # 被移除的物品
                foreach ($oldItem as $itemId => $itemArr)
                {
                    Server::$item->set($itemId, ['isTrade' => 0]);
                }
            }

            # 新配置物品更改了是否堆叠
            if ($changedOverlap)foreach ($changedOverlap as $itemId => $overlap)
            {
                $result = Server::$mysqlMaster->query("SELECT `item_key` FROM `trade_item_list` WHERE `item_id` = '{$itemId}'");
                if ($result)
                {
                    while ($row = $result->fetch_assoc())
                    {
                        Server::$itemList->set($row['item_key'], ['isOverlap' => $overlap]);
                    }
                    $result->free();
                }
            }

            if ($changedOverlap || $changedCategory)
            {
                # 通知列表更新缓存
                $this->task('list.reload', 0);
            }

            # 给所有的进程发送重新加载配置的请求
            $this->sendMessageToAllWorker('reloadConfig');
        }
        while (false);

        $isOpen = file_get_contents(Server::$configPath .'isTradeOpen.txt') == 1 ? true : false;
        if ($isOpen)
        {
            Server::openTrade();
        }
    }

    /**
     * 注册服务器
     *
     * @param int $fd
     * @param int $fromId
     * @param \RO\Cmd\RegistRegionSystemCmd $msg
     */
    protected function register($fd, $fromId, \RO\Cmd\RegistRegionSystemCmd $msg)
    {
        switch ($msg->servertype)
        {
            case self::SERVER_TYPE_STAT:
            case self::SERVER_TYPE_TRADE:
            case self::SERVER_TYPE_GLOBAL:
            case self::SERVER_TYPE_GUILD:
            case self::SERVER_TYPE_PROXY:
                $this->info("register server fd: $fd, fromId: $fromId, server type: {$msg->servertype}");
                break;

            default:
                if ($msg->zoneid)
                {
                    $data = [
                        'fd'     => $fd,
                        'fromId' => $fromId,
                    ];
                    Server::$zone->set($msg->zoneid, $data);
                }
                $this->info("register server fd: $fd, fromId: $fromId, zoneId: {$msg->zoneid}, server type: {$msg->servertype}");
                break;
        }
    }

    /**
     * 获取 bin2hex 的字符
     *
     * @param $data
     * @return string
     */
    protected static function hexString($data)
    {
        $str = '';
        for($i = 0; $i < strlen($data); ++ $i)
        {
            $str .= bin2hex($data[$i])." ";
        }

        return $str;
    }
}
