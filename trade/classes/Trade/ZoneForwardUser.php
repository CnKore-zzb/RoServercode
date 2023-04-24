<?php
namespace RO\Trade;

use RO\Booth\BoothOrderService;
use RO\Cmd\BoothPlayerPendingListCmd;
use RO\Cmd\BuyItemRecordTradeCmd;
use RO\Cmd\CancelItemRecordTrade;
use RO\Cmd\EOperType;
use RO\Cmd\ESource;
use RO\Cmd\ETakeStatus;
use RO\Cmd\ETradeType;
use RO\Cmd\GetTradeLogSessionCmd;
use RO\Cmd\ItemSellInfoRecordTradeCmd;
use RO\Cmd\LogItemInfo;
use RO\Cmd\MyPendingListRecordTradeCmd;
use RO\Cmd\MyTradeLogRecordTradeCmd;
use RO\Cmd\NameInfoList;
use RO\Cmd\NtfCanTakeCountTradeCmd;
use RO\Cmd\RankType;
use RO\Cmd\RecordUserTradeParam;
use RO\Cmd\ReqServerPriceRecordTradeCmd;
use RO\Cmd\ResellPendingRecordTrade;
use RO\Cmd\SellItemRecordTradeCmd;
use RO\Cmd\StateType;
use RO\Cmd\TodayFinanceItem;
use RO\Cmd\TradeItemBaseInfo;
use RO\Trade\Dao\Finance;
use RO\Trade\Dao\Goods;
use RO\Trade\Dao\ItemList;
use RO\Trade\Dao\Record;
use RO\Trade\Dao\RecordBought;
use RO\Trade\Dao\RecordSold;

/**
 * 来自用户的请求
 *
 * @package RO
 */
class ZoneForwardUser extends \RO\ZoneForward
{
    public function run()
    {
        switch ($this->param)
        {
            case RecordUserTradeParam::DETAIL_PENDING_LIST_RECORDTRADE:
                # 请求挂单列表，详情
                # see TradeManager.cpp line 2731
                if (IS_DEBUG)
                {
                    Server::$instance->debug("请求挂单列表，详情:" . json_encode(new \RO\Cmd\DetailPendingListRecordTradeCmd($this->proto)));
                }
                $msg = new \RO\Cmd\DetailPendingListRecordTradeCmd($this->proto);
                //$this->getItemList($msg);
                Server::$listChannel->push(pack('CQQSLCLL', 2, $msg->charid, $this->fd, $this->fromId, $msg->getSearchCond()->item_id, $msg->getSearchCond()->rank_type ?: RankType::RANKTYPE_ITEM_PRICE_INC, $msg->search_cond->page_index, $msg->search_cond->trade_type));
                break;

            case RecordUserTradeParam::BRIEF_PENDING_LIST_RECORDTRADE:
                # 请求挂单列表，简要的
                # see TradeManager.cpp line 2744
                if (IS_DEBUG)
                {
                    Server::$instance->debug("请求挂单列表，简要的:" . json_encode(new \RO\Cmd\BriefPendingListRecordTradeCmd($this->proto)));
                }
                $msg = new \RO\Cmd\BriefPendingListRecordTradeCmd($this->proto);

                # 投递数据给列表任务处理
                Server::$listChannel->push(pack('CQQSSSC', 1, $msg->charid, $this->fd, $this->fromId, $msg->category, $msg->job, $msg->fashion ?: 0));
                break;

            case RecordUserTradeParam::QUERY_ITEM_COUNT_TRADE_PARAM:
                if (IS_DEBUG)
                {
                    Server::$instance->debug("请求批量获取物品最低价格: " . json_encode(new \RO\Cmd\QueryItemCountTradeCmd($this->proto)));
                }
                Server::$listChannel->push(pack('CQSa*', 3, $this->fd, $this->fromId, $this->proto));
                break;

            case RecordUserTradeParam::HOT_ITEMID_RECORDTRADE:
                # 热门物品记录
                # see TradeManager.cpp line 2757
                if (IS_DEBUG)
                {
                    Server::$instance->debug("热门物品记录:" . json_encode(new \RO\Cmd\HotItemidRecordTrade($this->proto)));
                }
                break;

            case RecordUserTradeParam::REQ_SERVER_PRICE_RECORDTRADE:
                # 交易-请求服务器价格
                # see TradeManager.cpp line 2771
                if (IS_DEBUG)
                {
                    Server::$instance->debug("交易-请求服务器价格:" . json_encode(new \RO\Cmd\ReqServerPriceRecordTradeCmd($this->proto)));
                }

                $this->getPrice(new ReqServerPriceRecordTradeCmd($this->proto));
                break;

            case RecordUserTradeParam::BUY_ITEM_RECORDTRADE:
                # 交易-请求购买
                $cmd = new BuyItemRecordTradeCmd($this->proto);
                switch ($cmd->type)
                {
                    case ETradeType::ETRADETYPE_TRADE:
                        ActBuy::reqBuy($this, $cmd);
                        break;
                    case ETradeType::ETRADETYPE_BOOTH:
                        BoothOrderService::buy($this, $cmd);
                        break;
                }
                break;

            case RecordUserTradeParam::SELL_ITEM_RECORDTRADE:
                # 交易-请求出售
                # see TradeManager.cpp line 2860
                $cmd = new SellItemRecordTradeCmd($this->proto);
                switch ($cmd->type)
                {
                    case ETradeType::ETRADETYPE_TRADE:
                        ActSell::factory($this)->reqSell($cmd);
                        break;
                    case ETradeType::ETRADETYPE_BOOTH:
                        BoothOrderService::sell($this, $cmd);
                        break;
                }
                break;

            case RecordUserTradeParam::RESELL_PENDING_RECORDTRADE:
                # 交易-重新上架
                # see TradeManager.cpp line 2890
                $cmd = new ResellPendingRecordTrade($this->proto);
                switch ($cmd->type)
                {
                    case ETradeType::ETRADETYPE_TRADE:
                        ActSell::factory($this)->reqReSell($cmd);
                        break;
                    case ETradeType::ETRADETYPE_BOOTH:
                        BoothOrderService::reSell($this, $cmd);
                        break;
                }
                break;

            case RecordUserTradeParam::CANCEL_PENDING_RECORDTRADE:
                # 交易-取消订单
                # see TradeManager.cpp line 2914
                $cmd = new CancelItemRecordTrade($this->proto);
                switch ($cmd->type)
                {
                    case ETradeType::ETRADETYPE_TRADE:
                        ActSell::factory($this)->reqCancel($cmd);
                        break;
                    case ETradeType::ETRADETYPE_BOOTH:
                        BoothOrderService::playerCancelOrder($this, $cmd);
                        break;
                }
                break;

            case RecordUserTradeParam::PANEL_RECORDTRADE:
                # 交易-面板操作, 游戏原来的缓存 zoneid 用
                $msg = new \RO\Cmd\PanelRecordTrade($this->proto);
                if (IS_DEBUG)
                {
                    Server::$instance->debug("打开、关闭面板:". json_encode($msg));
                }
                switch ($msg->oper)
                {
                    case \RO\Cmd\EPanelOperType::EPANEL_OPEN:
                        Player::setZoneId($this->charId, $this->zoneId);
                        break;

                    case \RO\Cmd\EPanelOperType::EPANEL_CLOSE:
                        Player::setZoneId($this->charId, $this->zoneId);
                        break;
                }
                break;

            case RecordUserTradeParam::NTF_CAN_TAKE_COUNT_TRADE_PARAM:
                # 请求-可以领取的总个数
                $count = self::getCanTakeCount($this->charId);
                $msg   = new NtfCanTakeCountTradeCmd($this->proto);
                $msg->setCount($count);
                $this->sendToUser($msg);
                break;

            case RecordUserTradeParam::MY_PENDING_LIST_RECORDTRADE:
                # 我的挂单列表
                if (IS_DEBUG)
                {
                    Server::$instance->debug("交易-我的挂单:" . json_encode(new \RO\Cmd\MyPendingListRecordTradeCmd($this->proto)));
                }
                $this->getMyOrders(new MyPendingListRecordTradeCmd($this->proto));
                break;

            case RecordUserTradeParam::ITEM_SELL_INFO_RECORDTRADE:
                $cmd = new ItemSellInfoRecordTradeCmd($this->proto);
                switch ($cmd->type)
                {
                    case ETradeType::ETRADETYPE_TRADE:
                        $this->getItemSellInfo($cmd);
                        break;
                    case ETradeType::ETRADETYPE_BOOTH:
                        BoothOrderService::getItemSellInfo($this, $cmd);
                        break;
                }
                break;
            case RecordUserTradeParam::MY_TRADE_LOG_LIST_RECORDTRADE:
                if (IS_DEBUG)
                {
                    Server::$instance->debug("交易-交易记录:" . json_encode(new MyTradeLogRecordTradeCmd($this->proto)));
                }
                $this->getMyTradeRecord(new MyTradeLogRecordTradeCmd($this->proto));
                break;
            case RecordUserTradeParam::TAKE_LOG_TRADE_PARAM:
                if (IS_DEBUG)
                {
                    Server::$instance->debug("交易-领取操作:" . json_encode(new \RO\Cmd\TakeLogCmd($this->proto)));
                }

                $this->takeLogTrade(new \RO\Cmd\TakeLogCmd($this->proto));
                break;
            case RecordUserTradeParam::FETCH_NAMEINFO_TRADE_PARAM:
                if (IS_DEBUG)
                {
                    Server::$instance->debug("交易-获取交易记录玩家信息:" . json_encode(new \RO\Cmd\FetchNameInfoCmd($this->proto)));
                }
                $this->getTradeNameInfo(new \RO\Cmd\FetchNameInfoCmd($this->proto));
                break;
            case RecordUserTradeParam::QUERY_SERVANT_FINANCE_RANK:
                if (IS_DEBUG)
                {
                    Server::$instance->debug("交易-获取今日财经排行:" . json_encode(new \RO\Cmd\TodayFinanceRank($this->proto)));
                }
                $this->getTodayFinanceRank(new \RO\Cmd\TodayFinanceRank($this->proto));
                break;
            case RecordUserTradeParam::QUERY_SERVANT_FINANCE_DETAIL:
                if (IS_DEBUG)
                {
                    Server::$instance->debug("交易-获取今日财经详情:" . json_encode(new \RO\Cmd\TodayFinanceDetail($this->proto)));
                }
                $this->getTodayFinanceDetail(new \RO\Cmd\TodayFinanceDetail($this->proto));
                break;

            case RecordUserTradeParam::BOOTH_PLAYER_PENDING_LIST:
                BoothOrderService::getPlayerOrders($this, new BoothPlayerPendingListCmd($this->proto));
                break;

            default:
                Server::$instance->warn("交易-未知 RecordUserTradeParam : {$this->param}");
                break;
                //Server::$instance->warn('No support cmd: '. $this->cmd);
                //$msg = new \RO\Cmd\TradeFastEvent();
                //$msg->setRet(\RO\Cmd\TradeFastRet::TRADE_FAST_RET_NO_SUPPORT_PARAM);
                //
                //$this->send($msg);
                //break;
        }

        //Server::$instance->warn('No support cmd: '. $this->cmd);
        //$msg = new \RO\Cmd\TradeFastEvent();
        //$msg->setRet(\RO\Cmd\TradeFastRet::TRADE_FAST_RET_NO_SUPPORT_CMD);
        //
        //$this->send($msg);
    }

    /**
     * 获取服务器价格
     *
     * @param \RO\Cmd\ReqServerPriceRecordTradeCmd $msg
     */
    protected function getPrice(ReqServerPriceRecordTradeCmd $msg)
    {
        if (!isset($msg->itemData))
            return;

        if (!isset($msg->itemData->base))
            return;

        $itemId = $msg->itemData->base->id;
        $item   = Item::get($itemId, $msg->getItemData());
        if (!$item)
        {
            return;
        }

        $msg->price = $item->getPrice();
        if ($msg->trade_type !== ETradeType::ETRADETYPE_ALL)
        {
            if ($msg->trade_type === ETradeType::ETRADETYPE_BOOTH)
            {
                if ($stateType = Item::getBoothPublicityState($item))
                {
                    $msg->statetype = $stateType;
                }
            }
            else
            {
                $msg->count = $item->getItemListStock();
                $itemList   = $item->getItemList();
                if ($stateType = Item::getPublicityState($item, $itemList))
                {
                    $msg->statetype = $stateType;
                    if ($stateType === StateType::St_InPublicity)
                    {
                        $msg->price       = $itemList->pubPrice;
                        $msg->end_time    = $itemList->getEndTime();
                        $msg->buyer_count = $itemList->pubBuyPeople;
                    }
                }
            }
        }

        if (IS_DEBUG)
        {
            $log = "[交易所-物品详情] " . $msg->charid . ' - ' . $this->zoneId
                . ", itemId:"       . $itemId
                . ", 价格: "         . $msg->price
                . ", 公示状态: "    . ($msg->statetype ?: '-')
                . ", 交易所数量: "    . $msg->count
                . ", 摆摊数量: "      . (isset($itemList) && $itemList ? $itemList->boothStock : 0)
                . ", 购买人数: "      . $msg->buyer_count
                . ", 是否处于公示期: " . (isset($stateType) && $stateType === StateType::St_InPublicity ? '是' : '不是')
                . ", 公示期结束时间: " . date('Y-m-d H:i:s', $msg->end_time)
                . ($item->isOverlap ? '' : ", itemData: " . json_encode($msg->itemData))
                . ($item->isOverlap ? '' : ", hexItemData: " .  bin2hex($msg->itemData->serialize()));
            ;
            Server::$instance->debug($log);
        }
//        Server::$instance->debug('[协议处理-获取服务器价格] 耗时:'. (microtime(1) - $time));
        $this->sendToUser($msg);
    }

    protected function getItemSellInfo(\RO\Cmd\ItemSellInfoRecordTradeCmd $msg)
    {
        if (IS_DEBUG)
        {
            Server::$instance->debug("交易-物品交易信息:" . json_encode($msg));
        }

        do
        {
            $itemId = $msg->itemid;
            if ($msg->hasPublicityId() && $msg->publicity_id)
            {
                $itemList   = ItemList::getById($msg->publicity_id);
                if (!$itemList)
                    break;

                $item       = Item::get($itemId, $itemList->getItemData());
                if (!$item)
                    break;

                if ($stateType = Item::getPublicityState($item, $itemList))
                {
                    $msg->statetype   = $stateType;
                    $msg->count       = $itemList->stock;
                    $msg->buyer_count = $itemList->pubBuyPeople;
                }
            }
            else
            {
                $itemList = ItemList::getById($itemId);
                if (!$itemList)
                    break;

                if ($itemList->isOverlap)
                {
                    $msg->statetype = StateType::St_OverlapNormal;
                    $msg->count     = $itemList->stock;
                }
                else
                {
                    break;
                }
            }
        }
        while (false);

        $this->sendToUser($msg);
    }

    /**
     * 获取我的订单
     *
     * @param \RO\Cmd\MyPendingListRecordTradeCmd $msg
     */
    protected function getMyOrders(\RO\Cmd\MyPendingListRecordTradeCmd $msg)
    {
        $time   = time();
        $charId = (int) $msg->charid;
        $data   = Goods::getMyGoods($charId);

        /** @var Goods $goods */
        foreach ($data as $goods)
        {
            $info = new TradeItemBaseInfo();
            $info->setItemid($goods->itemId);
            $info->setCount($goods->stock);
            $info->setOrderId($goods->id);
            $info->setPrice($goods->getPrice());
            $info->setIsExpired($goods->status == Dao\Goods::STATUS_EXPIRED || $goods->time <= $time - (Server::$configExchange['ExchangeHour'] ?: 86400));
            if ($itemData = $goods->getItemData())
            {
                $info->setItemData($itemData);
            }

            if ($goods->isPublicity == 1 && $goods->endTime > $time)
            {
                $info->setPublicityId($goods->itemListId);
                $info->setEndTime($goods->endTime);
                $info->setPrice($goods->pubPrice);
            }

            $msg->addLists($info);
        }

        $this->sendToUser($msg);
    }

    /**
     * 获取领取数
     *
     * @param $charId
     * @return int
     */
    public static function getCanTakeCount($charId)
    {
        if (false === ($count = Server::$redis->get('take_count_' . $charId)))
        {
            $buyerCount  = RecordBought::getCanTakeCount($charId) ?: 0;
            $sellerCount = RecordSold::getCanTakeCount($charId) ?: 0;
            $count       = $buyerCount + $sellerCount;
            Server::$redis->set('take_count_' . $charId, $count, 3600);
        }

        return $count;
    }

    public static function cantTakeInfo(LogItemInfo $logInfo)
    {
        if ($logInfo->logtype == EOperType::EOperType_PublicityBuying)
        {
            return false;
        }

        if ($logInfo->status == ETakeStatus::ETakeStatus_Taking || $logInfo->status == ETakeStatus::ETakeStatus_Took)
        {
            return false;
        }

        return true;
    }

    public static function canTakeBoughtRecord(RecordBought $record)
    {
        if ($record->takeStatus !== Record::TAKE_STATUS_CAN_TAKE_GIVE)
        {
            return false;
        }

        if (in_array($record->status, [RecordBought::STATUS_PAY_SUCCESS, RecordBought::STATUS_PUBLICITY_SUCCESS, RecordBought::STATUS_PUBLICITY_CANCEL]))
        {
            return true;
        }

        return false;
    }

    public static function canTakeSoldRecord(RecordSold $record)
    {
        if ($record->takeStatus !== Record::TAKE_STATUS_CAN_TAKE_GIVE)
        {
            return false;
        }

        if ($record->status == RecordSold::STATUS_ADDING)
        {
            return true;
        }

        return false;
    }

    /**
     * 获取我的交易记录
     *
     * @param \RO\Cmd\MyTradeLogRecordTradeCmd $msg
     */
    protected function getMyTradeRecord(\RO\Cmd\MyTradeLogRecordTradeCmd $msg)
    {
        $time = microtime(1);
        $delTime = time() - 30 * 86400;     //30天前未领取的清除掉
        $canTakeCount = 0;
        /** @var Record[] $records */
        $records = [];

        $buyList = RecordBought::getBuyList($msg->charid);
        if ($buyList)
        {
            /** @var RecordBought $buy */
            foreach ($buyList as $buy)
            {
                if($buy->time <= $delTime)
                    continue;

                if (self::canTakeBoughtRecord($buy)) $canTakeCount++;

                $records[] = $buy;
            }
        }

        $sellList = RecordSold::getSellList($this->charId);
        if($sellList)
        {
            /** @var RecordSold $sell */
            foreach ($sellList as $sell)
            {
                if($sell->time <= $delTime)
                    continue;

                if (self::canTakeSoldRecord($sell)) $canTakeCount++;

                $records[] = $sell;
            }
        }

        // 缓存领取数
        Server::$redis->set('take_count_' . $this->charId, $canTakeCount, 3600);

        if (empty($records))
        {
            $this->sendToUser($msg);
            return;
        }

        usort($records, function($a, $b) {
            return $a->time < $b->time ? 1 : -1;
        });

        $recordCount = count($records);
        $pageNumber = Server::$configExchange['PageNumber'];
        $pageCount = ceil($recordCount / $pageNumber);

        $msg->setTotalPageCount($pageCount);
        $start = $msg->index * $pageNumber;

        if ($start >= $recordCount)
        {
            $maxIndex = $pageCount * $pageNumber - $pageNumber;
            $records = array_slice($records, $maxIndex, $pageNumber);
        }
        else
        {
            $records = array_slice($records, $start, $pageNumber);
        }

        foreach ($records as $index => $record)
        {
            $logInfo = $record->getLogItemInfo();
            $msg->addLogList($logInfo);
        }

        $this->sendToUser($msg);
        if ($canTakeCount > 0)
        {
            $cmd = new NtfCanTakeCountTradeCmd();
            $cmd->count = $canTakeCount;
            $this->sendToUser($cmd);
        }

        if (IS_DEBUG)
        {
            Server::$instance->debug('[协议处理-获取我的交易记录]  耗时:'. (microtime(1) - $time));
        }
    }

    public function getTradeNameInfo(\RO\Cmd\FetchNameInfoCmd $msg)
    {
        $time = microtime(1);

        /** @var NameInfoList $list */
        switch ($msg->type)
        {
            case EOperType::EOperType_NormalSell:
            case EOperType::EOperType_PublicitySellSuccess:
            case EOperType::EOperType_PublicitySellFail:
                $record = RecordSold::getById($msg->id);
                $list   = $record->getBuyersInfo();
                break;
            default:
                $record = RecordBought::getById($msg->id);
                $list   = $record->getSellersInfo();
        }

        if ($record->isManyPeople !== 1) return;

        if ($list === null) return;

        $count      = count($list->name_infos);
        $pageNumber = Server::$configExchange['PageNumber'];
        $pageCount  = ceil($count / $pageNumber);
        $start      = $msg->index * $pageNumber;

        if ($start >= $count) return;

        $nameInfoList             = new NameInfoList();
        $nameInfoList->name_infos = array_slice($list->name_infos, $start, $pageNumber);
        $msg->name_list           = $nameInfoList;
        $msg->total_page_count    = $pageCount;
        $this->sendToUser($msg);

        if (IS_DEBUG)
        {
            Server::$instance->debug('[协议处理-获取我的交易记录] 耗时:'. (microtime(1) - $time));
        }
    }

    /**
     * 领取物品,并向场景服务器发送请求
     * 需要场景服务器回调做最后确认和释放锁。
     *
     * @param \RO\Cmd\TakeLogCmd $msg
     */
    public function takeLogTrade(\RO\Cmd\TakeLogCmd $msg)
    {
        $time = microtime(1);

        do
        {
            if (!self::tryLockUser($this->charId, 60))
            {
                $info = "charId: {$this->charId} 获取用户锁失败";
                break;
            }

            switch ($msg->log->logtype)
            {
                case EOperType::EOperType_NormalSell:
                case EOperType::EOperType_PublicitySellSuccess:
                case EOperType::EOperType_PublicitySellFail:
                    $record = RecordSold::getById($msg->log->id);
                    $type   = 'sell';
                    break;
                default:
                    $record = RecordBought::getById($msg->log->id);
                    $type   = 'buy';
                    break;
            }

            if (!$record)
            {
                $info = '获取交易记录失败, ' . $type . ' record id ' . $msg->log->id;
                break;
            }

            if ($record->charId != $this->charId)
            {
                $info = '[交易记录] 用户id不一致';
                break;
            }

            if ($type === 'buy' && !self::canTakeBoughtRecord($record))
            {
                $info = '[买家交易记录] 不符合条件,物品不能领取';
                break;
            }
            else if ($type === 'sell' && !self::canTakeSoldRecord($record))
            {
                $info = '[卖家交易记录] 不符合条件,物品不能领取';
                break;
            }

            if (!$record->setMyTakeStatus(Record::TAKE_STATUS_TAKING))
            {
                $this->sendSysMsgToUser(ActBuy::SYS_MSG_DB_ERROR);
                $info = '领取失败';
                Server::$instance->warn('[协议处理-领取操作] 修改操作状态失败, ' . $type . ' record id:' . $msg->log->id);
                break;
            }

            $cmd = new GetTradeLogSessionCmd();
            $cmd->setCharid($this->charId);
            $cmd->setId($msg->log->id);
            $cmd->setLogtype($msg->log->logtype);

            // 卖家加钱
            if ($type === 'sell' && $record->status == RecordSold::STATUS_ADDING)
            {
                $itemInfo = new \RO\Cmd\ItemInfo();
                $itemInfo->setId(100);
                $itemInfo->setCount($record->price * $record->count - $record->tax);
                $itemInfo->setSource(ESource::ESOURCE_TRADE);
                $cmd->setItem($itemInfo);
            }
            // 买家抢购失败,加钱
            else if ($type === 'buy' && $record->status == RecordBought::STATUS_PUBLICITY_CANCEL)
            {
                $itemInfo = new \RO\Cmd\ItemInfo();
                $itemInfo->setId(100);
                $itemInfo->setCount(($record->totalCount - $record->count) * $record->price);
                $itemInfo->setSource(ESource::ESOURCE_TRADE_PUBLICITY_FAILRET);
                $cmd->setItem($itemInfo);
            }
            // 买家加装备或材料
            else if ($type === 'buy' && ($record->status == RecordBought::STATUS_PAY_SUCCESS || $record->status == RecordBought::STATUS_PUBLICITY_SUCCESS))
            {
                $item = Item::get($record->itemId);
                if ($item->isOverlap)
                {
                    $itemInfo = new \RO\Cmd\ItemInfo();
                    $itemInfo->setId($record->itemId);
                    $itemInfo->setCount($record->count);
                    $itemInfo->setSource(ESource::ESOURCE_TRADE);
                    $cmd->setItem($itemInfo);
                }
                else if ($itemData = $record->getItemData())
                {
                    if ($record->status == RecordBought::STATUS_PAY_SUCCESS)
                    {
                        $itemData->base->setSource(ESource::ESOURCE_TRADE);
                    }
                    else
                    {
                        $itemData->base->setSource(ESource::ESOURCE_TRADE_PUBLICITY);
                    }

                    $cmd->setItemData($itemData);
                }
                else
                {
                    $this->sendSysMsgToUser(ActBuy::SYS_MSG_DB_ERROR);
                    $info = '领取失败';
                    Server::$instance->warn('[协议处理-领取操作] 获取不到物品信息, ' . $type . ' record id:' . $msg->log->id);
                    break;
                }
            }

            if(IS_DEBUG)
            {
                Server::$instance->debug('[协议处理-领取操作] 领取命令:'. json_encode($cmd));
            }

            $this->sendToZone($cmd);

            if (IS_DEBUG)
            {
                $info = '符合领取条件,发送场景服务器,等待回调成功';
                Server::$instance->debug('[协议处理-领取操作] ' . $info);
                Server::$instance->debug('[协议处理-领取操作] 耗时:' . (microtime(1) - $time));
            }

            return;
        }
        while (0);

        self::unLockUser($this->charId);

        if (IS_DEBUG)
        {
            Server::$instance->debug('[协议处理-领取操作] '. $info);
            Server::$instance->debug('[协议处理-领取操作] 耗时:'. (microtime(1) - $time));
        }
    }

    /**
     * 给客户端推送一个新的购买、销售记录
     *
     * @param Dao\RecordBought|Dao\RecordSold $obj
     * @return bool
     */
    public static function sendNewRecordMsg($obj, $fd = null, $fromId = null)
    {
        if ($obj instanceof Dao\RecordSold)
        {
            $charId = $obj->charId;
            $zoneId = $obj->playerZoneId;
        }
        elseif ($obj instanceof Dao\RecordBought)
        {
            $charId = $obj->charId;
            $zoneId = $obj->playerZoneId;
        }
        else
        {
            return false;
        }

        if (null === $fd)
        {
            $zone = Server::$zone->get($zoneId);
            if (!$zone)return false;

            $fd     = $zone['fd'];
            $fromId = $zone['fromId'];
        }

        # 给列表增加统计
        $key = 'take_count_' . $charId;
        if (Server::$redis->exists($key))
        {
            Server::$redis->incr($key);
        }

        # 发送新的个数
        $msg = new \RO\Cmd\NtfCanTakeCountTradeCmd();
        $msg->count = self::getCanTakeCount($charId);
        self::sendToUserByFd($msg, $charId, $fd, $fromId);

        $msg         = new \RO\Cmd\AddNewLog();
        $msg->charid = $charId;
        $msg->log    = $obj->getLogItemInfo();

        return self::sendToUserByFd($msg, $charId, $fd, $fromId);
    }

    public function getTodayFinanceRank(\RO\Cmd\TodayFinanceRank $msg)
    {
        $time = microtime(1);

        do
        {
            try
            {
                $rs = Finance::getTodayFinanceRank($msg->rank_type, $msg->date_type);
                if (empty($rs)) {
                    break;
                }

                $lists = [];
                foreach ($rs as $v) {
                    $item = new TodayFinanceItem();
                    $item->item_id = $v['item_id'];
                    $item->ratio = $v['ratio'];
                    $lists[] = $item;
                }

                $msg->lists = $lists;
            }
            catch (\Exception $e)
            {
                Server::$instance->warn("[协议处理-今日财经] 请求参数 rank_type:{$msg->rank_type}, date_type:{$msg->date_type}, 错误信息: {$e->getMessage()}");
            }

        }while (0);


        if (IS_DEBUG)
        {
            Server::$instance->debug('[协议处理-今日财经排行] 耗时:'. (microtime(1) - $time) . ', 排行数据: ' . json_encode($msg->lists, JSON_UNESCAPED_UNICODE));
        }

        return $this->sendToUser($msg);
    }

    public function getTodayFinanceDetail(\RO\Cmd\TodayFinanceDetail $msg)
    {
        $time = microtime(1);

        do
        {
            try
            {
                $rs = Finance::getTodayFinanceDetail($msg->item_id, $msg->rank_type, $msg->date_type);
                if (empty($rs)) {
                    break;
                }

                $lists = [];
                foreach ($rs as $v) {
                    $item = new TodayFinanceItem();
                    $item->item_id = $v['item_id'];
                    $item->ratio = $v['val'];
                    $item->time  = $v['time'];
                    $lists[] = $item;
                }

                $msg->lists = $lists;
            }
            catch (\Exception $e)
            {
                Server::$instance->warn("[协议处理-今日财经] 请求参数 item_id:{$msg->item_id}, rank_type:{$msg->rank_type}, date_type:{$msg->date_type}, 错误信息: {$e->getMessage()}");
            }
        }while (0);

        if (IS_DEBUG)
        {
            Server::$instance->debug('[协议处理-今日财经排行] 耗时:'. (microtime(1) - $time) . ', 详情数据', $msg->lists);
        }

        return $this->sendToUser($msg);
    }


}