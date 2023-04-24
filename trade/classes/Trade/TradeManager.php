<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/7
 * Time: 下午5:44
 */

namespace RO\Trade;


use RO\Trade\Dao\Goods;

class TradeManager
{
    //清除时间, 表示获取logTime前的成交量
    public $logTime = 3 * 24 * 3600;

    public $currentTime;

    public $lastIncreaseTime;

    // 计时器更新时间
    private $updateTime = 1000 * 60 * 3;
//    private $updateTime = 1000 * 3;

    function __construct()
    {
    }

    /**
     * 创建子进程
     */
    public static function create()
    {
        # 初始化连接
        Server::initConnection();

        # 读取配置
        Server::loadTableConfig();

        $obj = new TradeManager();
        $obj->logTime     = Server::$configExchange['LogTime'] ?: $obj->logTime;
        $obj->currentTime = time() - $obj->logTime;
        $obj->lastIncreaseTime = time() + 1;    # 避免和第一次加载成交量的时间点重叠

        // 延迟执行一段时间,避免和其他进程在同一个时间点执行
        swoole_timer_after(1000 * 70, function() use ($obj)
        {
            swoole_timer_tick($obj->updateTime, function() use ($obj)
            {
                try
                {
                    $obj->updateSoldCount();
                }
                catch (\Exception $e)
                {
                    Server::$instance->warn($e->getFile().'('.$e->getLine().') '.$e->getMessage());
                }
            });
        });

        # 通知玩家有物品不能交易
        if (!empty(Server::$cannotTradeItems))
        {
            $func = function () {
                foreach (Server::$cannotTradeItems as $itemId)
                {
                    Goods::noticePlayerItemCannotTrade($itemId);
                }
            };

            # 等待场景服务器连接一段时间后执行
            swoole_timer_after(1000 * 60, $func);
        }
    }

    public function updateSoldCount()
    {
        $curTime = time();
        $time = $curTime - $this->logTime;
//        if (IS_DEBUG)
//        {
//            Server::$instance->debug("[交易所-更新成交量] 我在执行噢 时间段:{$this->currentTime} ~ {$time}");
//        }

        $sql  = "SELECT SUM(`count`) as sold_num, `item_id` FROM `trade_record_sold` WHERE `time` >= {$this->currentTime} AND `time` < {$time} GROUP BY `item_id`";
        $rs   = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            if (Server::$mysqlMaster->affected_rows)
            {
                while ($row = $rs->fetch_assoc())
                {
                    $soldNum = self::deductSoldCount($row['item_id'], $row['sold_num']);
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[交易所-更新成交量] 物品ID:{$row['item_id']} 近期成交量:{$soldNum} 过期的成交量为:{$row['sold_num']} 时间段:{$this->currentTime} ~ $time");
                    }
                }

                $rs->free();
                $this->currentTime = $time;
            }
        }

        unset($rs);

        # 提前2分钟的成交量, 可避免无法统计异步插入的数据在最新的时间点插入一个以前的时间记录。
        $curTime -= 120;
        if ($curTime <= $this->lastIncreaseTime)
            return;

        $sql  = "SELECT SUM(`count`) as sold_num, `item_id` FROM `trade_record_sold` WHERE `time` >= {$this->lastIncreaseTime} AND `time` < {$curTime} GROUP BY `item_id`";
        $rs   = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            if (Server::$mysqlMaster->affected_rows)
            {
                while ($row = $rs->fetch_assoc())
                {
                    $soldNum = self::addSoldCount($row['item_id'], $row['sold_num']);
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[交易所-更新成交量] 物品ID:{$row['item_id']} 近期成交量:{$soldNum} 新增成交量为:{$row['sold_num']} 时间段:{$this->lastIncreaseTime} ~ $curTime");
                    }
                }

                $rs->free();
                $this->lastIncreaseTime = $curTime;
            }
        }
    }

    /**
     * 获取内存中的物品成交量
     *
     *
     * @param $itemId
     * @return int
     */
    public static function getSoldCount($itemId)
    {
        if ($item = Server::$item->get($itemId))
        {
            return $item['soldNum'] ?: 0;
        }

        return 0;
    }

    /**
     * 增加物品成交数量
     *
     *
     * @param $itemId
     * @param int $count
     * @return bool|int
     */
    public static function addSoldCount($itemId, $count = 1)
    {
        return Server::$item->incr($itemId, 'soldNum', $count);
    }

    /**
     * 减少物品成交数量
     *
     * @param $itemId
     * @param int $count
     * @return bool|int
     */
    public static function deductSoldCount($itemId, $count = 1)
    {
        return Server::$item->decr($itemId, 'soldNum', $count);
    }

    /**
     * 统计手续费
     *
     * @param int $fee
     * @param null $date 默认为当天
     * @return bool|int
     */
    public static function incrCounterFee(int $fee, $date = null)
    {
        if ($date === null) $date = time();

        try
        {
            return Server::$redis->hIncrBy('fee', $date, $fee);
        }
        catch (\Exception $e)
        {
            Server::$instance->warn('[交易所-统计手续费] 发生异常。 错误信息:' . $e->getMessage());
            return false;
        }
    }

    /**
     * 获取统计后的手续费
     *
     * @param null $date 默认为当天
     * @return string
     */
    public static function getCounterFee($date = null)
    {
        if ($date === null) $date = time();
        try
        {
            return Server::$redis->hGet('fee', $date);
        }
        catch (\Exception $e)
        {
            Server::$instance->warn('[交易所-统计税费] Redis读取失败。 错误信息: ' . $e->getMessage());
            return false;
        }
    }

    /**
     * 统计交易税
     *
     * @param int $tax
     * @param null $date 默认为当天
     * @return bool|int
     */
    public static function incrTax(int $tax, $date = null)
    {
        if ($date === null) $date = time();

        try
        {
            return Server::$redis->hIncrBy('tax', $date, $tax);
        }
        catch (\Exception $e)
        {
            Server::$instance->warn('[交易所-统计税费] 发生异常。 错误信息:' . $e->getMessage());
            return false;
        }
    }

    /**
     * 获取统计后的交易税
     *
     * @param null $date 默认为当天
     * @return string
     */
    public static function getTax($date = null)
    {
        if ($date === null) $date = time();
        try
        {
            return Server::$redis->hGet('tax', $date);
        }
        catch (\Exception $e)
        {
            Server::$instance->warn('[交易所-统计税费] Redis失败。 错误信息:' . $e->getMessage());
            return false;
        }
    }
}