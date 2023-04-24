<?php
namespace RO\Trade;

use RO\Booth\BoothPlayerLock;
use RO\Booth\Tasks\BoothTask;

/**
 * 进程管理对象
 *
 * @package RO\Trade
 */
class WorkerTask extends \MyQEE\Server\WorkerTask
{
    /**
     * 是否停服
     *
     * 在收到一个停止服务器的请求时，此参数会被标记成 true
     *
     * @var bool
     */
    protected $isStop = false;

    /**
     * 根据 ItemListId 分类的列队
     *
     * @var \Ds\Map
     */
    protected static $allQueue;

    /**
     * 根据 ItemListId 分类的上架库存数
     *
     * @var \Ds\Map
     */
    protected static $goodsStocks;

    /**
     * 记录有数据的Item队列
     *
     * @var \Ds\Set
     */
    protected static $buyQueue;

    /**
     * 当前渠道通道
     *
     * @var \Swoole\Channel
     */
    protected static $channel;

    /**
     * 正在购买中
     *
     * @var \Ds\Map
     */
    protected static $buying;

    /**
     * 异步任务
     *
     * @var int
     */
    protected static $timer;

    /**
     * @var \RO\Booth\BoothPlayerLock
     */
    public static $boothLock;

    /**
     * 异步执行sql的文件列表
     *
     * @var array
     */
    static $delaySqlFiles = [];

    public function onStart()
    {
        # 加载策划表内容
        Server::loadTableConfig();

        if ($this->taskId === 0)
        {
            # 处理列表
            self::$Server->setProcessTag("task#{$this->taskId}-list");
            swoole_timer_tick(100, function()
            {
                ActList::outList();
            });
        }
        elseif ($this->taskId === 1)
        {
            self::$Server->setProcessTag("task#{$this->taskId}-delay-sql");

            # 读取所有文件
            foreach (glob(Server::$dataDir.'*.sql') as $item)
            {
                $name = basename($item);
                if (preg_match('#^000\-(.*)\-running\.sql$#', $name, $m))
                {
                    self::$delaySqlFiles[$m[1]] = $name;
                }
                elseif (!isset(self::$delaySqlFiles[$name]))
                {
                    self::$delaySqlFiles[$name] = true;
                }
            }

            swoole_timer_tick(1000, function()
            {
                if (count(self::$delaySqlFiles))foreach (self::$delaySqlFiles as $key => & $sqlFile)
                {
                    if ($sqlFile === true)
                    {
                        $sqlFile = Server::$dataDir . '000-'. substr($key, 0, -4) . '-running.sql';
                        if (!is_file($sqlFile))
                        {
                            $oldFile = Server::$dataDir . $key;
                            $rs      = rename($oldFile, $sqlFile);
                            if ($rs)
                            {
                                $oldPosFile = $oldFile . '.pos';
                                if (is_file($oldPosFile))
                                {
                                    rename($oldPosFile, $sqlFile . '.pos');
                                }
                            }
                            else
                            {
                                # 重命名失败
                                continue;
                            }
                        }
                    }

                    if (is_file($sqlFile))
                    {
                        $rs = Server::sqlFileExecute($sqlFile, Server::$mysqlMaster);
                        if ($rs)
                        {
                            # 成功则移除
                            unset(self::$delaySqlFiles[$key]);
                            Server::$instance->warn("[执行SQL] 执行SQL文件成功: $sqlFile");
                        }
                    }
                    else
                    {
                        unset(self::$delaySqlFiles[$key]);
                        Server::$instance->warn("[执行SQL] 指定的文件不存在: $sqlFile");
                    }
                }
            });
        }
        else
        {
            self::$allQueue    = new \Ds\Map();
            self::$goodsStocks = new \Ds\Map();
            self::$buying      = new \Ds\Map();
            self::$buyQueue    = new \Ds\Set();
            self::$channel     = Server::$taskChannel->get($this->taskId);

            self::$allQueue->allocate(4096);
            self::$goodsStocks->allocate(8192);
            self::$buying->allocate(256);
            self::$buyQueue->allocate(1024);

            $this->timeTick(300, [$this, 'buyInQueue']);
            $this->timeTick(3050, [$this, 'cleanTimeoutBuying']);

            # 重新开启进程后自动修正在售数
            swoole_timer_after(1000 * 60 + ($this->taskId * 10), [$this, 'checkBuyingCount']);
        }

        Server::initConnection();
        self::$boothLock = new BoothPlayerLock();
    }

    public function onTask($server, $taskId, $fromId, $task, $fromServerId = -1)
    {
        if (is_object($task))
        {
            try
            {
                if ($task instanceof ItemTask)
                {
                    switch ($task->type)
                    {
                        case ItemTask::TYPE_BUY:
                            # 用户请求购买
                            if (true === $this->isStop)
                            {
                                # 已停止
                                return;
                            }

                            $this->addToQueue($task);
                            break;

                        case ItemTask::TYPE_BUY_ON_RETURN:
                            # 购买请求服务器返回
                            $itemList = Dao\ItemList::getById($task->id)->reload();
                            if (!$itemList)continue;

                            # 是否公示购买
                            $isPublicityBuy = $itemList->isPublicity == 1 && $itemList->getEndTime() > 0 ? true : false;

                            $cmd         = new \RO\Cmd\BuyItemRecordTradeCmd();
                            $charId      = $cmd->charid = $task->forward->charId;
                            $cmd->ret    = $task->ret;

                            if ($isPublicityBuy)
                            {
                                # 公示购买
                                ActBuy::onPayPublicityBuy($itemList, $task, $cmd);
                            }
                            else
                            {
                                ActBuy::onPayInTask($itemList, $this->getListStocks($itemList), $task, $cmd);

                                $buy = self::$buying[$task->id]->get($charId, null);
                                if (null !== $buy)
                                {
                                    # 减去购买中计数
                                    unset(self::$buying[$task->id][$charId]);
                                    self::$buying[$task->id]['count'] -= $buy->key;       # value: $buy->key, time: $buy->value
                                    Server::$itemBuying->set($task->id, ['count' => self::$buying[$task->id]['count']]);
                                }
                            }

                            break;

                        case ItemTask::TYPE_GOODS_CANCEL:
                            if (true === $this->isStop)
                            {
                                # 已停止
                                return;
                            }

                            # 物品下架，see AtcShell.php line 673
                            if ($rs = ActSell::doCancelInTask($task->gid, $task->fd, $task->fromId))
                            {
                                $itemList = Dao\ItemList::getById($task->id, true);
                                if (false !== $itemList)
                                {
                                    $itemList = $itemList->reload();
                                    Dao\Goods::removeGoodsInStockMap($itemList, $task->gid, $this->getListStocks($itemList));
                                }
                            }

                            break;

                        default:
                            Server::$instance->warn("未知任务 stdClass 数据: ". json_encode($task));
                            break;
                    }
                }
                else if ($task instanceof BoothTask)
                {
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug('[摆摊任务] task ' . get_class($task) . ' is running');
                    }
                    $result = $task->execute($taskId);
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug('[摆摊任务] task ' . get_class($task) . ' is finish');
                    }
                    if ($result !== null) {
                        $this->finish($result);
                    }
                }
                else
                {
                    Server::$instance->warn("未知任务数据: ". print_r($task, true));
                }
            }
            catch (\Exception $e)
            {
                Server::$instance->warn($e->getMessage());
            }
        }
        else
        {
            $taskArr = explode('|', $task);
            switch ($taskArr[0])
            {
                case 'stop':
                    # 停止服务
                    $this->isStop = true;
                    $this->finish('ok');
                    break;

                case 'sql':
                    # 执行延迟执行的SQL
                    $this->runDelaySQL($taskArr[1]);

                    break;

                case 'list.reload':
                    if ($this->taskId != 0)
                    {
                        Server::$instance->warn("清理列表缓存，数据投递错误，当前taskId不是0，是 {$this->taskId}");
                        return;
                    }
                    ActList::cleanCache();

                    break;

                case 'exit':
                    exit();

                default:

                    # 其它方式处理的数据
                    $len = strlen($task);
                    Server::$instance->warn("未知任务数据($len): ". ($len > 100 ? substr($task, 0, 100)."... (len:$len)" : $task));
                    break;
            }
        }
    }

    public function onPipeMessage($server, $fromWorkerId, $message, $fromServerId = -1)
    {
        switch ($message)
        {
            case 'reloadConfig':
                # 重新加载配置
                Server::loadTableConfig();
                break;

            default:
                $this->warn('未知Task进程 onPipeMessage 数据');
                print_r($message);
                break;
        }
    }

    /**
     * 获取库存
     *
     * @param Dao\ItemList $itemList
     * @return bool|\Ds\Map|mixed
     */
    protected function getListStocks(Dao\ItemList $itemList)
    {
        $itemListId = $itemList->id;

        if (isset(self::$goodsStocks[$itemListId]))
        {
            return self::$goodsStocks[$itemListId];
        }

        # 读取库存
        $stockMap = Dao\Goods::getGracefulStock($itemList);

        if (false !== $stockMap)
        {
            self::$goodsStocks[$itemListId] = $stockMap;

            # 重置库存数
            $itemList->setStock($stockMap['stock'], true);
        }
        else
        {
            $stockMap          = new \Ds\Map();
            $stockMap['list']  = new \Ds\Map();
            $stockMap['stock'] = 0;
            $stockMap['error'] = true;

            Server::$instance->warn("Get stock error, $itemListId");
            return $stockMap;
        }

        return $stockMap;
    }

    /**
     * 添加到处理队列
     *
     * @param ItemTask $task
     */
    protected function addToQueue(ItemTask $task)
    {
        $id = $task->id;
        if (isset(self::$allQueue[$id]))
        {
            $obj   = self::$allQueue[$id];
            $queue = $obj['queue'];
        }
        else
        {
            $obj                 = new \Ds\Map();
            $obj['id']           = $id;
            $obj['queue']        = $queue = new \SplQueue();
            self::$allQueue[$id] = $obj;
        }

        # 任务数递增
        Server::$taskStatus->incr($this->taskId, 'queue');

        # 加入到按TaskId分组的队列里
        $queue->enqueue($task->data);

        # 将此队列加入到需要处理的对象里，已经存在的不会重复设置
        self::$buyQueue->add($obj);

        //if (!self::$timer)
        //{
        //    # 优化并发执行效率
        //    # 如果没有异步任务定时器，加到异步列队里，200ms 后批量执行
        //    self::$timer = swoole_timer_after(200, [$this, 'buyInQueue']);
        //    Server::$instance->debug("---------------增加异步回调事件完成");
        //}
    }

    /**
     * 在任务处理队列里进行处理
     */
    public function buyInQueue()
    {
        # 清除定时器
        //self::$timer = null;
        //Server::$instance->debug("---------------执行");

        $i = 0;
        $c = null;
        try
        {
            while (false !== ($data = self::$channel->pop()))
            {
                if (null === $c)
                {
                    $c = self::$channel->stats()['queue_num'] + 1;
                }
                $i++;

                if (is_object($data) && $data instanceof ItemTask)
                {
                    switch ($data->type)
                    {
                        case ItemTask::TYPE_CHANNEL_GOODS_NEW:
                        case ItemTask::TYPE_CHANNEL_GOODS_RESELL:
                            $this->changeStock($data);
                            break;

                        case ItemTask::TYPE_CHANNEL_OFF_GOODS:
                            $this->offStock($data);
                            break;

                        case ItemTask::TYPE_CHANNEL_RESET_STOCK:
                            # 重新加载库存
                            if (isset(self::$goodsStocks[$data->id]))
                            {
                                # 移除
                                self::$goodsStocks[$data->id]->clear();
                                unset(self::$goodsStocks[$data->id]);
                            }

                            $itemList = Dao\ItemList::getById($data->id, true);
                            if (false !== $itemList)
                            {
                                $this->getListStocks($itemList);
                            }

                            if ($data->isPub && isset(self::$goodsStocks[$data->id]))
                            {
                                # 公示物品再移除
                                self::$goodsStocks[$data->id]->clear();
                                unset(self::$goodsStocks[$data->id]);
                            }
                            break;

                        case ItemTask::TYPE_CHANNEL_DEL_STOCK_CACHE:
                            # 移除库存缓存
                            if (isset(self::$goodsStocks[$data->id]))
                            {
                                # 移除
                                self::$goodsStocks[$data->id]->clear();
                                unset(self::$goodsStocks[$data->id]);
                            }
                            break;
                    }
                }
                if ($i == $c)break;
            }
        }
        catch (\Exception $e)
        {
            Server::$instance->warn($e->getFile().'('.$e->getLine().') '.$e->getMessage());
        }

        if (self::$buyQueue->isEmpty())return;

        $time         = time();
        $failCmd      = new \RO\Cmd\BuyItemRecordTradeCmd();
        $failCmd->ret = \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL;

        foreach (self::$buyQueue as $map)
        {
            /**
             * @var \Ds\Map $map
             * @var \SplQueue $queue
             */
            $queue      = $map['queue'];
            $itemListId = $map['id'];
            $itemList   = Dao\ItemList::getById($itemListId, true);
            if (true === $this->isStop || false === $itemList)
            {
                # 给所有客户端返回错误信息
                while (!$queue->isEmpty())
                {
                    try
                    {
                        $buyInfo = $queue->dequeue();
                        ActBuy::buyInTaskFail($failCmd, $buyInfo[0], $buyInfo[1], $buyInfo[2]);
                    }
                    catch (\Exception $e){}
                }
                continue;
            }
            $itemList->reload();

            if ($itemList->isPublicity == 1 && $itemList->getEndTime() > 0)
            {
                if ($itemList->getEndTime() < $time)
                {
                    # 公示已到期，但是还没有执行下架操作的
                    while (!$queue->isEmpty())
                    {
                        try
                        {
                            $buyInfo = $queue->dequeue();
                            ActBuy::buyInTaskFail($failCmd, $buyInfo[0], $buyInfo[1], $buyInfo[2], ActBuy::SYS_MSG_BUY_PENDING_WAS_LOCKED);
                        }
                        catch (\Exception $e){}
                    }
                    continue;
                }

                # 公示购买不判断这个
                $oldBuying = 0;
                $buying    = ['count' => 0];
            }
            else
            {
                # 正在购买中的数量
                $buying = self::$buying->get($itemListId, null);
                if (null === $buying)
                {
                    $myCount = Server::$itemBuying->get($itemListId);
                    $buying  = self::$buying[$itemList->id] = new \Ds\Map(['count' => $myCount ? $myCount['count'] : 0]);
                }
                $oldBuying = $buying['count'];
            }

            # 读取库存
            $stock = $this->getListStocks($itemList);
            if ($stock->get('error', false) === true)
            {
                # 获取库存错误
                while (!$queue->isEmpty())
                {
                    try
                    {
                        $buyInfo = $queue->dequeue();
                        ActBuy::buyInTaskFail($failCmd, $buyInfo[0], $buyInfo[1], $buyInfo[2], ActBuy::SYS_MSG_DB_ERROR);
                    }
                    catch (\Exception $e){}
                }
                continue;
            }

            if ($stock['stock'] != $itemList->stock)
            {
                # 库存不相同，重新读取
                $old = $itemList->stock;
                unset(self::$goodsStocks[$itemListId]);
                $stockNew = $this->getListStocks($itemList);            # 重新读取库存
                $itemList = Dao\ItemList::getById($itemListId);         # 重新加载
                Server::$instance->warn("库存不一致, 已重新加载 itemListId: $itemListId, task内存: {$stock['stock']}, 共享内存: {$old}；新库存: task:{$stockNew['stock']}, share: {$itemList->stock}");
            }

            $price = $itemList->getPrice();

            while (!$queue->isEmpty())
            {
                ActBuy::buyInTask($itemList, $buying, $price, $queue->dequeue(), $failCmd);
            }

            if ($buying['count'] > $oldBuying)
            {
                # 更新购买中计数器
                Server::$itemBuying->set($itemListId, ['count' => $buying['count']]);
            }
        }

        # 重置列队数
        Server::$taskStatus->set($this->taskId, ['queue' => 0]);

        self::$buyQueue = new \Ds\Set();
    }

    /**
     * 移除过期的购买
     */
    public function cleanTimeoutBuying()
    {
        $time = time() - 60;
        foreach (self::$buying as $itemId => $list)
        {
            $exp = 0;
            $ids = [];
            foreach ($list as $charId => $item)
            {
                if (is_int($item))continue;       // $list['count'] = 0 计数

                if ($item->value > $time)
                {
                    $ids[] = $charId;
                    $exp  += $item->key;
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[清理] 移除过期的购买请求, charId: $charId, count: {$item->key}, createTime: " . date('Y-m-d H:i:s', $item->value));
                    }
                }
                else
                {
                    break;
                }
            }

            if ($exp > 0)
            {
                foreach ($ids as $id)
                {
                    unset($list[$id]);
                }
                $list['count'] -= $exp;
            }
        }
    }

    /**
     * 启动后1分钟检查购买数是否正确
     */
    public function checkBuyingCount()
    {
        foreach (self::$buying as $listId => $list)
        {
            $count = 0;
            foreach ($list as $item)
            {
                if (is_int($item))continue;       // $list['count'] = 0 计数
                $count += $item->key;
            }

            if ($list['count'] != $count)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[检查] 购买中数值不对，已调整, 进程中: {$count}, 内存中 {$list['count']}");
                }

                $list['count'] = $count;
                Server::$itemBuying->set($listId, ['count' => $count]);
            }
        }
    }


    /**
     * 修改库存（上架、过期物品重新上架）
     *
     * @param ItemTask $task
     */
    protected function changeStock(ItemTask $task)
    {
        # $task->itemId  物品id
        # $task->gid     goodsId
        # $task->stock   新库存
        $itemList = Dao\ItemList::getById($task->id, true)->reload();
        if (false === $itemList)return;

        if (isset(self::$goodsStocks[$task->id]))
        {
            # 获取旧数据
            $stockMap = self::$goodsStocks[$task->id];
        }
        else
        {
            # 获取新库存数据
            $stockMap = self::getListStocks($itemList);
            if (isset($stockMap['error']) && true === $stockMap['error'])
            {
                return;
            }
        }

        $goodsId = $task->gid;
        $list    = $stockMap['list'];
        if (isset($list[$goodsId]))
        {
            $diff = $task->stock - $list[$goodsId];
        }
        else
        {
            $list[$goodsId] = $task->stock;
            $diff           = $task->stock;

            # 非堆叠的随机排序
            if (false == $itemList->isOverlap)
            {
                $list->ksort(function($a, $b)
                {
                    $aa = mt_rand(0, 9999999);
                    $bb = mt_rand(0, 9999999);
                    if ($aa == $bb)
                    {
                        return 0;
                    }
                    return ($aa < $bb) ? -1 : 1;
                });
            }
        }

        if ($diff !== 0)
        {
            # 更新库存值
            $stockMap['stock'] += $diff;
            $list[$goodsId]     = $task->stock;
            $itemList->incrStock($diff);
        }
    }

    /**
     * 自动批量下架过期物品
     *
     * @param ItemTask $task
     */
    protected function offStock(ItemTask $task)
    {
        if (!isset(self::$goodsStocks[$task->id]))
        {
            # 读取库存
            $itemList = Dao\ItemList::getById($task->id, true);
            if (false === $itemList)return;

            $stockMap = $this->getListStocks($itemList);
            if ($stockMap->get('error', false) == true)
            {
                # 有错
                return;
            }
        }
        else
        {
            $stockMap = self::$goodsStocks[$task->id];
        }

        /**
         * @var \Ds\Map $stockMap
         * @var \Ds\Map $list
         */
        $list     = $stockMap['list'];
        $oldStock = $stockMap['stock'];

        if ($task->ids)
        {
            $lostIds = [];
            foreach ($task->ids as $id)
            {
                if (isset($list[$id]))
                {
                    /**
                     * @var \Ds\Pair $obj
                     */
                    $stockMap['stock'] -= $list[$id];
                    unset($stockMap['list'][$id]);
                }
                else
                {
                    $lostIds[] = $id;
                }
            }

            if ($lostIds)
            {
                Server::$instance->info("[自动下架] 批量下架库存不存在这些挂单id，可能玩家已下架: ". implode(',', $lostIds));
            }

            # 给数据库的数据设置过期
            Dao\Goods::setStatusExpired($task->ids);

            //if (IS_DEBUG)
            //{
            //    Server::$instance->debug("[自动下架] " . implode(', ', $task->ids));
            //}
        }

        # 更新库存数
        if ($oldStock != $stockMap['stock'])
        {
            Dao\ItemList::getById($task->id)->setStock($stockMap['stock']);
        }
    }

    protected function runDelaySQL($file)
    {
        if (!$file)return;

        if (!isset(self::$delaySqlFiles))
        {
            self::$delaySqlFiles[$file] = true;
        }
    }
}