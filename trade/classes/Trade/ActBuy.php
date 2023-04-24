<?php
namespace RO\Trade;

use RO\Trade\Dao\ItemList;

/**
 * 购买物品动作对象
 *
 * @package RO\Trade
 */
class ActBuy
{
    /**
     * 每个任务进程最大处理排队购买列表数
     */
    //const MAX_QUEUE_PER_TASK = 300;

    const SYS_MSG_CFG_ERROR                 = 10000;       //策划表不一致
    const SYS_MSG_DB_ERROR                  = 10001;       //数据库操作失败
    const SYS_MSG_SYS_ERROR                 = 10002;       //系统异常

    const SYS_MSG_BUY_PUBLICITY_BUY_SUCCESS = 10401;       //您在公示期抢购中成功购买[63cd4e]%s[-]个[63cd4e]%s[-]，共扣除[63cd4e]%s[-]Zeny，返还[63cd4e]%s[-]Zeny。
    const SYS_MSG_BUY_PUBLICITY_PAY_SUCCESS = 10402;       //您在公示期活动中抢购了[63cd4e]%s[-]个[63cd4e]%s[-]，共预先扣除[63cd4e]%s[-]Zeny。
    const SYS_MSG_BUY_PUBLICITY_FAIL_COUNT  = 10403;       //购买数量超过限制
    const SYS_MSG_BUY_PUBLICITY_FAIL2       = 10404;       //抢购失败
    const SYS_MSG_BUY_PUBLICITY_FAIL        = 10405;       //您在抢购期期间抢购[63cd4e]%s[-]失败，返还[63cd4e]%s[-]Zeny，请在[63cd4e]交易记录[-]中领取退款。

    const SYS_MSG_BUY_INVALID_COUNT         = 10150;       //购买-购买的个数不正确
    const SYS_MSG_BUY_CANNOT_FIND_PENDING   = 10151;       //购买-找不到相应的挂单，挂单已售空
    const SYS_MSG_BUY_PENDING_WAS_LOCKED    = 10152;       //购买-无法购买，挂单被锁定
    const SYS_MSG_BUY_INVALID_PARAMS        = 10153;       //购买-无法购买，请求参数有误
    const SYS_MSG_BUY_MONEY_NOT_ENOUGH      = 10154;       //购买-无法购买，金钱不足
    const SYS_MSG_BUY_INVALID_PRICE         = 10156;       //购买-价格非法
    const SYS_MSG_BUY_SUCCESS               = 10159;       //您成功购买了[63cd4e]%s[-]个[63cd4e]%s[-]，共扣除[63cd4e]%s[-]Zeny，请在[63cd4e]交易记录[-]中领取商品。
    const SYS_MSG_BUY_STOCK_NOT_ENOUGH      = 25702;       //库存不足
    const SYS_MSG_BUY_BOOTH_NOT_OPEN        = 25704;       //小店已打烊

    const ESTATTYPE_MIN_TRADE_PRICE = 6;        //某个物品交易所购买最低价格，不分等级
    const ESTATTYPE_AVG_TRADE_PRICE = 7;        //某个物品交易所购买平均价格，和交易次数 不分等级
    const ESTATTYPE_MAX_TRADE_PRICE = 8;        //某个物品交易所购买最高价格，不分等级

    const SYS_MSG_BOOTH_PUBLICITY_BUY_SUCCESS = 25694;       //您在抢购期抢购中成功购买[63cd4e]%s[-]个[63cd4e]%s[-]，共扣除[63cd4e]%s[-]Zeny，[63cd4e]%s[-]打赏积分，请在[63cd4e]交易记录[-]中领取退款。
    const SYS_MSG_BOOTH_PUBLICITY_PAY_SUCCESS = 25695;       //您在抢购期期间抢购了[63cd4e]%s[-]个[63cd4e]%s[-]，共预先扣除[63cd4e]%s[-]Zeny，并冻结[63cd4e]%s[-]打赏积分。
    const SYS_MSG_BOOTH_PUBLICITY_FAIL        = 25696;       //您在抢购期期间抢购[63cd4e]%s[-]失败，返还[63cd4e]%s[-]Zeny，解冻[63cd4e]%s[-]打赏积分，请在[63cd4e]交易记录[-]中领取退款。


    /**
     * 执行购买请求 (worker进程)
     *
     * @param \RO\Cmd\BuyItemRecordTradeCmd $cmd
     */
    public static function reqBuy(ZoneForwardUser $forward, \RO\Cmd\BuyItemRecordTradeCmd $cmd)
    {
        # see TradeManager.cpp line 2819
        if (IS_DEBUG)
        {
            Server::$instance->debug("交易-请求购买:" . json_encode($cmd));
        }

        if ($cmd->charid != $forward->charId)
        {
            # 购买物品人和请求人不一致
            $forward->sendSysMsgToUser(self::SYS_MSG_BUY_INVALID_PARAMS);
            $cmd->setRet(\RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forward->sendToUser($cmd);
            return;
        }

        $itemInfo = $cmd->item_info;

        if (!$cmd->item_info || !$itemInfo->price > 0)
        {
            # 参数错误，给客户端返回内容
            $forward->sendSysMsgToUser(self::SYS_MSG_BUY_INVALID_PARAMS);
            $cmd->setRet(\RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forward->sendToUser($cmd);
            return;
        }

        if (!$itemInfo->count > 0)
        {
            $forward->sendSysMsgToUser(self::SYS_MSG_BUY_INVALID_COUNT);
            $cmd->setRet(\RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forward->sendToUser($cmd);
            return;
        }

        if (!$forward->tryLockUser($cmd->charid))
        {
            # 防止玩家重复点击操作, 不用提示
            //$cmd->setRet(\RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            //$forward->sendToUser($cmd);
            return;
        }

        $itemList = Dao\ItemList::getById($itemInfo->order_id);
        if (!$itemList)
        {
            Server::$instance->warn("[购买请求] 获取 itemListId: {$itemInfo->order_id} 数据失败");

            $forward->sendSysMsgToUser(self::SYS_MSG_SYS_ERROR);
            $cmd->setRet(\RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forward->sendToUser($cmd);
            return;
        }

        if (!$itemList->isOverlap && $itemInfo->count != 1)
        {
            # 非堆叠物品购买数必须是1
            $forward->sendSysMsgToUser(self::SYS_MSG_BUY_INVALID_COUNT);
            $cmd->setRet(\RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forward->sendToUser($cmd);
            return;
        }

        # 购买数据里只有 itemid, price, count, order_id 这几个参数
        $task       = new ItemTask();
        $task->id   = $itemInfo->order_id;
        $task->type = ItemTask::TYPE_BUY;
        $task->data = self::getByInfo($forward, $itemInfo);

        if ($itemList->isPublicity == 1 && $itemList->getEndTime() > 0)
        {
            if ($itemList->getEndTime() <= time())
            {
                $forward->sendSysMsgToUser(self::SYS_MSG_BUY_PENDING_WAS_LOCKED);
                $cmd->setRet(\RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_PENDING_IS_LOCKED);
                $forward->sendToUser($cmd);
                return;
            }

            # 公示购买
            $buying   = 0;
            $cmd->ret = \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL;
            self::buyInTask($itemList, $buying, $itemList->pubPrice, $task->data, $cmd);

            return;
        }

        # 投递任务，由task给 `ActBuy::buyInTask()` 方法进行处理
        if (false === $task->send())
        {
            # 投递任务失败
            Server::$instance->warn("[购买请求] 玩家 charId: {$cmd->charid}, 任务投递失败");

            $forward->sendSysMsgToUser(self::SYS_MSG_SYS_ERROR);
            $cmd->setRet(\RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forward->sendToUser($cmd);
        }

        # 购买中的计数器 +1
        Server::$counterBuying->add();
    }

    /**
     * 扣钱返回 (worker进程)
     *
     * @param \RO\Cmd\ReduceMoneyRecordTradeCmd $cmd
     */
    public static function onPayReceive(ZoneForwardScene $forward, \RO\Cmd\ReduceMoneyRecordTradeCmd $cmd)
    {
        if ($cmd->charid != $forward->charId)
        {
            return;
        }

        Server::$instance->info('[购买请求返回] - '.($cmd->ret == \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS ? '成功' : "失败 (ret: {$cmd->ret})").', '. json_encode($cmd));

        $task          = new ItemTask();
        $task->id      = $cmd->item_info->order_id;
        $task->type    = ItemTask::TYPE_BUY_ON_RETURN;
        $task->forward = $forward;
        $task->count   = $cmd->item_info->count;
        $task->price   = $cmd->item_info->price;
        $task->ret     = $cmd->ret;

        # 拿到此玩家最后设置的兑换ID
        $covert = Server::$userLastConvertId->get($forward->charId);
        if ($covert)
        {
            # 更新返回信息到数据库
            Dao\ConvertRecord::updateRst($covert['id'], $forward->charId, $cmd->ret);

            # 移除临时数据
            Server::$userLastConvertId->del($forward->charId);
            $task->covertId = $covert['id'];
            unset($covert);
        }

        $itemList = Dao\ItemList::getById($task->id);
        if ($itemList->isPublicity == 1 && $itemList->getEndTime() > 0)
        {
            # 公示购买
            $cmd         = new \RO\Cmd\BuyItemRecordTradeCmd();
            $cmd->charid = $task->forward->charId;
            $cmd->ret    = $task->ret;

            self::onPayPublicityBuy($itemList, $task, $cmd);
            return;
        }

        if (false === $task->send())
        {
            # 投递任务失败
            Server::$instance->warn('投递Task数据失败: '. json_encode($task));

            if ($cmd->ret == \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS)
            {
                # 已经扣款成功了，再重试
                $try = function(){};
                $try = function() use ($task, & $try)
                {
                    ++$task->try;
                    swoole_timer_after(300, function() use ($task, & $try)
                    {
                        if (false === $task->send())
                        {
                            $try();
                        }
                    });
                };
                $try();
            }
            else
            {
                # 购买中计数 -1
                Server::$itemBuying->decr($task->id, 'count', $task->count);

                $forward->sendSysMsgToUser(self::SYS_MSG_SYS_ERROR);
                self::buyDoneClean($cmd->charid);
            }
        }
    }

    /**
     * 批量处理在Task进程中运行
     *
     * @param ItemList $itemList 物品列表信息
     * @param \Ds\Map|int $buying 正在购买的列表
     * @param int $itemListPrice 当前服务器价格
     * @param \Ds\Map $buyQueue 购买中
     * @param array $buyInfo 购买信息
     */
    public static function buyInTask(ItemList $itemList, $buying, $itemListPrice, array $buyInfo, \RO\Cmd\BuyItemRecordTradeCmd $failCmd)
    {
        /**
        $buyInfo  = [
            $forward->fd,         # fd
            $forward->fromId,     # fromId
            $this->charId,        # 角色Id
            $itemInfo->count,     # 购买数
            $itemInfo->price,     # 购买价格
        ];
         */
        $err = null;
        do
        {
            list($fd, $fromId, $charId, $count, $price) = $buyInfo;
            $itemListId = $itemList->id;

            if ($itemListPrice != $price)
            {
                $err = self::SYS_MSG_BUY_INVALID_PRICE;
                Server::$instance->warn("价格不合法，charId: {$charId}, itemId: {$itemList->itemId}, 用户价格: {$price} 和服务器不一致: {$itemListPrice}");
                break;
            }

            $pubBuy = false;
            if ($itemList->isPublicity == 1 && $itemList->getEndTime() > 0)
            {
                # 购买公示物品，每个人抢购最多不超过公示物品数量
                try
                {
                    $buyCount = intval(Server::$redis->hGet('pub_buy_count_'. $itemList->id, $charId));
                }
                catch (\Exception $e)
                {
                    $err = self::SYS_MSG_BUY_PUBLICITY_FAIL2;
                    break;
                }

                if ($count + $buyCount > $itemList->stock)
                {
                    $err = self::SYS_MSG_BUY_PUBLICITY_FAIL_COUNT;
                    break;
                }
                $pubBuy = true;
            }
            elseif (0 == $itemList->stock || $count > $itemList->stock - $buying->get('count'))
            {
                # 没有库存或超过库存
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[购买] 没有库存，购买请求 $count, 库存: {$itemList->stock}, 购买中 ". $buying->get('count'));
                }
                $err = self::SYS_MSG_BUY_CANNOT_FIND_PENDING;
                break;
            }

            $convert         = new Dao\ConvertRecord();
            $convert->type   = Dao\ConvertRecord::TYPE_REDUCE_MONEY;
            $convert->status = Dao\ConvertRecord::STATUS_DOING;
            $convert->charId = $charId;
            $convert->time   = time();
            $convert->zeny   = $count * $price;
            $convert->vars   = [
                'itemListId' => $itemListId,
            ];

            if (!$convert->insert())
            {
                $err = self::SYS_MSG_DB_ERROR;
                Server::$instance->warn('[购买] 插入数据失败'. json_decode($convert, JSON_UNESCAPED_UNICODE));
                break;
            }

            # 插入到共享内存表里
            Server::$userLastConvertId->set($charId, ['id' => $convert->id, 'time' => $convert->time]);

            if (self::payByBuyInfo($itemList, $fd, $fromId, $charId, $count, $price))
            {
                # 购买中数量递增，非公示购买
                if (!$pubBuy)
                {
                    $buying[$charId]  = new \Ds\Pair($count, time());
                    $buying['count'] += $count;
                }

                # 发起支付请求成功，等待服务器返回
                return;
            }
            else
            {
                Server::$userLastConvertId->del($charId);

                # 移除失败的记录
                $convert->delete();

                $err = self::SYS_MSG_BUY_INVALID_PRICE;
                Server::$instance->warn('[购买] 发送支付请求数据失败');
            }
        }
        while (false);

        # 给客户端发送失败信息
        self::buyInTaskFail($failCmd, $fd, $fromId, $charId, $err);
    }

    /**
     * 在Task进程里给失败玩家推送信息
     *
     * @param \RO\Cmd\BuyItemRecordTradeCmd $failCmd
     * @param array $buyInfo
     */
    public static function buyInTaskFail(\RO\Cmd\BuyItemRecordTradeCmd $failCmd, $fd, $fromId, $charId, $err = self::SYS_MSG_SYS_ERROR)
    {
        $failCmd->charid = $charId;
        ZoneForwardUser::sendToUserByFd($failCmd, $charId, $fd, $fromId);
        if ($err > 0)
        {
            ZoneForwardUser::sendSysMsgToUserByCharId($fd, $fromId, $charId, $err);
        }

        self::buyDoneClean($charId);
    }

    /**
     * 在任务进程里执行购买扣钱操作
     *
     * @param ItemList $itemList
     * @param \Ds\Map  $stockMap 由 Dao\Goods::getGracefulStock() 返回的库存数据
     * @param ItemTask $task
     * @param \RO\Cmd\BuyItemRecordTradeCmd $cmd
     */
    public static function onPayInTask(ItemList $itemList, \Ds\Map $stockMap, ItemTask $task, \RO\Cmd\BuyItemRecordTradeCmd $cmd)
    {
        # 处理返回状态
        /**
         * @var ZoneForwardScene $forward;
         */
        $forward = $task->forward;

        switch ($task->ret)
        {
            case \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS:
                # 扣钱成功

                # 是否公示购买
                $mysql = Server::$mysqlMaster;

                /**
                 * @var Dao\Goods $first
                 */
                $stocks = Dao\Goods::getGracefulGoods($stockMap, $task->count);

                # 更新库存数
                $itemList->setStock($stockMap['stock']);

                if ($stocks->isEmpty())
                {
                    # 防止意外错误
                    $first = new Dao\Goods();
                    $first->id           = 0;
                    $first->charId       = 0;
                    $first->playerName   = '';
                    $first->playerZoneId = 100010002;
                    $first->itemId       = $itemList->itemId;
                    $first->itemKey      = $itemList->itemKey;
                    $first->itemData     = $itemList->getItemData();
                    $first->count        = $task->count;
                    $first->stock        = $task->count;
                    $list = [$first];
                    $stocks[0] = $task->count;

                    Server::$instance->warn("[购买] 获取了一个空的库存, itemId: {$itemList->itemId}, count: {$task->count}");
                }
                else
                {
                    $ids   = $stocks->keys()->toArray();
                    $list  = Dao\Goods::getByIds($ids);
                    $first = current($list);
                }

                # 购买记录
                $bought               = new Dao\RecordBought();
                $bought->goodsId      = $first->id;
                $bought->itemId       = $itemList->itemId;
                $bought->charId       = $forward->charId;
                $bought->playerName   = $forward->name;
                $bought->playerZoneId = $forward->zoneId;
                $bought->time         = time();
                $bought->count        = $task->count;
                $bought->totalCount   = $task->count;
                $bought->price        = $task->price;
                $bought->status       = Dao\RecordBought::STATUS_PAY_SUCCESS;
                $bought->publicityId  = 0;
                $bought->endTime      = 0;
                $bought->refineLv     = $itemList->refineLv;
                $bought->isDamage     = $itemList->isDamage;
                $bought->isManyPeople = $stocks->count() > 1 ? 1 : 0;
                $bought->logId        = $bought->createLogId();
                $bought->sellerInfo   = new \RO\Cmd\NameInfo();
                $bought->sellerInfo->name   = $first->playerName;
                $bought->sellerInfo->zoneid = $first->playerZoneId;
                $bought->sellerInfo->count  = $stocks->get(intval($first->id), 1);
                $sellerIds = $first->charId;

                if ($bought->isManyPeople == 1)
                {
                    $sellerIds = '';
                    $nameList = $bought->sellersInfo = new \RO\Cmd\NameInfoList();
                    foreach ($list as $l)
                    {
                        /**
                         * @var Dao\Goods $l
                         */
                        $nameInfo         = new \RO\Cmd\NameInfo();
                        $nameInfo->name   = $l->playerName;
                        $nameInfo->zoneid = $l->playerZoneId;
                        $nameInfo->count  = $stocks->get(intval($l->id), 0);
                        $nameList->addNameInfos($nameInfo);
                        $sellerIds .= $l->charId . ';';
                    }
                }

                # 推送 fluent log, see TradeManager.cpp line 4544
                $logCmd             = new \RO\Cmd\TradeLogCmd();
                $logCmd->pid        = $forward->charId;
                $logCmd->time       = $bought->time;
                $logCmd->type       = \RO\ZoneForward::ETRADETYPE_TRUEBUY;
                $logCmd->itemid     = $itemList->itemId;
                $logCmd->count      = $task->count;
                $logCmd->price      = $task->price;
                $logCmd->tax        = 0;
                $logCmd->moneycount = $task->count * $task->price;
                $logCmd->logid      = $bought->logId;
                $logCmd->strotherid = $sellerIds;
                if (!$itemList->isOverlap && $first->itemId != $first->itemKey)
                {
                    $logCmd->iteminfo = json_encode($first->getItemData());
                }
                ZoneForwardScene::pushToFluent($logCmd);

                # 非堆叠
                if (!$itemList->getItem()->isOverlap)
                {
                    $bought->itemData = is_string($first->itemData) ? $first->itemData : (($tmpItemData = $first->getItemData()) !== null ? $tmpItemData->getCachedSerialize() : null);
                }

                if (!$bought->insert())
                {
                    Server::saveDelaySQL($bought->getMySQL()->last_query, 'insert_bought.sql');
                }

                # 给返回信息设置数据
                if (!$cmd->item_info)
                {
                    $cmd->item_info               = new \RO\Cmd\TradeItemBaseInfo();
                    $cmd->item_info->itemid       = $itemList->itemId;
                    $cmd->item_info->price        = $task->price;
                    $cmd->item_info->count        = $task->count;
                    $cmd->item_info->order_id     = $task->id;
                    $cmd->item_info->overlap      = $itemList->isOverlap;
                    $cmd->item_info->publicity_id = $itemList->id;
                    $cmd->item_info->end_time     = $itemList->getEndTime();
                    if ($first->itemData)
                    {
                        $cmd->item_info->guid      = $first->getItemData()->base->guid;
                        $cmd->item_info->item_data = $first->getItemData();
                    }
                }

                # 买家信息
                $nameInfo         = new \RO\Cmd\NameInfo();
                $nameInfo->name   = $forward->name;
                $nameInfo->zoneid = $forward->zoneId;
                $buyerIds         = $forward->charId;

                # 会插入数据库的key名称（非字段名）
                $listKey   = [
                    'charId',
                    'status',
                    'playerZoneId',
                    'goodsId',
                    'itemId',
                    'time',
                    'count',
                    'price',
                    'tax',
                    'isManyPeople',
                    'refineLv',
                    'isDamage',
                    'buyerInfo',
                    'logId',
                ];
                $fields    = [];
                $allFields = Dao\RecordSold::allFields();
                foreach ($listKey as $key)
                {
                    $fields[] = $allFields[$key];
                }
                $sqlBefore = "INSERT IGNORE INTO `". Dao\RecordSold::getTableName() ."` (`". implode('`,`', $fields) ."`) VALUES ";
                $sqlBuffer = $sqlBefore;
                $sqlLength = 0;
                $sellers   = [];
                unset($fields, $allFields);

                # 延迟处理的文件
                $delayJob = JobFifo::instance('delay-buy.job');
                /**
                 * @var JobFifo $delayJob
                 */
                foreach ($stocks as $goodsId => $count)
                {
                    if ($goodsId == 0)continue;

                    /**
                     * @var Dao\Goods $goods
                     */
                    $goods = $list[$goodsId];

                    if ($count > $goods->stock)
                    {
                        # 超卖？
                        Server::$instance->warn("[购买] 分单的Goods库存超过需求 charId: {$forward->charId}, itemListId: {$itemList->itemId}, goodsId: {$goods->id}, 当前: {$goods->stock}, 需求: {$count}");
                        $count  = $goods->stock;     # 设置成当前库存数
                        $upData = [
                            $goods->id,
                            $count,
                            Dao\Goods::STATUS_SOLD,
                        ];
                    }
                    elseif ($goods->stock == $count)
                    {
                        # 正好卖光
                        $upData = [
                            $goods->id,
                            $count,
                            Dao\Goods::STATUS_SOLD,
                        ];
                    }
                    else
                    {
                        # 还有剩余库存
                        $upData = [
                            $goods->id,
                            $count,
                            0,           // 表示不更新状态
                        ];
                    }
                    if ($itemList->isOverlap || Server::$updatingGoodsStock->exist($goodsId))
                    {
                        # 堆叠物品，或者是已经有延迟更新的
                        $asyncMode = true;
                        $asyncMode = false;
                    }
                    else
                    {
                        $asyncMode = false;
                    }

                    if (false === $asyncMode)
                    {
                        # 同步模式但是更新失败
                        $rs = self::delayUpStock($upData, true);
                        if (false === $rs)
                        {
                            $asyncMode = true;
                        }
                        elseif (is_int($rs))
                        {
                            # 库存有差异返回了一个实际更新的库存数
                            $count     = $rs;
                            $upData[1] = $count;
                        }
                    }

                    if (true === $asyncMode)
                    {
                        $delayJob->push($upData, JobFifo::CALL_DELAY_UP_STOCK);

                        # 增加待更新库存数
                        if ($count > 0)
                        {
                            Server::$updatingGoodsStock->incr($goodsId, 'count', $count);
                        }
                    }

                    # 销售数是0的记录数不用设置
                    if ($count == 0)continue;

                    # 销售记录
                    $nameInfo->count    = $count;
                    $sold               = new Dao\RecordSold();
                    $sold->charId       = $goods->charId;
                    $sold->status       = Dao\RecordSold::STATUS_ADDING;
                    $sold->playerZoneId = $goods->playerZoneId;
                    $sold->goodsId      = $goods->id;
                    $sold->itemId       = $goods->itemId;
                    $sold->time         = time();
                    $sold->count        = $count;
                    $sold->price        = $task->price;
                    $sold->tax          = $sold->getTax();
                    $sold->isManyPeople = 0;
                    $sold->refineLv     = $itemList->refineLv;
                    $sold->isDamage     = $itemList->isDamage;
                    $sold->buyerInfo    = $nameInfo;
                    $sold->logId        = $sold->createLogId();
                    # !!! 如果需要增加 $sold 参数，需要增加 $listKey 数组

                    $values = [];
                    foreach ($listKey as $key)
                    {
                        $values[] = $mysql->quote($sold->$key);
                    }

                    # 销售者信息
                    $seller  = [
                        'charId' => $sold->charId,
                        'zoneId' => $sold->playerZoneId,
                        'tax'    => $sold->getTax(),
                        'count'  => $count,
                    ];

                    if (false === $asyncMode)
                    {
                        if ($sold->insert())
                        {
                            if ($sold->playerZoneId > 0 && false !== ($zone = Server::$zone->get($sold->playerZoneId)))
                            {
                                $msg         = new \RO\Cmd\ListNtfRecordTrade();
                                $msg->charid = $sold->charId;
                                $msg->type   = \RO\Cmd\EListNtfType::ELIST_NTF_MY_LOG;
                                ZoneForwardUser::sendToUserByFd($msg, $sold->charId, $zone['fd'], $zone['fromId']);

                                $totalMoney = $count * $sold->price - $sold->getTax();
                                # 通知弹出提示
                                // 成功出售：恭喜！您成功地售出了[63cd4e]%s[-]个[63cd4e]%s[-]，扣税 %s, 共获得了[63cd4e]%s[-]金币
                                $params = [
                                    $count,
                                    $itemList->getItem()->name,
                                    $sold->getTax(),
                                    $totalMoney,
                                ];
                                ZoneForwardUser::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $sold->charId, ActSell::SYS_MGS_SELL_SUCCESS, $params);

                                # 发送离线消息给场景服务器
                                $offlineMsg              = new \RO\Cmd\AddMoneyRecordTradeCmd();
                                $offlineMsg->charid      = $sold->charId;
                                $offlineMsg->money_type  = Server::DEFAULT_MONEY_TYPE;
                                $offlineMsg->count       = $count;
                                $offlineMsg->type        = \RO\Cmd\EOperType::EOperType_NormalSell;
                                $offlineMsg->total_money = $totalMoney;
                                $offlineMsg->itemid      = $sold->itemId;
                                $offlineMsg->price       = $sold->price;
                                ZoneForwardScene::sendToZoneByFd($offlineMsg, $zone['fd'], $zone['fromId']);
                            }
                        }
                        else
                        {
                            # 插入数据失败重新使用异步更新模式
                            $asyncMode = true;
                            Server::$instance->warn("[出售] 插入销售记录失败，将转为异步更新模式， SQL:" . $sold->getMySQL()->last_query .', Error: '. $sold->getMySQL()->error);
                        }
                        unset($tmp, $sql);
                    }

                    if (true === $asyncMode)
                    {
                        $sellers[] = $seller;

                        $tmpStr     = '('. implode(',', $values) .'),';
                        $sqlLength += strlen($tmpStr);
                        $sqlBuffer .= $tmpStr;
                        if ($sqlLength > 100000)
                        {
                            # 100k
                            $tmp = [
                                'sql'     => substr($sqlBuffer, 0, -1),       # -1 是为了移除最后的逗号
                                'name'    => $itemList->getItem()->name,
                                'price'   => $task->price,
                                'itemId'  => $itemList->itemId,
                                'sellers' => $sellers,
                            ];
                            $sqlBuffer  = $sqlBefore;
                            $sqlLength  = 0;
                            $sellers    = [];
                            $delayJob->push($tmp, JobFifo::CALL_DELAY_INSERT_SOLD);
                        }
                    }

                    # 推送 fluent log, see TradeManager.cpp line 4612
                    $logCmd             = new \RO\Cmd\TradeLogCmd();
                    $logCmd->pid        = $goods->charId;
                    $logCmd->time       = $sold->time;
                    $logCmd->type       = \RO\ZoneForward::ETRADETYPE_TRUESELL;
                    $logCmd->itemid     = $goods->itemId;
                    $logCmd->count      = $count;
                    $logCmd->price      = $task->price;
                    $logCmd->tax        = $sold->getTax();
                    $logCmd->moneycount = $count * $task->price;
                    $logCmd->logid      = $sold->logId;
                    $logCmd->strotherid = $buyerIds;
                    if (!$goods->isOverlap && $goods->itemId != $goods->itemKey)
                    {
                        $logCmd->iteminfo = json_encode($goods->getItemData());
                    }
                    ZoneForwardScene::pushToFluent($logCmd);

                    Server::$instance->info("[出售成功] 挂单id:{$goods->id} 卖家id:{$goods->charId}, 卖家记录id:{$sold->id} 价格:{$task->price} 数量:{$count} 物品id:{$goods->itemId} item_data:" . json_encode($goods->itemData));
                }

                if ($sqlLength > 0)
                {
                    # 500k
                    $tmp = [
                        'sql'     => substr($sqlBuffer, 0, -1),       # -1 是为了移除最后的逗号
                        'name'    => $itemList->getItem()->name,
                        'price'   => $task->price,
                        'itemId'  => $itemList->itemId,
                        'sellers' => $sellers,
                    ];
                    $delayJob->push($tmp, JobFifo::CALL_DELAY_INSERT_SOLD);
                    unset($sqlBuffer, $sellers, $tmp);
                }

                # 给客户端发消息
                $params = [
                    $task->count,
                    $itemList->getItem()->name,
                    $task->count * $task->price,
                ];
                $forward->sendSysMsgToUser(self::SYS_MSG_BUY_SUCCESS, $params);

                $msg         = new \RO\Cmd\ListNtfRecordTrade();
                $msg->charid = $forward->charId;
                $msg->type   = \RO\Cmd\EListNtfType::ELIST_NTF_MY_LOG;
                $forward->sendToUser($msg);

                # 发送统计信息
                ZoneForwardUser::sendStatLog(self::ESTATTYPE_MIN_TRADE_PRICE, $itemList->itemId, 0, 0, $task->price);
                ZoneForwardUser::sendStatLog(self::ESTATTYPE_AVG_TRADE_PRICE, $itemList->itemId, 0, 0, $task->price);
                ZoneForwardUser::sendStatLog(self::ESTATTYPE_MAX_TRADE_PRICE, $itemList->itemId, 0, 0, $task->price);

                break;

            case \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_MONEY_IS_NOT_ENOUGH:
                # 金额不足, 场景服务器已自动回复了
                break;

            case \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_FIND_USER_IN_SCENE:
                # 交易失败，找不到玩家
                break;

            default:
                # 未知错误
                Server::$instance->warn("[购买-场景服返回] 场景服务器返回了一个未知返回码:{$task->ret} 买家id:{$forward->charId} 购买物品key:{$itemList->itemKey} 价格:{$task->price} 数量:{$task->count}");
                break;
        }

        self::buyDoneClean($forward->charId);

        $forward->sendToUser($cmd);

        # 清理掉兑换
        if (isset($task->covertId) && $task->covertId)
        {
            Dao\ConvertRecord::deleteById($task->covertId);
        }
    }

    /**
     * 公示购买
     *
     * @param ItemList $itemList
     * @param ItemTask $task
     */
    public static function onPayPublicityBuy(ItemList $itemList, ItemTask $task, \RO\Cmd\BuyItemRecordTradeCmd $cmd)
    {
        # 处理返回状态
        /**
         * @var ZoneForwardScene $forward;
         */
        $forward = $task->forward;

        switch ($task->ret)
        {
            case \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS:
                # 扣钱成功

                # 是否公示购买
                $mysql = Server::$mysqlMaster;
                $count = $task->count;

                # 推送 fluent log, see TradeManager.cpp line 2253
                $obj             = new \RO\Cmd\TradeLogCmd();
                $obj->pid        = $forward->charId;
                $obj->time       = time();
                $obj->type       = \RO\ZoneForward::ETRADETYPE_PUBLICITY_SEIZURE;
                $obj->itemid     = $itemList->itemId;
                $obj->count      = $task->count;
                $obj->price      = $task->price;
                $obj->tax        = 0;
                $obj->moneycount = $task->count * $task->price;
                $obj->logid      = "pub-buy-{$forward->charId}-". time();
                if (!$itemList->isOverlap && $itemList->itemId != $itemList->itemKey)
                {
                    $obj->iteminfo = json_encode($itemList->getItemData());
                }
                ZoneForwardScene::pushToFluent($obj);

                # 购买记录
                $bought               = new Dao\RecordBought();
                $bought->goodsId      = 0;
                $bought->itemId       = $itemList->itemId;
                $bought->charId       = $forward->charId;
                $bought->playerName   = $forward->name;
                $bought->playerZoneId = $forward->zoneId;
                $bought->time         = time();
                $bought->count        = 0;
                $bought->totalCount   = $task->count;
                $bought->price        = $task->price;
                $bought->status       = Dao\RecordBought::STATUS_PUBLICITY_PAY_SUCCESS;
                $bought->publicityId  = $itemList->id;
                $bought->endTime      = $itemList->getEndTime();
                $bought->refineLv     = $itemList->refineLv;
                $bought->isDamage     = $itemList->isDamage;
                $bought->isManyPeople = 0;
                if ($itemList->itemData)
                {
                    $bought->itemData = $itemList->itemData;
                }

                $nameInfo             = new \RO\Cmd\NameInfo();
                $nameInfo->name       = '';
                $nameInfo->zoneid     = 0;
                $nameInfo->count      = $count;
                $bought->sellerInfo   = $nameInfo;

                if ($itemList->isOverlap)
                {
                    if (!$bought->insertForPublicity())
                    {
                        Server::saveDelaySQL($mysql->last_query);
                    }
                }
                else
                {
                    # todo 游戏服装备物品暂不支持1条数据多个领取功能
                    if (!$bought->insert())
                    {
                        Server::saveDelaySQL($mysql->last_query);
                    }
                }

                # 给返回信息设置数据
                if (!$cmd->item_info)
                {
                    $cmd->item_info               = new \RO\Cmd\TradeItemBaseInfo();
                    $cmd->item_info->price        = $task->price;
                    $cmd->item_info->count        = $task->count;
                    $cmd->item_info->order_id     = $task->id;
                    $cmd->item_info->overlap      = $itemList->getItem()->isOverlap;
                    $cmd->item_info->publicity_id = $itemList->id;
                    $cmd->item_info->end_time     = $itemList->getEndTime();
                }

                try
                {
                    # 递增玩家自己的购买数
                    $buyCount = Server::$redis->hIncrBy('pub_buy_count_' . $itemList->id, $forward->charId, $task->count) ?: $task->count;
                }
                catch (\Exception $e)
                {
                    $buyCount = $task->count;
                }

                # 如果购买总数大于当前数，则表明此玩家之前购买过，不用递增购买人数
                if ($buyCount == $task->count)
                {
                    $rs = Dao\ItemList::incrPublicityBuyPeople($itemList, 1);
                    if (!$rs)
                    {
                        # 写入延迟更新sql
                        Server::saveDelaySQL($mysql->last_query);

                        # 更新内存的购买人数
                        $itemList->pubBuyPeople += 1;
                        Server::$itemList->incr($itemList->itemKey, 'pub_buy_people', 1);
                    }
                }


                # 信息提示
                $params = [
                    $task->count,
                    $itemList->getItem()->name,
                    $task->count * $task->price,
                ];
                $forward->sendSysMsgToUser(self::SYS_MSG_BUY_PUBLICITY_PAY_SUCCESS, $params);

                # 购买记录推送
                //$msg         = new \RO\Cmd\AddNewLog();
                //$msg->charid = $forward->charId;
                //$msg->log    = $bought->getLogItemInfo();
                //$forward->sendToUser($msg);
                $msg         = new \RO\Cmd\ListNtfRecordTrade();
                $msg->charid = $forward->charId;
                $msg->type   = \RO\Cmd\EListNtfType::ELIST_NTF_MY_LOG;
                $forward->sendToUser($msg);

                break;

            case \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_MONEY_IS_NOT_ENOUGH:
                # 金额不足
                Server::$instance->info("[公示购买-场景服返回] 金额不足 返回码:{$task->ret} 买家id:{$forward->charId} 购买物品key:{$itemList->itemKey} 公示id:{$itemList->id} 价格:{$task->price} 数量:{$task->count}");
                break;

            case \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_FIND_USER_IN_SCENE:
                $forward->sendSysMsgToUser(self::SYS_MSG_SYS_ERROR);
                # 交易失败，找不到玩家
                Server::$instance->info("[公示购买-场景服返回] 交易失败，找不到玩家 返回码:{$task->ret} 买家id:{$forward->charId} 购买物品key:{$itemList->itemKey} 公示id:{$itemList->id} 价格:{$task->price} 数量:{$task->count}");
                break;

            default:
                # 未知错误
                $forward->sendSysMsgToUser(self::SYS_MSG_SYS_ERROR);
                Server::$instance->warn("[公示购买-场景服返回] 场景服务器返回了一个未知返回码:{$task->ret} 买家id:{$forward->charId} 购买物品key:{$itemList->itemKey} 公示id:{$itemList->id} 价格:{$task->price} 数量:{$task->count}");
                break;
        }

        self::buyDoneClean($forward->charId, 2.6);

        $forward->sendToUser($cmd);

        # 清理掉兑换
        if (isset($task->covertId) && $task->covertId)
        {
            Dao\ConvertRecord::deleteById($task->covertId);
        }
    }

    /**
     * 返回一个待处理的购买信息
     *
     * @param ZoneForwardUser $forward
     * @param \RO\Cmd\TradeItemBaseInfo $itemInfo
     * @return array
     */
    protected static function getByInfo(ZoneForwardUser $forward, \RO\Cmd\TradeItemBaseInfo $itemInfo)
    {
        return [
            $forward->fd,         # fd
            $forward->fromId,     # fromId
            $forward->charId,     # 角色Id
            $itemInfo->count,     # 购买数
            $itemInfo->price,     # 购买价格
        ];
    }

    /**
     * 请求服务器支付
     *
     * @param ItemList $itemList
     * @param int $fd
     * @param int $fromId
     * @param int $charId
     * @param int $count
     * @param int $price
     * @return bool
     */
    protected static function payByBuyInfo(Dao\ItemList $itemList, $fd, $fromId, $charId, $count, $price)
    {
        $itemInfo               = new \RO\Cmd\TradeItemBaseInfo();
        $itemInfo->price        = $price;
        $itemInfo->count        = $count;
        $itemInfo->itemid       = $itemList->itemId;
        $itemInfo->order_id     = $itemList->id;
        $itemInfo->refine_lv    = $itemList->refineLv;
        $itemInfo->overlap      = $itemList->getItem()->isOverlap;
        $itemInfo->publicity_id = $itemList->id;
        $itemInfo->end_time     = $itemList->endTime;

        return self::doPay($fd, $fromId, $charId, $itemInfo);
    }

    /**
     * 执行一个扣款请求
     *
     * @param int $fd
     * @param int $fromId
     * @param int $charId
     * @param int $money
     * @param \RO\Cmd\TradeItemBaseInfo $itemInfo
     * @return bool
     */
    public static function doPay($fd, $fromId, $charId, \RO\Cmd\TradeItemBaseInfo $itemInfo)
    {
        $proto              = new \RO\Cmd\ReduceMoneyRecordTradeCmd();
        $proto->money_type  = Server::DEFAULT_MONEY_TYPE;
        $proto->total_money = $itemInfo->count * $itemInfo->price;
        $proto->charid      = $charId;
        $proto->item_info   = $itemInfo;

        if (ZoneForwardUser::sendToZoneByFd($proto, $fd, $fromId))
        {
            Server::$instance->info('[购买支付] '. json_encode($proto));
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * 清理用户数据
     *
     * @param int $charId
     * @param bool|int|float $delayTime 延迟几秒后解锁
     */
    public static function buyDoneClean($charId, $delayTime = false)
    {
        if ($delayTime > 0)
        {
            # 延迟解锁
            swoole_timer_after(ceil($delayTime * 1000), function() use ($charId)
            {
                ZoneForwardUser::unLockUser($charId);
            });
        }
        else
        {
            # 解锁用户操作
            ZoneForwardUser::unLockUser($charId);
        }

        # 购买中计数器 -1
        Server::$counterBuying->sub();
    }

    /**
     * 延迟更新库存数据库
     *
     * @param $data
     * @param bool $syncMode 是否同步模式
     * @return bool|int 如果返回int则表示实际更新数
     */
    public static function delayUpStock($data, $syncMode = false)
    {
        if (!is_array($data) || !$data[0] > 0)
        {
            Server::$instance->warn("异步更新库存读出数据异常, 内容: ". var_export($data, true));
            return true;
        }

        list($goodsId, $count, $status) = $data;

        if ($status == 0)
        {
            # 递减库存
            $sql = "UPDATE `trade_goods` SET `stock` = `stock` - {$count} WHERE `id` = {$goodsId}";
        }
        else
        {
            # 卖光
            $sql = "UPDATE `trade_goods` SET `stock` = 0, `status` = '{$status}' WHERE `id` = {$goodsId}";
        }

        $result = true;
        $mysql  = Server::$mysqlMaster;
        $rs     = $mysql->query($sql);
        if (false === $rs)
        {
            if ($mysql->errno == 1690 || strpos($mysql->error, "out of range") !== false)
            {
                # 递减数导致 stock 变成负数，执行sql失败
                Server::$instance->warn("[更新库存] 产生负值 SQL: $sql, ErrNo: {$mysql->errno}, Error: {$mysql->error}");

                $sql = "SELECT `stock` FROM `trade_goods` WHERE `id` = {$goodsId}";
                $rs  = $mysql->query($sql);
                if (false === $rs)
                {
                    Server::$instance->warn("[更新库存] 读取旧库存失败 SQL: $sql, ErrNo: {$mysql->errno}, Error: {$mysql->error}");
                    return false;
                }
                if ($rs->num_rows > 0)
                {
                    $row      = $rs->fetch_object();
                    $oldCount = (int)$row->stock;
                }
                else
                {
                    $oldCount = 0;
                }
                $rs->free();

                # 设置成 0
                $sql = "UPDATE `trade_goods` SET `stock` = '0', `status` = '". Dao\Goods::STATUS_SOLD ."' WHERE `id` = {$goodsId} AND `status` = '". Dao\Goods::STATUS_SELLING ."'";
                $rs  = $mysql->query($sql);
                if (false === $rs)
                {
                    Server::$instance->warn("[更新库存] SQL: $sql, ErrNo: {$mysql->errno}, Error: {$mysql->error}");
                    return false;
                }
                else
                {
                    Server::$instance->info("[更新库存] 挂单 {$goodsId} 待扣除库存数 {$count}, 实际扣除 {$oldCount}");
                    $result = $oldCount;
                }
            }
            else
            {
                Server::$instance->warn("[更新库存] SQL: $sql, ErrNo: {$mysql->errno}, Error: {$mysql->error}");
                return false;
            }
        }

        if (!$syncMode)
        {
            # 递减正在更新的库存数
            $after = Server::$updatingGoodsStock->decr($goodsId, 'count', $count);
            if ($after <= 0)
            {
                Server::$updatingGoodsStock->del($goodsId);
            }
        }

        return $result;
    }

    /**
     * 执行一个异步延迟更新库存数据库
     *
     * @param $data
     * @param \Swoole\MySQL $mysql
     * @return false|array
     */
    public static function delayUpStockAsync($data)
    {
        if (!is_array($data) || !$data[0] > 0)
        {
            Server::$instance->warn("异步更新库存读出数据异常, 内容: " . var_export($data, true));
            return false;
        }

        list($goodsId, $count, $status) = $data;

        if ($status == 0)
        {
            # 递减库存
            $sql = "UPDATE `trade_goods` SET `stock` = `stock` - {$count} WHERE `id` = {$goodsId}";
        }
        else
        {
            # 卖光
            $sql = "UPDATE `trade_goods` SET `stock` = 0, `status` = '{$status}' WHERE `id` = {$goodsId}";
        }

        return [$sql, function($result) use ($goodsId, $count, $sql)
        {
            if (false !== $result)
            {
                $after = Server::$updatingGoodsStock->decr($goodsId, 'count', $count);
                if ($after <= 0)
                {
                    Server::$updatingGoodsStock->del($goodsId);
                }
            }
        }];
    }

    /**
     * 一个延迟插入数据的定时回调方法
     *
     * @param $data
     * @param bool $quickMode 是否快速模式
     * @return bool
     */
    public static function delayInsertSold($data)
    {
        $mysql = Server::$mysqlMaster;
        $sql   = $data['sql'];

        if ($mysql->query($sql))
        {
            self::delayInsertSoldSendToUser($data);
            return true;
        }
        else
        {
            Server::$instance->warn("delaySold 执行失败, Error:: {$mysql->error}, SQL: ". $data['sql']);
            return false;
        }
    }

    /**
     * 一个延迟插入数据的定时回调方法
     *
     * @param $data
     * @return array
     */
    public static function delayInsertSoldAsync($data)
    {
        return [$data['sql'], function($status) use ($data)
        {
            if (false !== $status)
            {
                self::delayInsertSoldSendToUser($data);
            }
        }];
    }

    /**
     * 给玩家发送消息
     *
     * @param $data
     */
    protected static function delayInsertSoldSendToUser($data)
    {
        $zoneInfo = [];
        $name     = $data['name'];
        $price    = $data['price'];
        $itemId   = $data['itemId'];

        foreach ($data['sellers'] as $seller)
        {
            $zoneId = $seller['zoneId'];
            $charId = $seller['charId'];
            $tax    = $seller['tax'];
            $count  = $seller['count'];

            if (!isset($zoneInfo[$zoneId]))
            {
                $zoneInfo[$zoneId] = Server::$zone->get($zoneId);
            }

            $zone = $zoneInfo[$zoneId];
            if ($zone)
            {
                # 通知更新列表
                $msg         = new \RO\Cmd\ListNtfRecordTrade();
                $msg->charid = $charId;
                $msg->type   = \RO\Cmd\EListNtfType::ELIST_NTF_MY_LOG;
                ZoneForwardUser::sendToUserByFd($msg, $charId, $zone['fd'], $zone['fromId']);

                # 通知弹出提示
                // 成功出售：恭喜！您成功地售出了[63cd4e]%s[-]个[63cd4e]%s[-]，扣税 %s, 共获得了[63cd4e]%s[-]金币
                $params = [
                    $count,
                    $name,
                    $tax,
                    $count * $price - $tax,
                ];
                ZoneForwardUser::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $charId, ActSell::SYS_MGS_SELL_SUCCESS, $params);

                # 发送离线消息给场景服务器
                $offlineMsg              = new \RO\Cmd\AddMoneyRecordTradeCmd();
                $offlineMsg->charid      = $charId;
                $offlineMsg->money_type  = Server::DEFAULT_MONEY_TYPE;
                $offlineMsg->count       = $count;
                $offlineMsg->type        = \RO\Cmd\EOperType::EOperType_NormalSell;
                $offlineMsg->total_money = $count * $price - $tax;
                $offlineMsg->itemid      = $itemId;
                $offlineMsg->price       = $price;
                ZoneForwardScene::sendToZoneByFd($offlineMsg, $zone['fd'], $zone['fromId']);
            }
        }
    }
}