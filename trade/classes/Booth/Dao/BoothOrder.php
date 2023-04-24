<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/6/20
 * Time: 下午12:13
 */

namespace RO\Booth\Dao;

use Ds\Set;
use RO\Cmd\ItemData;
use RO\MySQLi;
use RO\Trade\Dao\Dao;
use RO\Trade\Dao\ItemList;
use RO\Trade\Item;
use RO\Trade\Player;
use RO\Trade\Server;

class BoothOrder extends Dao
{
    const STATUS_PENDING  = 0;
    const STATUS_SELLING  = 1;   // 正常挂单中（物品已扣除成功）
    const STATUS_EXPIRED  = 2;   // 已失效挂单，物品未退回
    const STATUS_CANCELED = 3;   // 已下架挂单，物品已退回
    const STATUS_SOLD     = 4;   // 已卖光

    protected static $TableName = 'booth_order';

    protected static $Fields = [
        'id'           => 'id',
        'itemId'       => 'item_id',         # 物品ID
        'itemKey'      => 'item_key',        # 物品唯一ID
        'charId'       => 'char_id',         # 卖家ID
        'playerName'   => 'player_name',     # 卖家名称
        'playerZoneId' => 'player_zone_id',  # 卖家ZoneId
        'time'         => 'time',            # 挂单时间
        'createdTime'  => 'created_time',    # 创建时间
        'status'       => 'status',          # 状态，见下面状态常量设置
        'count'        => 'count',           # 物品总数
        'stock'        => 'stock',           # 库存数
        'lockStock'    => 'lock_stock',      # 预占库存
        'originCount'  => 'origin_count',    # 源上架订单物品总数
        'isPublicity'  => 'is_publicity',    # 是否公示物品
        'endTime'      => 'end_time',        # 公示结束时间
        'pubPrice'     => 'pub_price',       # 公示价格
        'pubBuyPeople' => 'pub_buy_people',  # 公示购买人数
        'upRate'       => 'up_rate',         # 上调比率
        'downRate'     => 'down_rate',       # 下调比率
        'boothFee'     => 'booth_fee',       # 上架费用
        'quota'        => 'quota',           # 当时上架物品额度单价
        'itemData'     => 'item_data',       # 物品信息protoBuf数据
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
  `item_key` varchar(32) NOT NULL,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_name` varchar(128) CHARACTER SET utf8mb4 DEFAULT '',
  `player_zone_id` int(10) UNSIGNED NOT NULL,
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `created_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `status` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `stock` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `lock_stock` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `origin_count` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `is_publicity` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `end_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `pub_price` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `pub_buy_people` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `up_rate` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `down_rate` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `booth_fee` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `quota` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `status_time` (`status`, `time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='摆摊订单存档表';
EOF;

    public $id;

    public $itemId;

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

    public $createdTime;

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

    public $lockStock;

    public $originCount;

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

    public $upRate;

    public $downRate;

    public $boothFee;

    /**
     * @var \RO\Cmd\ItemData|null
     */
    public $itemData;

    /**
     * 公示购买人数
     * @var int
     */
    public $pubBuyPeople;

    /**
     * 当时上架物品额度单价
     *
     * @var int
     */
    public $quota;

    /**
     * @var ItemList
     */
    private $itemList;

    /**
     * @var Item
     */
    private $item;

    public function insert()
    {
        if ($this->createdTime <= 0) {
            $this->createdTime = time();
        }

        if ($this->time <= 0) {
            $this->time = time();
        }

        return parent::insert();
    }

    /**
     * 挂单是否过期
     *
     * @return bool
     */
    public function isExpired()
    {
        return time() - $this->time >= (Server::$configBooth['exchange_hour'] ?? 86400);
    }

    /**
     * 实际库存
     *
     * @return int
     */
    public function getActualStock()
    {
        return $this->stock - $this->lockStock;
    }

    /**
     * 获取 ItemData
     * @return null|\RO\Cmd\ItemData
     */
    public function getItemData()
    {
        if (null === $this->itemData || '' === $this->itemData)
        {
            return null;
        }

        if (is_string($this->itemData))
        {
            try
            {
                $this->itemData = new ItemData($this->itemData);
            }
            catch (\RuntimeException $e)
            {
                $this->itemData = null;
                Server::$instance->warn("解析 BoothOrder: {$this->id} itemData 数据失败");
            }
        }

        return $this->itemData;
    }

    /**
     * 返回一个itemList数据映射对象
     *
     * @return null|ItemList
     */
    public function getItemList()
    {
        if (null === $this->itemList)
        {
            $this->itemList = ItemList::getByKey($this->itemKey);
        }

        if (!$this->itemList) return null;

        return $this->itemList;
    }

    /**
     * 获取内存item对象
     *
     * @return null|Item
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
                $this->item = Item::get($this->itemId, $this->getItemData());
            }
        }

        return $this->item ? $this->item : null;
    }

    public function getTotalQuota()
    {

    }

    /**
     * 将过期的订单设为待上架状态
     *
     * @return int
     */
    public function setPendingStatus()
    {
        $rs = self::setStatusById($this->id, self::STATUS_PENDING, self::STATUS_EXPIRED);
        if (false !== $rs)
        {
            $this->_old['status'] = $this->status = self::STATUS_PENDING;
        }
        return $rs;
    }

    /**
     * 设置状态
     * 更新数据库时会使用原状态作为乐观锁
     *
     * @param $status
     * @return bool|int
     * 成功返回更新数量,等于0则代表内存中的原状态与数据库的状态不一致,这时代表更新失败,需要重试
     */
    public function setStatus($status)
    {
        $rs = self::setStatusById($this->id, $status, $this->status);
        if (false !== $rs)
        {
            $this->_old['status'] = $this->status = $status;
        }
        return $rs;
    }

    /**
     * 获取是否折叠物品
     *
     * @return int
     */
    public function getIsOverlap()
    {
        if ($this->item !== null)
        {
            $overLap = $this->item->isOverlap;
        }
        else
        {
            $overLap = Server::$item->get($this->itemId, 'isOverlap');
        }

        return $overLap === 1 ? 1 : 0;
    }

    /**
     * 获取当前订单价格
     *
     * @return int
     */
    public function getPrice()
    {
        if ($this->isPublicity == 1)
        {
            return $this->pubPrice;
        }
        else
        {
            $item = $this->getItem();
            return $item !== null ? $item->getPrice() : 0;
        }
    }

    /**
     * 获取调整后的真实价格
     * @return int
     */
    public function getRealPrice()
    {
        return self::calcRealPrice($this, $this->getPrice());
    }

    /**
     * 获取物品的额度单价
     *
     * @return float
     */
    public function getQuota()
    {
        return self::calcQuota($this->getRealPrice());
    }

    /**
     * 计算物品的额度单价
     *
     * @param int $price 物品价格
     * @param int $isPublicity 是否公示
     * @return int
     */
    public static function calcQuota(int $price, int $isPublicity = 0)
    {
        if ($isPublicity)
        {
            $rate = Server::$configBooth['quota_exchange_rate_pub'] ?? 1;
        }
        else
        {
            $rate = Server::$configBooth['quota_exchange_rate'] ?? 1;
        }
        return intval($price * (1 / $rate));
    }

    public static function calcTotalQuota(int $quota, int $count)
    {
        return intval(min($quota * $count, Server::$configBooth['quota_cost_max'] ?? PHP_INT_MAX));
    }

    /**
     * 计算订单调整后价格
     *
     * @param BoothOrder $order
     * @param int $itemPrice 物品价格
     * @return int
     */
    public static function calcRealPrice(BoothOrder $order, int $itemPrice)
    {
        return self::calcAdjustPrice($itemPrice, $order->upRate, $order->downRate);
    }

    /**
     * 计算调整后价格
     *
     * @param int $itemPrice
     * @param int $upRate
     * @param int $downRate
     * @return int
     */
    public static function calcAdjustPrice(int $itemPrice, int $upRate = 0, int $downRate = 0)
    {
        if ($upRate > 0)
        {
            $itemPrice = self::calcUpRatePrice($itemPrice, $upRate);
        }
        else if ($downRate > 0)
        {
            $itemPrice = self::calcDownRatePrice($itemPrice, $downRate);
        }

        return $itemPrice;
    }

    /**
     * 计算上调比率价格
     *
     * @param int $price
     * @param int $rate
     * @return int
     */
    public static function calcUpRatePrice(int $price, int $rate)
    {
        return intval($price * (1 + $rate / 1000));
    }

    /**
     * 计算下调比率价格
     *
     * @param int $price
     * @param int $rate
     * @return int
     */
    public static function calcDownRatePrice(int $price, int $rate)
    {
        return intval($price * (1 - $rate / 1000));
    }

    public static function lockStock(int $orderId, int $stock)
    {
        $sql = "UPDATE `". self::$TableName ."` SET `lock_stock` = `lock_stock` + {$stock} WHERE `stock` - `lock_stock` + {$stock} >= 0 AND `id` = {$orderId}";
        if (IS_DEBUG) {
            Server::$instance->debug($sql);
        }
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs === false)
        {
            return false;
        }
        else
        {
            return Server::$mysqlMaster->affected_rows > 0;
        }
    }

    public static function releaseLockStock(int $orderId, int $stock)
    {
        $sql = "UPDATE `". self::$TableName ."` SET `lock_stock` = `lock_stock` - {$stock} WHERE `lock_stock` - {$stock} >= 0 AND `id` = {$orderId}";
        if (IS_DEBUG) {
            Server::$instance->debug($sql);
        }
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs === false)
        {
            return false;
        }
        else
        {
            return Server::$mysqlMaster->affected_rows > 0;
        }
    }

    public static function deductStock(int $orderId, int $stock)
    {
        $sql = "UPDATE `". self::$TableName ."` SET `stock` = `stock` - {$stock}, `lock_stock` = `lock_stock` - {$stock} WHERE `stock` - {$stock} >= 0 AND `id` = {$orderId}";
        if (IS_DEBUG) {
            Server::$instance->debug($sql);
        }
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs === false)
        {
            return false;
        }
        else
        {
            return Server::$mysqlMaster->affected_rows > 0;
        }
    }

    public static function isSellFull(int $charId)
    {
        $maxCount = Server::$configBooth['base_pending_count'] ?? 8;
        $rs = Server::$mysqlMaster->query("SELECT COUNT(1) as `count` FROM `". self::$TableName ."` WHERE `char_id` = '". intval($charId) ."' AND `status` in (". self::STATUS_SELLING .', '. self::STATUS_EXPIRED .")");
        if ($rs)
        {
            $q = $rs->num_rows ? $rs->fetch_assoc() : false;
            $rs->free();

            if ($q)
            {
                if ($q['count'] >= $maxCount)
                {
                    $pendingLimit = Player::getBoothPendingLimit($charId);
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[摆摊订单] 当前挂单数量:{$q['count']}, 可上架数量:{$pendingLimit}, char_id:{$charId}");
                    }
                    if ($q['count'] >= $pendingLimit)
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
     * 移除一个订单
     *
     * @param     $orderId
     * @param int $status
     * @return bool|int
     */
    public static function removeById(int $orderId, $status = self::STATUS_PENDING)
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
     * 更新订单状态
     *
     * @param $goodsId
     * @param $status
     * @param int $curStatus 符合当前状态的订单才更新
     * @return bool|int
     */
    public static function setStatusById($goodsId, $status, $curStatus = null)
    {
        $goodsId   = self::_quoteValue($goodsId);
        $status    = self::_quoteValue($status);
        $andSql = '';
        if ($curStatus !== null) {
            $curStatus = self::_quoteValue($curStatus);
            $andSql = " AND `status` = {$curStatus}";
        }
        $rs = Server::$mysqlMaster->query("UPDATE `". self::$TableName ."` SET `status` = {$status} WHERE `id` = {$goodsId}{$andSql}");
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
     * 获取卖家订单列表
     *
     * @param int $charId
     * @return BoothOrder[]|null
     */
    public static function getOrdersBySellerId(int $charId)
    {
        $sql = "SELECT `id`, `char_id`, `item_id`, `item_key`, `player_name`, `time`, `status`, `stock`, `lock_stock`, `is_publicity`, `end_time`, `pub_price`, `pub_buy_people`, `up_rate`, `down_rate` FROM `". self::$TableName ."` WHERE `char_id` = '". intval($charId) ."' AND `stock` - `lock_stock` > '0' AND `status` in (". self::STATUS_SELLING .', '. self::STATUS_EXPIRED .") ORDER BY `time` DESC LIMIT 20";
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs && $rs->num_rows > 0)
        {
            $keys  = new Set();
            $others = [];
            /**
             * @var BoothOrder $order
             */
            while ($order = $rs->fetch_object(__CLASS__))
            {
                $others[] = $order;
                $keys->add($order->itemKey);
            }
            $rs->free();

            $itemList = ItemList::getByKeys($keys->toArray());
            foreach ($others as $order)
            {
                $order->itemList = $itemList[$order->itemKey];
            }
            return $others;
        }

        return null;
    }

    public static function getLastPubOrders()
    {
        static $lastRunTime = 0;
        $sql = "SELECT `id`, `item_id`, `item_key`, `end_time`, `time` FROM `". self::$TableName ."` WHERE `status` = " . self::STATUS_SELLING . " AND `is_publicity` = 1 " . ($lastRunTime > 0 ? " AND `time` >= '{$lastRunTime}'" : '') . ' LIMIT 100';
        $rs = Server::$mysqlMaster->query($sql);
        $ret = [];
        if ($rs && $rs->num_rows > 0)
        {
            while ($row = $rs->fetch_assoc())
            {
                $ret[] = $row;
            }
            $lastRunTime = time() - 1200;
        }
        else
        {
            $lastRunTime = time() - 1800;
        }

        return $ret;
    }

    /**
     * 存档过期的记录
     *
     * @param \mysqli $mysql
     */
    public static function archiveExpireRecord($mysql)
    {
        $tookExpiredTime = time() - (3 * 86400);
        $tookLogWhere    = "`status` IN ('". self::STATUS_SOLD ."', '". self::STATUS_CANCELED ."', '". self::STATUS_PENDING ."') AND `time` > 0 AND `time` < '{$tookExpiredTime}'";

        $sql = "SELECT `id`, `time` FROM `" . self::$TableName . "` WHERE {$tookLogWhere} LIMIT 50000";
        $rs  = $mysql->query($sql);
        if ($rs)
        {
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
            Server::$instance->info("[清理] 长时间未上线玩家交易所订单存档. ". self::$TableName ." 表 {$countAll} 行数据, 成功 {$countSuccess}, 失败: {$countFail}");
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

    /**
     * 获取订单额度单价
     *
     * @param $orderId
     * @return bool|int
     */
    public static function getQuotaById($orderId)
    {
        $orderId = intval($orderId);
        $sql = "SELECT `quota` FROM `" . self::$TableName . "` WHERE `id` = '{$orderId}' LIMIT 1";
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs === false)
        {
            return false;
        }

        if ($data = $rs->fetch_assoc())
        {
            return $data['quota'] ?? 0;
        }

        return false;
    }
}