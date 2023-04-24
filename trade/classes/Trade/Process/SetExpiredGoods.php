<?php
namespace RO\Trade\Process;
use RO\Booth\BoothOrderService;
use RO\Trade\Dao\Goods;
use RO\Trade\ItemTask;
use RO\Trade\Server;

/**
 * 清理过期挂单的子进程
 *
 * @package RO\Trade\Process
 */
class SetExpiredGoods extends \MyQEE\Server\WorkerCustom
{
    public function onStart()
    {
        # 读取配置
        Server::loadTableConfig();

        # 计数器只支持42亿的计数, 所以每小时检查计数器是否快溢出
        swoole_timer_tick(1000 * 60 * 60 + 1000, function()
        {
            if (($count = Server::$counterSuccess->get()) > 200000000)
            {
                # 将2亿的余数记录下来
                Server::$counterSuccess->set($count % 200000000);
                Server::$counterSuccessX->add(intval($count / 200000000));
            }
        });


        # 每10秒执行下架过期物品
        swoole_timer_tick(1000 * 10, function()
        {
            $mysql = Server::createMySQL(true);
            if (!$mysql)
            {
                return;
            }

            static $lastRunTime      = 0;
            static $randOffShelfList = [];                                  # 随机下架挂单列表
            $exp  = time() - Server::$configExchange['ExchangeHour'];       # 过期时间
            $time = $exp + 600;                                             # 读还有 10 分钟就到期的订单
            $sql  = "SELECT `id`, `item_list_id` as `listId`, `time` FROM `trade_goods` WHERE `time` < '{$time}'". ($lastRunTime > 0 ? " AND `time` >= '{$lastRunTime}'" : '') ." AND `status` = '". Goods::STATUS_SELLING ."' AND `is_publicity` = '0' ORDER BY `time` ASC LIMIT 200";
            $list = [];

            if ($rs = $mysql->query($sql))
            {
                if ($rs->num_rows > 0)
                {
                    while ($row = $rs->fetch_object())
                    {
                        $row->id     = intval($row->id);
                        $row->listId = intval($row->listId);
                        $row->time   = intval($row->time);
                        $lastRunTime = $row->time;

                        if ($row->time - $exp > 100)
                        {
                            # 放入到随机下架列表里
                            $randOffShelfList[$row->id] = $row;
                        }
                        else
                        {
                            $list[$row->listId][] = $row->id;
                        }
                    }
                }
                else
                {
                    $lastRunTime = $time;
                }
                $rs->free();
            }

            if (count($randOffShelfList) > 0)
            {
                $rmId = [];
                foreach ($randOffShelfList as $k => $row)
                {
                    $t = $row->time - $exp;
                    if ($t > 300)
                    {
                        # 程序每10秒会执行1次，5-10分钟内即将过期的挂单会在5分钟内会被执行30次，这样整体会有1/2的单在这个期间被下架
                        if (mt_rand(1, 60) !== 1)
                        {
                            continue;
                        }
                    }
                    elseif ($t > 100)
                    {
                        # 剩余时间在 300 - 100  之间会被执行 20 次，剩下的大部分会被随机到
                        if (mt_rand(1, 20) !== 1)
                        {
                            # 1/10 命中率
                            continue;
                        }
                    }

                    $list[$row->listId][] = $row->id;
                    $rmId[] = $k;     // 如果直接unset对应的key会产生一个大数组的 copy，这样性能不行，所以记录下key然后再批量 unset
                }

                # 删除
                foreach ($rmId as $k)
                {
                    unset($randOffShelfList[$k]);
                }
            }

            if ($list)
            {
                $task       = new ItemTask();
                $task->type = ItemTask::TYPE_CHANNEL_OFF_GOODS;

                foreach ($list as $taskId => $ids)
                {
                    $task->id  = $taskId;
                    $task->ids = $ids;
                    $task->sendToChannel();
                }
            }
        });

        swoole_timer_tick(1000 * 10, function() {
            $time1 = microtime(1);
            BoothOrderService::handleExpiredBoothOrder();
            $time2 = microtime(1);
            $t1 = $time2 - $time1;
            if (IS_DEBUG) {
                Server::$instance->debug("[摆摊过期] 程序耗时:{$t1}");
            }
        });

        swoole_timer_tick(1000 * 60, function() {
            $time1 = microtime(1);
            BoothOrderService::handlePayTimeoutRecord();
            $time2 = microtime(1);
            $t1 = $time2 - $time1;
            if (IS_DEBUG) {
                Server::$instance->debug("[摆摊支付过期] 程序耗时:{$t1}");
            }
        });
    }



    public function onPipeMessage($server, $fromWorkerId, $message, $fromServerId = -1)
    {
        switch ($message)
        {
            case 'reloadConfig':
                Server::loadTableConfig();
                break;
        }
    }
}