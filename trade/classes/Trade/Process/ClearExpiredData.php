<?php
namespace RO\Trade\Process;
use RO\Booth\Dao\BoothOrder;
use RO\Booth\Dao\BoothRecordBought;
use RO\Booth\Dao\BoothRecordSold;
use RO\Trade\Server;
use RO\Trade\Dao\Goods;
use RO\Trade\Dao\RecordBought;
use RO\Trade\Dao\RecordSold;

/**
 * 清理过期物品的子进程
 *
 * @package RO\Trade\Process
 */
class ClearExpiredData extends \MyQEE\Server\WorkerCustom
{
    public function onStart()
    {
        # 每3分钟清理过期数据
        swoole_timer_tick(1000 * 60 * 3, function()
        {
            $mysql = Server::createMySQL(true);
            if (!$mysql)
            {
                return;
            }

            $time1 = microtime(1);
            # 清理购买、卖出记录
            RecordBought::archiveExpireRecord($mysql);
            $time2 = microtime(1);
            usleep(5000);
            RecordSold::archiveExpireRecord($mysql);
            $time3 = microtime(1);
            usleep(5000);
            Goods::archiveExpireRecord($mysql);
            $time4 = microtime(1);
            usleep(5000);
            BoothOrder::archiveExpireRecord($mysql);
            $time5 = microtime(1);
            usleep(5000);
            BoothRecordBought::archiveExpireRecord($mysql);
            $time6 = microtime(1);
            usleep(5000);
            BoothRecordSold::archiveExpireRecord($mysql);
            $time7 = microtime(1);
            usleep(5000);
            Goods::clearNotTookRecord($mysql);
            $time8 = microtime(1);
            usleep(5000);
            BoothOrder::clearNotTookRecord($mysql);
            $time9 = microtime(1);

            $t0 = $time9 - $time1;
            $t1 = $time2 - $time1;
            $t2 = $time3 - $time2;
            $t3 = $time4 - $time3;
            $t4 = $time5 - $time4;
            $t5 = $time6 - $time5;
            $t6 = $time7 - $time6;
            $t7 = $time8 - $time7;
            $t8 = $time9 - $time8;

            Server::$instance->info("[清理数据] 清理购买耗时: 购买:{$t1}, 卖出:{$t2}, 挂单:{$t3}，摆摊挂单:{$t4}, 摆摊购买:{$t5}, 摆摊出售:{$t6}, 交易所未取回订单:{$t7}, 摆摊未取回订单:{$t8}, 累计:{$t0}");

            $mysql->close();
        });

        # 重置库存和公示数
        swoole_timer_after(1000 * 60 * 5 + 10, function()
        {
            swoole_timer_tick(1000 * 60 * 60, function()
            {
                $arr = [];
                foreach (Server::$item as $k => $v)
                {
                    $arr[$k] = [0, 0, $v['stock'], $v['publicityNum'], $v['lastPubEndTime']];
                }

                foreach (Server::$itemList as $k => $v)
                {
                    $itemId = $v['item_id'];
                    if ($v['stock'] > 0)
                    {
                        $arr[$itemId][0] += $v['stock'];
                    }
                    if ($v['is_publicity'])
                    {
                        $arr[$itemId][1] += 1;
                    }
                }

                foreach ($arr as $itemId => $v)
                {
                    list($newStock, $newPubNum, $oldStock, $oldPubNum, $lastPubEndTime) = $v;
                    $diffStock  = $newStock - $oldStock;
                    $diffPubNum = $newPubNum - $oldPubNum;
                    if ($diffStock != 0)
                    {
                        $rs = Server::$item->incr($itemId, 'stock', $diffStock);
                        if ($rs !== false && $rs < 0)
                        {
                            Server::$item->set($itemId, ['stock' => 0]);
                        }
                    }

                    if ($diffPubNum != 0 && $lastPubEndTime < time())
                    {
                        $rs = Server::$item->incr($itemId, 'publicityNum', $diffPubNum);
                        if ($rs !== false && $rs < 0)
                        {
                            Server::$item->set($itemId, ['publicityNum' => 0]);
                        }
                    }
                }
            });
        });
    }
}