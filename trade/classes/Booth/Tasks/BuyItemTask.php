<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/6/29
 * Time: 下午4:49
 */

namespace RO\Booth\Tasks;

use RO\Booth\BoothOrderService;
use RO\Booth\Dao\BoothOrder;
use RO\Booth\Dao\BoothRecordBought;
use RO\Booth\Dao\BoothRecordTakeStatus;
use RO\Cmd\BuyItemRecordTradeCmd;
use RO\Cmd\ETRADE_RET_CODE;
use RO\Cmd\ETradeType;
use RO\Cmd\ReduceMoneyRecordTradeCmd;
use RO\Trade\ActBuy;
use RO\Trade\Player;
use RO\Trade\ROCache;
use RO\Trade\Server;
use RO\ZoneForward;

class BuyItemTask extends BoothTask
{
    public $fd;

    public $charId;

    public $sellerId;

    public $orderId;

    public $buyCount;

    public $price;

    public function execute($taskId)
    {
        $order = BoothOrder::getById($this->orderId);
        do
        {
            if ($order === false)
            {
                $this->sendSysMsgToUser(ActBuy::SYS_MSG_DB_ERROR);
                break;
            }
            else if ($order === null)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊购买] order_id:{$this->orderId}不存在");
                }
                break;
            }

            if ($order->charId !== $this->sellerId)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊购买] 投递的玩家id和订单不一致");
                }
                break;
            }

            if ($order->status != BoothOrder::STATUS_SELLING)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊下架] 不处于出售状态");
                }
                $this->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_CANNOT_FIND_PENDING);
                break;
            }

            if ($order->isExpired())
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊下架] 订单已过期");
                }
                $this->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_CANNOT_FIND_PENDING);
                break;
            }

            if ($this->buyCount > $order->getActualStock())
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊购买] 订单库存不足");
                }
                $this->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_STOCK_NOT_ENOUGH);
                break;
            }

            // 玩家传过来的价格不一致
            if ($this->price != $order->getPrice())
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊购买] 购买价格不一致");
                }
                $this->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_INVALID_PRICE);
                break;
            }

            if ($order->isPublicity)
            {
                $rs = $this->doPubBuy($order);
            }
            else
            {
                $rs = $this->doBuy($order);
            }

            if ($rs === false)
            {
                break;
            }

            return;
        }
        while (false);

        $msg         = new BuyItemRecordTradeCmd();
        $msg->charid = $this->charId;
        $msg->type   = ETradeType::ETRADETYPE_BOOTH;
        $msg->ret    = ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL;
        $this->sendToUser($msg);

        ZoneForward::unBoothLock($this->charId);
    }

    protected function doBuy(BoothOrder $order)
    {
        // 摊位没开不能购买
        if (!IS_DEBUG && !Player::getBoothOpenStatus($order->charId))
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊购买] order_id:{$this->orderId}, char_id:{$order->charId}, 摆摊不在开启状态");
            }
            $this->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_BOOTH_NOT_OPEN);
            return false;
        }

        $price = BoothOrder::calcRealPrice($order, $this->price);
        if ($price <= 0)
        {
            Server::$instance->warn("[摆摊购买] 计算实际价格为负数, price:{$price}, order_id:{$order->id}");
            return false;
        }

        $itemData             = $order->getItemData();
        $record               = new BoothRecordBought();
        $record->charId       = $this->charId;
        $record->orderId      = $order->id;
        $record->sellerId     = $order->charId;
        $record->sellerName   = $order->playerName;
        $record->sellerZoneId = $order->playerZoneId;
        $record->status       = BoothRecordBought::STATUS_PAY_PENDING;
        $record->takeStatus   = BoothRecordTakeStatus::TAKE_STATUS_CAN_TAKE_GIVE;
        $record->itemId       = $order->itemId;
        $record->itemKey      = $order->itemKey;
        $record->time         = time();
        $record->isPublicity  = 0;
        $record->endTime      = 0;
        $record->count        = $this->buyCount;
        $record->totalCount   = $this->buyCount;
        $record->spendQuota   = $record->getSpendQuota($price);
        $record->price        = $record->getDiscountPrice($price);
        $record->realPrice    = $price;
        $record->itemData     = $itemData;
        if ($itemData !== null)
        {
            $record->refineLv = $itemData->equip->refinelv;
            $record->isDamage = $itemData->equip->damage == true ? 1 : 0;
        }

        $db    = Server::$mysqlMaster;
        $begin = $db->begin_transaction();
        if ($begin === false)
        {
            $this->sendSysMsgToUser(ActBuy::SYS_MSG_DB_ERROR);
            Server::$instance->warn("[摆摊购买] 开启事务失败");
            return false;
        }

        try
        {
            if ($record->insert() === false)
            {
                $this->sendSysMsgToUser(ActBuy::SYS_MSG_DB_ERROR);
                throw new \Exception("插入买家记录失败!");
            }

            if (BoothOrder::lockStock($order->id, $record->count) === false)
            {
                $this->sendSysMsgToUser(ActBuy::SYS_MSG_DB_ERROR);
                throw new \Exception("预占库存失败!");
            }

            if ($db->commit() === false)
            {
                $this->sendSysMsgToUser(ActBuy::SYS_MSG_DB_ERROR);
                Server::$instance->warn("[摆摊购买] 提交事务失败!");
                return false;
            }

            $this->doPay($record);

            return true;
        }
        catch (\Exception $e)
        {
            $db->rollback();
            Server::$instance->warn("[摆摊购买] {$e->getMessage()}, buy_id:{$this->charId}, order_id:{$order->id}, buy_count:{$this->buyCount}, order_stock:{$order->stock}, order_lock_stock:{$order->lockStock}");
            return false;
        }
    }

    protected function doPubBuy(BoothOrder $order)
    {
        if ($order->endTime <= time()) {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊购买] 公示已经结束, 即将分单处理");
            }
            $this->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_PENDING_WAS_LOCKED);
            return false;
        }

        $boughtCount = BoothOrderService::getPubBuyCount($order->id, $this->charId);
        if ($boughtCount === false)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊购买] 获取已抢购数失败");
            }
            $this->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_PUBLICITY_FAIL2);
            return false;
        }

        if ($this->buyCount + $boughtCount > $order->getActualStock())
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[摆摊购买] 抢购超过限制");
            }
            $this->sendSysMsgToUser(ActBuy::SYS_MSG_BUY_PUBLICITY_FAIL_COUNT);
            return false;
        }

        $price = BoothOrder::calcRealPrice($order, $this->price);
        if ($price <= 0)
        {
            Server::$instance->warn("[摆摊购买] 计算实际价格为负数, price:{$price}, order_id:{$order->id}");
            return false;
        }

        $itemData             = $order->getItemData();
        $record               = new BoothRecordBought();
        $record->charId       = $this->charId;
        $record->orderId      = $order->id;
        $record->sellerId     = $order->charId;
        $record->sellerName   = $order->playerName;
        $record->sellerZoneId = $order->playerZoneId;
        $record->status       = BoothRecordBought::STATUS_PAY_PENDING;
        $record->takeStatus   = BoothRecordTakeStatus::TAKE_STATUS_CAN_TAKE_GIVE;
        $record->itemId       = $order->itemId;
        $record->itemKey      = $order->itemKey;
        $record->time         = time();
        $record->isPublicity  = $order->isPublicity;
        $record->endTime      = $order->endTime;
        $record->count        = 0;
        $record->totalCount   = $this->buyCount;
        $record->spendQuota   = $record->getSpendQuota($price);
        $record->price        = $record->getDiscountPrice($price);
        $record->realPrice    = $price;
        $record->itemData     = $itemData;

        if ($itemData !== null)
        {
            $record->refineLv = $itemData->equip->refinelv;
            $record->isDamage = $itemData->equip->damage == true ? 1 : 0;
        }

        if ($record->insert() === false)
        {
            $this->sendSysMsgToUser(ActBuy::SYS_MSG_DB_ERROR);
            return false;
        }

        if (BoothOrderService::incrPubBuyCount($record->orderId, $record->charId, $record->totalCount) === false)
        {
            Server::$instance->warn("[摆摊购买] 公示购买递增购买数量失败, order_id:{$order->id}, char_id:{$record->charId}");
        }

        $this->doPay($record);

        return true;
    }

    protected function doPay(BoothRecordBought $record)
    {
        $cmd              = new ReduceMoneyRecordTradeCmd();
        $cmd->charid      = $record->charId;
        $cmd->record_id   = $record->id;
        $cmd->total_money = $record->price * $record->totalCount;
        $cmd->money_type  = Server::DEFAULT_MONEY_TYPE;
        $totalQuota = $record->spendQuota * $record->totalCount;
        if ($record->isPublicity)
        {
            $cmd->lock_quota = $totalQuota;
        }
        else
        {
            $cmd->quota = $totalQuota;
        }
        $cmd->type = ETradeType::ETRADETYPE_BOOTH;

        if (ZoneForward::sendToZoneByFd($cmd, $this->fd))
        {
            Server::$instance->info("[摆摊购买] 发起扣钱请求成功, char_id:{$record->charId}, record_id:{$record->id}, order_id:{$record->orderId}, item_id:{$record->itemId}, count:{$this->buyCount}, total_money:{$cmd->total_money}");
            return true;
        }
        else
        {
            Server::$instance->warn("[摆摊购买] 发起扣钱请求失败, char_id:{$record->charId}, record_id:{$record->id}, order_id:{$record->orderId}, item_id:{$record->itemId}, count:{$this->buyCount}, total_money:{$cmd->total_money}");
            return false;
        }
    }

    public function sendSysMsgToUser(int $msgId)
    {
        return ZoneForward::sendSysMsgToUserByCharId($this->fd, 0, $this->charId, $msgId);
    }

    public function sendToUser($msg)
    {
        return ZoneForward::sendToUserByFd($msg, $this->charId, $this->fd);
    }
}