--TEST--
摆摊上架
--SKIPIF--
<?php require  __DIR__ . "/../skipif.inc"; ?>
--FILE--
<?php
include __DIR__ . '/../BaseTest.php';
class TestCase extends BaseTest
{
    /**
     * @var \RO\Cmd\TradeItemBaseInfo
     */
    public $itemInfo;

    public $count = 1;

    public $buyCmd;

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
            $sellMsg->item_info->count               = $this->count;
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

            foreach ($msg->lists as $item)
            {
                if (!$item->is_expired)
                {
                    $this->itemInfo = $item;
                    $this->buyCmd = $buyCmd = new \RO\Cmd\BuyItemRecordTradeCmd();
                    $buyCmd->charid = $this->charId;
                    $buyCmd->item_info = $item;
                    $buyCmd->type = \RO\Cmd\ETradeType::ETRADETYPE_BOOTH;
                    $buyCmd->item_info->count = $this->count + 1;
                    $this->sendUserMsg($buyCmd);

                    $this->buyCmd->item_info->count = $this->count;
                    $this->sendUserMsg($this->buyCmd, 300);
                    return;
                }
            }

            $this->fail("没有订单");
        }
        else if ($msg instanceof \RO\Cmd\SysMsg)
        {
            echo 'sys msg id:' . $msg->id, PHP_EOL;
        }
        else if ($msg instanceof \RO\Cmd\ReduceMoneyRecordTradeCmd)
        {
            if ($msg->total_money !== $this->calcDiscountPrice(\RO\Booth\Dao\BoothOrder::calcAdjustPrice($this->itemInfo->price, $this->itemInfo->up_rate, $this->itemInfo->down_rate)) * $this->itemInfo->count) {
                $this->fail("购买的数量和价格不一致");
            }
        }
        else if ($msg instanceof \RO\Cmd\BuyItemRecordTradeCmd)
        {
            if ($msg->ret !== \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS) {
                echo 'buy fail', PHP_EOL;
            } else {
                $this->success();
            }
        }
    }
}
new TestCase(TRADE_SERVER_HOST, TRADE_SERVER_PORT);
?>
--EXPECT--
sys msg id:25702
buy fail
success