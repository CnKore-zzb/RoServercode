<?php
namespace RO\Trade\Dao;

use RO\MySQLi;
use RO\Trade\ActSell;
use RO\Trade\Item;
use RO\Trade\Player;
use RO\Trade\Server;
use RO\ZoneForward;

/**
 * 挂单表
 *
 * @package RO\Trade\Dao
 */
class Goods extends Dao
{
    public $id;

    public $itemId;

    /**
     * 物品列表ID
     *
     * @var string
     */
    public $itemListId;

    /**
     * 物品的唯一ID
     *
     * @var string
     */
    public $itemKey;

    /**
     * 挂单角色ID
     *
     * @var int
     */
    public $charId;

    /**
     * 挂单角色名
     *
     * @var string
     */
    public $playerName;

    /**
     * 挂单角色所在线
     *
     * @var int
     */
    public $playerZoneId;

    /**
     * 挂单时间
     *
     * @var int
     */
    public $time;

    /**
     * 状态
     *
     * @var int
     */
    public $status = self::STATUS_PENDING;

    /**
     * 物品数
     *
     * @var int
     */
    public $count;

    /**
     * 还剩库存数
     *
     * @var int
     */
    public $stock;

    /**
     * 是否堆叠
     *
     * @var int
     */
    public $isOverlap;

    /**
     * 是否公示
     *
     * @var int
     */
    public $isPublicity;

    /**
     * 公示结束时间
     *
     * @var int
     */
    public $endTime;

    /**
     * 公示价格
     *
     * @var int
     */
    public $pubPrice;

    /**
     * 物品信息
     *
     * @var \RO\Cmd\ItemData|null
     */
    public $itemData;

    /**
     * @var Item
     */
    protected $item;

    /**
     * @var ItemList
     */
    protected $itemList;

    protected static $TableName = 'trade_goods';

    protected static $Fields = [
        'id'          => 'id',
        'itemListId'  => 'item_list_id',    # 物品列表ID
        'itemId'      => 'item_id',         # 物品ID
        'itemKey'     => 'item_key',        # 物品唯一ID
        'charId'      => 'char_id',         # 卖家ID
        'playerName'  => 'player_name',     # 卖家名称
        'playerZoneId'=> 'player_zone_id',  # 卖家ZoneId
        'time'        => 'time',            # 挂单时间
        'status'      => 'status',          # 状态，见下面状态常量设置
        'count'       => 'count',           # 销售总数
        'stock'       => 'stock',           # 库存数
        'isOverlap'   => 'is_overlap',      # 是否堆叠物品
        'isPublicity' => 'is_publicity',    # 是否公示物品
        'endTime'     => 'end_time',        # 公示结束时间
        'pubPrice'    => 'pub_price',       # 公示价格
        'itemData'    => 'item_data',       # 物品信息protoBuf数据
    ];

    /**
     * 存档记录SQL语句
     *
     * @var string
     */
    protected static $createArchiveTableSql = <<<EOF
CREATE TABLE IF NOT EXISTS `%table` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `item_id` int(10) UNSIGNED NOT NULL,
  `item_list_id` bigint(20) UNSIGNED NOT NULL,
  `item_key` varchar(32) NOT NULL,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_name` varchar(128) CHARACTER SET utf8mb4 DEFAULT '',
  `player_zone_id` int(10) UNSIGNED NOT NULL,
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `status` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `stock` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `is_overlap` tinyint(1) UNSIGNED NOT NULL,
  `is_publicity` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `end_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `pub_price` int(10) UNSIGNED NOT NULL,
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `charId_stock` (`char_id`,`stock`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='挂单表存档';
EOF;

    const STATUS_PENDING   = 0;   // 待挂单，挂单处理扣物品中
    const STATUS_SELLING   = 1;   // 正常挂单中（物品已扣除成功）
    const STATUS_EXPIRED   = 2;   // 已失效挂单，物品未退回（可以重新挂单）
    const STATUS_CANCELED  = 3;   // 已下架挂单，物品已退回（不可以重新挂单）
    const STATUS_SOLD      = 4;   // 已卖光

    /**
     * 单个人最多上架数目
     *
     * @var int
     */
    const FULL_NUM = 8;

    /**
     * 每个订单获取库存数
     *
     * @var int
     */
    const GET_STOCK_PER_GOODS = 10;

    public function insert()
    {
        if (!$this->time)
        {
            $this->time = time();
        }

        # 库存
        $this->stock = $this->count;

        return parent::insert();
    }

    /**
     * 返回一个itemList数据映射对象
     *
     * @return bool|null|ItemList
     */
    public function getItemList()
    {
        if (null === $this->itemList)
        {
            $this->itemList = ItemList::getByKey($this->itemKey);
        }

        if (!$this->itemList) return false;

        return $this->itemList;
    }

    /**
     * 挂单是否过期
     *
     * @return bool
     */
    public function isExpired()
    {
        return time() - $this->time >= Server::$configExchange['ExchangeHour'];
    }

    public function getPrice()
    {
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
                Server::$instance->warn("解析 Goods: {$this->id} itemData 数据失败");
            }
        }

        return $this->itemData;
    }

    /**
     * 获取内存item对象
     *
     * @return false|Item
     */
    public function getItem()
    {
        if (null === $this->item)
        {
            if ($this->itemId === $this->itemKey)
            {
                $this->item = Item::get($this->itemId);
            }
            else
            {
                $meta = $this->getItemList();
                if (!$meta) return false;

                $this->item = $meta->getItem();
            }
        }

        return $this->item;
    }

    /**
     * 设置 Item 对象
     *
     * @param Item $item
     * @return $this
     */
    public function setItem(Item $item)
    {
        $this->item = $item;
        return $this;
    }

    /**
     * 递减库存 (数据库原子操作), 成功后不更新内存中库存
     *
     * 需要更新的话执行：
     *
     *  `Dao\ItemList::decrStock($this->itemKey, $count);
     *
     * @param int $count
     * @return bool|int
     */
    public function decrStock($count = 1)
    {
        $count = self::_quoteValue($count);
        $rs    = Server::$mysqlMaster->query($sql = "UPDATE `". self::$TableName ."` SET `stock` = `stock` - {$count} WHERE `id` = '{$this->id}' AND `stock` >= {$count}");
        if ($rs)
        {
            if (Server::$mysqlMaster->affected_rows)
            {
                return true;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            Server::$instance->warn($sql .', '. Server::$mysqlMaster->error);
        }

        return false;
    }

    /**
     * 设置状态
     *
     * @param $status
     * @return bool|int
     */
    public function setStatus($status)
    {
        $rs = self::setStatusById($this->id, $status);
        if (false !== $rs)
        {
            $this->_old['status'] = $this->status = $status;
        }
        return $rs;
    }

    /**
     * 获取列表
     *
     * @param $charId
     * @return array|bool
     */
    public static function getMyGoods($charId)
    {
        $rs = Server::$mysqlMaster->query("SELECT * FROM `". self::$TableName ."` WHERE `char_id` = '". intval($charId) ."' AND `stock` > '0' AND `status` in (". self::STATUS_SELLING .', '. self::STATUS_EXPIRED .") ORDER BY `time` DESC LIMIT 13");
        if ($rs)
        {
            $keys  = new \Ds\Set();
            $goods = [];
            while ($good = $rs->fetch_object(__CLASS__))
            {
                /**
                 * @var Goods $good
                 */
                $goods[] = $good;
                $keys->add($good->itemKey);
            }
            $rs->free();

            $itemList = ItemList::getByKeys($keys->toArray());
            foreach ($goods as $good)
            {
                /**
                 * @var Goods $good
                 */
                $good->itemList = $itemList[$good->itemKey];
            }

            return $goods;
        }
        else
        {
            return false;
        }
    }

    /**
     * 获取多个挂单对象
     *
     * @param array $ids
     * @return array|bool
     */
    public static function getByIds($ids)
    {
        if (!$ids)return false;

        foreach ($ids as &$tmp)
        {
            $tmp = self::_quoteValue($tmp);
        }

        if (count($ids) == 1)
        {
            $sql = "SELECT * FROM `" . static::$TableName . "` WHERE `id` = " . current($ids);
        }
        else
        {
            $sql = "SELECT * FROM `" . static::$TableName . "` WHERE `id` in (" . implode(", ", $ids) . ")";
        }
        $rs  = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            $list = [];
            while ($ret = $rs->fetch_object(static::class))
            {
                $list[$ret->id] = $ret;
            }
            $rs->free();

            /**
             * @var self $ret
             */
            return $list;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error. ', sql: '. $sql);

            return false;
        }
    }

    /**
     * 上架是否已满
     *
     * @param $charId
     * @return bool
     */
    public static function isSellFull($charId)
    {
        $rs = Server::$mysqlMaster->query("SELECT COUNT(1) as `count` FROM `". self::$TableName ."` WHERE `char_id` = '". intval($charId) ."' AND `stock` > '0' AND `status` in (". self::STATUS_SELLING .', '. self::STATUS_EXPIRED .")");
        if ($rs)
        {
            $q = $rs->num_rows ? $rs->fetch_assoc() : false;
            $rs->free();

            if ($q)
            {
                if ($q['count'] >= self::FULL_NUM)
                {
                    # 商人技能增加挂单数量
                    if ($q['count'] >= Player::getPendingLimit($charId))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else
            {
                return false;
            }
        }

        return false;
    }

    /**
     * 更新订单状态
     *
     * @param $goodsId
     * @param $status
     * @return bool|int
     */
    public static function setStatusById($goodsId, $status)
    {
        $goodsId = self::_quoteValue($goodsId);
        $status  = self::_quoteValue($status);
        $rs = Server::$mysqlMaster->query("UPDATE `". self::$TableName ."` SET `status` = {$status} WHERE `id` = ". $goodsId);
        if ($rs)
        {
            return Server::$mysqlMaster->affected_rows;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error);
        }

        return false;
    }

    /**
     * 将正在销售的挂单批量更新状态为过期
     *
     * @param $goodsId
     * @return bool|int
     */
    public static function setStatusExpired($goodsIds)
    {
        foreach ($goodsIds as & $id)
        {
            $id = self::_quoteValue($id);
        }
        $rs = Server::$mysqlMaster->query("UPDATE `". self::$TableName ."` SET `status` = ". self::STATUS_EXPIRED ." WHERE `id` in (". implode(',', $goodsIds) .') AND `status` = '. self::STATUS_SELLING);
        if ($rs)
        {
            return Server::$mysqlMaster->affected_rows;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error);
        }

        return false;
    }

    /**
     * 移除一个订单
     *
     * @param     $orderId
     * @param int $status
     * @return bool|int
     */
    public static function removeById($orderId, $status = self::STATUS_PENDING)
    {
        $orderId = self::_quoteValue($orderId);
        $status  = self::_quoteValue($status);
        $rs = Server::$mysqlMaster->query("DELETE FROM `". self::$TableName ."` WHERE `id` = ". $orderId ." AND `status` = $status");

        if ($rs)
        {
            return Server::$mysqlMaster->affected_rows;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error);
            return false;
        }
    }

    /**
     * 获取库存
     *
     * @param $itemListId
     * @return bool|int
     */
    public static function getDbStockByItemListId($itemListId)
    {
        $eTime = (int)Server::$configExchange['ReceiveTime'];
        $rs    = Server::$mysqlMaster->query($sql = "SELECT SUM(`stock`) as `stock` FROM `". self::$TableName ."` WHERE `item_list_id` = ". self::_quoteValue($itemListId) ." AND `status` = '". self::STATUS_SELLING ."' AND `stock` > 0 AND `time` > '". (time() - $eTime) . "'");
        if ($rs)
        {
            $row = $rs->fetch_object();
            return (int)$row->stock;
        }
        else
        {
            Server::$instance->warn('Error: '. $sql .' '. Server::$mysqlMaster->error);
            return false;
        }
    }

    /**
     * 获取库存列队
     *
     * @param ItemList $itemList
     * @return bool|\Ds\Map
     */
    public static function getGracefulStock(ItemList $itemList)
    {
        if (!$itemList->id > 0)return false;
        $eTime = (int)Server::$configExchange['ReceiveTime'];
        $rs    = Server::$mysqlMaster->query($sql = "SELECT `id`, `stock` FROM `". self::$TableName ."` WHERE `item_list_id` = ". self::_quoteValue($itemList->id) ." AND `status` = '". self::STATUS_SELLING ."' AND `stock` > 0 AND `time` > '". (time() - $eTime) ."' ORDER BY `time` ASC");

        if ($rs)
        {
            $stockMap = new \Ds\Map();
            $list     = new \Ds\Map();
            $list->allocate(128);
            if ($rs->num_rows > 0)
            {
                $count = 0;
                while ($row = $rs->fetch_object())
                {
                    $upStock = Server::$updatingGoodsStock->get($row->id);
                    $id      = (int)$row->id;
                    $stock   = (int)$row->stock;
                    if (false !== $upStock)
                    {
                        $stock -= $upStock['count'];
                    }
                    if ($stock <= 0)
                    {
                        # 减去待更新的库存数 <0
                        continue;
                    }

                    $count    += $stock;
                    $list[$id] = $stock;
                }

                if (false == $itemList->isOverlap)
                {
                    # 非堆叠，随机排序
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
            else
            {
                $count = 0;
            }

            $stockMap['list']  = $list;      # 库存队列
            $stockMap['stock'] = $count;     # 总库存数

            $rs->free();

            return $stockMap;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error);
            return false;
        }
    }

    /**
     * 获取订单
     *
     * 返回的是一个Map，key是GoodsId，value 是给的库存数
     *
     * @param \Ds\Map $stockMap
     * @param int $count
     * @return \Ds\Map
     */
    public static function getGracefulGoods(\Ds\Map $stockMap, $count)
    {
        /**
         * @var \Ds\Map $list
         */
        $found     = new \Ds\Map();
        $found->allocate(128);
        $lastCount = $count;
        $list      = $stockMap['list'];
        if (0 == $count)return $found;
        if ($list->isEmpty())return $found;

        $used = 0;
        while (true)
        {
            $item  = $list->first();
            $id    = $item->key;
            $stock = $item->value;
            unset($item);

            # 在队列里移除
            $list->remove($id);

            $minus = min($lastCount, 30);

            if ($stock == 0)continue;

            if ($minus < $stock)
            {
                if ($list->isEmpty())
                {
                    # 此时说明只有这1个列队的挂单，全部给它
                    $tmp = min($lastCount, $stock);
                }
                else
                {
                    $tmp = $minus;
                }

                # 库存数减少
                $stock -= $tmp;

                if ($stock > 0)
                {
                    # 没有卖光的重新入队列
                    $list[$id] = $stock;
                }
            }
            else
            {
                $tmp = $stock;
            }

            if (isset($found[$id]))
            {
                # 如果已经有这个，追加进去
                $found[$id] += $tmp;
            }
            else
            {
                $found[$id] = $tmp;
            }

            $used      += $tmp;
            $lastCount -= $tmp;

            if ($lastCount == 0 || $list->isEmpty())break;
        }

        if ($used != $count)
        {
            Server::$instance->warn("[购买] 分配库存数不一致, 需要 {$count}, 实际分配数 {$used}");
        }

        $stockMap['stock'] -= $used;

        return $found;
    }

    /**
     * 移除库存
     *
     * @param ItemList $itemList
     * @param int $goodsId
     * @param \Ds\Map  $stockMap
     */
    public static function removeGoodsInStockMap(ItemList $itemList, $goodsId, \Ds\Map $stockMap)
    {
        if (isset($stockMap['list'][$goodsId]))
        {
            $diff = $stockMap['list'][$goodsId];
            $stockMap['stock'] -= $diff;
            $stockMap['list']->remove($goodsId);
        }

        # 更新库存
        $itemList->stock = $stockMap['stock'];
        $itemList->update();
    }

    /**
     * 通知玩家物品已经无法在交易所出售
     *
     * @param $itemId
     */
    public static function noticePlayerItemCannotTrade($itemId)
    {
        $item = Item::get($itemId);
        if ($item)
        {
            $sql = "SELECT `char_id`, `player_zone_id` FROM `" . self::$TableName . "` WHERE `item_id` = '{$itemId}' AND `stock` > 0 AND `status` IN ('" . self::STATUS_SELLING . "', '" . self::STATUS_EXPIRED . "')";
            $rs  = Server::$mysqlMaster->query($sql);
            if ($rs)
            {
                if ($rs->num_rows)
                {
                    while ($record = $rs->fetch_object())
                    {
                        $zone = Server::$zone->get($record->player_zone_id);
                        if ($zone)
                        {
                            $params = [
                                $item->name
                            ];
                            ZoneForward::sendSysMsgToUserByCharId($zone['fd'], $zone['fromId'], $record->char_id, ActSell::SYS_MSG_ITEM_CANNOT_TRADE, $params);
                        }
                    }

                    $rs->free();
                }

                if (!Server::$mysqlMaster->query("UPDATE `trade_item_info` SET `is_trade` = 0 WHERE `itemid` = '{$itemId}'"))
                {
                    Server::$instance->warn('[交易所挂单] 通知玩家商品不能交易完毕, 但是修改trade_item_info表为不能交易失败!');
                }
                else
                {
                    Server::$instance->info('[交易所挂单] 通知玩家商品不能交易完毕, item id:'. $itemId);
                }
            }
        }
    }

    /**
     * 获取库存列队
     *
     * @param $itemListId
     * @return bool|\Ds\Map
     */
    /*
    public static function getGracefulStockBak($itemListId)
    {
        $eTime = (int)Server::$configExchange['ReceiveTime'];
        $time  = time() - $eTime;
        $rs    = Server::$mysqlMaster->query("SELECT `id`, `stock`, `time` FROM `". self::$TableName ."` WHERE `item_list_id` = ". self::_quoteValue($itemListId) ." AND `stock` > 0 AND `time` > '{$time}' AND `status` = '". self::STATUS_SELLING ."'");

        if ($rs)
        {
            $rst = new \Ds\Map();
            if ($rs->num_rows)
            {
                $map   = new \Ds\Map();
                $list  = new \Ds\Set();
                $maxId = 0;
                $count = 0;
                while ($item = $rs->fetch_object())
                {
                    $stock = (int)$item->stock;
                    if (!isset($map[$stock]))
                    {
                        $map[$stock] = new \Ds\Queue();
                        $list[]      = $stock;
                    }
                    $obj   = new \Ds\Vector();
                    $obj[] = (int)$item->id;            # goodsId
                    $obj[] = $item->time + $eTime;      # 结束时间
                    $obj[] = $stock;                    # 剩余库存数
                    $map[$stock]->push($obj);
                    $maxId = max($maxId, (int)$item->id);
                    $count += $stock;
                }

                $list->sort();
                $rst['list']  = $list;      # 库存数列表
                $rst['map']   = $map;       # 按库存数分组存放的库存对象
                $rst['stock'] = $count;     # 总库存数
            }

            $rs->free();

            return $rst;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error);
            return false;
        }
    }
    */

    /**
     * 获取订单
     *
     * @param \Ds\Map $stockMap
     * @param int $count
     * @return \Ds\Vector
     */
    /*
    public static function getGracefulGoodsBak(\Ds\Map $stockMap, $count)
    {
        $time = time();


        # 读取一个指定数量库存的可用的 goodsId（背包01问题优解）
        #
        # @param int $count 指定库存数
        # @param int $need 需要的库存数
        # @return \Ds\Pair

        $get = function($count, $need = null) use ($time, $stockMap)
        {
            if (!isset($stockMap['map'][$count]))
            {
                return null;
            }

            while (true)
            {
                try
                {

                     # @var \Ds\Vector $obj
                    $obj = $stockMap['map'][$count]->pop();
                    # $obj[0]   # key
                    # $obj[1]   # time
                    # $obj[2]   # stock

                    if ($obj[1] > $time && $obj[3] == 0)
                    {
                        if ($need && $need < $count)
                        {
                            # 只需要一部分库存，把剩下的放在一个新的库存数里
                            $surplus = $count - $need;
                            if (!isset($stockMap['map'][$surplus]))
                            {
                                $stockMap['map'][$surplus] = new \Ds\Queue();
                                $stockMap['list'][]        = $surplus;
                                $stockMap['list']->sort();
                            }

                            # 重新加入队列
                            $stockMap['map'][$surplus]->push($obj);
                        }
                        else
                        {
                            $obj[3] = $count;
                        }

                        # 对应库数空了
                        if ($stockMap['map'][$count]->isEmpty())
                        {
                            $stockMap['list']->remove($count);
                            unset($stockMap['map'][$count]);
                        }

                        return new \Ds\Pair($obj, $need ? $need : $count);
                    }
                }
                catch (\UnderflowException $e)
                {
                    # 没有符合的队列
                    $stockMap['list']->remove($count);
                    unset($stockMap['map'][$count]);
                    break;
                }
            }

            return null;
        };

        $ids = new \Ds\Vector();  # 找到的ID

        # 优先获取完全相同的库存的id
        $found = $get($count);
        if (null !== $found)
        {
            $ids[] = $found;
            return $ids;
        }

        $listCount = $stockMap['list']->count();

        if (!$listCount)return $ids;

        $lastCount = $count;            # 剩余需要的数
        $reserve   = new \Ds\Deque();   # 可选数，先进后出

        # 从最高的库存数往下查找
        for ($i = $listCount - 1; $i >= 0; $i--)
        {
            $cInt = $stockMap['list'][$i];

            if ($cInt == $lastCount)
            {
                $found = $get($cInt);
                if (null !== $found)
                {
                    $ids[] = $found;
                    return $ids;
                }
            }
            elseif ($cInt > $lastCount)
            {
                # 放进储备
                $reserve->push($cInt);

                # 还没到结束
                if ($i > 0)continue;
            }

            # 有储备的
            while (!$reserve->isEmpty())
            {
                $rInt  = $reserve->pop();      # 可选库存数
                $found = $get($rInt, $cInt);   # 在 $rInt 里找 $cInt 个
                if (null !== $found)
                {
                    $ids[] = $found;
                    return $ids;
                }
            }

            # 如果在大于自己的库存数里都没有找到，那么需要在少的里面进行合并了
            $j = 0;
            while (true)
            {
                $j++;
                if ($lastCount >= $cInt)
                {
                    # 剩余需要的数目大于当前数目，直接去获取
                    $found = $get($cInt);
                    if (null !== $found)
                    {
                        $ids[]      = $found;
                        $lastCount -= $cInt;

                        if ($lastCount === 0 || $i === 0)
                        {
                            # 结束
                            return $ids;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    # 剩余数目比当前的小
                    if ($i == 0)
                    {
                        # 已经是结尾了
                        $found = $get($cInt, $lastCount);
                        if (null !== $found)
                        {
                            $ids[] = $found;
                        }

                        # 至此已经没有可用的订单了
                        return $ids;
                    }
                    else
                    {
                        # 加入到可用队列
                        $reserve->push($cInt);
                        break;
                    }
                }
            }
        }

        return $ids;
    }
    */

    /**
     * 更新所有已过期的上架单
     *
     * @return bool
     */
    public static function updateExpiredGoods()
    {
        $time = time() - Server::$configExchange['LogTime'];
        $sql = "UPDATE `". self::$TableName ."` SET `status` = ". self::STATUS_EXPIRED ." WHERE `time` <= {$time}";
        $rs = Server::$mysqlMaster->query($sql);

        if ($rs)
        {
            return true;
        }
        else
        {
            Server::$instance->warn('Query Sql:' . $sql . ' Error:' . Server::$mysqlMaster->error);
            return false;
        }
    }

    /**
     * 存档过期的记录
     *
     * @param \mysqli $mysql
     */
    public static function archiveExpireRecord($mysql)
    {
        $tookExpiredTime = time() - (3 * 86400);
        $tookLogWhere    = "`status` IN ('". self::STATUS_SOLD ."', '". self::STATUS_CANCELED ."') AND `time` < '{$tookExpiredTime}'";

        $sql = "SELECT `id`, `time` FROM `" . self::$TableName . "` WHERE {$tookLogWhere} LIMIT 50000";
        $rs  = $mysql->query($sql);
        if ($rs)
        {
            $fields  = '`'. implode('`, `', self::$Fields) . '`';
            $tmpList = [];

            $countSuccess = 0;
            $countFail    = 0;
            $countAll     = $rs->num_rows;
            while ($row = $rs->fetch_object())
            {
                $month = date('Ym', $row->time);
                $tmpList[$month][] = $row->id;
                if (count($tmpList[$month]) == 1000)
                {
                    # 批量处理
                    $multiRs         = self::multiArchive($mysql, $month, $tmpList[$month]);
                    $tmpList[$month] = [];
                    unset($tmpList[$month]);
                    if ($multiRs)
                    {
                        $countSuccess += 1000;
                    }
                    else
                    {
                        $countFail += 1000;
                    }
                }
            }

            foreach ($tmpList as $month => $idsArr)
            {
                $multiRs = self::multiArchive($mysql, $month, $idsArr);
                if ($multiRs)
                {
                    $countSuccess += count($idsArr);
                }
                else
                {
                    $countFail += count($idsArr);
                }
            }
            unset($tmpList);
            Server::$instance->debug("[清理] 存档 ". self::$TableName ." 表 {$countAll} 行数据, 成功 {$countSuccess}, 失败: {$countFail}");
        }
        else
        {
            Server::$instance->warn('[清理] 执行SQL失败: ' . $sql . '; Error:' . $mysql->error);
        }
    }

    public static function clearNotTookRecord(\RO\MySQLi $mysql)
    {
        static $charLoginTimeCache = [];
        static $count = 0;

        if ($count === 10) {
            $count = 0;
            $charLoginTimeCache = [];
        }

        $globalDb = new MySQLi(Server::$instance->config['gameDb']);
        if (!$globalDb)
        {
            Server::$instance->warn('[清理] 游戏数据库连接失败. Error:' . $globalDb->error);
            return;
        }

        $noLoginTime = time() - (120 * 86400);
        $tookExpiredTime = time() - (198 * 86400);
        $tookLogWhere    = "`status` IN ('". self::STATUS_EXPIRED ."') AND `time` > 0 AND `time` < '{$tookExpiredTime}'";
        $sql = "SELECT `id`, `char_id`, `time`, `item_data` FROM `" . self::$TableName . "` WHERE {$tookLogWhere} LIMIT 10000";
        $rs  = $mysql->query($sql);
        if ($rs && $rs->num_rows > 0)
        {
            $tmpList = [];
            $countSuccess = 0;
            $countFail    = 0;
            $countAll     = 0;
            while ($row = $rs->fetch_object())
            {
                $charId = $row->char_id;
                if (!isset($charLoginTimeCache[$charId]))
                {
                    $sql = "SELECT `offlinetime` FROM `charbase` WHERE `charid` = '{$charId}' LIMIT 1";
                    $globalRs = $globalDb->query($sql);
                    if (!$globalRs)
                    {
                        continue;
                    }
                    else
                    {
                        $charData = $globalRs->fetch_assoc();
                        if ($charData)
                        {
                            $charLoginTimeCache[$charId] = (int)$charData['offlinetime'];
                        }
                        else
                        {
                            $charLoginTimeCache[$charId] = 0;
                        }
                    }
                }

                // 120天未上线的用户, 清理过期订单
                if ($charLoginTimeCache[$charId] > $noLoginTime)
                {
                    continue;
                }

                $month = date('Ym', $row->time);
                $tmpList[$month][] = $row->id;
                if (count($tmpList[$month]) == 1000)
                {
                    # 批量处理
                    $multiRs         = self::multiArchive($mysql, $month, $tmpList[$month]);
                    $tmpList[$month] = [];
                    unset($tmpList[$month]);
                    if ($multiRs)
                    {
                        $countSuccess += 1000;
                    }
                    else
                    {
                        $countFail += 1000;
                    }
                }

                $countAll++;
            }

            foreach ($tmpList as $month => $idsArr)
            {
                $multiRs = self::multiArchive($mysql, $month, $idsArr);
                if ($multiRs)
                {
                    $countSuccess += count($idsArr);
                }
                else
                {
                    $countFail += count($idsArr);
                }
            }
            $count++;
            Server::$instance->info("[清理] 长时间未上线玩家订单存档. ". self::$TableName ." 表 {$countAll} 行数据, 成功 {$countSuccess}, 失败: {$countFail}");
        }

        $globalDb->close();
    }

    /**
     * 批量归档记录
     *
     * @param \mysqli $mysql
     * @param $month
     * @param $idArr
     * @return bool
     */
    protected static function multiArchive($mysql, $month, $idArr)
    {
        $fields  = '`'. implode('`, `', self::$Fields) . '`';
        $ids = implode(',', $idArr);
        $tb  = "zzz_archived_". self::$TableName ."_{$month}";
        $rs  = $mysql->begin_transaction();
        if ($rs)
        {
            try
            {
                $rs = $mysql->query(str_replace('%table', $tb, self::$createArchiveTableSql));
                if (!$rs)return false;
                $rs = $mysql->query("REPLACE INTO `{$tb}` ({$fields}) SELECT {$fields} FROM ". self::$TableName ." WHERE `id` IN ({$ids})");
                if (!$rs)throw new \Exception('err1');
                $rs = $mysql->query("DELETE FROM `" . self::$TableName . "` WHERE `id` IN ({$ids})");
                if (!$rs)throw new \Exception('err2');

                return $mysql->commit();
            }
            catch (\Exception $e)
            {
                $mysql->rollback();
                return false;
            }
        }
        else
        {
            return false;
        }
    }
}