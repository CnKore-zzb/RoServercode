<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/2/26
 * Time: 下午4:25
 */

namespace RO\Trade\Process;

use RO\Trade\Dao\Finance;
use RO\Trade\Dao\RecordBought;
use RO\Trade\Server;
use RO\Utils\Common;

class StatisticProcess extends \MyQEE\Server\WorkerCustom
{
    private $financeData = [];

    public function onStart()
    {
        Server::initConnection();

        $this->runTask();
        swoole_timer_tick(1000 * 3600, [$this, 'runTask']);
    }

    public function runTask()
    {
        $time1 = microtime(1);
        try
        {
            $this->statisticItemFinance();
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[今日财经] 统计发生异常, 错误信息:{$e->getMessage()}");
        }
        $time2 = microtime(1);

        $time3 = microtime(1);
        Finance::clearData(14);
        $time4 = microtime(1);

        $t1 = $time2 - $time1;
        $t2 = $time4 - $time3;
        Server::$instance->info("[今日财经] 程序耗时, 统计:{$t1}, 清理:{$t2}");
    }

    public function statisticItemFinance()
    {
        try
        {
            $curTime = time();
            // 凌晨5点作为起始
            $firstTime = strtotime(date('Y-m-d', time())) + 18000;
            if (empty($this->financeData))
            {
                $lastRecords = Finance::getLastRecords();
                foreach (Server::$item as $key => $item)
                {
                    if (!$item['isTrade'])
                    {
                        continue;
                    }

                    if (isset($lastRecords[$key]))
                    {
                        $this->financeData[] = $lastRecords[$key];
                    }
                    else
                    {
                        $soldCount = RecordBought::getSoldCount($key, $firstTime - Finance::CIRCLE_TIME, $firstTime);
                        if ($soldCount === false)
                        {
                            $soldCount = 0;
                        }

                        $data = [
                            'item_id'          => $key,
                            'price'            => $item['price'],
                            'last_price'       => $item['price'],
                            'deal_count'       => $soldCount,
                            'last_deal_count'  => $soldCount,
                            'price_ratio'      => 0,
                            'deal_count_ratio' => 0,
                            'time'             => $firstTime,
                        ];
                        $dao  = new Finance($data);
                        if ($dao->insert() === false)
                        {
                            // 数据库可能出现问题, 等待下个计时器重新执行
                            Server::$instance->warn("[今日财经] 物品item id: {$data['item_id']}初始化统计数据失败 data:", $data);
                            return;
                        }
                        else
                        {
                            $this->financeData[] = $data;
                        }
                    }
                    Server::$redis->del(sprintf(Finance::CACHE_DETAIL_KEY, $key));
                }
                Server::$redis->del(Finance::CACHE_RANK_KEY);
            }

            $hasNew = false;
            foreach ($this->financeData as $key => $value)
            {
                $nextTime = $value['time'] + Finance::CIRCLE_TIME;
                if ($curTime >= $nextTime)
                {
                    $price                     = Server::$item->get($value['item_id'], 'price');
                    $lastPrice                 = $value['price'];
                    $priceRatio                = Common::calcRatio($price, $lastPrice);
                    $dealCount                 = RecordBought::getSoldCount($value['item_id'], $value['time'], $nextTime);
                    $lastDealCount             = $value['deal_count'];
                    $dealCountRatio            = Common::calcRatio($dealCount, $lastDealCount);
                    $value['price']            = $price;
                    $value['last_price']       = $lastPrice;
                    $value['deal_count']       = $dealCount;
                    $value['last_deal_count']  = $lastDealCount;
                    $value['price_ratio']      = $priceRatio;
                    $value['deal_count_ratio'] = $dealCountRatio;
                    $value['time']             = $nextTime;
                    $dao                       = new Finance($value);
                    if ($dao->insert() === false)
                    {
                        Server::$instance->warn("[今日财经] 物品item id: {$value['item_id']}统计下个周期数据失败 data:", $value);
                    }
                    else
                    {
                        $this->financeData[$key] = $value;
                        $key                     = sprintf(Finance::CACHE_DETAIL_KEY, $value['item_id']);
                        Server::$redis->del($key);
                        $hasNew = true;
                    }
                }
            }

            if ($hasNew)
            {
                # 再执行一遍检查是否还有新的数据
                $this->statisticItemFinance();
                Server::$redis->del(Finance::CACHE_RANK_KEY);
            }
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[今日财经统计] 发生异常, 错误信息: {$e->getMessage()}");
        }
    }
}