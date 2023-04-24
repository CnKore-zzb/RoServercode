<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/6/26
 * Time: 下午5:17
 */

namespace RO\Booth\Tasks;

use RO\Booth\Dao\BoothOrder;
use RO\Booth\BoothOrderService;
use RO\Cmd\CancelItemRecordTrade;
use RO\Cmd\ETRADE_RET_CODE;
use RO\Cmd\ETradeType;
use RO\Trade\ActSell;
use RO\Trade\Server;
use RO\ZoneForward;

class PlayerCancelOrderTask extends BoothTask
{
    public $fd;

    public $charId;

    public $orderId;

    public function execute($taskId)
    {
        $err = false;
        $ret = ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL;
        do
        {
            $order = BoothOrder::getById($this->orderId);
            if (false === $order)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊下架] 数据库出错");
                }
                $this->sendSysMsgToUser(ActSell::SYS_MSG_DB_ERROR);
                break;
            }
            else if (null === $order)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊下架] 找不到订单");
                }
                $this->sendSysMsgToUser(ActSell::SYS_MSG_CANCEL_ALREADY_SELLED);
                break;
            }

            if ($this->charId != $order->charId)
            {
                $err = "非法下架，指定的订单ID:$this->orderId,不是当前用户: {$this->charId} != {$order->charId}";
                break;
            }

            if ($order->getActualStock() <= 0)
            {
                if ($order->lockStock > 0)
                {
                    Server::$instance->info("[摆摊下架] 下架存在预占库存的订单, order_id:{$order->id}, char_id:{$order->charId}, lock_stock:{$order->lockStock}, is_expire:{$order->isExpired()}");
                    $this->sendSysMsgToUser(ActSell::SYS_MSG_CANCEL_WAS_LOCKED);
                    break;
                }

                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊下架] 下架库存少于0的订单");
                }
                $this->sendSysMsgToUser(ActSell::SYS_MSG_CANCEL_ALREADY_SELLED);
                break;
            }

            # 非在售状态
            if ($order->status != BoothOrder::STATUS_SELLING && $order->status != BoothOrder::STATUS_EXPIRED)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊下架] 非允许下架的状态");
                }
                $this->sendSysMsgToUser(ActSell::SYS_MSG_CANCEL_WAS_LOCKED);
                break;
            }

//            # 处于出售状态,但面临转为过期状态时,不能下架
//            if ($order->isExpired() && $order->status == BoothOrder::STATUS_SELLING)
//            {
//                if (IS_DEBUG)
//                {
//                    Server::$instance->debug("[摆摊下架] 处于出售状态,但面临转为过期状态时,不能下架");
//                }
//                $this->sendSysMsgToUser(ActSell::SYS_MSG_CANCEL_WAS_LOCKED);
//                break;
//            }

            # 公示期物品不能下架
            if ($order->isPublicity == 1)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[摆摊下架] 公示期物品不能下架");
                }
                $this->sendSysMsgToUser(ActSell::SYS_MSG_CANCEL_WAS_LOCKED);
                break;
            }

            $cmd           = new CancelItemRecordTrade();
            $cmd->charid   = $order->charId;
            $cmd->order_id = $order->id;
            $cmd->type     = ETradeType::ETRADETYPE_BOOTH;
            if (BoothOrderService::cancelOrder($order, $this->fd))
            {
                $ret = ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS;
            }
        }
        while(false);

        if ($err)
        {
            Server::$instance->warn("[摆摊下架] {$err}");
        }

        ZoneForward::unBoothLock($this->charId);

        $cmd           = new CancelItemRecordTrade();
        $cmd->charid   = $this->charId;
        $cmd->order_id = $this->orderId;
        $cmd->type     = ETradeType::ETRADETYPE_BOOTH;
        $cmd->ret      = $ret;
        ZoneForward::sendToUserByFd($cmd, $this->charId, $this->fd);
    }

    public function sendSysMsgToUser(int $msgId)
    {
        return ZoneForward::sendSysMsgToUserByCharId($this->fd, 0, $this->charId, $msgId);
    }
}