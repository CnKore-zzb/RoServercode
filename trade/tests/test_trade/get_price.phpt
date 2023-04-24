--TEST--
摆摊上架
--SKIPIF--
<?php require  __DIR__ . "/../skipif.inc"; ?>
--FILE--
<?php
include __DIR__ . '/../BaseTest.php';
class TestCase extends BaseTest
{
    public function onTest(\Swoole\Client $cli)
    {
        $itemData = new \RO\Cmd\ItemData();
        $itemData->equip = new \RO\Cmd\EquipData();
        $itemData->equip->refinelv = 10;
        $itemData->equip->damage = true;
        $itemData->base = new \RO\Cmd\ItemInfo();
        $this->reqItemPrice(40359, \RO\Cmd\ETradeType::ETRADETYPE_TRADE, $itemData);
    }

    public function onReceive(\Swoole\Client $cli, string $data)
    {
        $msg = parent::onReceive($cli, $data);
        if ($msg instanceof \RO\Cmd\ReqServerPriceRecordTradeCmd)
        {
            var_dump($msg->price);
            $this->success();
        }
        else if ($msg instanceof \RO\Cmd\SysMsg)
        {
            echo 'sys msg id:' . $msg->id, PHP_EOL;
        }
    }
}
new TestCase(TRADE_SERVER_HOST, TRADE_SERVER_PORT);
?>
--EXPECTF--
int(%d)
success