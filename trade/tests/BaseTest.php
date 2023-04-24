<?php
include __DIR__ . '/../vendor/autoload.php';
include __DIR__ . '/config.php';

/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/6/27
 * Time: 下午3:09
 */
class BaseTest
{
    /**
     * @var \Swoole\Client
     */
    public $cli;

    public $ip = "0.0.0.0";

    public $port = "7090";

    public $charId;

    public $autoForward = true;

    protected $isClose = false;

    /** @var \RO\MySQLi */
    public $db;

    public $config = [];

    public function __construct($host, $port)
    {
        $this->ip = $host;
        $this->port = $port;
        if (!$this->charId)
        {
           $this->randCharId();
        }

        $cli = new \Swoole\Client(SWOOLE_SOCK_TCP, SWOOLE_SOCK_ASYNC);
        $cli->set([
                      'open_eof_check'        => false,
                      'open_eof_split'        => false,
                      'open_length_check'     => true,
                      'package_length_type'   => 'S',
                      'package_length_offset' => 1,
                      'package_body_offset'   => 3,
                      'package_max_length'    => 65535,
                  ]);

        $cli->on("connect", [
            $this,
            "onConnect"
        ]);
        $cli->on("receive", [
            $this,
            "onReceive"
        ]);
        $cli->on("error", [
            $this,
            "onError"
        ]);
        $cli->on("close", [
            $this,
            "onClose"
        ]);
        $this->cli = $cli;
        $cli->connect($this->ip, $this->port);

        $this->initConfig();
        $branchConfig = self::loadBranchConfig($this->config['BranchConfig']);
        $this->db = new \RO\MySQLi($branchConfig['mysql']);
    }

    public function randCharId()
    {
        static $n = 0;
        $n++;
        return $this->charId = time() + $n + mt_rand(1, 99999);
    }

    public function initConfig()
    {
        $conf = [
            'BoothConfig',
            'BranchConfig',
            'Exchange',
        ];

        foreach ($conf as $value)
        {
            $this->config[$value] = $this->loadConfig($value);
        }
    }

    /**
     * 加载一个json配置
     *
     * @param string $file 不需要带 .json 后缀
     * @return array|false
     */
    public function loadConfig($file)
    {
        return @json_decode(file_get_contents(TRADE_CONFIG . $file .'.json'), true);
    }

    public function calcDiscountPrice($price, $quota = null)
    {
        if ($quota === null)
        {
            $quota = $price;
        }
        $discount = $this->config['BoothConfig']['quota_zeny_discount'] ?? 4;
        return $price - intval($quota * 1 / $discount);
    }

    public static function loadBranchConfig($config)
    {
        $branchKey = TRADE_BRANCH;

        if (!isset($config[$branchKey]))
        {
            throw new \Exception("指定的分支 {$branchKey} 配置不存在");
        }

        $config = $config[$branchKey];
        $rs     = [
            'mysql'    => [],
            'redis'    => [],
            'globalDB' => [],
            'gameDb'   => [],
        ];

        $myDb = TRADE_DATABASE;
        $list = [];
        foreach ($config['TradeDataBase'] as $st)
        {
            # 遍历一遍获取所有含 database 的配置，过滤掉不需要的
            if (isset($st['database']) && $st['database'] === $myDb)
            {
                $list[] = $st;
            }
        }
        # 如果一个都没有，说明是老的配置，使用全部的配置
        if (!$list)
        {
            $list = $config['TradeDataBase'];
        }
        foreach ($list as $st)
        {
            if (!isset($rs['mysql']['master']))
            {
                $rs['mysql'] = [
                    'user'    => $st['user'],
                    'pass'    => $st['password'],
                    'master'  => "{$st['ip']}:{$st['port']}",
                    'slave'   => [],
                    'charset' => 'utf8mb4',
                    'db'      => TRADE_DATABASE,
                ];
            }
            else
            {
                $rs['mysql']['slave'][] = "{$st['ip']}:{$st['port']}";
            }
        }
        unset($myDb, $list);

        # Global 数据库
        $st = null;
        foreach ($config['DataBase'] as $tmp)
        {
            if (isset($tmp['database']) && $tmp['database'] === 'ro_global')
            {
                $st = $tmp;
            }
        }

        if (!$st)
        {
            reset($config['DataBase']);
            $st = current($config['DataBase']);
        }

        $rs['globalDB'] = [
            'user'    => $st['user'],
            'pass'    => $st['password'],
            'master'  => "{$st['ip']}:{$st['port']}",
            'slave'   => [],
            'charset' => 'utf8mb4',
            'db'      => 'ro_global',
        ];

        // 游戏数据库
        reset($config['DataBase']);
        $st           = current($config['DataBase']);
        $rs['gameDb'] = [
            'user'    => $st['user'],
            'pass'    => $st['password'],
            'master'  => "{$st['ip']}:{$st['port']}",
            'slave'   => [],
            'charset' => 'utf8mb4',
            'db'      => TRADE_DATABASE,
        ];

        $rs['redis'][] = "{$config['Redis']['ip']}:{$config['Redis']['port']}";

        return $rs;
    }

    public function onConnect(\Swoole\Client $cli)
    {
        $msg             = new \RO\Cmd\RegistRegionSystemCmd();
        $msg->servertype = 2;
        $cli->send($this->createMsgData($msg));
        $this->onTest($cli);
    }

    public function onTest(\Swoole\Client $cli)
    {

    }

    public function reqItemPrice(int $itemId, int $tradeType = \RO\Cmd\ETradeType::ETRADETYPE_ALL, \RO\Cmd\ItemData $itemData = null)
    {
        if ($itemData === null)
        {
            $itemData = new \RO\Cmd\ItemData();
            $itemData->equip    = new \RO\Cmd\EquipData();
            $itemData->base     = new \RO\Cmd\ItemInfo();
        }

        $itemData->base->id = $itemId;
        $reqMsg = new \RO\Cmd\ReqServerPriceRecordTradeCmd();
        $reqMsg->charid = $this->charId;
        $reqMsg->itemData = $itemData;
        $reqMsg->trade_type = $tradeType;
        $this->sendUserMsg($reqMsg);
    }

    public function boothOpen($charId = null)
    {
        $cmd = new \RO\Cmd\BoothOpenTradeCmd();
        if ($charId === null)
        {
            $cmd->charid = $this->charId;
        }
        else
        {
            $cmd->charid = $charId;
        }
        $this->sendSceneMsg($cmd);
    }

    public function boothClose($charId = null)
    {
        $cmd = new \RO\Cmd\BoothOpenTradeCmd();
        if ($charId === null)
        {
            $cmd->charid = $this->charId;
        }
        else
        {
            $cmd->charid = $charId;
        }
        $cmd->open = 0;
        $this->sendSceneMsg($cmd);
    }

    public function decode($data)
    {
        if (false === ($header = @unpack('Cflags/Slen', substr($data, 0, 3))))
        {
            return null;
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
                    return null;
                }
            }

            $buf = @unpack('Ccmd/Cparam/a*proto', $buf);
        }
        return $buf;
    }

    public function onReceive(\Swoole\Client $cli, string $data)
    {
        $params = $this->decode($data);
        $cmd = $params['cmd'] ?? 0;
        $param = $params['param'] ?? 0;
        $proto = $params['proto'] ?? 0;
        $msg = null;
        switch ($cmd)
        {
            case \RO\Cmd\Command::TRADE_PROTOCMD:
                switch ($param)
                {
                    case \RO\Cmd\RecordServerTradeParam::REDUCE_ITEM_RECORDTRADE:
                        $msg      = new \RO\Cmd\ReduceItemRecordTrade($proto);
                        if ($this->autoForward)
                        {
                            $msg->ret = \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS;
                            $this->sendSceneMsg($msg);
                        }
                        break;
                    case \RO\Cmd\RecordServerTradeParam::REDUCE_MONEY_RECORDTRADE:
                        $msg = new \RO\Cmd\ReduceMoneyRecordTradeCmd($proto);
                        if ($this->autoForward) {
                            $msg->ret = \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS;
                            $this->sendSceneMsg($msg);
                        }
                        break;
                    case \RO\Cmd\RecordServerTradeParam::ADD_ITEM_RECORDTRADE:
                        $msg = new \RO\Cmd\AddItemRecordTradeCmd($proto);
                        break;
                    case \RO\Cmd\RecordServerTradeParam::SESSION_TO_ME_RECORDTRADE:
                        $sessionMsg = new \RO\Cmd\SessionToMeRecordTrade($proto);
                        $params = @unpack('Ccmd/Cparam/a*proto', $sessionMsg->data);
                        $cmd = $params['cmd'] ?? 0;
                        $param = $params['param'] ?? 0;
                        $proto = $params['proto'] ?? 0;
                        switch ($cmd)
                        {
                            case \RO\Cmd\Command::RECORD_USER_TRADE_PROTOCMD:
                                switch ($param)
                                {
                                    case \RO\Cmd\RecordUserTradeParam::BOOTH_PLAYER_PENDING_LIST:
                                        $msg = new \RO\Cmd\BoothPlayerPendingListCmd($proto);
                                        break;
                                    case \RO\Cmd\RecordUserTradeParam::REQ_SERVER_PRICE_RECORDTRADE:
                                        $msg = new \RO\Cmd\ReqServerPriceRecordTradeCmd($proto);
                                        break;
                                    case \RO\Cmd\RecordUserTradeParam::DETAIL_PENDING_LIST_RECORDTRADE:
                                        $msg = new \RO\Cmd\DetailPendingListRecordTradeCmd($proto);
                                        break;
                                    case \RO\Cmd\RecordUserTradeParam::SELL_ITEM_RECORDTRADE:
                                        $msg = new \RO\Cmd\SellItemRecordTradeCmd($proto);
                                        break;
                                    case \RO\Cmd\RecordUserTradeParam::RESELL_PENDING_RECORDTRADE:
                                        $msg = new \RO\Cmd\ResellPendingRecordTrade($proto);
                                        break;
                                    case \RO\Cmd\RecordUserTradeParam::BUY_ITEM_RECORDTRADE:
                                        $msg = new \RO\Cmd\BuyItemRecordTradeCmd($proto);
                                        break;
                                    case \RO\Cmd\RecordUserTradeParam::CANCEL_PENDING_RECORDTRADE:
                                        $msg = new \RO\Cmd\CancelItemRecordTrade($proto);
                                        break;
                                    case \RO\Cmd\RecordUserTradeParam::UPDATE_ORDER_TRADE_PARAM:
                                        $msg = new \RO\Cmd\UpdateOrderTradeCmd($proto);
                                        break;
                                    case \RO\Cmd\RecordUserTradeParam::ITEM_SELL_INFO_RECORDTRADE:
                                        $msg = new \RO\Cmd\ItemSellInfoRecordTradeCmd($proto);
                                        break;
                                    default:
                                        if (IS_DEBUG) echo "receive RecordServerTradeParam cmd:{$cmd}, param:{$param}", PHP_EOL;
                                }
                                break;
                            case \RO\Cmd\Command::SCENE_USER2_PROTOCMD:
                                switch ($param)
                                {
                                    case \RO\Cmd\User2Param::USER2PARAM_SYSMSG:
                                        $msg = new \RO\Cmd\SysMsg($proto);
                                        if (IS_DEBUG) echo "receive SysCmdMsg id:{$msg->id}", PHP_EOL;
                                        break;
                                }
                                break;
                        }
                        break;
                    default:
                        if (IS_DEBUG) echo "receive cmd:{$cmd}, param:{$param}", PHP_EOL;
                }
                break;

            default:
                if (IS_DEBUG) echo "receive cmd:{$cmd}, param:{$param}", PHP_EOL;
        }

        return $msg;
    }

    public function onClose(\Swoole\Client $cli)
    {
        swoole_event_exit();
    }

    public function onError(\Swoole\Client $cli)
    {
        $this->fail(socket_strerror($cli->errCode));
    }

    public function send($data):bool
    {
        return $this->cli->send($data);
    }

    /**
     * @param mixed $msg
     * @return string
     */
    public function createForwardSceneMsg($msg):string
    {
        $charId = $this->charId;
        if (isset($msg->charid) && $msg->charid != $charId)
        {
            $charId = $msg->charid;
        }

        $forward         = new \RO\Cmd\SessionForwardScenecmdTrade();
        $forward->charid = $charId;
        $forward->zoneid = 1;
        $forward->name   = $charId;
        $forward->data   = pack('CCa*', $msg->cmd, $msg->param, $msg->serialize());
        return $this->createMsgData($forward);
    }

    /**
     * @param mixed $msg
     * @return string
     */
    public function createForwardUserMsg($msg):string
    {
        $charId = $this->charId;
        if (isset($msg->charid) && $msg->charid != $charId)
        {
            $charId = $msg->charid;
        }

        $forward         = new \RO\Cmd\SessionForwardUsercmdTrade();
        $forward->charid = $charId;
        $forward->zoneid = 1;
        $forward->data   = pack('CCa*', $msg->cmd, $msg->param, $msg->serialize());
        return $this->createMsgData($forward);
    }

    public function createMsgData($msg):string
    {
        try
        {
            $flags = 0;
            $body  = pack('CCa*', $msg->cmd, $msg->param, $msg->serialize());
            $len   = strlen($body);
            return pack('CS', $flags, $len) . $body;
        }
        catch (\Exception $e)
        {
            throw new Exception('[解析失败]' . get_class($msg) . ', ' . $e->getMessage());
        }
    }

    public function sendUserMsg($msg, int $delay = null)
    {
        if ($delay === null)
        {
            return $this->cli->send($this->createForwardUserMsg($msg));
        }
        else
        {
            $class = get_class($msg);
            $msg = new $class($msg->serialize());
            swoole_timer_after($delay, function() use ($msg) {
                $this->cli->send($this->createForwardUserMsg($msg));
            });
        }
    }

    public function sendSceneMsg($msg)
    {
        return $this->cli->send($this->createForwardSceneMsg($msg));
    }

    public function fail($msg = "fail")
    {
        if (!$this->isClose)
        {
            $this->isClose = true;
            swoole_timer_after(100, function() {
                $this->isClose = $this->cli->close();
            });
        }

        throw new Exception($msg);
    }

    public function success($msg = "")
    {
        if ($msg === '')
        {
            echo 'success', PHP_EOL;
        }
        else
        {
            echo $msg;
        }

        if (!$this->isClose)
        {
            $this->isClose = true;
            swoole_timer_after(100, function()
            {
                $this->isClose = $this->cli->close();
            });
        }
    }
}

class TestTradeGoodsList extends BaseTest
{
    public function onTest(\Swoole\Client $cli)
    {
        $i = 10;
        while ($i--)
        {
            $cmd = new \RO\Cmd\DetailPendingListRecordTradeCmd();
            $cmd->charid = $this->randCharId();
            $cmd->search_cond = $search = new \RO\Cmd\SearchCond();
            $search->item_id = 52320;
            $search->page_index = 0;
            $search->page_count = 10;
            $search->trade_type = \RO\Cmd\ETradeType::ETRADETYPE_ALL;
            $this->sendUserMsg($cmd);
        }
    }

    public function onReceive(\Swoole\Client $cli, string $data)
    {
        $msg = parent::onReceive($cli, $data);
        if ($msg instanceof \RO\Cmd\DetailPendingListRecordTradeCmd)
        {
            var_dump(json_encode($msg->lists));
        }
    }
}

class TestCancel extends BaseTest
{
    public function onTest(\Swoole\Client $cli)
    {
        $i = 2;
        while ($i--)
        {
            $cancelMsg           = new \RO\Cmd\CancelItemRecordTrade();
            $cancelMsg->type     = \RO\Cmd\ETradeType::ETRADETYPE_BOOTH;
            $cancelMsg->order_id = 1123;
            $cancelMsg->charid   = 1531294882;
            $this->sendUserMsg($cancelMsg);
        }
    }

    public function onReceive(\Swoole\Client $cli, string $data)
    {
        $msg = parent::onReceive($cli, $data);
        if ($msg instanceof \RO\Cmd\CancelItemRecordTrade)
        {
            if ($msg->ret == \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS)
            {
                $this->success("取消成功");
            }
            else
            {
                $this->fail("取消订单失败");
            }
        }
    }
}

class TestBriefList extends BaseTest
{
    public function onTest(\Swoole\Client $cli)
    {
        parent::onTest($cli); // TODO: Change the autogenerated stub
        $i = 10;
        while($i--)
        {
            $msg = new \RO\Cmd\BriefPendingListRecordTradeCmd();
            $msg->charid = 4294967298;
            $msg->category = 1004;
            $msg->job = 0;
            $msg->pub_lists = [];
            $msg->lists = [];
            $this->sendUserMsg($msg);

            $msg = new \RO\Cmd\DetailPendingListRecordTradeCmd();
            $msg->charid = 4294967298;
            $msg->search_cond = new \RO\Cmd\SearchCond();
            $msg->lists = [];
            $this->sendUserMsg($msg);


            $msg = new \RO\Cmd\DetailPendingListRecordTradeCmd();
            $msg->charid = 4294967299;
            $msg->search_cond = new \RO\Cmd\SearchCond();
            $msg->lists = [];
            $this->sendUserMsg($msg);
        }

        $msg = new \RO\Cmd\BriefPendingListRecordTradeCmd();
        $msg->charid = 4294967299;
        $msg->category = 1004;
        $msg->job = 0;
        $msg->pub_lists = [];
        $msg->lists = [];
        $this->sendUserMsg($msg);
    }

    public function onReceive(\Swoole\Client $cli, string $data)
    {
        $msg = parent::onReceive($cli, $data);

    }
}
