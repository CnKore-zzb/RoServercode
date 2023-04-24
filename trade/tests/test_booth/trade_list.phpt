--TEST--
交易所列表显示摆摊公示订单
--SKIPIF--
<?php require  __DIR__ . "/../skipif.inc"; ?>
--FILE--
<?php
include __DIR__ . '/../BaseTest.php';
class TestCase extends BaseTest
{
    public $orderId;

    public function onTest(\Swoole\Client $cli)
    {
        $this->reqItemPrice(140919, \RO\Cmd\ETradeType::ETRADETYPE_BOOTH);
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
            $sellMsg->item_info->up_rate      = 400;
            $this->sendUserMsg($sellMsg);
        }
        else if ($msg instanceof \RO\Cmd\SellItemRecordTradeCmd)
        {
            if ($msg->ret === \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS)
            {
                $this->orderId = $msg->item_info->order_id;
                $cmd = new \RO\Cmd\DetailPendingListRecordTradeCmd();
                $cmd->charid = $this->randCharId();
                $cmd->search_cond = $search = new \RO\Cmd\SearchCond();
                $search->rank_type = \RO\Cmd\RankType::RANKTYPE_REFINE_LV_DES;
                $search->item_id = 140919;
                $search->page_index = 0;
                $search->page_count = 10;
                $search->trade_type = \RO\Cmd\ETradeType::ETRADETYPE_ALL;
                $this->sendUserMsg($cmd);
            }
            else
            {
                $this->fail('出售失败');
            }
        }
        else if ($msg instanceof \RO\Cmd\DetailPendingListRecordTradeCmd)
        {
//            var_dump(json_encode($msg->lists));
            foreach ($msg->lists as $itemBaseInfo)
            {
                if ($itemBaseInfo->order_id === $this->orderId)
                {
                    $this->success();
                    return;
                }
            }
            $this->fail();
        }
    }
}
new TestCase(TRADE_SERVER_HOST, TRADE_SERVER_PORT);
?>
--EXPECT--
success
