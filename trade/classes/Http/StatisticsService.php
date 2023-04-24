<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/17
 * Time: 下午7:31
 */

namespace RO\Http;

use \Exception;
use RO\Trade\Server;
use RO\Trade\TradeManager;

class StatisticsService
{
    public static function getPriceAdjustLog($itemId, $startTime = 0, $endTime = 0)
    {
        $startTime = (int) $startTime ?: time();
        $endTime = (int) $endTime ?: strtotime('-1 week');
        $itemId = (int) $itemId;

        $sql = "SELECT * FROM `trade_price_adjust_log` WHERE `itemid` = {$itemId} AND `last_time` >= {$startTime} AND `last_time` <= {$endTime}";
        $rs = Server::$mysqlMaster->query($sql);
        $data = [];
        if ($rs)
        {
            while($r = $rs->fetch_assoc())
            {
                $data[] = $r;
            }

            $rs->free();
        }

        return $data;
    }

    public static function getCounterFee($date = null)
    {
        if ($counterFee = TradeManager::getCounterFee($date))
        {
            return [ 'counterFee' => $counterFee];
        }
        else
        {
            throw new Exception('获取手续费失败');
        }
    }

    public static function getTax($date = null)
    {
        if ($tax = TradeManager::getTax($date))
        {
            return [ 'tax' => $tax];
        }
        else
        {
            throw new Exception('获取交易税失败');
        }
    }
}