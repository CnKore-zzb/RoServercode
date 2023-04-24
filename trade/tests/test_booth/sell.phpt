--TEST--
摆摊上架
--SKIPIF--
<?php require  __DIR__ . "/../skipif.inc"; ?>
--FILE--
<?php
include __DIR__ . '/../BaseTest.php';

class TestBoothSell extends BaseTest
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
            if ($msg->ret !== \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS){
                $this->fail();
            } else {
                $this->success();
            }
        }
    }
}

new TestBoothSell(TRADE_SERVER_HOST, TRADE_SERVER_PORT);
?>
--EXPECT--
success