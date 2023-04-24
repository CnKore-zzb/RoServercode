--TEST--
摆摊重新上架
--SKIPIF--
<?php require  __DIR__ . "/../skipif.inc"; ?>
--FILE--
<?php
include __DIR__ . '/../BaseTest.php';

class TestCase extends BaseTest
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

            if (!isset($msg->item_info))
            {
                $this->fail('没有item_info');
            }

            $orderId = $msg->item_info->order_id;
            $rs = $this->db->query("UPDATE `booth_order` SET `status` = " . \RO\Booth\Dao\BoothOrder::STATUS_EXPIRED . " WHERE `id` = {$orderId}");
            if ($rs && $this->db->affected_rows > 0) {
                $reSell = new \RO\Cmd\ResellPendingRecordTrade();
                $reSell->order_id = $orderId;
                $reSell->charid = $this->charId;
                $reSell->item_info = $msg->item_info;
                # 下调一个非法参数
                $reSell->item_info->down_rate = 800;
                $reSell->type = \RO\Cmd\ETradeType::ETRADETYPE_BOOTH;
                $this->sendUserMsg($reSell);

                # 上调一个非法参数
                $reSell->item_info->up_rate = 300;
                $this->sendUserMsg($reSell, 300);

                # 下调一个正常参数
                $reSell->item_info->up_rate = 0;
                $reSell->item_info->down_rate = 300;
                $reSell->type = \RO\Cmd\ETradeType::ETRADETYPE_BOOTH;
                $this->sendUserMsg($reSell, 300);
            } else {
                $this->fail('更新为过期订单记录失败');
            }
        }
        else if ($msg instanceof \RO\Cmd\SysMsg)
        {
            echo 'sys msg id:' . $msg->id, PHP_EOL;
        }
        else if ($msg instanceof \RO\Cmd\ResellPendingRecordTrade)
        {
            if ($msg->ret !== \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS )
            {
                $this->fail();
            }
            else
            {
                $this->success();
            }
        }
    }
}

new TestCase(TRADE_SERVER_HOST, TRADE_SERVER_PORT);
?>
--EXPECT--
sys msg id:10103
sys msg id:10103
success
