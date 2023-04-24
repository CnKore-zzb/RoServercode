--TEST--
摆摊出售成功后下架
--SKIPIF--
<?php require  __DIR__ . "/../skipif.inc"; ?>
--FILE--
<?php
include __DIR__ . '/../BaseTest.php';

class TestBoothSellAndCancel extends BaseTest
{
    public function onTest(\Swoole\Client $cli)
    {
        $this->boothOpen();
        $this->reqItemPrice(40016);
    }

    public function onReceive(\Swoole\Client $cli, string $data)
    {
        $msg = parent::onReceive($cli, $data);
        if ($msg instanceof \RO\Cmd\ReqServerPriceRecordTradeCmd)
        {
            // 出售
            $sellMsg                                 = new \RO\Cmd\SellItemRecordTradeCmd();
            $sellMsg->type                           = \RO\Cmd\ETradeType::ETRADETYPE_BOOTH;
            $sellMsg->charid                         = $this->charId;
            $sellMsg->item_info                      = new \RO\Cmd\TradeItemBaseInfo();
            $sellMsg->item_info->price               = $msg->price;
            $sellMsg->item_info->itemid              = $msg->itemData->base->id;
            $sellMsg->item_info->count               = 1;
            $sellMsg->item_info->item_data           = $msg->itemData;
            # 上调
//        $sellMsg->item_info->up_rate = 1000;
            # 下调
            $sellMsg->item_info->down_rate = 500;
            $this->sendUserMsg($sellMsg);
        }
        else if ($msg instanceof \RO\Cmd\SellItemRecordTradeCmd)
        {
            if ($msg->ret !== \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS)
            {
                $this->fail();
            }

            $listMsg         = new \RO\Cmd\BoothPlayerPendingListCmd();
            $listMsg->charid = $this->charId;
            $this->sendUserMsg($listMsg);
        }
        else if ($msg instanceof \RO\Cmd\BoothPlayerPendingListCmd)
        {
            if (empty($msg->lists))
            {
                $this->fail();
            }

            $list = end($msg->lists);
            $orderId             = $list->order_id;
            $cancelMsg           = new \RO\Cmd\CancelItemRecordTrade();
            $cancelMsg->type     = \RO\Cmd\ETradeType::ETRADETYPE_BOOTH;
            $cancelMsg->order_id = $orderId;
            $cancelMsg->charid   = $this->charId;
            $this->sendUserMsg($cancelMsg);
        }
        else if ($msg instanceof RO\Cmd\CancelItemRecordTrade)
        {
            if ($msg->ret != \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS) {
                $this->fail();
            }

            $this->success();
        }
    }
}

new TestBoothSellAndCancel(TRADE_SERVER_HOST, TRADE_SERVER_PORT);
?>
--EXPECT--
success