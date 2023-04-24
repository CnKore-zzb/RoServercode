--TEST--
公示出售
--SKIPIF--
<?php require  __DIR__ . "/../skipif.inc"; ?>
--FILE--
<?php
include __DIR__ . '/../BaseTest.php';
class TestCase extends BaseTest
{
    public function onTest(\Swoole\Client $cli)
    {
//        $itemData = new \RO\Cmd\ItemData();
//        $itemData->equip = new \RO\Cmd\EquipData();
//        $itemData->equip->refinelv = 6;
//        $itemData->base = new \RO\Cmd\ItemInfo();
//        $itemData->base->id = 140919;
        $itemData = null;
        $this->reqItemPrice(140919, \RO\Cmd\ETradeType::ETRADETYPE_BOOTH, $itemData);
    }

    public function onReceive(\Swoole\Client $cli, string $data)
    {
        $msg = parent::onReceive($cli, $data);
        if ($msg instanceof \RO\Cmd\ReqServerPriceRecordTradeCmd)
        {
            if ($msg->statetype !== \RO\Cmd\StateType::St_InPublicity && $msg->statetype !== \RO\Cmd\StateType::St_WillPublicity)
            {
                $this->fail('物品不处于公示中');
            }

            $sellMsg                                 = new \RO\Cmd\SellItemRecordTradeCmd();
            $sellMsg->type                           = \RO\Cmd\ETradeType::ETRADETYPE_BOOTH;
            $sellMsg->charid                         = $this->charId;
            $sellMsg->item_info                      = new \RO\Cmd\TradeItemBaseInfo();
            $sellMsg->item_info->price               = $msg->price;
            $sellMsg->item_info->itemid              = $msg->itemData->base->id;
            $sellMsg->item_info->count               = 1;
            $sellMsg->item_info->item_data           = $msg->itemData;
            $sellMsg->item_info->publicity_id = 1;
            $sellMsg->item_info->up_rate      = 10000;
            $this->sendUserMsg($sellMsg);

            $sellMsg->item_info->up_rate = 700;
            $this->sendUserMsg($sellMsg, 300);

        }
        else if ($msg instanceof \RO\Cmd\SysMsg)
        {
            echo 'sys msg id:' . $msg->id, PHP_EOL;
        }
        else if ($msg instanceof \RO\Cmd\SellItemRecordTradeCmd)
        {
            if ($msg->ret === \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS)
            {
                $reqMsg           = new \RO\Cmd\ItemSellInfoRecordTradeCmd();
                $reqMsg->type     = \RO\Cmd\ETradeType::ETRADETYPE_BOOTH;
                $reqMsg->charid   = $this->charId;
                $reqMsg->order_id = $msg->item_info->order_id;
                $this->sendUserMsg($reqMsg);
            }
        }
        else if ($msg instanceof \RO\Cmd\ItemSellInfoRecordTradeCmd)
        {
            if ($msg->statetype !== \RO\Cmd\StateType::St_InPublicity)
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
success
