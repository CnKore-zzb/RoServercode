<?php
namespace RO\Trade\Process;

use RO\Trade\Server;
use RO\Trade\Dao\ItemList;

/**
 * 处理库存数数据库更新的子进程
 *
 * @package RO\Trade\Process
 */
class UpdateStock extends \MyQEE\Server\WorkerCustom
{
    /**
     * @var array
     */
    protected static $updateStocks;

    public function onStart()
    {
        Server::initConnection();

        self::$updateStocks = [];

        swoole_timer_tick(300, function()
        {
            $i = 0;
            $c = null;
            while (false !== ($data = Server::$stockUpdateChannel->pop()))
            {
                $i++;
                if (null === $c)
                {
                    $c = Server::$stockUpdateChannel->stats()['queue_num'] + 1;
                }

                $st = @unpack('LItemListId/Qstock', $data);
                if (false !== $st)
                {
                    self::$updateStocks[$st['ItemListId']] = $st['stock'];
                }

                if ($i == $c)break;
            }
        });

        # 每10秒更新一次
        swoole_timer_tick(1000 * 10, [$this, 'updateData']);
    }

    public function onStop()
    {
        $this->updateData();
    }

    public function updateData()
    {
        $tb = ItemList::getTableName();
        foreach (self::$updateStocks as $itemListId => $stock)
        {
            $sql = "UPDATE `{$tb}` SET `stock` = '{$stock}' WHERE `id` = '{$itemListId}'";
            Server::$mysqlMaster->query($sql);
        }

        self::$updateStocks = [];
    }
}