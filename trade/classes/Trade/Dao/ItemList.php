<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/9
 * Time: 下午5:02
 */

namespace RO\Trade\Dao;

use RO\Booth\BoothOrderService;
use RO\Booth\Dao\BoothOrder;
use RO\Cmd\ItemData;
use RO\Cmd\NameInfo;
use RO\Cmd\NameInfoList;
use RO\Trade\ActBuy;
use RO\Trade\ActSell;
use RO\Trade\Item;
use RO\Trade\Player;
use RO\Trade\Server;
use RO\Trade\ZoneForwardScene;
use RO\ZoneForward;

class ItemList extends Dao
{
    public $id;

    public $itemKey;

    public $itemId;

    public $stock;

    public $boothStock;

    /** @var int 是否公示 */
    public $isPublicity;

    /** @var int 公示结束时间 */
    public $endTime;

    /** @var int 公示开始时间 */
    public $startTime;

    public $isOverlap;

    /**
     * 公示价格
     *
     * @var int
     */
    public $pubPrice;

    /**
     * 公示期购买人数
     *
     * @var int
     */
    public $pubBuyPeople;

    /**
     * 精炼等级
     *
     * @var int
     */
    public $refineLv;

    /**
     * 是否损坏
     *
     * @var int
     */
    public $isDamage;

    /**
     * 是否好的附魔
     *
     * @var int
     */
    public $isGoodEnchant;

    /** @var ItemData $itemData */
    public $itemData;

    /** @var Item  */
    public $item;

    /**
     * 公示期延长时间
     *
     * @var int
     */
    public $delayTime = 0;

    /**
     * 服务器启动后是否已重置过库存
     *
     * @var int
     */
    public $isResetStock = 0;

    protected $_cached = false;

    protected static $TableName = 'trade_item_list';

    /**
     * 缓存的对象
     *
     * @var array
     */
    protected static $instance = [];

    protected static $Fields = [
        'id'             => 'id',
        'itemKey'        => 'item_key',
        'itemId'         => 'item_id',
        'stock'          => 'stock',
        'isPublicity'    => 'is_publicity',
        'pubBuyPeople'   => 'pub_buy_people',
        'pubPrice'       => 'pub_price',
        'startTime'      => 'start_time',
        'endTime'        => 'end_time',
        'refineLv'       => 'refine_lv',
        'isDamage'       => 'is_damage',
        'isGoodEnchant'  => 'is_good_enchant',
        'itemData'       => 'item_data',
        'delayTime'      => 'delay_time',
    ];

    function __construct($data, $itemKey)
    {
        $this->itemKey = $itemKey;
        parent::__construct($data);
    }

    /**
     * 获取价格（公示期物品价格会返回公示期物品价格）
     *
     * @return int
     */
    public function getPrice()
    {
        if ($this->isPublicity == 1)
        {
            return $this->pubPrice;
        }

        return $this->getItem() ? $this->item->getPrice() : 0;
    }

    /**
     * 获取 ItemData
     *
     * @return null|\RO\Cmd\ItemData
     */
    public function getItemData()
    {
        if (null === $this->itemData || '' === $this->itemData)return null;

        if (is_string($this->itemData))
        {
            try
            {
                $this->itemData = new \RO\Cmd\ItemData($this->itemData);
            }
            catch (\RuntimeException $e)
            {
                $this->itemData = null;
                Server::$instance->warn("解析 itemList: {$this->id} itemData 数据失败");
            }
        }

        return $this->itemData;
    }

    /**
     * 设置库存
     *
     * @param int $stock
     * @return bool
     */
    public function setStock($stock, $isReset = false)
    {
        $rs = ItemList::saveItemStock($this->itemId, $this->itemKey, $this->id, $stock, $stock - $this->stock());
        if (false !== $rs)
        {
            $this->stock = $this->_old['stock'] = $stock;
        }
        if ($isReset)
        {
            if (Server::$itemList->set($this->itemKey, ['isResetStock' => 1]))
            {
                $this->isResetStock = 1;
            }
        }
        return $rs;
    }

    /**
     * 递增库存
     *
     * @param int $incrBy
     * @return bool|int
     */
    public function incrStock($incrBy)
    {
        $rs = ItemList::incrItemStock($this->itemId, $this->itemKey, $this->id, $incrBy);
        if (false !== $rs)
        {
            $this->stock = $this->_old['stock'] = $rs;
        }
        return $rs;
    }

    /**
     * 递减库存
     *
     * @param int $decrBy
     * @return bool|int
     */
    public function decrStock($decrBy)
    {
        $rs = ItemList::decrItemStock($this->itemId, $this->itemKey, $this->id, $decrBy);
        if (false !== $rs)
        {
            $this->stock = $this->_old['stock'] = $rs;
        }
        return $rs;
    }

    /**
     * 获取库存
     *
     * 和 `$this->stock` 的差异在于，如果这个对象是一个被 cached 的对象，通过 `$this->stock()` 方法可以获取到更新后的 stock
     *
     * @return int
     */
    public function stock()
    {
        if (true === $this->_cached)
        {
            $this->reload();
        }

        return $this->stock;
    }

    /**
     * 从数据库里读取库存数重置库存
     *
     * @return bool
     */
    public function resetStockFromDB()
    {
        $stock = Goods::getDbStockByItemListId($this->id);

        if (false === $stock)
        {
            return false;
        }
        else
        {
            return $this->setStock($stock, true);
        }
    }

    /**
     * 加载共享内存
     *
     * @return $this
     */
    public function reload()
    {
        $arr = Server::$itemList->get($this->itemKey);
        unset($arr['item_data']);

        if ($arr['stock'] < 0)
        {
            $arr['stock'] = 0;
            Server::$instance->warn("库存数溢出, itemId: {$this->itemId}, itemKey: {$this->itemKey}, stock: {$this->stock}");

            # 重置库存
            $task       = new \RO\Trade\ItemTask();
            $task->id   = $this->id;
            $task->type = \RO\Trade\ItemTask::TYPE_CHANNEL_RESET_STOCK;
            $task->isPub = $this->isPublicity ? 1 : 0;
            $task->sendToChannel();
        }

        if (null != $this->item)
        {
            $this->item->reload();
        }

        Server::$instance->debug("更新内存", $arr);
        $this->setData($arr);

        return $this;
    }

    /**
     * 获取内存item对象
     *
     * @return bool|Item
     */
    public function getItem()
    {
        if (null === $this->item)
        {
            $this->item = Item::get($this->itemId, $this->getItemData());
        }

        return $this->item;
    }

    public function insert()
    {
        if (null === $this->isOverlap)
        {
            $this->isOverlap = $this->getItem()->isOverlap;
        }

        if ($this->itemKey == $this->itemId)
        {
            $this->id = $this->itemId;
        }

        if (1 == $this->isOverlap)
        {
            $this->itemData = null;
        }
        else
        {
            if (!\RO\Utils\ROConfig::isGoodEnchant($this->getItemData()))
            {
                # 清除掉附魔属性
                $itemData = $this->getItemData();

                if ($itemData)
                {
                    $obj            = clone $itemData;
                    $this->itemData = $obj;
                    $this->itemData->clearEnchant();
                }
                $this->isGoodEnchant = 0;
            }
            else
            {
                $this->isGoodEnchant = 1;
            }
        }

        $rs = parent::insert();
        if (false === $rs && Server::$mysqlMaster->errno === 1062)
        {
            # 唯一索引冲突
            Server::$instance->warn("[ItemList] 忽略插入冲突数据, item key:{$this->itemKey}");
            $rs = true;
        }

        if ($rs)
        {
            Server::$itemListId2KeyMap->set($this->id, ['key' => $this->itemKey]);
            Server::$itemList->set($this->itemKey, $this->getTableData());
        }

        return $rs;
    }

    /**
     * @return bool|int
     */
    public function delete()
    {
        $rs = parent::delete();
        if ($rs)
        {
            Server::$itemList->del($this->itemKey);
            Server::$itemListId2KeyMap->del($this->id);
        }

        return $rs;
    }

    public function update()
    {
        $this->_old['item_key'] = $this->itemKey;   # 此字段不用更新

        $changed = [];
        foreach (static::$Fields as $key => $field)
        {
            $now = $this->$key;
            $old = isset($this->_old[$field]) ? $this->_old[$field] : null;
            if ($now !== $old)
            {
                $oldQuoted = self::_quoteValue($old);
                $nowQuoted = self::_quoteValue($now);

                # 如果对象、数组序列化后也不相同
                if ($oldQuoted !== $nowQuoted)
                {
                    if ($now instanceof \DrSlump\Protobuf\Message)
                    {
                        $changed[$field] = $now->serialize();
                    }
                    else
                    {
                        $changed[$field] = self::_getFieldTypeValue($now);
                    }
                }
            }
        }

        $oldStock = isset($this->_old['stock']) ? $this->_old['stock'] : $this->stock;
        $rs = parent::update();
        if ($rs)
        {
            Server::$itemList->set($this->itemKey, $changed);
            if ($oldStock != $this->stock)
            {
                # 推送到更新进程
                Server::$stockUpdateChannel->push(pack('LQ', $this->id, $this->stock));

                # 修改过库存
                $diff = $this->stock - $oldStock;
                $s = Server::$item->incr($this->itemId, 'stock', $diff);

                if ($s !== false && $s < 0)
                {
                    list($count) = Item::resetStockAndPubNum($this->itemId);
                    Server::$instance->info("[item库存数调整] ItemId: {$this->itemId}, 原库存: {$oldStock}, 新库存: {$this->stock}, item库存更新后: {$s}, 递增数: {$diff}, 重置后: {$count}");
                }
            }
        }

        return $rs;
    }

    /**
     * 尝试重新开启公示
     *
     * @param int|null $pubPrice
     * @return bool
     */
    public function tryOpenPublicity(int $pubPrice = null)
    {
        if ($this->isPublicity != 1 && true == $this->checkIsPublicity())
        {
            $this->startTime    = time();
            $this->endTime      = time() + $this->getItem()->publicityShowTime;
            $this->isPublicity  = 1;
            $this->pubPrice     = $pubPrice === null ? $this->getItem()->getPrice() : $pubPrice;
            $this->pubBuyPeople = 0;
            $this->delayTime    = 0;

            # 公示数递增
            Server::$item->incr($this->itemId, 'publicityNum', 1);
            return true;
        }

        return false;
    }

    /**
     * 更新价格是最后时间
     *
     * @param $price
     * @param $endTime
     * @return bool
     */
    public function updatePriceAndEndTime($price, $endTime)
    {
        $rs = Server::$mysqlMaster->query("UPDATE `trade_item_list` SET `pub_price` = '{$price}', `end_time` = '{$endTime}' WHERE `id` = '{$this->id}'");
        if ($rs)
        {
            $this->pubPrice = $price;
            $this->endTime  = $endTime;
            Server::$itemList->set($this->itemKey, ['pubPrice' => $price, 'endTime' => $endTime]);

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * 获取公示期结束时间（包含延长时间）
     *
     * @return int
     */
    public function getEndTime()
    {
        return $this->endTime + $this->delayTime;
    }

    /**
     * 检查是否可以公示
     *
     * @return bool
     */
    public function checkIsPublicity()
    {
        if ($this->isPublicity == 1 && $this->getEndTime() > time())
        {
            return true;
        }

        $item = $this->getItem();

        if (!$item->isOverlap && \RO\Utils\ROConfig::isGoodEnchant($this->getItemData()))
        {
            $stock = $this->stock();
            if ($stock > 0)
            {
                return false;
            }

            if ($this->isResetStock == 0)
            {
                # 防止0库存导致程序判断错误，此时立即重置库存数
                $this->resetStockFromDB();
                $stock = $this->stock;
            }

            if ($stock > 0)
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        //成交量小于配置的不公示
        if ($item->soldNum < $item->exchangeNum)
        {
            return false;
        }

        $stock = $this->stock();
        if ($stock > 0)
        {
            return false;
        }

        if ($this->isResetStock == 0)
        {
            # 防止0库存导致程序判断错误，此时立即重置库存数
            $this->resetStockFromDB();
            $stock = $this->stock;
        }

        if ($stock > 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    protected function getTableData()
    {
        return [
            'id'              => $this->id,
            'item_id'         => $this->itemId,
            'stock'           => $this->stock,
            'boothStock'      => $this->boothStock,
            'is_publicity'    => $this->isPublicity,
            'pub_price'       => $this->pubPrice,
            'pub_buy_people'  => $this->pubBuyPeople,
            'end_time'        => $this->endTime,
            'start_time'      => $this->startTime,
            'delay_time'      => $this->delayTime,
            'refine_lv'       => $this->refineLv,
            'is_damage'       => $this->isDamage,
            'is_good_enchant' => $this->isGoodEnchant,
            'isOverlap'       => $this->isOverlap,
            'isResetStock'    => $this->isResetStock,
            'item_data'       => $this->itemData ? (is_string($this->itemData) ? $this->itemData : $this->itemData->serialize()) : '',
        ];
    }

    /**
     * 下架公示到期挂单
     *
     * @return bool
     */
    public function offShelf()
    {
        if (!$this->id)return false;
        if (Server::$mysqlMaster->begin_transaction())
        {
            try
            {
                $sql = "UPDATE `" . Goods::getTableName() ."` SET `end_time` = 0, `is_publicity` = 0 WHERE `item_list_id` = '{$this->id}' AND `status` = " . Goods::STATUS_SELLING;
                $goodsRs  = Server::$mysqlMaster->query($sql);
                if(!$goodsRs)
                {
                    throw new \Exception("[交易所-公示处理] 将所有公示挂单转为普通挂单失败! 公示id:{$this->id}");
                }

                $this->isPublicity = 0;
                $this->pubBuyPeople = 0;
                $this->delayTime    = 0;

                if ($this->update())
                {
                    # 删除公示购买人数缓存
                    try
                    {
                        Server::$redis->del('pub_buy_count_'. $this->id);
                    }
                    catch (\Exception $e)
                    {
                        Server::$instance->warn("[删除公示购买人数缓存] 发生异常, 错误信息: {$e->getMessage()}");
                    }

                    # 提交事务
                    if (Server::$mysqlMaster->commit())
                    {
                        $rs = Server::$item->decr($this->itemId, 'publicityNum', 1);
                        if ($rs !== false && $rs < 0)
                        {
                            Server::$item->set($this->itemId, ['publicityNum' => 0]);
                            Server::$instance->warn("Item 公示数异常, ItemId: {$this->itemId}, publicity number: {$rs}");
                        }
                        return true;
                    }
                    else
                    {
                        Server::$instance->warn("[交易所-公示处理] 提交事务失败 公示id:{$this->id}");
                        return false;
                    }
                }
                else
                {
                    throw new \Exception('[交易所-公示处理] itemList更新失败');
                }
            }
            catch (\Exception $e)
            {
                Server::$mysqlMaster->rollback();
                Server::$instance->warn($e->getMessage());
            }

            return false;
        }
        else
        {
            Server::$instance->warn("[交易所-公示处理] 申请事务失败 公示id:{$this->id}");
            return false;
        }
    }

    /**
     * 公示物品分发
     *
     * @param Itemlist $itemList
     * @param array $notifyChars 需用通知的用户id
     * @return bool|string
     */
    public static function goodsDispatch(ItemList $itemList, &$notifyChars)
    {
        if ($itemList->isPublicity == 0) return false;

        $time = microtime(1);

        Server::$instance->debug("[交易所-公示分发] 公示id:{$itemList->id} 抢购物品:{$itemList->itemId} 总库存数:{$itemList->stock} 开始分单处理...");

        $buyList  = [];
        # 玩家购买总数量
        $buyTotalCount = 0;
        $rs      = Server::$mysqlMaster->query("SELECT * FROM `trade_record_bought` WHERE `publicity_id` = '{$itemList->id}' AND `status` = " . RecordBought::STATUS_PUBLICITY_PAY_SUCCESS);
        if ($rs && $rs->num_rows)
        {
            while ($data = $rs->fetch_assoc())
            {
                $buy = new RecordBought($data);
                $buyList[] = $buy;
                $buyTotalCount += $buy->totalCount;
            }

            $rs->free();
        }
        else
        {
            if ($itemList->offShelf())
            {
                Server::$instance->info("[交易所-公示处理] 公示物品无人购买, 下架所有挂单成功! 公示id:{$itemList->id}");
            }
            else
            {
                Server::$instance->warn("[交易所-公示处理] 公示物品下架失败! 公示id:{$itemList->id}");
            }
            return true;
        }

        # 挂单列表
        $sellList = [];
        $sellTotalCount = 0;
        $rs2      = Server::$mysqlMaster->query("SELECT * FROM `trade_goods` WHERE `item_list_id` = '{$itemList->id}' AND `is_publicity` = 1 AND `stock` > 0 AND `end_time` > 0 AND `status` = " . Goods::STATUS_SELLING);
        if ($rs2 && $rs2->num_rows)
        {
            while($data = $rs2->fetch_assoc())
            {
                $goods = new Goods($data);
                $sellList[$goods->id] = $goods;
                $sellTotalCount += $goods->stock;
            }
        }
        else
        {
            Server::$instance->warn("[交易所-公示分发] 公示id:{$itemList->id} 找不到任何出售该公示物品的记录");
        }

        $itemListStock = $itemList->stock();
        if ($itemListStock != $sellTotalCount)
        {
            $itemList->setStock($sellTotalCount);
            Server::$instance->warn("[交易所-公示分发] 公示id:{$itemList->id} itemList中的库存与数据库汇总的公示挂单库存数不一致, itemListStock:{$itemListStock}, 挂单stock:{$sellTotalCount}");
        }

        $buyAssignList = [];
        $sellAssignList = [];

        $time2 = microtime(1);
//        $t = 0;
//        $c = 0;

        $buyerIndexCount = count($buyList) - 1;

        /**
         * 一个一个物品分配
         * @var Goods $sell
         * @var RecordBought $buy
         */
        while(true)
        {
//            $c++;
//            $time5 = microtime(1);
            if ($buyTotalCount === 0 || $sellTotalCount === 0)
            {
                break;
            }

            // 随机抽取买家
            $random = mt_rand(0, $buyerIndexCount);
            $buy  = $buyList[$random];

            // 判断该买家是否达到总抢购数
            if ($buy->count == $buy->totalCount)
            {
                continue;
            }

            $buy->count++;
            // 总买家抢购数
            $buyTotalCount--;

            $sell = current($sellList);
            $sell->stock = $sell->stock - 1;
            --$sellTotalCount;
            if ($sell->stock == 0)
            {
                next($sellList);
            }

//            if (IS_DEBUG)
//            {
//                Server::$instance->debug("[交易所-公示分单] {$buy->playerName} (ID:{$buy->charId}) 从 {$sell->playerName} (ID:{$sell->charId}) 抢购到1个物品");
//                Server::$instance->debug("[交易所-公示分单] 抢购订单ID: {$buy->id}, 当前已抢购数:{$buy->count}, 总抢购数:{$buy->totalCount}");
//                Server::$instance->debug("[交易所-公示分单] 卖家挂单ID: {$sell->id}, 卖家剩余库存: {$sell->stock}, 原库存数: {$sell->count}");
//            }

            $buyId = (int) $buy->id;
            $sellId = (int) $sell->id;

            if (isset($buyAssignList[$buyId][$sell->charId]))
            {
                $buyAssignList[$buyId][$sell->charId]['count']++;
            }
            else
            {
                $buyAssignList[$buyId][$sell->charId] = [
                    'id'     => $sellId,
                    'name'   => $sell->playerName,
                    'zoneId' => $sell->playerZoneId,
                    'count'  => 1,
                ];
            }

            if (isset($sellAssignList[$sellId][$buy->charId]))
            {
                $sellAssignList[$sellId][$buy->charId]['count']++;
            }
            else
            {
                $sellAssignList[$sellId][$buy->charId] = [
                    'id'     => $buyId,
                    'name'   => $buy->playerName,
                    'zoneId' => $buy->playerZoneId,
                    'count'  => 1,
                ];
            }

//            $t += microtime(1) - $time5;
//            Server::$instance->debug('分配一个物品耗时:'. (microtime(1) - $time5));
        }

//        Server::$instance->debug('[交易所-公示分发] 总次数:' . $c . ' 平均耗时:' . ($t / $c));
        Server::$instance->debug('[交易所-公示分发] 内存分单耗时:'. (microtime(1) - $time2));

        // 下面对交易记录,写入文件后,由别的进程定时执行。
        $time3 = microtime(1);

        /**
         * @var Goods $goods
         * @var RecordBought $record
         * @var array $refundRecords
         */
        try
        {
            /**
             * 批量写入买家更新记录sql语句
             *
             * @param RecordBought[] $records
             * @param string $tmpFile
             * @throws \Exception
             */
            $writeBuyUpdateSql = function($records, $tmpFile) {
                if(!empty($records))
                {
                    $updateSql = '';
                    $time = time();
                    foreach ($records as $record)
                    {
                        $updateSql .= "UPDATE `trade_record_bought` SET `status` = {$record->status}, "
                            . "`sellers_info` = " . ($record->sellersInfo === null ? 'null' : '0x' . bin2hex($record->sellersInfo->serialize())) . ", "
                            . "`seller_info` = " . ($record->sellerInfo === null ? 'null' : '0x' . bin2hex($record->sellerInfo->serialize())) . ", "
                            . "`goods_id` = '{$record->goodsId}', "
                            . "`is_many_people` = '{$record->isManyPeople}', "
                            . "`item_data` = " . ($record->getItemData() === null ? 'null' : '0x' . bin2hex($record->getItemData()->serialize())) . ", "
                            . "`count` = '{$record->count}', "
                            . "`can_give` = '" . ($record->canGive ? 1 : 0) .  "', "
                            . "`time` = '{$time}' "
                            . "WHERE `id` = '{$record->id}';" . PHP_EOL;
                    }
                    if (file_put_contents($tmpFile, $updateSql, FILE_APPEND) === false)
                    {
                        throw new \Exception('Sql语句插入临时文件失败');
                    }
                }
            };

            /**
             * 批量写入买家退款记录sql语句
             *
             * @param array $refundRecords
             * @param string $tmpFile
             * @throws \Exception
             */
            $writeRecordBuyInsertSql = function($refundRecords, $tmpFile)
            {
                if(!empty($refundRecords))
                {
                    $refundSql = 'INSERT IGNORE INTO `trade_record_bought` (`char_id`, `player_name`, `player_zone_id`, `goods_id`, `status`, `item_id`, `time`, `publicity_id`, `end_time`, `count`, `total_count`, `price`, `refine_lv`, `is_damage`, `is_many_people`, `seller_info`, `sellers_info`, `item_data`, `log_id`) VALUES ';
                    foreach ($refundRecords as $data)
                    {
                        $refundSql .= "(" . implode(',', $data) . "), ";
                    }
                    $refundSql = rtrim($refundSql, ', ') . ';' . PHP_EOL;
                    if (file_put_contents($tmpFile, $refundSql, FILE_APPEND) === false)
                    {
                        throw new \Exception('Sql语句插入临时文件失败');
                    }
                }
            };

            /**
             * 批量写入卖家记录sql语句
             *
             * @param array $records
             * @param string $tmpFile
             * @throws \Exception
             */
            $writeSellInsertSql = function($records, $tmpFile) {
                if (!empty($records))
                {
                    $values = '';
                    /** @var RecordSold $record */
                    foreach ($records as $record)
                    {
                        $buyerInfoHex = $record->buyerInfo === null ? 'null' : '0x' . bin2hex($record->buyerInfo->serialize());
                        $buyersInfoHex = $record->buyersInfo === null ? 'null' : '0x' . bin2hex($record->buyersInfo->serialize());
                        $logId = md5($record->charId . $record->itemId . $record->time . $record->price . mt_rand(0, 999999) . microtime());
                        $values .= "('{$record->charId}', '{$record->playerZoneId}', '{$record->goodsId}', '{$record->itemId}',"
                            . " '{$record->status}', '{$record->time}', '{$record->count}', '{$record->price}',"
                            . " '{$record->refineLv}', '{$record->isDamage}', '{$record->tax}', '{$record->isManyPeople}', "
                            . $buyerInfoHex . ", "
                            . $buyersInfoHex . ", "
                            . "'". $logId . "'), ";
                    }

                    $sql = 'INSERT IGNORE INTO `trade_record_sold` (`char_id`, `player_zone_id`, `goods_id`, `item_id`, `status`, `time`, `count`, `price`, `refine_lv`, `is_damage`, `tax`, `is_many_people`, `buyer_info`, `buyers_info`, `log_id`) VALUES ';
                    $sql .= rtrim($values, ', ') . ';' . PHP_EOL;
                    if (file_put_contents($tmpFile, $sql, FILE_APPEND) === false)
                    {
                        throw new \Exception('Sql语句插入临时文件失败');
                    }
                }
            };

            $tmpFile = Server::$dataDir . 'pub_' . $itemList->id . '_' . $itemList->endTime . '.tmp';
            # 清空临时文件
            if (false === file_put_contents($tmpFile, ''))
            {
                Server::$instance->warn("[交易所-公示分发] 清空临时文件失败. publicity_id:{$itemList->id} item_id:{$itemList->itemId}");
                return false;
            }

            if (!empty($buyAssignList))
            {
                $batchNum      = 0;
                $records       = [];
                $refundRecords = [];

                foreach ($buyList as $record)
                {
                    $zone = Server::$zone->get($record->playerZoneId);

                    # 通知场景服务器,更新交易记录
                    $notifyChars[$record->charId] = $record->playerZoneId;
//                $msg         = new \RO\Cmd\ListNtfRecordTrade();
//                $msg->charid = $record->charId;
//                $msg->type   = \RO\Cmd\EListNtfType::ELIST_NTF_MY_LOG;
//                if ($zone)
//                {
//                    ZoneForward::sendToUserByFd($msg, $msg->charid, $zone['fd'], $zone['fromId']);
//                }

                    // count等于0表示一个都没抢购成功
                    if ($record->count == 0)
                    {
                        if ($zone)
                        {
                            ZoneForward::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $record->charId, ActBuy::SYS_MSG_BUY_PUBLICITY_FAIL, [
                                $itemList->getItem()->name,
                                $record->price * $record->totalCount
                            ]);
                        }

                        # 推送 fluent log, 公示退款
                        $obj             = new \RO\Cmd\TradeLogCmd();
                        $obj->pid        = $record->charId;
                        $obj->time       = time();
                        $obj->type       = \RO\ZoneForward::ETRADETYPE_PUBLICITY_RETURN;
                        $obj->itemid     = $record->itemId;
                        $obj->count      = $record->totalCount;
                        $obj->price      = $record->price;
                        $obj->tax        = 0;
                        $obj->moneycount = $record->totalCount * $record->price;
                        $obj->logid      = "pub-refund-{$record->charId}-" . time();
                        if (!$itemList->isOverlap && $itemList->itemId != $itemList->itemKey)
                        {
                            $obj->iteminfo = json_encode($itemList->getItemData());
                        }
                        ZoneForward::pushToFluent($obj);
                        continue;
                    }

                    $refundCount = $record->totalCount - $record->count;

                    $info      = current($buyAssignList[$record->id]);
                    $sellerIds = key($buyAssignList[$record->id]);

                    $goods                = $sellList[$info['id']];
                    $record->goodsId      = $info['id'];
                    $record->itemData     = $goods->getItemData();
                    $record->isManyPeople = 0;
                    $record->sellersInfo  = null;
                    $record->sellerInfo   = null;
                    $record->status       = RecordBought::STATUS_PUBLICITY_SUCCESS;
                    $record->canGive      = $record->isCanGive();
                    Server::$instance->info("[公示-是否能赠送] charId:{$record->charId}, 玩家:{$record->playerName}, 订单id:{$record->goodsId}, 公示id:{$record->publicityId}, "
                                                . "itemId:{$record->itemId}, 价格:{$record->price}, 抢购成功数量:{$record->count}, 赠送情况:" . ($record->canGive ? '能' : '不能') . ", 额度:{$record->quota}");

                    $nameInfo = new NameInfo();
                    $nameInfo->setCount($info['count']);
                    $nameInfo->setName($info['name']);
                    $nameInfo->setZoneid($info['zoneId']);
                    $record->sellerInfo = $nameInfo;

                    if (count($buyAssignList[$record->id]) > 1)
                    {
                        $sellerIds = '';
                        $record->isManyPeople = 1;
                        $nameInfoList         = new NameInfoList();
                        foreach ($buyAssignList[$record->id] as $charId => $info)
                        {
                            $nameInfo = new NameInfo();
                            $nameInfo->setCount($info['count']);
                            $nameInfo->setName($info['name']);
                            $nameInfo->setZoneid($info['zoneId']);
                            $nameInfoList->addNameInfos($nameInfo);
                            $sellerIds .= $charId . ';';
                        }
                        $record->sellersInfo = $nameInfoList;
                    }

                    $records[] = $record;

                    # 推送 fluent log, 公示抢购成功
                    $obj             = new \RO\Cmd\TradeLogCmd();
                    $obj->pid        = $record->charId;
                    $obj->time       = time();
                    $obj->type       = \RO\ZoneForward::ETRADETYPE_PUBLICITY_BUY;
                    $obj->itemid     = $record->itemId;
                    $obj->count      = $record->count;
                    $obj->price      = $record->price;
                    $obj->tax        = 0;
                    $obj->moneycount = $record->count * $record->price;
                    $obj->logid      = "pub-buy-success-{$record->id}-" . time() . '-' . mt_rand(1, 999);
                    $obj->strotherid = $sellerIds;
                    if (!$itemList->isOverlap && $itemList->itemId != $itemList->itemKey)
                    {
                        $obj->iteminfo = json_encode($itemList->getItemData());
                    }
                    ZoneForward::pushToFluent($obj);

                    // 出现部分抢购成功, 则需要插入一条记录作为退款使用
                    if ($refundCount > 0)
                    {
                        $logId = md5($record->charId . $record->itemId . $record->time . $record->price . $record->count . $record->totalCount . mt_rand(0, 999999) . microtime());
                        $refundRecords[] = [
                            "'" . $record->charId . "'",
                            "'" . Server::$mysqlMaster->real_escape_string($record->playerName) . "'",
                            "'" . $record->playerZoneId . "'",
                            "'0'",
                            "'" . RecordBought::STATUS_PUBLICITY_CANCEL . "'",
                            "'" . $record->itemId . "'",
                            "'" . time() . "'",
                            "'" . $record->publicityId . "'",
                            "'" . $record->endTime . "'",
                            "'" . $record->count . "'",
                            "'" . $record->totalCount . "'",
                            "'" . $record->price . "'",
                            "'" . $record->refineLv . "'",
                            "'" . $record->isDamage . "'",
                            "'0'",
                            "null",
                            "null",
                            $record->itemData === null ? 'null' : '0x' . bin2hex($record->itemData->serialize()),
                            "'" . $logId . "'",
                        ];

                        # 推送 fluent log, 公示退款
                        $obj             = new \RO\Cmd\TradeLogCmd();
                        $obj->pid        = $record->charId;
                        $obj->time       = time();
                        $obj->type       = \RO\ZoneForward::ETRADETYPE_PUBLICITY_RETURN;
                        $obj->itemid     = $record->itemId;
                        $obj->count      = $refundCount;
                        $obj->price      = $record->price;
                        $obj->tax        = 0;
                        $obj->moneycount = $refundCount * $record->price;
                        $obj->logid      = "pub-refund-$logId";
                        if (!$itemList->isOverlap && $itemList->itemId != $itemList->itemKey)
                        {
                            $obj->iteminfo = json_encode($itemList->getItemData());
                        }
                        ZoneForward::pushToFluent($obj);
                    }

                    if (++$batchNum % 1000 === 0)
                    {
                        $writeBuyUpdateSql($records, $tmpFile);
                        $writeRecordBuyInsertSql($refundRecords, $tmpFile);
                        $records       = [];
                        $refundRecords = [];
                    }

                    if ($zone)
                    {
                        $param = [
                            $record->count,
                            $itemList->getItem()->name,
                            $record->count * $record->price,
                            ($record->totalCount - $record->count) * $record->price
                        ];
                        ZoneForward::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $record->charId, ActBuy::SYS_MSG_BUY_PUBLICITY_BUY_SUCCESS, $param);
                    }
                }

                // 处理剩余数据
                $writeBuyUpdateSql($records, $tmpFile);
                $writeRecordBuyInsertSql($refundRecords, $tmpFile);
            }

            if (!empty($sellAssignList))
            {
                $batchNum = 0;
                $records  = [];
                foreach ($sellList as $goodsId => $goods)
                {
                    if ($goods->count > $goods->stock)
                    {
                        $record               = new RecordSold();
                        $record->charId       = $goods->charId;
                        $record->playerZoneId = $goods->playerZoneId;
                        $record->goodsId      = $goodsId;
                        $record->itemId       = $goods->itemId;
                        $record->status       = RecordSold::STATUS_ADDING;
                        $record->time         = time();
                        $record->count        = $goods->count - $goods->stock;
                        $record->price        = $goods->pubPrice;
                        $record->refineLv     = $itemList->refineLv;
                        $record->isDamage     = $itemList->isDamage;
                        $record->tax          = $record->getTax();
                        $record->isManyPeople = 0;
                        $record->buyersInfo   = null;
                        $record->buyerInfo    = null;

                        $buyerIds = key($sellAssignList[$goodsId]);
                        $info     = current($sellAssignList[$goodsId]);

                        $nameInfo = new NameInfo();
                        $nameInfo->setCount($info['count']);
                        $nameInfo->setName($info['name']);
                        $nameInfo->setZoneid($info['zoneId']);
                        $record->buyerInfo = $nameInfo;

                        if (count($sellAssignList[$goodsId]) > 1)
                        {
                            $buyerIds = '';
                            $record->isManyPeople = 1;
                            $nameInfoList         = new NameInfoList();
                            foreach ($sellAssignList[$goodsId] as $charId => $info)
                            {
                                $nameInfo = new NameInfo();
                                $nameInfo->setCount($info['count']);
                                $nameInfo->setName($info['name']);
                                $nameInfo->setZoneid($info['zoneId']);
                                $nameInfoList->addNameInfos($nameInfo);
                                $buyerIds .= $charId . ';';
                            }
                            $record->buyersInfo = $nameInfoList;
                        }

                        $records[] = $record;

                        # 推送 fluent log, 公示出售成功
                        $obj             = new \RO\Cmd\TradeLogCmd();
                        $obj->pid        = $record->charId;
                        $obj->time       = time();
                        $obj->type       = \RO\ZoneForward::ETRADETYPE_TRUESELL;
                        $obj->itemid     = $record->itemId;
                        $obj->count      = $record->count;
                        $obj->price      = $record->price;
                        $obj->tax        = $record->tax;
                        $obj->moneycount = $record->count * $record->price;
                        $obj->logid      = "pub-sell-success-{$record->goodsId}-" . time() . '-'. mt_rand(1, 999);
                        $obj->strotherid = $buyerIds;
                        if (!$itemList->isOverlap && $itemList->itemId != $itemList->itemKey)
                        {
                            $obj->iteminfo = json_encode($itemList->getItemData());
                        }
                        ZoneForward::pushToFluent($obj);

                        # 通知场景服务器,更新交易记录
                        $notifyChars[$record->charId] = $record->playerZoneId;

//                    $msg         = new \RO\Cmd\ListNtfRecordTrade();
//                    $msg->charid = $record->charId;
//                    $msg->type   = \RO\Cmd\EListNtfType::ELIST_NTF_MY_LOG;
                        $zone = Server::$zone->get($record->playerZoneId);
                        if ($zone)
                        {
//                        ZoneForward::sendToUserByFd($msg, $msg->charid, $zone['fd'], $zone['fromId']);
                            $params = [
                                $obj->count,
                                $itemList->getItem()->name,
                                $obj->tax,
                                $obj->moneycount - $obj->tax,
                            ];
                            ZoneForward::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $record->charId, ActSell::SYS_MGS_SELL_SUCCESS, $params);

                            # 发送离线消息给场景服务器
                            $offlineMsg              = new \RO\Cmd\AddMoneyRecordTradeCmd();
                            $offlineMsg->charid      = $record->charId;
                            $offlineMsg->money_type  = Server::DEFAULT_MONEY_TYPE;
                            $offlineMsg->count       = $obj->count;
                            $offlineMsg->type        = \RO\Cmd\EOperType::EOperType_NormalSell;
                            $offlineMsg->total_money = $obj->moneycount - $obj->tax;
                            $offlineMsg->itemid      = $obj->itemid;
                            $offlineMsg->price       = $obj->price;
                            ZoneForwardScene::sendToZoneByFd($offlineMsg, $zone['fd'], $zone['fromId']);
                        }

                        if (++$batchNum % 1000 === 0)
                        {
                            $writeSellInsertSql($records, $tmpFile);
                            $records = [];
                        }
                    }
                }

                $writeSellInsertSql($records, $tmpFile);
                unset($records);
            }

            // 对没有抢购到的买家记录进行退款
            $sql = 'UPDATE `trade_record_bought` SET `status` = ' . RecordBought::STATUS_PUBLICITY_CANCEL . " WHERE `count` = 0 AND `publicity_id` = '{$itemList->id}' AND `status` = " . RecordBought::STATUS_PUBLICITY_PAY_SUCCESS . ";" . PHP_EOL;
            if (file_put_contents($tmpFile, $sql, FILE_APPEND) === false)
            {
                throw new \Exception('Sql语句插入临时文件失败');
            }
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[交易所-公示分发] 分单失败. publicity_id:{$itemList->id} item_id:{$itemList->itemId}, err:{$e->getMessage()} LINE:{$e->getLine()}");
            return false;
        }

        if (IS_DEBUG)
        {
            Server::$instance->debug('[交易所-公示分发] 写入sql文件耗时:'. (microtime(1) - $time3));
        }

        // 交易记录写入文件成功后,更新数据库挂单记录,并更新物品表库存。
        if (!Server::$mysqlMaster->begin_transaction())
        {
            Server::$instance->warn("[交易所-公示分发] 分单失败. publicity_id:{$itemList->id} item_id:{$itemList->itemId}, err:" . Server::$mysqlMaster->error);
            return false;
        }

        try
        {
            $count = 0;
            foreach ($sellList as $goodsId => $goods)
            {
                $remain = $goods->count - $goods->stock;
                $count += $remain;

                // 挂单公示期结束时间设为0表示已经公示处理结束
                $goods->endTime = 0;
                $goods->isPublicity = 0;

                // 卖光记录一直跳过,等后面一并更新
                if ($goods->stock === 0) continue;

                if($goods->update())
                {
                    # 推送 fluent log, 公示结束转为普通上架
                    $obj             = new \RO\Cmd\TradeLogCmd();
                    $obj->pid        = $goods->charId;
                    $obj->time       = time();
                    $obj->type       = \RO\ZoneForward::ETRADETYPE_RESELL_AUTO;
                    $obj->itemid     = $goods->itemId;
                    $obj->count      = $goods->count;
                    $obj->price      = $goods->pubPrice;
                    $obj->tax        = ActSell::getBoothFee($obj->price, $obj->count);
                    $obj->moneycount = 0;
                    $obj->logid      = isset($record) && $record ? "pub-resell-auto-{$record->goodsId}" : "pub-resell-{$obj->pid}-{$obj->time}-{$obj->itemid}";
                    if (!$goods->isOverlap && $goods->itemId != $goods->itemKey)
                    {
                        $obj->iteminfo = json_encode($goods->getItemData());
                    }
                    ZoneForward::pushToFluent($obj);
                }
                else
                {
                    throw new \Exception('更新挂单表失败, id:' . $goods->id);
                }
            }

            // 挂单已全部出售的记录,更新状态和库存
            $sql = "UPDATE `trade_goods` SET `stock` = 0, `status` = '". Goods::STATUS_SOLD ."' WHERE `item_list_id` = '{$itemList->id}' AND `is_publicity` = 1 AND `stock` = `count` AND `end_time` > 0 AND `status` = " . Goods::STATUS_SELLING;
            $rs  = Server::$mysqlMaster->query($sql);
            if (!$rs)
            {
                throw new \Exception('更新挂单状态和库存记录失败');
            }

            $itemList->isPublicity  = 0;
            $itemList->pubBuyPeople = 0;
            $itemList->delayTime    = 0;
            $itemList->endTime      = 0;
            $itemList->startTime    = 0;
            $itemList->decrStock($count);
            if (!$itemList->update())
            {
                throw new \Exception('更新总库存失败, id:' . $itemList->id);
            }

            Server::$mysqlMaster->commit();

            $rs = Server::$item->decr($itemList->itemId, 'publicityNum', 1);
            if ($rs !== false && $rs < 0)
            {
                Server::$item->set($itemList->itemId, ['publicityNum' => 0]);
                Server::$instance->warn("Item 公示数异常, ItemId: {$itemList->itemId}, publicity number: {$rs}");
            }

            try
            {
                # 清除购买人数缓存
                Server::$redis->del('pub_buy_count_' . $itemList->id);
            }
            catch (\Exception $e)
            {
                Server::$instance->warn("[删除公示购买人数缓存] 发生异常, 错误信息: {$e->getMessage()}");
            }

            // 等待数据库提交成功后,将临时文件名改为有效的文件名
            $newName = Server::$dataDir . 'pub_' . $itemList->id . '_' . $itemList->endTime . '.sql';
            $rs = rename($tmpFile, $newName);
            if (!$rs)
            {
                unlink($tmpFile);
                throw new \Exception('修改临时文件名失败');
            }
        }
        catch (\Exception $e)
        {
            Server::$mysqlMaster->rollback();
            Server::$instance->warn("[交易所-公示分发] 分单失败. publicity_id:{$itemList->id} item_id:{$itemList->itemId}, err:{$e->getMessage()} LINE:{$e->getLine()}");
            return false;
        }

        if (IS_DEBUG)
        {
            Server::$instance->debug('[交易所-公示分发] 数据库操作耗时:' . (microtime(1) - $time3));
            Server::$instance->debug('[交易所-公示分发] 耗时:' . (microtime(1) - $time));
        }

        Server::$instance->info("[交易所-公示处理] 公示id:{$itemList->id} 公示物品:{$itemList->itemId} 分单完毕!");
        return 'pub_' . $itemList->id . '_' . $itemList->endTime . '.sql';
    }

    // 保持单进程下使用下列两个静态属性
    // 存储正在公示期的物品结束时间
    private static $publicityMap = [];
    private static $counter = 0;
    private static $pubOrders = [];

    /**
     * 系统定时处理对公示期物品发货
     *
     * 每分钟由 taskId = 0 的进程调用
     */
    public static function timerTaskOffShelf()
    {
        // 延时3秒
        $time = time() - 3;
        if (self::$counter === 0)
        {
            self::$publicityMap = [];
            foreach (Server::$itemList as $key => $itemList)
            {
                if ($itemList['is_publicity'])
                {
                    self::$publicityMap[$key] = $itemList['end_time'] + $itemList['delay_time'];
                }
            }

            $lastPubOrders = BoothOrder::getLastPubOrders();
            foreach ($lastPubOrders as $order)
            {
                if (!isset(self::$pubOrders[$order['id']])) {
                    self::$pubOrders[$order['id']] = $order;
                }
            }
        }

        // 计数器, 当达到一定次数时, 重新从数据库加载所有未结束的公示物品
        self::$counter++;
        if (self::$counter >= 60) self::$counter = 0;

        $dispatchFinish = [];
        $finishItemIds = [];
        foreach (self::$publicityMap as $key => $endTime)
        {
            if ($endTime <= $time)
            {
                $obj = ItemList::getByKey($key);

                if (!$obj)
                {
                    Server::$instance->warn("[交易所-公示处理] 获取ItemList对象失败 itemKey:{$key}");
                    continue;
                }

                if (!$obj->isPublicity)
                {
                    continue;
                }

                // 避免内存中的延长时间被修改
                if ($obj->getEndTime() > $time)
                    continue;

                // 记录需要通知的玩家id
                $notifyChars = [];
                $rs = self::goodsDispatch($obj, $notifyChars);
                if (is_string($rs))
                {
                    $dispatchFinish['trade_' . $obj->id] = [$rs, $notifyChars];
                }
                unset(self::$publicityMap[$key]);
                $finishItemIds[$obj->itemId] = 1;
            }
        }

        $clearOrder = [];
        foreach (self::$pubOrders as $orderId => $order)
        {
            $delayTime = Server::$itemList->get($order['item_key'], 'delay_time');
            $orderEndTime = intval($order['end_time']) + $delayTime;
            if ($orderEndTime > $time)
                continue;

            $rs = BoothOrderService::handlePubOrder($orderId);
            if (is_string($rs)) {
                $dispatchFinish['booth_' . $orderId] = [ $rs, [] ];
            }

            $clearOrder[] = $orderId;
//            $finishItemIds[$order['item_id']] = 1;
        }

        foreach ($clearOrder as $orderId)
        {
            unset(self::$pubOrders[$orderId]);
        }

        if (!empty($dispatchFinish))
        {
            // 等待所有公示分单成功后,执行交易记录生成和更新操作
            foreach ($dispatchFinish as $data)
            {
                list($name, $notifyChars) = $data;
                if (!is_string($name)) continue;
                if (Server::sqlFileExecute(Server::$dataDir . $name, Server::$mysqlMaster))
                {
                    // 通知场景服务器更新交易记录
                    if (!empty($notifyChars))
                    {
                        foreach ($notifyChars as $charId => $playerZoneId)
                        {
                            $zoneId = Player::getZoneId($charId, true);
                            if (!$zoneId)
                            {
                                $zoneId = $playerZoneId;
                            }
                            $msg         = new \RO\Cmd\ListNtfRecordTrade();
                            $msg->charid = $charId;
                            $msg->type   = \RO\Cmd\EListNtfType::ELIST_NTF_MY_LOG;
                            ZoneForward::sendToUserByZoneId($msg, $charId, $zoneId);
                        }
                    }

                    Server::$instance->info("[交易所-公示处理] 公示{$name}文件执行完毕!");
                }
                else
                {
                    //TODO: 如果发现失败可以发短信通知。
                }
            }
        }

        if (!empty($finishItemIds))
        {
            foreach ($finishItemIds as $itemId => $val)
            {
                # 公示结束执行调价
                $process = Server::$instance->getCustomWorkerProcess('PriceManager');
                $rs = $process->write("adjustPrice $itemId");
                if (!$rs)
                {
                    Server::$instance->warn("[交易所-公示处理] 执行公示结束调价失败 itemId: {$itemId}");
                }
            }
        }
    }

    /**
     * 根据物品KEY获取对象
     *
     * @param $itemKey
     * @return bool|ItemList
     */
    public static function getByKey($itemKey, $cached = false)
    {
        if (!$itemKey)return false;

        if ($cached && isset(self::$instance[$itemKey]))
        {
            return self::$instance[$itemKey];
        }

        $data = Server::$itemList->get($itemKey);
        if (false === $data)
        {
            $key = self::_quoteValue($itemKey);
            $rs  = Server::$mysqlMaster->query('SELECT * FROM `'. self::$TableName ."` WHERE `item_key` = {$key}");
            if ($rs)
            {
                if ($rs->num_rows > 0)
                {
                    $row = $rs->fetch_assoc();
                    $obj = new ItemList($row, $itemKey);
                    Server::$itemList->set($obj->itemKey, $obj->getTableData());
                    Server::$itemListId2KeyMap->set($row['id'], ['key' => $row['item_key']]);

                    // 把冷数据转为热数据
                    if (!$row['is_hot'])
                    {
                        Server::$mysqlMaster->query("UPDATE `trade_item_list` SET `is_hot` = 1 WHERE `id` = '{$row['id']}'");
                    }

                    if ($cached)
                    {
                        $obj->_cached = true;
                        self::$instance[$itemKey] = $obj;
                    }
                    return $obj;
                }
                else
                {
                    return null;
                }
            }
            else
            {
                return false;
            }
        }

        $obj = new ItemList($data, $itemKey);

        if ($cached)
        {
            $obj->_cached = true;
            self::$instance[$itemKey] = $obj;
        }

        return $obj;
    }

    /**
     * 根据物品ID获取对象
     *
     * @param $id
     * @return bool|ItemList
     */
    public static function getById($id, $cached = false)
    {
        if (!$id > 0)return false;

        $key = self::getKeyById($id);

        if (!$key)
        {
            return false;
        }

        return self::getByKey($key, $cached);
    }

    /**
     * 根据物品KEY获取对象
     *
     * @param $itemKeys
     * @return false|array
     */
    public static function getByKeys($itemKeys, $cached = false)
    {
        if (!$itemKeys)return false;

        $rs = [];
        foreach ($itemKeys as $key)
        {
            $rs[$key] = self::getByKey($key, $cached);
        }

        return $rs;
    }

    /**
     * 增加一个购买
     *
     * @param $publicityId
     * @param $buyerId
     * @param $count
     * @return bool
     */
    //public static function addNewPublicityBuyer($publicityId, $buyerId, $count)
    //{
    //    $publicityId = self::_quoteValue($publicityId);
    //    $buyerId     = self::_quoteValue($buyerId);
    //    $count       = self::_quoteValue($count);
    //    $rs2 = Server::$mysqlMaster->query("INSERT INTO `trade_publicity_buy` (`publicity_id`, `buyerid`, `count`) VALUES ({$publicityId}, {$buyerId}, {$count})");
    //
    //    if ($rs2)
    //    {
    //        return true;
    //    }
    //    else
    //    {
    //        return false;
    //    }
    //}

    /**
     * 递增购买人数
     *
     * @param itemList $itemList
     * @param $incrby
     * @return bool
     */
    public static function incrPublicityBuyPeople($itemList, $incrby)
    {
        $rs = Server::$mysqlMaster->query("UPDATE `". self::$TableName ."` SET `pub_buy_people` = `pub_buy_people` + {$incrby} WHERE `id` = '{$itemList->id}'");
        if ($rs)
        {
            # 更新内存的购买人数
            $itemList->pubBuyPeople += $incrby;
            $itemList->_old['pub_buy_people'] = $itemList->pubBuyPeople;
            Server::$itemList->incr($itemList->itemKey, 'pub_buy_people', $incrby);

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * 更新库存
     *
     * @param int $itemId
     * @param string $itemKey
     * @param int $itemListId
     * @param int $stock 新库存
     * @param int $diff 较之前的差异, 不设置则自动对比
     * @return bool
     */
    public static function saveItemStock($itemId, $itemKey, $itemListId, $stock, $diff = null)
    {
        if ($stock < 0)$stock = 0;
        if (null === $diff)
        {
            $list   = Server::$itemList->get($itemKey) ?: ['stock' => 0];
            $diff   = $list['stock'] - $stock;
            $itemId = $list['item_id'];
        }

        $rs = Server::$itemList->set($itemKey, ['stock' => $stock]);
        if ($rs)
        {
            if ($diff != 0)
            {
                $s = Server::$item->incr($itemId, 'stock', $diff);
                if ($s !== false && $s < 0)
                {
                    Server::$item->set($itemId, ['stock' => 0]);
                    Server::$instance->warn("[更新库存] SaveItem时增加Item库存数异常, ItemId: {$itemId}, 更新后的stock: {$s}, diff: {$diff}");
                }
            }

            Server::$stockUpdateChannel->push(pack('LQ', $itemListId, $stock));
        }

        return $rs;
    }

    /**
     * 递增库存
     *
     * @param int $itemId
     * @param string $itemKey
     * @param int $itemListId
     * @param int $incrBy
     * @return bool|int
     */
    public static function incrItemStock($itemId, $itemKey, $itemListId, $incrBy)
    {
        if ($incrBy < 0)
        {
            # 递减
            return self::decrItemStock($itemId, $itemKey, $itemListId, -$incrBy);
        }

        $rs = Server::$itemList->incr($itemKey, 'stock', $incrBy);
        if (false !== $rs)
        {
            Server::$item->incr($itemId, 'stock', $incrBy);
            Server::$stockUpdateChannel->push(pack('LQ', $itemListId, $rs));
        }

        return $rs;
    }

    /**
     * 递减库存
     *
     * @param int    $itemId
     * @param string $itemKey
     * @param int    $itemListId
     * @param int    $decrBy
     * @param $stock
     * @return bool|int
     */
    public static function decrItemStock($itemId, $itemKey, $itemListId, $decrBy)
    {
        $rs = Server::$itemList->decr($itemKey, 'stock', $decrBy);
        if (false !== $rs)
        {
            if ($rs < 0)
            {
                # 如果 < 0 则修复
                Server::$itemList->set($itemKey, ['stock' => 0]);
                Server::$instance->warn("ItemList 库存数异常, ItemKey: {$itemKey}, stock: {$rs}，传入的decrBy: {$decrBy}");
                $decrBy += $rs;
                $rs      = 0;
            }
            $s = Server::$item->decr($itemId, 'stock', $decrBy);
            if ($s !== false && $s < 0)
            {
                Item::resetStockAndPubNum($itemId);
            }

            Server::$stockUpdateChannel->push(pack('LQ', $itemListId, $rs));
        }

        return $rs;
    }

    /**
     * 递增摆摊库存
     *
     * @param $itemId
     * @param $itemKey
     * @param $incrBy
     * @return bool|int
     */
    public static function incrItemBoothStock($itemId, $itemKey, $incrBy)
    {
        $rs = Server::$itemList->incr($itemKey, 'boothStock', $incrBy);
        if (false !== $rs)
        {
            Server::$item->incr($itemId, 'boothStock', $incrBy);
        }

        return $rs;
    }

    /**
     * 递减摆摊库存
     *
     * @param int    $itemId
     * @param string $itemKey
     * @param int    $decrBy
     * @return bool|int
     */
    public static function decrItemBoothStock($itemId, $itemKey, $decrBy)
    {
        $rs = Server::$itemList->decr($itemKey, 'boothStock', $decrBy);
        if (false !== $rs)
        {
            if ($rs < 0)
            {
                Server::$instance->warn("ItemList 摆摊库存数异常, item_id:{$itemId}, ItemKey: {$itemKey}, booth_stock: {$rs}，传入的decrBy: {$decrBy}");
                self::resetBoothStock($itemKey, $itemId);
            }
            else
            {
                Server::$item->decr($itemId, 'boothStock', $decrBy);
            }
        }

        return $rs;
    }

    /**
     * 重置库存
     *
     * @param $itemKey
     * @param null $itemId 该id一定要和itemKey匹配, 否则会计算错误
     * @return bool
     */
    public static function resetBoothStock($itemKey, $itemId = null)
    {
        $sql = 'SELECT SUM(`stock`) as `total_stock` FROM `' . BoothOrder::getTableName() . '` WHERE `status` = ' . BoothOrder::STATUS_SELLING . " AND `is_publicity` = 0 AND `item_key` = '{$itemKey}'";
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs === false) {
            return false;
        } else {
            $c = $rs->fetch_assoc();
        }

        $totalStock = $c['total_stock'] ?? 0;
        if (Server::$itemList->set($itemKey, ['boothStock' => $totalStock]))
        {
            if ($itemId === null) {
                $itemId = Server::$itemList->get($itemKey, 'item_id');
            }

            $itemStocks[$itemId] = 0;
            foreach (Server::$itemList as $v)
            {
                if ($itemId === $v['item_id']) {
                    $itemStocks[$itemId] += $v['boothStock'];
                }
            }

            if ($itemId === null) {
                Server::$item->set($itemId, ['boothStock' => $itemStocks[$itemId]]);
            }

            return true;
        }
        else
        {
            return false;
        }
    }


    /**
     * 获取库存
     *
     * @param $itemId
     * @param $itemKey
     * @return int
     */
    public static function getItemStock($itemKey)
    {
        $rs = Server::$itemList->get($itemKey);
        if ($rs)
        {
            return $rs['stock'];
        }
        else
        {
            return 0;
        }
    }

    /**
     * 将当前库存数值保存到数据库
     *
     * @param $itemListId
     * @param $stock
     * @return bool
     */
    public static function saveStockToDb($itemListId, $stock)
    {
        $sql = "UPDATE `". self::$TableName ."` SET `stock` = '{$stock}' WHERE `id` = '{$itemListId}'";
        $rs  = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * 根据ID获取Key，没有则返回 false
     *
     * @param $id
     * @return bool|ItemList
     */
    public static function getKeyById($id)
    {
        if (isset(Server::$itemListId2KeyMapCached[$id]))
        {
            $key = Server::$itemListId2KeyMapCached[$id];
        }
        else
        {
            $keyMap = Server::$itemListId2KeyMap->get($id);     # $itemListId 对应的 itemKey
            if ($keyMap)
            {
                $key = Server::$itemListId2KeyMapCached[$id] = $keyMap['key'];
            }
            else
            {
                return false;
            }
        }

        return $key;
    }

    /**
     * @param \RO\Cmd\ItemData $itemData
     * @return $this|null
     */
//    public static function getByItemData($itemData)
//    {
//        $item = Item::get($itemData->base->id, $itemData);
//        if (!$item) {
//            return null;
//        }
//
//        $itemKey = $item->getKey();
//        $itemList = self::getByKey($itemKey);
//        if ($itemList)
//        {
//            $itemList->item = $item;
//            return $itemList;
//        }
//        else if ($itemList === null)
//        {
//            $itemList              = new ItemList([], $itemKey);
//            $itemList->item        = $item;
//            $itemList->itemId      = $item->id;
//            $itemList->isOverlap   = $item->isOverlap;
//            $itemList->itemData    = !$item->isOverlap ? $itemData : null;
//            $itemList->refineLv    = !$item->isOverlap ? $itemData->equip->refinelv : 0;
//            $itemList->isDamage    = !$item->isOverlap ? intval($itemData->equip->damage) : 0;
//            $itemList->isPublicity = $itemList->checkIsPublicity() ? 1 : 0;
//            if ($itemList->isPublicity == 1)
//            {
//                $itemList->startTime    = time();
//                $itemList->endTime      = time() + $item->publicityShowTime;
//                $itemList->pubPrice     = $item->getPrice();
//            }
//
//            return $itemList;
//        }
//        else
//        {
//            return null;
//        }
//    }
}