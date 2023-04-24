<?php
namespace RO\Booth;

use RO\Booth\Dao\BoothOrder;
use RO\Booth\Dao\BoothRecordBought;
use RO\Booth\Dao\BoothRecordSold;
use RO\Booth\Dao\BoothRecordTakeStatus;
use RO\Booth\Tasks\BuyItemTask;
use RO\Booth\Tasks\PlayerCancelOrderTask;
use RO\Cmd\AddItemRecordTradeCmd;
use RO\Cmd\BoothPlayerPendingListCmd;
use RO\Cmd\BuyItemRecordTradeCmd;
use RO\Cmd\CancelItemRecordTrade;
use RO\Cmd\EAddItemType;
use RO\Cmd\EListNtfType;
use RO\Cmd\EOperType;
use RO\Cmd\ETRADE_RET_CODE;
use RO\Cmd\ETradeType;
use RO\Cmd\ItemData;
use RO\Cmd\ItemSellInfoRecordTradeCmd;
use RO\Cmd\ListNtfRecordTrade;
use RO\Cmd\NameInfo;
use RO\Cmd\NameInfoList;
use RO\Cmd\ReduceItemRecordTrade;
use RO\Cmd\ReduceMoneyRecordTradeCmd;
use RO\Cmd\ResellPendingRecordTrade;
use RO\Cmd\SellItemRecordTradeCmd;
use RO\Cmd\StateType;
use RO\Cmd\TradeItemBaseInfo;
use RO\Cmd\TradeLogCmd;
use RO\Cmd\UpdateOrderTradeCmd;
use RO\Cmd\UpdateTradeLogCmd;
use RO\Trade\ActBuy;
use RO\Trade\ActSell;
use RO\Trade\Dao\ItemList;
use RO\Trade\Dao\Prohibition;
use RO\Trade\Item;
use RO\Trade\Player;
use RO\Trade\Server;
use RO\Trade\ZoneForwardScene;
use RO\Trade\ZoneForwardUser;
use RO\ZoneForward;

class BoothOrderService
{
    /**
     * 购买
     * worker
     *
     * @param ZoneForwardUser $forwardUser
     * @param BuyItemRecordTradeCmd $cmd
     */
    public static function buy(ZoneForwardUser $forwardUser, BuyItemRecordTradeCmd $cmd)
    {
        if (IS_DEBUG)
        {
            Server::$instance->debug("[摆摊购买] 请求购买:" . json_encode($cmd));
        }

        $charId = $forwardUser->charId;
        if ($cmd->charid != $charId)
        {
            # 购买物品人和请求人不一致
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊购买] 购买物品人和请求人不一致");
            }
            $forwardUser->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_INVALID_PARAMS);
            $cmd->setRet(ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forwardUser->sendToUser($cmd);
            return;
        }

        $itemInfo = $cmd->item_info;
        if (!isset($itemInfo->charid) || !isset($itemInfo->order_id) || !isset($itemInfo->count))
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊购买] 参数错误");
            }
            $forwardUser->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_INVALID_PARAMS);
            $cmd->setRet(ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forwardUser->sendToUser($cmd);
            return;
        }

        if (!$itemInfo || !$itemInfo->price > 0)
        {
            # 参数错误，给客户端返回内容
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊购买] 参数错误");
            }
            $forwardUser->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_INVALID_PARAMS);
            $cmd->setRet(ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forwardUser->sendToUser($cmd);
            return;
        }

        if ($itemInfo->count <= 0)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊购买] 购买数量不合法");
            }
            $forwardUser->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_INVALID_COUNT);
            $cmd->setRet(ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forwardUser->sendToUser($cmd);
            return;
        }

        if (!ZoneForward::tryBoothLock($charId))
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊购买] 抢不到锁, char_id:{$charId}");
            }
            return;
        }

        $task           = new BuyItemTask();
        $task->fd       = $forwardUser->fd;
        $task->charId   = $forwardUser->charId;
        $task->sellerId = $itemInfo->charid;
        $task->orderId  = $itemInfo->order_id;
        $task->buyCount = $itemInfo->count;
        $task->price    = $itemInfo->price;
        if (false === $task->deliver($task->sellerId))
        {
            $forwardUser->sendSysMsgToUser(ActBuy::SYS_MSG_SYS_ERROR);
            Server::$instance->warn("[摆摊购买] 投递失败! BuyItemRecordTradeCmd:" . json_encode($cmd));
            $cmd->setRet(ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL);
            $forwardUser->sendToUser($cmd);
        }
    }

    /**
     * 扣钱返还
     *
     * @param ZoneForwardScene $forward
     * @param ReduceMoneyRecordTradeCmd $cmd
     */
    public static function onReduceMoneyCallback(ZoneForwardScene $forward, ReduceMoneyRecordTradeCmd $cmd)
    {
        switch ($cmd->ret)
        {
            case ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS:
                $logStr = '扣钱成功';
                break;
            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_MONEY_IS_NOT_ENOUGH:
                $logStr = '扣钱失败, 金钱不足';
                break;
            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_FIND_USER_IN_SCENE:
                $logStr = '扣钱失败, 找不到玩家';
                break;
            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_QUOTA_IS_NOT_ENOUGH:
                $logStr = '扣钱失败, 额度不足';
                break;
            default:
                $logStr = '扣钱失败, ret:' . $cmd->ret;
                break;
        }

        Server::$instance->info("[摆摊支付] {$logStr}. cmd:" . json_encode($cmd));

        $record = BoothRecordBought::getById($cmd->record_id);
        if (false === $record)
        {
            Server::$instance->warn('[摆摊支付] ' . $logStr . ', 查找买家记录失败. cmd:' . json_encode($cmd));
            return;
        }
        else if (null === $record)
        {
            Server::$instance->warn('[摆摊支付] ' . $logStr . ', 买家记录找不到对应的id. cmd:' . json_encode($cmd));
            return;
        }

        if ($record->status != BoothRecordBought::STATUS_PAY_PENDING)
        {
            Server::$instance->warn('[摆摊支付] ' . $logStr . ', 买家不处于待支付状态. status:' . $record->status . ', cmd:' . json_encode($cmd));
            return;
        }

        $record->playerName   = $forward->name;
        $record->playerZoneId = $forward->zoneId;
        $rs                   = false;
        if ($cmd->ret == ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS)
        {
            if ($record->isPublicity)
            {
                $rs = self::handlePubPaySuccess($record, $forward);
            }
            else
            {
                $rs = self::handlePaySuccess($record, $forward);
            }
        }
        else
        {
            self::handlePayFail($record, $forward, $cmd);
        }

        $buyRstCmd         = new BuyItemRecordTradeCmd();
        $buyRstCmd->charid = $forward->charId;
        $buyRstCmd->type   = ETradeType::ETRADETYPE_BOOTH;
        if ($rs)
        {
            $buyRstCmd->ret = ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS;
        }
        else
        {
            $buyRstCmd->ret = ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL;
        }
        $forward->sendToUser($buyRstCmd);
    }

    protected static function handlePaySuccess(BoothRecordBought $record, ZoneForwardScene $forward)
    {
        $buyerQuota      = null;
        $record->canGive = $record->isCanGive($buyerQuota);
        $record->status  = BoothRecordBought::STATUS_PAY_SUCCESS;
        $orderQuota      = 0;
//        $orderQuota      = BoothOrder::getQuotaById($record->orderId);
//        if ($orderQuota === false)
//        {
//            Server::$instance->warn("[摆摊支付] 获取订单额度失败, order_id:{$record->orderId}, buy_record_id:{$record->id}");
//            $orderQuota = 0;
//        }

        $soldRecord               = new BoothRecordSold();
        $soldRecord->charId       = $record->sellerId;
        $soldRecord->playerName   = $record->sellerName;
        $soldRecord->playerZoneId = $record->sellerZoneId;
        $soldRecord->orderId      = $record->orderId;
        $soldRecord->buyerId      = $record->charId;
        $soldRecord->buyerName    = $record->playerName;
        $soldRecord->buyerZoneId  = $record->playerZoneId;
        $soldRecord->itemId       = $record->itemId;
        $soldRecord->itemKey      = $record->itemKey;
        $soldRecord->itemData     = $record->getItemData();
        $soldRecord->status       = BoothRecordSold::STATUS_ADDING;
        $soldRecord->takeStatus   = BoothRecordTakeStatus::TAKE_STATUS_CAN_TAKE_GIVE;
        $soldRecord->time         = time();
        $soldRecord->count        = $record->count;
        $soldRecord->price        = $record->realPrice;
        $soldRecord->quota        = $orderQuota;
        $soldRecord->refineLv     = $record->refineLv;
        $soldRecord->isDamage     = $record->isDamage;
        $soldRecord->tax          = $soldRecord->getTax();
        $soldRecord->buyersInfo   = new NameInfoList();
        $nameInfo                 = new NameInfo();
        $nameInfo->name           = $record->playerName;
        $nameInfo->zoneid         = $record->playerZoneId;
        $nameInfo->count          = $record->count;
        $soldRecord->buyersInfo->addNameInfos($nameInfo);

        $db    = Server::$mysqlMaster;
        $begin = $db->begin_transaction();
        if ($begin === false)
        {
            Server::$instance->warn("[摆摊支付] 开启事务失败");
            return false;
        }

        try
        {
            if ($record->update() === false)
            {
                throw new \Exception('更新买家记录失败!');
            }

            if (BoothOrder::deductStock($record->orderId, $record->count) === false)
            {
                throw new \Exception("减少库存失败! deduct_stock:{$record->count}, order_id:{$record->orderId},");
            }

            $sql = "UPDATE `" . BoothOrder::getTableName() . "` SET `status` = " . BoothOrder::STATUS_SOLD . " WHERE `stock` <= 0 AND `id` = {$record->orderId}";
            if ($db->query($sql) === false)
            {
                throw new \Exception("检测库存卖光失败! order_id:{$record->orderId}");
            }
            else
            {
                if ($db->affected_rows > 0)
                {
                    Server::$instance->info("[摆摊支付] 已卖光, order_id:{$record->orderId}");
                }
            }

            if ($soldRecord->insert() === false)
            {
                throw new \Exception('插入卖家记录失败!');
            }

            if ($db->commit() === false)
            {
                Server::$instance->warn("[摆摊支付] 提交事务失败!");
                return false;
            }

            Server::$instance->info("[摆摊支付] 购买成功! order_id:{$record->orderId}, buyer_id:{$record->charId}, buyer_record_id:{$record->id}, price:{$record->price}, count:{$record->count}, item_id:{$record->itemId}, can_give:{$record->canGive}, buyer_quota:{$buyerQuota}, item_data:" . json_encode($record->itemData));
            $logCmd              = new TradeLogCmd();
            $logCmd->pid         = $forward->charId;
            $logCmd->time        = time();
            $logCmd->type        = ZoneForward::ETRADETYPE_BOOTH_TRUE_BUY;
            $logCmd->itemid      = $record->itemId;
            $logCmd->count       = $record->count;
            $logCmd->price       = $record->price;
            $logCmd->tax         = 0;
            $logCmd->moneycount  = $record->count * $record->price;
            $logCmd->spend_quota = $record->spendQuota;
            $logCmd->strotherid  = $record->sellerId;
            $logCmd->iteminfo    = json_encode($record->getItemData());
            $logCmd->logid       = 'buy-' . $record->id;
            ZoneForward::pushToFluent($logCmd);

            Server::$instance->info("[摆摊支付] 出售成功! order_id:{$record->orderId}, seller_id:{$soldRecord->charId}, sold_record_id:{$soldRecord->id}, price:{$soldRecord->price}, count:{$soldRecord->count}, tax:{$soldRecord->tax}, item_id:{$soldRecord->itemId}, item_data:" . json_encode($soldRecord->itemData));
            $logCmd               = new TradeLogCmd();
            $logCmd->pid          = $soldRecord->charId;
            $logCmd->time         = time();
            $logCmd->type         = ZoneForward::ETRADETYPE_BOOTH_TRUE_SELL;
            $logCmd->itemid       = $soldRecord->itemId;
            $logCmd->count        = $soldRecord->count;
            $logCmd->price        = $soldRecord->price;
            $logCmd->tax          = $soldRecord->tax;
            $logCmd->moneycount   = $soldRecord->count * $soldRecord->price;
            $logCmd->spend_quota  = 0;
//            $logCmd->quota_lock   = 0;
//            $logCmd->quota_unlock = $soldRecord->count * $soldRecord->quota;
            $logCmd->strotherid   = $soldRecord->buyerId;
            $logCmd->iteminfo     = json_encode($soldRecord->itemData);
            $logCmd->logid        = 'sell-' . $soldRecord->id;
            ZoneForward::pushToFluent($logCmd);

            ItemList::decrItemBoothStock($record->itemId, $record->itemKey, $record->count);

            $zoneId = Player::getZoneId($soldRecord->charId) ?: $soldRecord->playerZoneId;
            if (false !== ($zone = Server::$zone->get($zoneId)))
            {
                $totalMoney = $soldRecord->count * $soldRecord->price - $soldRecord->tax;
                # 通知弹出提示
                // 成功出售：恭喜！您成功地售出了[63cd4e]%s[-]个[63cd4e]%s[-]，扣税 %s, 共获得了[63cd4e]%s[-]金币
                $params = [
                    $soldRecord->count,
                    Server::$item->get($record->itemId, 'name') ?: '',
                    $soldRecord->tax,
                    $totalMoney,
                ];

                ZoneForward::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $soldRecord->charId, ActSell::SYS_MGS_SELL_SUCCESS, $params);

                // 更新卖家列表
                $order = BoothOrder::getById($soldRecord->orderId);
                if ($order)
                {
                    $itemList = $order->getItemList();
                    if ($itemList !== null)
                    {
                        $updateMsg            = new UpdateOrderTradeCmd();
                        $updateMsg->charid    = $soldRecord->charId;
                        $updateMsg->info      = $itemInfo = new TradeItemBaseInfo();
                        $updateMsg->type      = ETradeType::ETRADETYPE_BOOTH;
                        $itemInfo->charid     = $order->charId;
                        $itemInfo->name       = $order->playerName;
                        $itemInfo->count      = $order->getActualStock();
                        $itemInfo->price      = $order->getPrice();
                        $itemInfo->order_id   = $order->id;
                        $itemInfo->is_expired = $order->status === BoothOrder::STATUS_EXPIRED;
                        $itemInfo->item_data  = $order->getItemData();
                        $itemInfo->itemid     = $order->itemId;
                        $itemInfo->up_rate    = $order->upRate;
                        $itemInfo->down_rate  = $order->downRate;
                        $itemInfo->type       = ETradeType::ETRADETYPE_BOOTH;
                        $itemInfo->overlap    = $itemList->isOverlap == 1 ? true : false;
                        $itemInfo->refine_lv  = $itemList->refineLv;
                        ZoneForward::sendToUserByFd($updateMsg, $soldRecord->charId, $zone['fd']);
                    }
                }
            }

            // 通知场景有新的购买记录
            $msg             = new UpdateTradeLogCmd();
            $msg->type       = EOperType::EOperType_NoramlBuy;
            $msg->charid     = $record->charId;
            $msg->id         = $record->id;
            $msg->trade_type = ETradeType::ETRADETYPE_BOOTH;
            $forward->sendToZone($msg);

            ZoneForward::unBoothLock($forward->charId);

            return true;
        }
        catch (\Exception $e)
        {
            $db->rollback();
            Server::$instance->warn("[摆摊支付] {$e->getMessage()} record_id:{$record->id}, char_id:{$record->charId}, item_id:{$record->itemId}, price:{$record->price}, total_count:{$record->totalCount}");
            return false;
        }
    }

    protected static function handlePubPaySuccess(BoothRecordBought $record, ZoneForwardScene $forward)
    {
        $record->status = BoothRecordBought::STATUS_PUBLICITY_PAY_SUCCESS;
        if ($record->update() === false)
        {
            Server::$instance->warn("[摆摊支付] 更新支付成功的公示购买记录失败. record_id:{$record->id}, char_id:{$record->charId}, item_id:{$record->itemId}, price:{$record->price}, count:{$record->totalCount}");
            return false;
        }
        else
        {
            Server::$instance->info("[摆摊支付] 公示支付成功! record_id:{$record->id}, char_id:{$record->charId}, item_id:{$record->itemId}, price:{$record->price}, count:{$record->totalCount}, lock_quota:{$record->spendQuota}");
            $logCmd              = new TradeLogCmd();
            $logCmd->pid         = $record->charId;
            $logCmd->time        = time();
            $logCmd->type        = ZoneForward::ETRADETYPE_BOOTH_PUBLICITY_SEIZURE;
            $logCmd->itemid      = $record->itemId;
            $logCmd->count       = $record->count;
            $logCmd->price       = $record->price;
            $logCmd->tax         = 0;
            $logCmd->moneycount  = $record->totalCount * $record->price;
            $logCmd->spend_quota = $record->spendQuota;
            $logCmd->strotherid  = '';
            $logCmd->iteminfo    = json_encode($record->getItemData());
            $logCmd->logid       = 'pub-pay-' . $record->id;
            ZoneForward::pushToFluent($logCmd);
        }

        $sql = "UPDATE `" . BoothOrder::getTableName() . "` SET `pub_buy_people` = `pub_buy_people` + 1 WHERE `id` = {$record->orderId}";
        if (Server::$mysqlMaster->query($sql) === false)
        {
            Server::$instance->warn("[摆摊购买] 公示购买递增购买人数失败, order_id:{$record->orderId}, char_id:{$record->charId}");
        }

        # 信息提示
        $params = [
            $record->totalCount,
            Server::$item->get($record->itemId, 'name') ?: '',
            $record->totalCount * $record->price,
            $record->totalCount * $record->spendQuota,
        ];
        $forward->sendSysMsgToUser(ActBuy::SYS_MSG_BOOTH_PUBLICITY_PAY_SUCCESS, $params);

        $msg         = new ListNtfRecordTrade();
        $msg->charid = $forward->charId;
        $msg->type   = EListNtfType::ELIST_NTF_MY_LOG;
        $forward->sendToUser($msg);

        ZoneForwardUser::unBoothLock($forward->charId);

        return true;
    }

    protected static function handlePayFail(BoothRecordBought $record, ZoneForwardScene $forward, ReduceMoneyRecordTradeCmd $cmd)
    {
        $record->status = BoothRecordBought::STATUS_PAY_FAIL;
        if ($record->isPublicity)
        {
            self::decrPubBuyCount($record->orderId, $record->charId, $record->totalCount);
            if ($record->update() === false)
            {
                Server::$instance->warn("[摆摊支付] 更新支付失败的公示购买记录失败. record_id:{$record->id}, char_id:{$record->charId}, item_id:{$record->itemId}, price:{$record->price}, count:{$record->totalCount}");
            }
        }
        else
        {
            $db    = Server::$mysqlMaster;
            $begin = $db->begin_transaction();
            if ($begin === false)
            {
                Server::$instance->warn("[摆摊支付] 开启事务失败");
                return;
            }

            try
            {
                if ($record->update() === false)
                {
                    throw new \Exception('[摆摊支付] 更新买家记录失败!');
                }

                if (BoothOrder::releaseLockStock($record->orderId, $record->totalCount) === false)
                {
                    throw new \Exception("[摆摊支付] 释放预占库存失败! release_stock:{$record->totalCount}, order_id:{$record->orderId},");
                }

                if ($db->commit() === false)
                {
                    Server::$instance->warn("[摆摊支付] 提交事务失败!");
                    return;
                }

                return;
            }
            catch (\Exception $e)
            {
                $db->rollback();
                Server::$instance->warn("[摆摊支付] {$e->getMessage()} record_id:{$record->id}, char_id:{$record->charId}, item_id:{$record->itemId}, price:{$record->price}, count:{$record->totalCount}");
                return;
            }
        }

        ZoneForwardUser::unBoothLock($forward->charId);
    }

    /**
     * 出售
     * worker进程执行
     *
     * @param ZoneForwardUser $forwardUser
     * @param SellItemRecordTradeCmd $cmd
     * @return bool
     */
    public static function sell(ZoneForwardUser $forwardUser, SellItemRecordTradeCmd $cmd)
    {
        if (IS_DEBUG)
        {
            Server::$instance->debug("[摆摊上架] 出售, msg:" . json_encode($cmd));
        }

        # 检查是否同一个人
        if ($cmd->charid != $forwardUser->charId)
        {
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
            return false;
        }

        $item = self::getAllowForSellItem($forwardUser, $cmd->item_info);
        if ($item === false)
        {
            return false;
        }

        if (BoothOrder::isSellFull($forwardUser->charId))
        {
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_IS_FULL);
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊订单] 请求出售,挂单已满");
            }
            return false;
        }

        if (!ZoneForward::tryBoothLock($forwardUser->charId))
        {
            Server::$instance->warn("[摆摊上架] 设置摆摊玩家锁失败, charId: {$forwardUser->charId}");
            return false;
        }

        $order               = new BoothOrder();
        $order->itemId       = $item->id;
        $order->itemKey      = $item->getKey();
        $order->charId       = $forwardUser->charId;
        $order->playerName   = '';
        $order->playerZoneId = $forwardUser->zoneId;
        $order->status       = BoothOrder::STATUS_PENDING;
        $order->count        = $cmd->item_info->count;
        $order->stock        = $cmd->item_info->count;
        $order->originCount  = $cmd->item_info->count;
        $order->isPublicity  = 0;
        $order->endTime      = 0;
        $order->itemData     = null;
        $price               = $cmd->item_info->price;
        if ($cmd->item_info->up_rate > 0)
        {
            $order->upRate   = $cmd->item_info->up_rate;
            $order->downRate = 0;
            $price           = BoothOrder::calcUpRatePrice($price, $order->upRate);
        }
        else if ($cmd->item_info->down_rate > 0)
        {
            $order->upRate   = 0;
            $order->downRate = $cmd->item_info->down_rate;
            $price           = BoothOrder::calcDownRatePrice($price, $order->downRate);
        }
        else
        {
            $cmd->item_info->up_rate = 0;
            $cmd->item_info->down_rate = 0;
        }

        $order->boothFee = ActSell::getBoothFee($price, $cmd->item_info->count);
//        $order->quota    = BoothOrder::calcQuota($price, $cmd->item_info->publicity_id > 0 ? 1 : 0);

        if (!$order->insert())
        {
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_CANNOT_SELL3);
            ZoneForward::unBoothLock($forwardUser->charId);
            return false;
        }

        $msg       = new ReduceItemRecordTrade();
        $msg->type = ETradeType::ETRADETYPE_BOOTH;
        $msg->setItemInfo($cmd->item_info);
        $msg->setCharid($forwardUser->charId);
        $msg->setBoothfee($order->boothFee);
        $msg->setIsResell(false);
        $msg->setOrderid($order->id);
//        $msg->setQuota($order->quota);
//        $msg->setQuotaLock(BoothOrder::calcTotalQuota($order->quota, $order->stock));
        $forwardUser->sendToZone($msg);

        Server::$instance->info("[摆摊上架] 请求扣物品, order_id:{$order->id}, char_id:{$order->charId}, item_id:{$order->itemId}, item_key:{$order->itemKey}, count:{$order->count}, booth_fee:{$msg->boothfee}, quota:{$msg->quota}, quota_lock:{$msg->quota_lock}");

        return true;
    }

    /**
     * 重新上架
     * worker进程执行
     *
     * @param ZoneForwardUser $forwardUser
     * @param ResellPendingRecordTrade $cmd
     * @return bool
     */
    public static function reSell(ZoneForwardUser $forwardUser, ResellPendingRecordTrade $cmd)
    {
        if (IS_DEBUG)
        {
            Server::$instance->debug("[摆摊上架] 重新上架, msg:" . json_encode($cmd));
        }

        # 锁用户操作
        if (!ZoneForward::tryBoothLock($forwardUser->charId))
        {
            return false;
        }

        do
        {
            if (!$cmd->order_id)
            {
                $err = ActSell::SYS_MSG_SELL_INVALID_PARAMS;
                break;
            }

            # 检查是否同一个人
            if ($cmd->charid != $forwardUser->charId)
            {
                $err = ActSell::SYS_MSG_SELL_INVALID_PARAMS;
                break;
            }

            $order = BoothOrder::getById($cmd->order_id);
            if ($order)
            {
                if ($order->charId != $cmd->charid)
                {
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[摆摊上架] 重新上架,非本人订单!");
                    }
                    $err = ActSell::SYS_MSG_SELL_INVALID_PARAMS;
                    break;
                }

                if (!$cmd->item_info)
                {
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[摆摊上架] 重新上架,非法item_info!");
                    }
                    $err = ActSell::SYS_MSG_SELL_INVALID_PARAMS;
                    break;
                }

                if ($cmd->item_info->itemid != $order->itemId)
                {
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[摆摊上架] 重新上架,物品id不一致!");
                    }
                    $err = ActSell::SYS_MSG_SELL_INVALID_PARAMS;
                    break;
                }

                if ($order->getActualStock() <= 0)
                {
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[摆摊上架] 重新上架, 订单已卖完毕!");
                    }
                    $err = ActSell::SYS_MSG_CANCEL_ALREADY_SELLED;
                    break;
                }

                # 判断物品状态
                switch ($order->status)
                {
                    case BoothOrder::STATUS_EXPIRED:
                        # 通过
                        break;
                    default:
                        if (IS_DEBUG)
                        {
                            Server::$instance->debug("[摆摊上架] 重新上架, 状态不通过!");
                        }
                        $err = ActSell::SYS_MSG_CANCEL_WAS_LOCKED;
                        break 2;
                }

                if ($order->isPublicity == 1)
                {
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[摆摊上架] 重新上架, 公示物品还没有由系统处理完毕!");
                    }
                    $err = ActSell::SYS_MSG_CANCEL_WAS_LOCKED;
                    break;
                }
            }
            else if (false === $order)
            {
                $err = ActSell::SYS_MSG_DB_ERROR;
                break;
            }
            else
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊上架] 重新上架, 订单不存在!");
                }
                $err = ActSell::SYS_MSG_CANCEL_ALREADY_SELLED;
                break;
            }

            $cmd->item_info->count     = $order->stock;
            $cmd->item_info->item_data = $order->getItemData();
            $item                      = self::getAllowForSellItem($forwardUser, $cmd->item_info);
            if ($item === false)
            {
                ZoneForward::unBoothLock($forwardUser->charId);
                return false;
            }

            if ($order->setPendingStatus() <= 0)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊上架] 重新上架, 设为待处理状态失败!");
                }
                $err = ActSell::SYS_MSG_CANCEL_WAS_LOCKED;
                ZoneForward::unBoothLock($forwardUser->charId);
                break;
            }

            $price = $cmd->item_info->price;
            if ($cmd->item_info->up_rate > 0)
            {
                $cmd->item_info->down_rate = 0;
                $price = BoothOrder::calcUpRatePrice($price, $cmd->item_info->up_rate);
            }
            else if ($cmd->item_info->down_rate > 0)
            {
                $cmd->item_info->up_rate = 0;
                $price = BoothOrder::calcDownRatePrice($price, $cmd->item_info->down_rate);
            }
            else
            {
                $cmd->item_info->up_rate = 0;
                $cmd->item_info->down_rate = 0;
            }

            // 计算重新上架要增加或减少的额度
//            $curQuota   = BoothOrder::calcQuota($price, $cmd->item_info->publicity_id > 0 ? 1 : 0);
//            $totalQuota = BoothOrder::calcTotalQuota($order->quota, $order->stock);
//            $lockQuota  = BoothOrder::calcTotalQuota($curQuota, $order->stock);

            $msg       = new ReduceItemRecordTrade();
            $msg->type = ETradeType::ETRADETYPE_BOOTH;
            $msg->setItemInfo($cmd->item_info);
            $msg->setCharid($forwardUser->charId);
            $msg->setBoothfee(ActSell::getBoothFee($price, $cmd->item_info->count));
            $msg->setIsResell(true);
            $msg->setOrderid($order->id);
//            $msg->setQuota($curQuota);
//            $msg->setQuotaUnlock($totalQuota);
//            $msg->setQuotaLock($lockQuota);
            $forwardUser->sendToZone($msg);

            Server::$instance->info("[摆摊上架] 重新上架,请求扣上架费, order_id:{$order->id}, char_id:{$order->charId}, item_id:{$order->itemId}, item_key:{$order->itemKey}, count:{$order->count}, booth_fee:{$msg->boothfee}, up_rate:{$cmd->item_info->up_rate}, down_rate:{$cmd->item_info->down_rate}, last_quota:{$order->quota}, lock_quota:{$msg->quota_lock}, total_quota:{$msg->quota_unlock}");
            return true;
        }
        while (false);

        if ($err)
        {
            $forwardUser->sendSysMsgToUser($err);
        }

        ZoneForward::unBoothLock($forwardUser->charId);

        return false;
    }

    public static function playerCancelOrder(ZoneForwardUser $forwardUser, CancelItemRecordTrade $cmd)
    {
        if (IS_DEBUG)
        {
            Server::$instance->debug("[摆摊下架] " . json_encode($cmd));
        }

        $orderId = intval($cmd->order_id);
        if (!$orderId)
        {
            return;
        }

        if ($cmd->charid != $forwardUser->charId)
        {
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_CANCEL_WAS_LOCKED);
            Server::$instance->warn("[摆摊下架] charId 不一致: {$cmd->charid} != {$forwardUser->charId}, orderId: $cmd->order_id");
            return;
        }

//        if (false === ZoneForward::tryBoothLock($forwardUser->charId))
//        {
//            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_CANCEL_WAS_LOCKED);
//            return;
//        }

        $task          = new PlayerCancelOrderTask();
        $task->fd      = $forwardUser->fd;
        $task->charId  = $forwardUser->charId;
        $task->orderId = $cmd->order_id;
        if ($task->deliver($forwardUser->charId) === false)
        {
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SYS_ERROR);
            ZoneForward::unBoothLock($forwardUser->charId);
        }
    }

    /**
     * 扣物品回调
     *
     * @param ZoneForwardScene $forward
     * @param ReduceItemRecordTrade $cmd
     */
    public static function onReduceItemRecordCallback(ZoneForwardScene $forward, ReduceItemRecordTrade $cmd)
    {
        if ($cmd->is_resell)
        {
            Server::$instance->info('[摆摊扣物品返回] 重新上架扣手续费返回:' . json_encode($cmd));
        }
        else
        {
            Server::$instance->info('[摆摊扣物品返回] 上架扣手续费、装备返回:' . json_encode($cmd));
        }

        switch ($cmd->ret)
        {
            # 扣物品成功或扣手续费成功
            case ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS:
                $rs = self::putOnSell($forward, $cmd);
                if ($rs)
                {
                    $ret = ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS;
                    $cmd->item_info->order_id = $cmd->orderid;
                }
                else
                {
                    $ret = ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL;
                }

                if ($cmd->is_resell)
                {
                    $msg           = new ResellPendingRecordTrade();
                    $msg->type     = ETradeType::ETRADETYPE_BOOTH;
                    $msg->charid   = $forward->charId;
                    $msg->order_id = $cmd->orderid;
                    $msg->ret      = $ret;
                }
                else
                {
                    $msg            = new SellItemRecordTradeCmd();
                    $msg->type      = ETradeType::ETRADETYPE_BOOTH;
                    $msg->charid    = $forward->charId;
                    $msg->item_info = $cmd->item_info;
                    $msg->ret       = $ret;
                }

                $forward->sendToUser($msg);
                return;
            # 金额不足
            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_MONEY_IS_NOT_ENOUGH:
                break;
            # 额度不足
            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_QUOTA_IS_NOT_ENOUGH:
                break;
            # 找不到物品
            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_ITEM_IS_FROSTED:
                $forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_CANNOT_SELL2);
                break;
            #出售失败
            case ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL:
                break;
            #不能出售,可能是背包里没这个装备，要么就是检测不通过
            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_CANNOT_SELL:
                $forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_CANNOT_SELL2);
                break;
            # 其它错误
            default:
                $forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_CANNOT_SELL3);
                Server::$instance->warn("[扣物品返回] 场景服务器返回了一个未知返回码:{$cmd->ret} 卖家id:{$cmd->charid} 挂单id:{$cmd->orderid} 物品id:{$cmd->item_info->itemid} 价格:{$cmd->item_info->price} 数量:{$cmd->item_info->count} msg:" . json_encode($cmd));
                break;
        }

        # 清理待上架状态订单
        if ($cmd->ret !== ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS)
        {
            if (false === $cmd->is_resell)
            {
                BoothOrder::removeById($cmd->orderid);
                Server::$instance->info("[摆摊上架] 上架失败, 移除订单. ret:{$cmd->ret}, order_id:{$cmd->orderid}, char_id:{$cmd->charid}, item_id:{$cmd->item_info->itemid}");
            }

            ZoneForward::unBoothLock($forward->charId);
        }
    }

    protected static function putOnSell(ZoneForwardScene $forward, ReduceItemRecordTrade $cmd)
    {
        $tag     = $cmd->is_resell ? '摆摊重新上架' : '摆摊上架';
        $orderId = $cmd->orderid;
        $order   = BoothOrder::getById($orderId);
        if ($order === false)
        {
            return false;
        }
        else if ($order === null)
        {
            Server::$instance->warn("[{$tag}] order_id:{$orderId}不存在");
            return false;
        }

        if ($order->status !== BoothOrder::STATUS_PENDING)
        {
            Server::$instance->warn("[{$tag}] 上架的订单, 处于非法状态! order_id:{$orderId}, status:{$order->status}");
            return false;
        }

        $itemInfo                        = $cmd->item_info;
        $item                            = Item::get($itemInfo->itemid, $itemInfo->item_data);
        if (!$item) {
            Server::$instance->warn("[{$tag}] 查找Item失败, item_id:{$itemInfo->itemid}");
            return false;
        }
        $itemKey        = $item->getKey();
        $itemList       = ItemList::getByKey($itemKey);
        if ($itemList)
        {
//            $itemList->item = $item;
//            $isNewPublicity = $itemList->tryOpenPublicity($itemInfo->price);
//            if ($isNewPublicity)
//            {
//                $itemList->update();
//            }
        }
        else
        {
            // 只有在内存表找不到时且数据库查找返回false出现
            if ($itemList === false)
            {
                Server::$instance->warn("[{$tag}] 查找ItemList失败, item_id:{$itemInfo->itemid}, item_key:{$itemKey}");
            }

            $itemList               = new ItemList([], $itemKey);
            $itemList->item         = $item;
            $itemList->itemId       = $item->id;
            $itemList->isOverlap    = $item->isOverlap;
            $itemList->itemData     = $itemInfo->item_data && $item->equipType > 0 ? $itemInfo->item_data : null;
            $itemList->refineLv     = $itemInfo->item_data && $item->equipType > 0 ? $itemInfo->item_data->equip->refinelv : 0;
            $itemList->isDamage     = $itemInfo->item_data && $item->equipType > 0 ? intval($itemInfo->item_data->equip->damage) : 0;
            $itemList->boothStock   = 0;
            $itemList->stock        = 0;
            $itemList->isResetStock = 1;
//            $isNewPublicity         = $itemList->tryOpenPublicity($itemInfo->price);

            if (!$itemList->insert())
            {
                Server::$instance->warn("[{$tag}] 添加新的ItemList失败, item_id:{$itemInfo->itemid}, item_key:{$itemKey}");
            }
        }

//        if ($isNewPublicity)
//        {
//            Server::$instance->info("[{$tag}] 新公示开启, item_id:{$item->id}, item_key:{$itemKey}, pub_price:{$order->pubPrice}, end_time:{$order->endTime}, order_id:{$orderId}, booth_stock:{$itemList->boothStock}, item_list_stock:{$itemList->stock}, last_sold_num:{$item->soldNum}");
//        }

        $lastQuota           = $order->quota;
        $order->playerZoneId = $forward->zoneId;
        $order->playerName   = $forward->name;
        $order->time         = time();
        $order->isPublicity  = $itemInfo->publicity_id > 0 ? 1 : 0;
        $order->pubPrice     = $order->isPublicity == 1 ? $itemInfo->price : 0;
        $order->endTime      = $order->isPublicity == 1 ? time() + $item->publicityShowTime : 0;
        $order->pubBuyPeople = 0;
        $order->status       = BoothOrder::STATUS_SELLING;
        $order->boothFee     = $cmd->boothfee;
        $order->quota        = $cmd->quota;

        if ($order->isPublicity == 1 && $item->lastPubEndTime < $order->endTime)
        {
            Server::$item->set($item->id, ['lastPubEndTime' => $order->endTime]);
        }

        if ($cmd->is_resell)
        {
            $order->count    = $order->stock;
            $order->upRate   = $itemInfo->up_rate;
            $order->downRate = $itemInfo->down_rate;

            # 商人技能退回上架费
            $percent = Player::getReturnRate($order->charId);
            if ($percent)
            {
                $boothFee = $cmd->boothfee;
                $retMoney = intval($boothFee * $percent / 1000);
                if ($retMoney)
                {
                    $retBoothFeeCmd = ActSell::getReturnBoothFeeCmd($order->charId, $retMoney);
                    if ($forward->sendToZone($retBoothFeeCmd))
                    {
                        Server::$instance->info("[{$tag}] 商人技能,上架费用返还. 上架费:{$boothFee}, 返还费:{$retMoney}, 商人上架费比率:{$percent}, order_id:{$order->id}, charId: {$order->charId}, itemId: {$order->itemId}, msg: " . json_encode($retBoothFeeCmd));
                    }
                    else
                    {
                        Server::$instance->warn("[{$tag}] 商人技能,上架费用返还失败,因发送消息失败. 上架费:{$boothFee}, 返还费:{$retMoney}, 商人上架费比率:{$percent}, order_id:{$order->id}, charId: {$order->charId}, itemId: {$order->itemId}, msg: " . json_encode($retBoothFeeCmd));
                    }
                }
            }
        }
        else
        {
            // 理论不存在该情况, 扣物品已经校验过物品的合法性
            if ($order->itemKey != $itemKey)
            {
                Server::$instance->warn("[{$tag}] order_id:{$order->id}, 原物品itemKey:{$order->itemKey}, 扣物品返回itemKey:{$itemKey}, 两者不一致");
            }

            $itemData = null;
            if ($item->equipType > 0)
            {
                $itemInfo->item_data->base->guid = $itemInfo->guid;
                $itemData                        = $itemInfo->item_data;
            }
            $order->itemKey  = $itemKey;
            $order->itemData = $itemData;
        }

        if ($order->isPublicity == 1)
        {
            Server::$instance->info("[{$tag}] 上架公示订单, item_id:{$item->id}, item_key:{$itemKey}, pub_price:{$order->pubPrice}, end_time:{$order->endTime}, order_id:{$orderId}, stock:{$order->stock}, up_rate:{$order->upRate}, down_rate:{$order->downRate}, booth_stock:{$itemList->boothStock}, item_list_stock:{$itemList->stock}, last_sold_num:{$item->soldNum}");
        }
        else
        {
            // 非公示订单增加公共库存
            ItemList::incrItemBoothStock($order->itemId, $order->itemKey, $order->stock);
        }

        $logCmd               = new TradeLogCmd();
        $logCmd->pid          = $order->charId;
        $logCmd->time         = time();
        $logCmd->type         = $cmd->is_resell ? ZoneForward::ETRADETYPE_BOOTH_RESELL : ZoneForward::ETRADETYPE_BOOTH_SELL;
        $logCmd->itemid       = $order->itemId;
        $logCmd->count        = $order->count;
        $logCmd->price        = $order->getRealPrice();
        $logCmd->tax          = $order->boothFee;
        $logCmd->moneycount   = $logCmd->price * $logCmd->count;
        $logCmd->spend_quota  = 0;
//        $logCmd->quota_lock   = $cmd->quota_lock;
//        $logCmd->quota_unlock = $cmd->quota_unlock;
        $logCmd->strotherid   = '';
        $logCmd->iteminfo     = json_encode($order->itemData);
        $logCmd->logid        = 'order-' . $order->id;
        ZoneForward::pushToFluent($logCmd);

        if (false === $order->update())
        {
            Server::$instance->info("[{$tag}] 摆摊上架失败, 写入延迟执行队列. order_id:{$order->id}, char_id:{$order->charId}, item_id:{$order->itemId}, item_key:{$order->itemKey}, quota:{$order->quota}, last_quota:{$lastQuota}, unlock_quota:{$cmd->quota_unlock}, lock_quota:{$cmd->quota_lock}");
            Server::saveDelaySQL(Server::$mysqlMaster->last_query);
        }
        else
        {
            Server::$instance->info("[{$tag}] 摆摊上架成功. order_id:{$order->id}, char_id:{$order->charId}, item_id:{$order->itemId}, item_key:{$order->itemKey}, quota:{$order->quota}, last_quota:{$lastQuota}, unlock_quota:{$cmd->quota_unlock}, lock_quota:{$cmd->quota_lock}");
        }


        ZoneForward::unBoothLock($forward->charId);

        return true;
    }

    /**
     * 获取允许出售的物品
     * 该函数会校验itemInfo的参数, 符合校验的才返回, 否则会向玩家发送信息
     *
     * @param ZoneForwardUser $forwardUser
     * @param TradeItemBaseInfo $itemInfo
     * @return false|Item
     */
    public static function getAllowForSellItem(ZoneForwardUser $forwardUser, TradeItemBaseInfo $itemInfo)
    {
        if ($itemInfo->item_data)
        {
            if (!$itemInfo->item_data->base || !$itemInfo->item_data->equip)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊订单] 装备参数不正确");
                }
                $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }

            if ($itemInfo->item_data->base->id != $itemInfo->itemid)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊订单] 请求出售,item信息不一致");
                }
                $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }

            if ($itemInfo->item_data->equip->lv > 0)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊订单] 请求出售,升级装备不能进入交易所");
                }
                $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }

            if (isset($itemInfo->item_data->enchant) && $itemInfo->item_data->enchant->hasAttrs())
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊订单] 请求出售,附魔装备不能进入交易所");
                }
                $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }
        }

        if ($itemInfo->count <= 0)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊订单] 请求出售,数量不正确");
            }
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_COUNT);
            return false;
        }

        if (!Prohibition::check(ETradeType::ETRADETYPE_BOOTH, $itemInfo))
        {
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MGS_PROHIBIT_SELL);
            return false;
        }

        $itemId = $itemInfo->itemid;
        $item   = Item::get($itemId, $itemInfo->item_data);
        if (!$item)
        {
            # 不可以交易或物品不存在
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊订单] 请求出售,不可以交易或物品不存在");
            }
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_ITEMID);
            return false;
        }

        if ($item->equipType > 0 && !$item->itemData instanceof ItemData)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊订单] 请求出售,属于装备物品,但没有传item_data");
            }
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_ITEMID);
            return false;
        }

        if (!$item->canTrade())
        {
            # 不可交易的物品
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_ITEMID);
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊订单] 请求出售,不可交易的物品。 isTrade:{$item->isTrade}, tradeTime:{$item->tradeTime}, unTradeTime:{$item->unTradeTime}");
            }
            return false;
        }

        # 不可堆叠的物品一次只能卖一个
        if (!$item->isOverlap && $itemInfo->count != 1)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊订单] 请求出售,不可堆叠的物品一次只能卖一个");
            }
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_COUNT);
            return false;
        }

        $stateType = Item::getBoothPublicityState($item);
        # 玩家发送请求时公示状态是否一致
        if ($stateType === StateType::St_InPublicity || $stateType === StateType::St_WillPublicity)
        {
            if ($itemInfo->publicity_id <= 0)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊订单] 请求出售,玩家发送请求时公示状态与服务端不一致, 服务端当前正在公示");
                }
                $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }
        }
        else
        {
            if ($itemInfo->publicity_id > 0)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊订单] 请求出售,玩家发送请求时公示状态与服务端不一致, 服务端当前非公示");
                }
                $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }
        }

        # 一个已经过了公示期但还没有处理发货的，禁止上架
//        if ($stateType === StateType::St_InPublicity && $itemList->getEndTime() <= time())
//        {
//            if (IS_DEBUG)
//            {
//                Server::$instance->debug("[摆摊订单] 请求出售,一个已经过了公示期但还没有处理发货的，禁止上架");
//            }
//            $forwardUser->sendSysMsgToUser(ActSell::SYS_MGS_PUBLICITY_LOCK);
//            return false;
//        }

        if ($stateType === StateType::St_InPublicity)
        {
            if ($itemList = $item->getItemList())
            {
                $price = $itemList->getPrice();
            }
            else
            {
                $price = $item->getPrice();
            }
        }
        else
        {
            $price = $item->getPrice();
        }

        if ($itemInfo->price != $price)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊订单] 请求出售,价格不一致, 服务器价格:{$price}, 请求价格:{$itemInfo->price}");
            }
            $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PRICE);
            return false;
        }

        if ($itemInfo->up_rate > 0)
        {
            // 公示才允许上调
            if ($stateType !== StateType::St_InPublicity && $stateType !== StateType::St_WillPublicity)
            {
                $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊订单] 请求出售,公示才允许上调");
                }
                return false;
            }

            if (!in_array($itemInfo->up_rate, Server::$configBooth['uprate_list'] ?? []))
            {
                $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊订单] 请求出售,非法上调参数");
                }
                return false;
            }

        }
        else if ($itemInfo->down_rate > 0)
        {
            if (!in_array($itemInfo->down_rate, Server::$configBooth['downrate_list'] ?? []))
            {
                $forwardUser->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊订单] 请求出售,非法下调参数");
                }
                return false;
            }
        }

        return $item;
    }

    /**
     * 获取摆摊订单
     *
     * @param ZoneForwardUser $forwardUser
     * @param BoothPlayerPendingListCmd $cmd
     */
    public static function getPlayerOrders(ZoneForwardUser $forwardUser, BoothPlayerPendingListCmd $cmd)
    {
        if (IS_DEBUG)
        {
            Server::$instance->debug("[摆摊列表] 请求摆摊列表, cmd:" . json_encode($cmd));
        }

        if ($cmd->charid <= 0)
        {
            return;
        }

        $orders = BoothOrder::getOrdersBySellerId($cmd->charid);
        if ($orders === null)
        {
            $forwardUser->sendToUser($cmd);
        }
        else
        {
            $priceMap = [];
            /** @var ItemList[] $itemListMap */
            $itemListMap = [];
            foreach ($orders as $order)
            {
                if ($order->charId != $forwardUser->charId)
                {
                    if ($order->isExpired() || $order->status === BoothOrder::STATUS_EXPIRED)
                    {
                        continue;
                    }
                }

                if (!isset($itemListMap[$order->itemKey]))
                {
                    $itemList = $order->getItemList();
                    if ($itemList === null)
                    {
                        continue;
                    }
                    else
                    {
                        $itemListMap[$order->itemKey] = $itemList;
                    }
                }

                $itemList = $itemListMap[$order->itemKey];
                $itemInfo = new TradeItemBaseInfo();
                if ($order->isPublicity == 1)
                {
                    $itemInfo->publicity_id = $order->id;
                    $itemInfo->end_time     = $order->endTime;
                    $itemInfo->price        = $order->pubPrice;
                }
                else
                {
                    if (!isset($priceMap[$order->itemKey]))
                    {
                        $item                      = $itemList->getItem();
                        $priceMap[$order->itemKey] = $item ? $item->getPrice() : 0;
                    }
                    $itemInfo->price = $priceMap[$order->itemKey];
                }

                $itemInfo->charid     = $order->charId;
                $itemInfo->name       = $order->playerName;
                $itemInfo->count      = $order->getActualStock();
                $itemInfo->order_id   = $order->id;
                $itemInfo->is_expired = $order->status === BoothOrder::STATUS_EXPIRED;   // 卖家请求自己的挂单,则根据状态显示是否过期
                $itemInfo->item_data  = $itemList->getItemData();
                $itemInfo->itemid     = $order->itemId;
                $itemInfo->up_rate    = $order->upRate;
                $itemInfo->down_rate  = $order->downRate;
                $itemInfo->type       = ETradeType::ETRADETYPE_BOOTH;
                $itemInfo->overlap    = $itemList->isOverlap == 1 ? true : false;
                $itemInfo->refine_lv  = $itemList->refineLv;
                $cmd->addLists($itemInfo);
            }

            $forwardUser->sendToUser($cmd);
        }

        if (IS_DEBUG)
        {
            Server::$instance->debug("[摆摊列表] 请求摆摊列表结果, cmd:" . json_encode($cmd));
        }
    }

    /**
     * 取消订单
     *
     * @param BoothOrder $order
     * @param int $fd 玩家所在的session服fd
     * @return bool
     */
    public static function cancelOrder(BoothOrder $order, int $fd)
    {
        # 处于出售状态,但面临转为过期状态时,不能下架
//        if ($order->isExpired() && $order->status === BoothOrder::STATUS_SELLING)
//        {
//            return false;
//        }

        if ($order->getActualStock() <= 0)
        {
            return false;
        }

        $itemInfo           = new TradeItemBaseInfo();
        $itemInfo->order_id = $order->id;
        $itemInfo->itemid   = $order->itemId;
        $itemInfo->count    = $order->stock;
        if ($itemData = $order->getItemData())
        {
            $itemData->base->count = $order->stock;
            $itemInfo->item_data   = $itemData;
        }

        $originalStatus = $order->status;
        $rs             = $order->setStatus(BoothOrder::STATUS_CANCELED);
        if ($rs > 0)
        {
            // 返还额度
            $totalQuota = BoothOrder::calcTotalQuota($order->quota, $order->stock);
            Server::$instance->info("[摆摊下架] 执行成功, order_id:{$order->id}, char_id:{$order->charId}, time:{$order->time}, stock:{$order->stock}, item_id:{$order->itemId}, quota:{$order->quota}, total_quota:{$totalQuota}, item_info:" . json_encode($itemInfo));
            $logCmd               = new TradeLogCmd();
            $logCmd->pid          = $order->charId;
            $logCmd->time         = time();
            $logCmd->type         = ZoneForward::ETRADETYPE_BOOTH_CANCEL;
            $logCmd->itemid       = $order->itemId;
            $logCmd->count        = $order->count;
            $logCmd->price        = 0;
            $logCmd->tax          = $order->boothFee;
            $logCmd->moneycount   = 0;
            $logCmd->spend_quota  = 0;
//            $logCmd->quota_unlock = $totalQuota;
//            $logCmd->quota_lock   = 0;
            $logCmd->strotherid   = '';
            $logCmd->iteminfo     = json_encode($itemData);
            $logCmd->logid        = 'order-cancel-' . $order->id;
            ZoneForward::pushToFluent($logCmd);

            $cmd              = new AddItemRecordTradeCmd();
            $cmd->charid      = $order->charId;
            $cmd->addtype     = EAddItemType::EADDITEMTYP_RETURN;
            $cmd->item_info   = $itemInfo;
            $cmd->total_quota = $totalQuota;

            if (ZoneForward::sendToZoneByFd($cmd, $fd))
            {
                if ($item = Item::get($order->itemId))
                {
                    # 若不能交易,需要退上架费用
                    if (!$item->isTrade)
                    {
                        $retMoney       = $order->boothFee;
                        $retBoothFeeCmd = ActSell::getReturnBoothFeeCmd($order->charId, $retMoney);
                        if (ZoneForward::sendToZoneByFd($retBoothFeeCmd, $fd))
                        {
                            Server::$instance->info("[摆摊下架] 退回不能交易物品的上架费用成功, order_id:{$order->id}, stock:{$order->stock}, char_id:{$order->charId}, item_id:{$order->itemId}, item_info:" . json_encode($itemInfo));
                        }
                    }
                    else
                    {
                        $percent = Player::getReturnRate($order->charId);
                        if ($percent)
                        {
                            $boothFee = $order->boothFee;
                            $retMoney = intval($boothFee * $percent / 1000);
                            if ($retMoney)
                            {
                                $retBoothFeeCmd = ActSell::getReturnBoothFeeCmd($order->charId, $retMoney);
                                if (ZoneForward::sendToZoneByFd($retBoothFeeCmd, $fd))
                                {
                                    Server::$instance->info("[摆摊下架] 商人技能,上架费用返还. 上架费:{$boothFee}, 返还费:{$retMoney}, 商人上架费比率:{$percent}, order_id:{$order->id}, stock:{$order->stock}, char_id:{$order->charId}, item_id:{$order->itemId}");
                                }
                                else
                                {
                                    Server::$instance->warn("[摆摊下架] 商人技能,上架费用返还失败,因发送消息失败. 上架费:{$boothFee}, 返还费:{$retMoney}, 商人上架费比率:{$percent}, order_id:{$order->id}, stock:{$order->stock}, char_id:{$order->charId}, item_id:{$order->itemId}");
                                }
                            }
                        }
                    }
                }
                else
                {
                    # 如果不存在是获取不到物品的价格且没法计算要返还的上架费用
                    Server::$instance->warn("[摆摊下架] 下架的商品itemId: {$order->itemId}不存在! 请检查配置表是否删除了");
                }
            }
            else
            {
                Server::$instance->warn("[摆摊下架] 发送加物品失败, order_id:{$order->id}, stock:{$order->stock}, char_id:{$order->charId}, item_id:{$order->itemId}, item_info:" . json_encode($itemInfo));
            }

            if ($originalStatus == BoothOrder::STATUS_SELLING)
            {
                ItemList::decrItemBoothStock($order->itemId, $order->itemKey, $order->stock);
            }

            return true;
        }
        elseif ($rs == 0)
        {
            // 存在被另外的进程设为过期的订单, 但是task进程查询的时候是出售状态情况, 这时候需要玩家重新下架
            ZoneForward::sendToUserByFd(ActSell::SYS_MSG_CANCEL_WAS_LOCKED, $order->charId, $fd);
            Server::$instance->warn("[摆摊下架] 更新下架状态失败, 数据库中的状态与内存中的状态不一致! order_id:{$order->id}, original_status:{$originalStatus}, stock:{$order->stock}, char_id:{$order->charId}, item_id:{$order->itemId}");
            return false;
        }
        else
        {
            Server::$instance->warn("[摆摊下架] 下架失败! order_id:{$order->id}, stock:{$order->stock}, char_id:{$order->charId}, item_id:{$order->itemId}, item_info:" . json_encode($itemInfo));
        }

        return false;
    }

    const PUB_BUY_COUNT_KEY = "booth_pub_buy_count:%s";

    /**
     * 获取玩家抢购数
     *
     * @param $orderId
     * @param $charId
     * @return bool|int
     */
    public static function getPubBuyCount($orderId, $charId)
    {
        do
        {
            try
            {
                $rs = Server::$redis->hGet(sprintf(self::PUB_BUY_COUNT_KEY, $orderId), $charId);
                if ($rs === null)
                {
                    break;
                }

                return intval($rs);
            }
            catch (\Exception $e)
            {
                break;
            }
        }
        while (false);

        $count = BoothRecordBought::getPubBuyCount($orderId, $charId);
        if ($count !== false)
        {
            Server::$redis->hSet(sprintf(self::PUB_BUY_COUNT_KEY, $orderId), $charId, $count);
            return $count;
        }

        return false;
    }

    /**
     * 递增玩家抢购数
     *
     * @param $orderId
     * @param $charId
     * @param $count
     * @return bool|int
     */
    public static function incrPubBuyCount($orderId, $charId, $count)
    {
        try
        {
            return Server::$redis->hIncrBy(sprintf(self::PUB_BUY_COUNT_KEY, $orderId), $charId, $count);
        }
        catch (\Exception $e)
        {
            return false;
        }
    }

    /**
     * 删除公示订单抢购数
     *
     * @param $orderId
     * @return bool|int
     */
    public static function delPubOrderBuyCount($orderId)
    {
        try
        {
            return Server::$redis->del(sprintf(self::PUB_BUY_COUNT_KEY, $orderId));
        }
        catch (\Exception $e)
        {
            return false;
        }
    }

    /**
     * 递减玩家抢购数
     *
     * @param $orderId
     * @param $charId
     * @param $count
     * @return bool|int
     */
    public static function decrPubBuyCount($orderId, $charId, $count)
    {
        if ($count > 0)
        {
            $count = -$count;
        }

        try
        {
            return Server::$redis->hIncrBy(sprintf(self::PUB_BUY_COUNT_KEY, $orderId), $charId, $count);
        }
        catch (\Exception $e)
        {
            return false;
        }
    }

    /**
     * 处理支付超时的购买记录
     */
    public static function handlePayTimeoutRecord()
    {
        $mysql = Server::createMySQL(true);
        if (!$mysql)
        {
            return;
        }

        static $lastRunTime = 0;

        $exp       = time() - 60 * 5; # 超时时间
        $sql       = "SELECT `id`, `order_id`, `char_id`, `price`, `total_count`, `is_publicity` FROM `" . BoothRecordBought::getTableName() . "` WHERE `status` = '" . BoothRecordBought::STATUS_PAY_PENDING . "' AND `time` < '{$exp}'" . ($lastRunTime > 0 ? " AND `time` >= '{$lastRunTime}'" : '') . ' LIMIT 100';
        $orderIds  = [];
        $recordIds = [];
        $records   = [];
        if ($rs = $mysql->query($sql))
        {
            if ($rs->num_rows > 0)
            {
                while ($row = $rs->fetch_object())
                {
                    if ($row->is_publicity == 0)
                    {
                        if (!isset($orderIds[$row->order_id]))
                        {
                            $orderIds[$row->order_id] = $row->total_count;
                        }
                        else
                        {
                            $orderIds[$row->order_id] += $row->total_count;
                        }
                    }

                    $recordIds[] = $row->id;
                    $records[]   = $row;
                }
            }
        }

        if (empty($recordIds) && empty($orderIds))
        {
            return;
        }

        $begin = $mysql->begin_transaction();
        if (!$begin)
        {
            return;
        }

        try
        {
            if (!empty($recordIds))
            {
                $idsStr = implode(',', $recordIds);
                $sql2   = "UPDATE `" . BoothRecordBought::getTableName() . "` SET `status` = " . BoothRecordBought::STATUS_PAY_TIMEOUT . " WHERE `id` in ({$idsStr}) AND `status` = " . BoothRecordBought::STATUS_PAY_PENDING;
                Server::$instance->info("[摆摊支付超时] 已超时的记录, record_ids:{$idsStr}");
                if (false === $mysql->query($sql2))
                {
                    throw new \Exception("更新为支付过期状态失败! record_ids:{$idsStr}");
                }
            }

            if (!empty($orderIds))
            {
                Server::$instance->info("[摆摊支付超时] 需要释放的库存 order_ids:", $orderIds);
                // 释放预占库存
                foreach ($orderIds as $orderId => $stock)
                {
                    $sql = "UPDATE `" . BoothOrder::getTableName() . "` SET `lock_stock` = `lock_stock` - {$stock} WHERE `lock_stock` - {$stock} >= 0 AND `id` = {$orderId}";
                    $rs  = $mysql->query($sql);
                    if ($rs === false || $mysql->affected_rows <= 0)
                    {
                        throw new \Exception("释放预占库存失败! order_id:{$orderId}, release_stock:{$stock}");
                    }
                }
            }

            if ($mysql->commit() === false)
            {
                Server::$instance->warn("[摆摊支付超时] 提交事务失败!");
                return;
            }

            foreach ($records as $record)
            {
                Server::$instance->info("[摆摊支付超时] 购买记录支付超时. char_id:{$record->char_id}, record_id:{$record->id}, order_id:{$record->order_id}, price:{$record->price}, total_count:{$record->total_count}, is_publicity:{$record->is_publicity}");
            }
        }
        catch (\Exception $e)
        {
            $mysql->rollback();
            Server::$instance->warn("[摆摊支付超时] {$e->getMessage()}");
        }
    }

    /**
     * 摆摊订单处理为过期
     */
    public static function handleExpiredBoothOrder()
    {
        $mysql = Server::createMySQL(true);
        if (!$mysql)
        {
            return;
        }

        static $lastRunTime = 0;
        $exp          = time() - Server::$configBooth['exchange_hour'] ?? 86400;       # 过期时间
        $sql          = "SELECT `id`, `item_id`, `item_key`, `stock`, `time` FROM `" . BoothOrder::getTableName() . "` WHERE `stock` > 0 AND `lock_stock` <= 0 AND `is_publicity` = '0' AND `status` = '" . BoothOrder::STATUS_SELLING . "' AND `time` < '{$exp}'" . ($lastRunTime > 0 ? " AND `time` >= '{$lastRunTime}'" : '') . ' LIMIT 100';
        $itemStockMap = [];
        $orderIds     = [];
        if ($rs = $mysql->query($sql))
        {
            if ($rs->num_rows > 0)
            {
                while ($row = $rs->fetch_object())
                {
                    $lastRunTime = intval($row->time);
                    $orderIds[]  = intval($row->id);
                    $row->stock  = intval($row->stock);

                    if ($row->stock > 0)
                    {
                        if (isset($itemStockMap[$row->item_key]))
                        {
                            $itemStockMap[$row->item_key]['stock'] += $row->stock;
                        }
                        else
                        {
                            $itemStockMap[$row->item_key] = [
                                'stock' => $row->stock,
                                'id'    => $row->item_id,
                            ];
                        }
                    }
                }
            }
            else
            {
                $lastRunTime = $exp;
            }
            $rs->free();
        }

        if (!empty($orderIds))
        {
            $idsStr = implode(',', $orderIds);
            $sql2   = "UPDATE `booth_order` SET `status` = " . BoothOrder::STATUS_EXPIRED . " WHERE `id` in ({$idsStr}) AND `status` = " . BoothOrder::STATUS_SELLING;
            if (false === $mysql->query($sql2))
            {
                return;
            }

            // 更新成功的数量不一致,重新查询更新成功为过期的订单,并计算最终需要扣减的库存数
            if (count($orderIds) !== $mysql->affected_rows)
            {
                $sql3          = "SELECT `id`, `item_id`, `item_key`, `stock`, `time` FROM `booth_order` WHERE `id` ({$idsStr}) AND `status` = " . BoothOrder::STATUS_EXPIRED;
                $rs2           = $mysql->query($sql3);
                $itemStockMap2 = [];
                if ($rs2 && $rs2->num_rows > 0)
                {
                    while ($row = $rs2->fetch_object())
                    {
                        if ($row->stock > 0)
                        {
                            if (isset($itemStockMap2[$row->item_key]))
                            {
                                $itemStockMap2[$row->item_key]['stock'] += $row->stock;
                            }
                            else
                            {
                                $itemStockMap2[$row->item_key] = [
                                    'stock' => $row->stock,
                                    'id'    => $row->item_id,
                                ];
                            }
                        }
                    }

                    foreach ($itemStockMap2 as $itemKey => $v)
                    {
                        $itemStockMap[$itemKey] = $v;
                    }
                }
            }

            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊过期] 更新过期订单数:{$mysql->affected_rows}");
            }

            if (!empty($itemStockMap))
            {
                foreach ($itemStockMap as $itemKey => $v)
                {
                    ItemList::decrItemBoothStock($v['id'], $itemKey, $v['stock']);
                }
            }
        }
    }

    /**
     * 处理公示订单
     * 成功返回sql文件名
     *
     * @param int $orderId
     * @return bool|string
     */
    public static function handlePubOrder(int $orderId)
    {
        Server::$instance->info("[摆摊公示分单] 开始处理公示, order_id:{$orderId}");
        $curTime = time();
        $order   = BoothOrder::getById($orderId);
        if ($order->isPublicity != 1)
        {
            Server::$instance->warn("[摆摊公示分单] 分单失败, 不是公示订单. order_id:{$orderId}, item_id:{$order->itemId}, item_key:{$order->itemKey}");
            return false;
        }

        if ($order->status != BoothOrder::STATUS_SELLING)
        {
            Server::$instance->warn("[摆摊公示分单] 分单失败, 不处在出售状态. order_id:{$orderId}, item_id:{$order->itemId}, item_key:{$order->itemKey}");
            return false;
        }

        if ($order->endTime > $curTime)
        {
            Server::$instance->warn("[摆摊公示分单] 分单失败, 还没到分单时间. order_id:{$orderId}, item_id:{$order->itemId}, item_key:{$order->itemKey}");
            return false;
        }

        $sql = "SELECT `id`, `char_id`, `player_name`, `player_zone_id`, `total_count`, `count`, `status`, `spend_quota`, `price`, `refine_lv`, `is_damage`, `time` FROM `" . BoothRecordBought::getTableName() . "` WHERE `order_id` = '{$orderId}' AND `is_publicity` = 1 AND `status` = " . BoothRecordBought::STATUS_PUBLICITY_PAY_SUCCESS . " AND `take_status` = " . BoothRecordTakeStatus::TAKE_STATUS_CAN_TAKE_GIVE;
        $rs  = Server::$mysqlMaster->query($sql);
        if (!$rs)
        {
            Server::$instance->warn("[摆摊公示分单] 分单失败, 查询购买记录失败. order_id:{$orderId}, item_id:{$order['item_id']}, item_key:{$order['item_key']}");
            return false;
        }

        // 无人购买转为普通订单
        if ($rs->num_rows <= 0)
        {
            $order->isPublicity  = 0;
            $order->endTime      = 0;
            $order->pubBuyPeople = 0;
            if ($order->update() > 0)
            {
                ItemList::incrItemBoothStock($order->itemId, $order->itemKey, $order->stock);
                Server::$instance->info("[摆摊公示分单] 无人购买, 转为普通订单成功. order_id:{$order->id}");
                return true;
            }

            Server::$instance->warn("[摆摊公示分单] 无人购买, 转为普通订单失败. order_id:{$order->id}");
            return false;
        }

        $buyers        = [];
        $buyTotalCount = 0;
        while ($row = $rs->fetch_object())
        {
            $row->id             = intval($row->id);
            $row->char_id        = intval($row->char_id);
            $row->total_count    = intval($row->total_count);
            $row->count          = intval($row->count);
            $row->player_zone_id = intval($row->player_zone_id);
            $row->status         = intval($row->status);
            $row->price          = intval($row->price);
            $row->spend_quota    = intval($row->spend_quota);
            $row->refine_lv      = intval($row->refine_lv);
            $row->is_damage      = intval($row->is_damage);
            $buyers[]            = $row;
            $buyTotalCount += $row->total_count;
        }

        // 总奖品数
        $prizeCount = $order->stock;
        // 每次获奖件数
        $awardCount = 1;

        while ($prizeCount && $buyTotalCount)
        {
            mt_srand();
            $map   = [];
            $prize = null;
            $index = 1;
            foreach ($buyers as $buy)
            {
                if ($buy->total_count === $buy->count)
                {
                    continue;
                }

                if (isset($map[$buy->char_id]))
                {
                    continue;
                }
                else
                {
                    $map[$buy->char_id] = 1;
                }

                if ($index < 1)
                {
                    $prize = $buy;
                }
                else
                {
                    $r = mt_rand() % (++$index);
                    if ($r < 1)
                    {
                        $prize = $buy;
                    }
                }
            }

            $c = min($awardCount, $prizeCount);
            if ($prize)
            {
                $c = min($c, $prize->total_count);
                $prize->count += $c;
                $prizeCount -= $c;
                $buyTotalCount -= $c;
            }
        }

        try
        {
            $tmpFile = Server::$dataDir . 'booth_pub_' . $order->id . '_' . $order->endTime . '.tmp';
            # 清空临时文件
            if (false === file_put_contents($tmpFile, ''))
            {
                Server::$instance->warn('[摆摊公示分单] 清空临时文件失败');
                return false;
            }

            $order->stock   = $prizeCount;
            $batchNum       = 0;
            $successRecords = [];
            $refundRecords  = [];
            $item           = $order->getItem();
            foreach ($buyers as $buyer)
            {
                // 一件都没抢到
                if ($buyer->count === 0)
                {
                    // 后面一次性更新没有一件都没抢购成功的记录
                    continue;
                }

                $totalPrice       = $buyer->count * $buyer->price;
                $buyer->quota     = null;
                $buyer->can_give  = BoothRecordBought::canGive($buyer->char_id, $totalPrice, $buyer->quota);
                $buyer->status    = BoothRecordBought::STATUS_PUBLICITY_SUCCESS;
                $successRecords[] = $buyer;

                // 抢到部分
                $refundCount = $buyer->total_count - $buyer->count;
                if ($refundCount > 0)
                {
                    $obj                 = new \stdClass();
                    $obj->id             = $buyer->id;
                    $obj->char_id        = $buyer->char_id;
                    $obj->player_zone_id = $buyer->player_zone_id;
                    $obj->player_name    = $buyer->player_name;
                    $obj->count          = $buyer->count;
                    $obj->total_count    = $buyer->total_count;
                    $obj->price          = $buyer->price;
                    $obj->refine_lv      = $buyer->refine_lv;
                    $obj->is_damage      = $buyer->is_damage;
                    $obj->status         = BoothRecordBought::STATUS_PUBLICITY_CANCEL;
                    $obj->spend_quota    = $buyer->spend_quota;
                    $obj->time           = $curTime;

                    $refundRecords[] = $obj;
                }

                if (++$batchNum % 1000 === 0)
                {
                    self::writeBoothBuyUpdateSql($successRecords, $tmpFile);
                    self::writeBoothBuyInsertSql($order, $refundRecords, $tmpFile);
                    $successRecords = [];
                    $refundRecords  = [];
                }
            }

            // 处理剩余数据
            self::writeBoothBuyUpdateSql($successRecords, $tmpFile);
            self::writeBoothBuyInsertSql($order, $refundRecords, $tmpFile);

            // 对没有抢购到的买家记录进行退款
            $sql = 'UPDATE `booth_record_bought` SET `status` = ' . BoothRecordBought::STATUS_PUBLICITY_CANCEL . " WHERE `count` = 0 AND `order_id` = '{$order->id}' AND `is_publicity` = 1 AND `status` = " . BoothRecordBought::STATUS_PUBLICITY_PAY_SUCCESS . ";\n";
            if (file_put_contents($tmpFile, $sql, FILE_APPEND) === false)
            {
                throw new \Exception('对没有抢购到的买家记录进行退款, Sql语句插入临时文件失败');
            }

            $itemData = $order->getItemData();
            $refineLv = 0;
            $isDamage = 0;
            if ($itemData)
            {
                $refineLv = $itemData->equip->refinelv;
                $isDamage = $itemData->equip->damage ? 1 : 0;
            }
            $soldRecord               = new BoothRecordSold();
            $soldRecord->charId       = $order->charId;
            $soldRecord->playerName   = $order->playerName;
            $soldRecord->playerZoneId = $order->playerZoneId;
            $soldRecord->orderId      = $order->id;
            $soldRecord->itemId       = $order->itemId;
            $soldRecord->itemKey      = $order->itemKey;
            $soldRecord->itemData     = $order->getItemData();
            $soldRecord->status       = BoothRecordSold::STATUS_ADDING;
            $soldRecord->takeStatus   = BoothRecordTakeStatus::TAKE_STATUS_CAN_TAKE_GIVE;
            $soldRecord->time         = $curTime;
            $soldRecord->count        = $order->count - $order->stock;
            $soldRecord->price        = $order->getRealPrice();
            $soldRecord->quota        = $order->quota;
            $soldRecord->refineLv     = $refineLv;
            $soldRecord->isDamage     = $isDamage;
            $soldRecord->tax          = $soldRecord->getTax();
            $soldRecord->buyersInfo   = new NameInfoList();
            $soldRecord->isPublicity  = 1;
            $totalMoney               = $soldRecord->count * $soldRecord->price;

            /** @var NameInfo[] $nameInfoMap */
            $nameInfoMap = [];
            $buyerIds    = '';
            foreach ($buyers as $buyer)
            {
                if ($buyer->count <= 0)
                {
                    continue;
                }

                if (isset($nameInfoMap[$buyer->char_id]))
                {
                    $nameInfoMap[$buyer->char_id]->count += $buyer->count;
                }
                else
                {
                    $nameInfo         = new NameInfo();
                    $nameInfo->name   = $buyer->player_name;
                    $nameInfo->zoneid = $buyer->player_zone_id;
                    $nameInfo->count  = $buyer->count;
                    $soldRecord->buyersInfo->addNameInfos($nameInfo);
                    $nameInfoMap[$buyer->char_id] = $nameInfo;
                    $buyerIds .= $buyer->char_id . ';';
                }
            }
        }
        catch (\Exception $e)
        {
            Server::$instance->warn('[摆摊公示分单] ' . $e->getMessage());
            return false;
        }

        $db = Server::$mysqlMaster;
        if (!$db->begin_transaction())
        {
            Server::$instance->warn('[摆摊公示分单] 申请事务失败, Error:' . Server::$mysqlMaster->error);
            return false;
        }

        try
        {
            if ($soldRecord->insert() === false)
            {
                throw new \Exception('卖家记录插入失败!');
            }

            $sellCount = $order->count - $order->stock;

            $autoPutOnSell = false;
            if ($order->stock > 0)
            {
                // 还有剩余库存, 转为普通订单
                $order->isPublicity  = 0;
                $order->endTime      = 0;
                $order->pubBuyPeople = 0;
                $order->pubPrice     = 0;
                $autoPutOnSell       = true;
            }
            else
            {
                $order->status = BoothOrder::STATUS_SOLD;
            }

            if ($order->update() === false)
            {
                throw new \Exception('更新订单失败!');
            }

            $newName = 'booth_pub_' . $order->id . '_' . $order->endTime . '.sql';
            $rs      = rename($tmpFile, Server::$dataDir . $newName);
            if (!$rs)
            {
                throw new \Exception("修改临时文件名失败");
            }

            if ($db->commit() === false)
            {
                unlink(Server::$dataDir . $newName);
                Server::$instance->warn("[摆摊公示分单] 提交事务失败. order_id:{$order->id}, char_id:{$order->charId}, item_id:{$order->itemId}, end_time:{$order->endTime}");
                return false;
            }

            // 有剩余库存加回公共库存
            if ($order->stock > 0)
            {
                ItemList::incrItemBoothStock($order->itemId, $order->itemKey, $order->stock);
            }

            if (self::delPubOrderBuyCount($order->id) === false)
            {
                Server::$instance->warn("[摆摊公示分单] 删除抢购数失败. order_id:{$order->id}");
            }

            // 写入日志
            foreach ($buyers as $buyer)
            {
                $refundCount = $buyer->total_count - $buyer->count;
                $refundPrice = $buyer->price * $refundCount;
                $zoneId = Player::getZoneId($buyer->char_id, true);
                $zone   = Server::$zone->get($zoneId);
                if ($buyer->count === 0)
                {
                    Server::$instance->info("[摆摊公示分单] 抢购失败, 生成退款记录. char_id:{$buyer->char_id}, record_id:{$buyer->id} order_id:{$order->id}, item_id:{$order->itemId}, price:{$buyer->price}, count:{$buyer->count}, total_count:{$buyer->total_count}, refund_price:{$refundPrice}");
                    $logCmd              = new TradeLogCmd();
                    $logCmd->pid         = $buyer->char_id;
                    $logCmd->time        = time();
                    $logCmd->type        = ZoneForward::ETRADETYPE_BOOTH_PUBLICITY_RETURN;
                    $logCmd->itemid      = $order->itemId;
                    $logCmd->count       = $refundCount;
                    $logCmd->price       = $buyer->price;
                    $logCmd->tax         = 0;
                    $logCmd->moneycount  = $refundPrice;
                    $logCmd->spend_quota = $buyer->spend_quota;
                    $logCmd->strotherid  = $soldRecord->charId;
                    $logCmd->iteminfo    = json_encode($itemData);
                    $logCmd->logid       = 'pub-refund-' . $buyer->id;
                    ZoneForward::pushToFluent($logCmd);

                    if ($zone)
                    {
                        ZoneForward::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $buyer->char_id, ActBuy::SYS_MSG_BOOTH_PUBLICITY_FAIL, [
                            $item !== null ? $item->name : '',
                            $refundPrice,
                            $refundCount * $buyer->spend_quota
                        ]);
                    }
                }
                else if ($refundCount > 0)
                {
                    Server::$instance->info("[摆摊公示分单] 抢购部分成功, 生成退款记录. success_record_id:{$buyer->id}, char_id:{$buyer->char_id}, order_id:{$order->id}, item_id:{$order->itemId}, refund_price:{$refundPrice}, price:{$buyer->price}, refund_count:{$refundCount}, success_count:{$buyer->count}, total_count:{$buyer->total_count}, can_give:{$buyer->can_give}, buyer_quota:{$buyer->quota}");
                    $logCmd              = new TradeLogCmd();
                    $logCmd->pid         = $buyer->char_id;
                    $logCmd->time        = time();
                    $logCmd->type        = ZoneForward::ETRADETYPE_BOOTH_PUBLICITY_BUY;
                    $logCmd->itemid      = $order->itemId;
                    $logCmd->count       = $buyer->count;
                    $logCmd->price       = $buyer->price;
                    $logCmd->tax         = 0;
                    $logCmd->moneycount  = $logCmd->count * $logCmd->price;
                    $logCmd->spend_quota = $buyer->spend_quota;
                    $logCmd->strotherid  = $soldRecord->charId;
                    $logCmd->iteminfo    = json_encode($itemData);
                    $logCmd->logid       = 'pub-buy-' . $buyer->id;
                    ZoneForward::pushToFluent($logCmd);

                    $logCmd              = new TradeLogCmd();
                    $logCmd->pid         = $buyer->char_id;
                    $logCmd->time        = time();
                    $logCmd->type        = ZoneForward::ETRADETYPE_BOOTH_PUBLICITY_RETURN;
                    $logCmd->itemid      = $order->itemId;
                    $logCmd->count       = $refundCount;
                    $logCmd->price       = $buyer->price;
                    $logCmd->tax         = 0;
                    $logCmd->moneycount  = $refundPrice;
                    $logCmd->spend_quota = $buyer->spend_quota;
                    $logCmd->strotherid  = $soldRecord->charId;
                    $logCmd->iteminfo    = json_encode($itemData);
                    $logCmd->logid       = 'pub-refund-' . $buyer->id;
                    ZoneForward::pushToFluent($logCmd);

                    if ($zone)
                    {
                        $param = [
                            $buyer->count,
                            $item !== null ? $item->name : '',
                            $logCmd->moneycount,
                            $buyer->count * $buyer->spend_quota
                        ];

                        ZoneForward::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $buyer->char_id, ActBuy::SYS_MSG_BOOTH_PUBLICITY_BUY_SUCCESS, $param);
                    }
                }
                else
                {
                    Server::$instance->info("[摆摊公示分单] 抢购成功. char_id:{$buyer->char_id}, record_id:{$buyer->id}, order_id:{$order->id}, item_id:{$order->itemId}, price:{$buyer->price}, count:{$buyer->count}, total_count:{$buyer->total_count}, can_give:{$buyer->can_give}, buyer_quota:{$buyer->quota}");
                    $logCmd              = new TradeLogCmd();
                    $logCmd->pid         = $buyer->char_id;
                    $logCmd->time        = time();
                    $logCmd->type        = ZoneForward::ETRADETYPE_BOOTH_PUBLICITY_BUY;
                    $logCmd->itemid      = $order->itemId;
                    $logCmd->count       = $buyer->count;
                    $logCmd->price       = $buyer->price;
                    $logCmd->tax         = 0;
                    $logCmd->moneycount  = $logCmd->count * $logCmd->price;
                    $logCmd->spend_quota = $buyer->spend_quota;
                    $logCmd->strotherid  = $soldRecord->charId;
                    $logCmd->iteminfo    = json_encode($itemData);
                    $logCmd->logid       = 'pub-buy-' . $buyer->id;
                    ZoneForward::pushToFluent($logCmd);

                    if ($zone)
                    {
                        $param = [
                            $buyer->count,
                            $item !== null ? $item->name : '',
                            $logCmd->moneycount,
                            $buyer->count * $buyer->spend_quota
                        ];

                        ZoneForward::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $buyer->char_id, ActBuy::SYS_MSG_BOOTH_PUBLICITY_BUY_SUCCESS, $param);
                    }
                }
            }

            Server::$instance->info("[摆摊公示分单] 分单完毕! 总共出售数量:{$order->count}, 成功出售数量:{$sellCount}, 剩余{$order->stock}. order_id:{$orderId}, char_id:{$order->charId} item_id:{$order->itemId}, item_key:{$order->itemKey}");
            $logCmd               = new TradeLogCmd();
            $logCmd->pid          = $soldRecord->charId;
            $logCmd->time         = time();
            $logCmd->type         = ZoneForward::ETRADETYPE_BOOTH_PUBLICITY_TRUE_SELL;
            $logCmd->itemid       = $soldRecord->itemId;
            $logCmd->count        = $sellCount;
            $logCmd->price        = $soldRecord->price;
            $logCmd->tax          = $soldRecord->tax;
            $logCmd->moneycount   = $logCmd->price * $logCmd->count;
            $logCmd->spend_quota  = 0;
//            $logCmd->quota_lock   = 0;
//            $logCmd->quota_unlock = $sellCount * $soldRecord->quota;
            $logCmd->strotherid   = $buyerIds;
            $logCmd->iteminfo     = json_encode($itemData);
            $logCmd->logid        = 'pub-sell-' . $soldRecord->id;
            ZoneForward::pushToFluent($logCmd);

            if ($autoPutOnSell)
            {
                $logCmd              = new TradeLogCmd();
                $logCmd->pid         = $order->charId;
                $logCmd->time        = time();
                $logCmd->type        = ZoneForward::ETRADETYPE_BOOTH_RESELL_AUTO;
                $logCmd->itemid      = $order->itemId;
                $logCmd->count       = $order->stock;
                $logCmd->price       = 0;
                $logCmd->tax         = 0;
                $logCmd->moneycount  = 0;
                $logCmd->spend_quota = 0;
                $logCmd->strotherid  = '';
                $logCmd->iteminfo    = json_encode($itemData);
                $logCmd->logid       = 'pub-resell-' . $order->id;
                ZoneForward::pushToFluent($logCmd);
            }

            $zoneId     = Player::getZoneId($soldRecord->charId, true);
            $sellerZone = Server::$zone->get($zoneId);

            if ($sellerZone)
            {
                // 通知更新卖家列表
                $updateMsg            = new UpdateOrderTradeCmd();
                $updateMsg->charid    = $order->charId;
                $updateMsg->info      = $itemInfo = new TradeItemBaseInfo();
                $updateMsg->type      = ETradeType::ETRADETYPE_BOOTH;
                $itemInfo->charid     = $order->charId;
                $itemInfo->name       = $order->playerName;
                $itemInfo->count      = $order->getActualStock();
                $itemInfo->order_id   = $order->id;
                $itemInfo->price      = $order->pubPrice;
                $itemInfo->is_expired = $order->status === BoothOrder::STATUS_EXPIRED;
                $itemInfo->item_data  = $itemData;
                $itemInfo->itemid     = $order->itemId;
                $itemInfo->up_rate    = $order->upRate;
                $itemInfo->down_rate  = $order->downRate;
                $itemInfo->type       = ETradeType::ETRADETYPE_BOOTH;
                $itemInfo->overlap    = Server::$item->get($order->itemId, 'isOverlap') === 1 ? true : false;
                $itemInfo->refine_lv  = $refineLv;
                ZoneForward::sendToUserByFd($updateMsg, $order->charId, $sellerZone['fd']);

                $params = [
                    $soldRecord->count,
                    $item !== null ? $item->name : '',
                    $soldRecord->tax,
                    $totalMoney - $soldRecord->tax,
                ];
                ZoneForward::sendSysMsgToUserByCharId($sellerZone['fd'], $sellerZone['fromId'], $soldRecord->charId, ActSell::SYS_MGS_SELL_SUCCESS, $params);
            }
            return $newName;
        }
        catch (\Exception $e)
        {
            $db->rollback();
            unlink($tmpFile);
            Server::$instance->warn('[摆摊公示分单] ' . $e->getMessage() . " order_id:{$order->id}, char_id:{$order->charId}, item_id:{$order->itemId}, end_time:{$order->endTime}");
            return false;
        }
    }

    /**
     * 写入摆摊购买记录更新语句
     *
     * @param $records
     * @param $file
     * @throws \Exception
     */
    protected static function writeBoothBuyUpdateSql($records, $file)
    {
        if (!empty($records))
        {
            $updateSql = '';
            foreach ($records as $record)
            {
                $updateSql .= "UPDATE `booth_record_bought` SET " . "`status` = {$record->status}, `count` = '{$record->count}', `can_give` = '{$record->can_give}' " . "WHERE `id` = '{$record->id}';\n";
            }

            if (file_put_contents($file, $updateSql, FILE_APPEND) === false)
            {
                throw new \Exception('写入摆摊购买记录更新语句, Sql语句插入临时文件失败');
            }
        }
    }

    /**
     * 写入摆摊退款记录
     *
     * @param BoothOrder $order
     * @param $records
     * @param $file
     * @throws \Exception
     */
    protected static function writeBoothBuyInsertSql(BoothOrder $order, $records, $file)
    {
        if (!empty($records))
        {
            $values          = '';
            $itemData        = $order->getItemData();
            $itemDataHex     = $itemData === null ? 'null' : '0x' . bin2hex($itemData->serialize());
            $orderPlayerName = Server::$mysqlMaster->real_escape_string($order->playerName);
            foreach ($records as $record)
            {
                $record->player_name = Server::$mysqlMaster->real_escape_string($record->player_name);
                $values
                    .= "('{$record->char_id}', '{$record->player_name}', '{$record->player_zone_id}', '{$order->id}',"
                    . " '{$order->charId}', '{$orderPlayerName}', '{$order->playerZoneId}', '{$record->status}',"
                    . " '{$order->itemId}', '{$order->itemKey}', '{$record->time}'," . " '1', '{$order->endTime}', '{$record->count}', '{$record->total_count}',"
                    . " '{$record->price}', '{$record->spend_quota}', '{$record->refine_lv}', '{$record->is_damage}',"
                    . " '" . $itemDataHex . "'), ";
            }

            $sql = 'INSERT INTO `booth_record_bought` (`char_id`,`player_name`,`player_zone_id`,`order_id`,`seller_id`,`seller_name`,`seller_zone_id`,`status`,`item_id`,`item_key`,`time`,`is_publicity`,`end_time`,`count`,`total_count`,`price`,`spend_quota`,`refine_lv`,`is_damage`,`item_data`) VALUES ';
            $sql .= rtrim($values, ', ') . ';' . PHP_EOL;
            if (file_put_contents($file, $sql, FILE_APPEND) === false)
            {
                throw new \Exception('写入摆摊退款记录, Sql语句插入临时文件失败');
            }
        }
    }

    /**
     * 获取订单信息
     *
     * @param ZoneForwardUser $forward
     * @param ItemSellInfoRecordTradeCmd $cmd
     */
    public static function getItemSellInfo(ZoneForwardUser $forward, ItemSellInfoRecordTradeCmd $cmd)
    {
        $order = BoothOrder::getById($cmd->order_id);
        if (!$order)
        {
            return;
        }

        $cmd->count = $order->stock;
        $cmd->quota = $order->quota;

        if ($order->isPublicity)
        {
            $cmd->statetype    = StateType::St_InPublicity;
            $cmd->buyer_count  = $order->pubBuyPeople;
            $cmd->publicity_id = $order->id;
        }
        else
        {
            if ($order->getIsOverlap())
            {
                $cmd->statetype = StateType::St_OverlapNormal;
            }
            else
            {
                $cmd->statetype = StateType::St_NonoverlapNormal;
            }
        }

        $forward->sendToUser($cmd);
        if (IS_DEBUG)
        {
            Server::$instance->debug("[摆摊订单交易信息] cmd:" . json_encode($cmd));
        }
    }

    /**
     * 摆摊强制下架
     * 根据参数下架物品
     * 公示的订单将会终止
     * 所有订单会设为过期状态
     *
     * @param Prohibition $prohibition
     */
    public static function cancelOffByCondition(Prohibition $prohibition)
    {
        if ($prohibition->tradeType != ETradeType::ETRADETYPE_BOOTH && $prohibition->tradeType != ETradeType::ETRADETYPE_ALL)
        {
            return;
        }

        $itemId    = $prohibition->itemId;
        $refineLv  = $prohibition->refineLv;
        $enchantId = $prohibition->enchantId;
        $buffId    = $prohibition->enchantBuffId;

        $item = null;
        if ($itemId !== null)
        {
            $item = Item::get($itemId);
            if (!$item)
            {
                Server::$instance->info("[摆摊-条件下架] itemId:{$itemId} 物品不存在!");
                return;
            }
        }

        $itemListIds = [];

        # 公示上锁,避免公示处理进程抢占
        Server::$publicityLock->lock();
        Server::$instance->info("[摆摊-条件下架] 正在执行下架操作. uniqueId:{$prohibition->uniqueId}, itemId:{$itemId}, refineLv:{$refineLv}, enchantId:{$enchantId}, buffId:{$buffId}");

        if ($itemId === null || $item->equipType > 0)
        {
            foreach (Server::$itemList as $key => $data)
            {
                $dataItemId   = $data['item_id'];
                $dataRefineLv = $data['refine_lv'];

                if ($itemId != null)
                {
                    if ($itemId != $dataItemId)
                        continue;

                    if ($refineLv == 0 && $enchantId == 0 && $buffId == 0)
                    {
                        $itemListIds[$key] = $dataItemId;
                        continue;
                    }

                    if ($refineLv > 0 && $refineLv == $dataRefineLv)
                    {
                        $itemListIds[$key] = $dataItemId;
                        continue;
                    }
                }

                # 不管itemId,只要符合附魔条件,都处理下架
                if ($data['item_data'])
                {
                    $itemData = new ItemData($data['item_data']);

                    if ($enchantId)
                    {
                        if (isset($itemData->enchant->attrs))
                        {
                            foreach ($itemData->enchant->attrs as $attr)
                            {
                                if ($enchantId == $attr->type)
                                {
                                    $itemListIds[$key] = $dataItemId;
                                    continue;
                                }
                            }
                        }
                    }

                    if ($buffId)
                    {
                        if (isset($itemData->enchant->extras))
                        {
                            foreach ($itemData->enchant->extras as $extra)
                            {
                                if ($buffId == $extra->buffid)
                                {
                                    $itemListIds[$key] = $dataItemId;
                                    continue;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            $itemList = Server::$itemList->get($itemId);
            if ($itemList)
                $itemListIds[$itemId] = $itemId;
        }

        if (!empty($itemListIds))
        {
            $mysql = Server::$mysqlMaster;
            foreach ($itemListIds as $key => $itemId)
            {
                // 公示订单处理
                $sql = "SELECT `id` FROM `" . BoothOrder::getTableName() . "` WHERE `is_publicity` = 1 AND `item_key` = '{$key}' AND `stock` > 0 AND `status` = '" . BoothOrder::STATUS_SELLING . "'";
                $rs = $mysql->query($sql);
                if ($rs && $rs->num_rows > 0)
                {
                    while($row = $rs->fetch_assoc())
                    {
                        $orderId = $row['id'];
                        $sql = 'UPDATE `'. BoothRecordBought::getTableName() .'` SET `status` = ' . BoothRecordBought::STATUS_PUBLICITY_CANCEL . " WHERE `count` = 0 AND `order_id` = '{$orderId}' AND `is_publicity` = 1 AND `status` = " . BoothRecordBought::STATUS_PUBLICITY_PAY_SUCCESS;
                        if (false === $mysql->query($sql))
                        {
                            Server::$instance->warn("[摆摊-条件下架] 更新成抢购退款记录失败. order_id:{$orderId}");
                        }

                        $sql = "UPDATE `" . BoothOrder::getTableName() . "` SET `is_publicity` = 0, `end_time` = 0, `pub_buy_people` = 0, `pub_price` = 0, `status` = " . BoothOrder::STATUS_EXPIRED . " WHERE `id` = {$orderId}";
                        if (false === $mysql->query($sql))
                        {
                            Server::$instance->warn("[摆摊-条件下架] 更新公示挂单记录失败. order_id:{$orderId}");
                        }

                        self::delPubOrderBuyCount($orderId);
                    }

                    $rs->free();
                }

                $sql = "UPDATE `" . BoothOrder::getTableName() . "` SET `status` = " . BoothOrder::STATUS_EXPIRED . " WHERE `item_key` = '{$key}' AND `status` = " . BoothOrder::STATUS_SELLING;
                if (false === $mysql->query($sql))
                {
                    Server::$instance->warn("[摆摊-条件下架] 更新挂单记录失败.");
                }

                if ($stock = Server::$itemList->get($key, 'boothStock'))
                {
                    ItemList::decrItemBoothStock($itemId, $key, $stock);
                }
            }
        }

        Server::$publicityLock->unlock();
        Server::$instance->info("[摆摊-条件下架] 执行完毕. uniqueId:{$prohibition->uniqueId}, itemId:{$itemId}, refineLv:{$refineLv}, enchantId:{$enchantId}, buffId:{$buffId}");
    }
}