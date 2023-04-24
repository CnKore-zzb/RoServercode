<?php
namespace RO\Trade\Dao;

use Ds\Set;
use RO\Cmd\EOperType;
use RO\Cmd\ItemData;
use RO\Cmd\LogItemInfo;
use RO\Trade\Server;

/**
 * 销售记录表
 *
 * @package RO\Trade\Dao
 */
class RecordSold extends Record
{
    public $id;

    public $charId;

    public $playerZoneId;

    public $goodsId;

    public $itemId;

    /**
     * 交易记录状态
     *
     * @var int
     */
    public $status = self::STATUS_ADDING;

    /**
     * 购买时间
     *
     * @var int
     */
    public $time;

    /**
     * 物品数
     *
     * @var int
     */
    public $count;

    /**
     * 购买价格
     *
     * @var int
     */
    public $price;

    /**
     * 是否损坏
     *
     * @var int
     */
    public $isDamage;

    /**
     * 精炼等级
     *
     * @var int
     */
    public $refineLv;

    /**
     * 手续费
     *
     * @var int
     */
    public $tax;

    /**
     * 是否来自多个买家
     *
     * 如果挂单数大于1，则在抢购的公示期订单会产生多个玩家购买1个挂单的情况，此时会产生1条销售记录，n条购买记录
     *
     * @var int
     */
    public $isManyPeople = 0;

    /**
     * 买家信息
     *
     * @var \RO\Cmd\NameInfo
     */
    public $buyerInfo;

    /**
     * 买家信息
     *
     * @var \RO\Cmd\NameInfoList
     */
    public $buyersInfo;

    /**
     * @var ItemData
     */
    public $itemData;

    public $logId;

    /** @var Goods */
    protected $goods;

    protected static $TableName = 'trade_record_sold';

    protected static $Fields = [
        'id'           => 'id',
        'logId'        => 'log_id',          # 防止重复插入
        'charId'       => 'char_id',         # 卖家ID
        'playerZoneId' => 'player_zone_id',  # 卖家ID
        'goodsId'      => 'goods_id',        # 挂单ID
        'itemId'       => 'item_id',         # 物品ID
        'status'       => 'status',          # 状态，见下面状态常量设置
        'takeStatus'   => 'take_status',     # 领取赠送状态, 常量见Record父类
        'time'         => 'time',            # 销售时间
        'count'        => 'count',           # 销售数
        'price'        => 'price',           # 销售价格
        'refineLv'     => 'refine_lv',       # 精炼等级
        'isDamage'     => 'is_damage',       # 是否损坏
        'tax'          => 'tax',             # 手续费
        'isManyPeople' => 'is_many_people',  # 是否来自多个玩家
        'buyerInfo'    => 'buyer_info',      # 买家信息（如果是公示物品被多个玩家抢购）
        'buyersInfo'   => 'buyers_info',     # 多个买家信息（如果是公示物品被多个玩家抢购）
        'itemData'     => 'item_data',       # 兼容迁移数据字段, 当goods_id为0时,可获取item_data
    ];

    /**
     * 存档记录SQL语句
     *
     * @var string
     */
    protected static $createArchiveTableSql = <<<EOF
CREATE TABLE IF NOT EXISTS `%table` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `log_id` char(32) NOT NULL,
  `player_zone_id` int(10) UNSIGNED NOT NULL,
  `goods_id` bigint(20) UNSIGNED NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL,
  `status` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `take_status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `price` int(10) UNSIGNED NOT NULL,
  `refine_lv` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `is_damage` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `tax` int(10) UNSIGNED NOT NULL,
  `is_many_people` tinyint(1) UNSIGNED NOT NULL,
  `buyer_info` tinyblob,
  `buyers_info` longblob,
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `char_id` (`char_id`),
  KEY `time` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='出售交易记录存档表';
EOF;


    const STATUS_ADDING     = 0;     # 待领取钱款
    const STATUS_AUTO_SHELF = 9;     # 兼容不能交易商品的自动下架状态
//    const STATUS_FINISH = 1;     //已完成

    function __construct($data = null)
    {
        parent::__construct($data);

        if ($this->buyerInfo && is_string($this->buyerInfo))
        {
            try
            {
                $this->buyerInfo = new \RO\Cmd\NameInfo($this->buyerInfo);
            }
            catch (\RuntimeException $e)
            {
                $this->buyerInfo = null;
                Server::$instance->warn("解析 RecordSold: {$this->id} buyerInfo 数据失败");
            }
        }
    }

    /**
     * 获取 logId
     *
     * @return string
     */
    public function createLogId()
    {
        return md5($this->charId . $this->itemId . $this->time . $this->price . mt_rand(0, 999999) . microtime());
    }

    public function insert()
    {
        if (!$this->time)
        {
            $this->time = time();
        }

        if (!$this->logId)
        {
            $this->logId = $this->createLogId();
        }

        return parent::insertIgnore();
    }

    /**
     * 交易手续费
     *
     * @return int
     */
    public function getTax()
    {
        return intval($this->count * $this->price * self::getExchangeRate($this->price));
    }

    /**
     * @return bool|Goods
     */
    public function getGoods()
    {
        if (null === $this->goods)
        {
            $this->goods = Goods::getById($this->goodsId);
        }

        return $this->goods;
    }

    public function setGoods($goods)
    {
        $this->goods = $goods;
    }

    public function setMyStatus($status)
    {
        return self::setStatus($this->id, $status);
    }

    public function setMyTakeStatus($status)
    {
        return self::setTakeStatus($this->id, $status);
    }

    /**
     * 获取买者信息
     * isManyPeople = 1时返回\RO\Cmd\NameInfoList,反之\RO\Cmd\NameInfo
     *
     * @return null|\RO\Cmd\NameInfo|\RO\Cmd\NameInfoList
     */
    public function getBuyersInfo()
    {
        if ($this->buyersInfo && is_string($this->buyersInfo))
        {
            try
            {
                $this->buyersInfo = new \RO\Cmd\NameInfoList($this->buyersInfo);
            }
            catch (\RuntimeException $e)
            {
                $this->buyersInfo = null;
                Server::$instance->warn("解析 RecordSold: {$this->id} buyersInfo 数据失败");
            }
        }

        return $this->buyersInfo;
    }

    /**
     * 获取兼容迁移数据的item_data
     *
     * @return ItemData
     */
    public function getItemData()
    {
        if ($this->goodsId != 0 && null === $this->itemData || '' === $this->itemData)
        {
            return null;
        }

        if (is_string($this->itemData))
        {
            try
            {
                $this->itemData = new \RO\Cmd\ItemData($this->itemData);
            }
            catch (\RuntimeException $e)
            {
                $this->itemData = null;
                Server::$instance->warn("解析 RecordSold: {$this->id} itemData 数据失败");
            }
        }

        return $this->itemData;
    }

    /**
     * 获取一个LogItemInfo对象
     *
     * @return LogItemInfo
     */
    public function getLogItemInfo()
    {
        if ($this->goodsId == 0)
        {
            $itemData = $this->getItemData();
        }
        else
        {
            $goods = $this->getGoods();
            $itemData = $goods->getItemData();
        }

        $logInfo = new LogItemInfo();
        $logInfo->setId($this->id);
        $logInfo->setItemid($this->itemId);
        $logInfo->setRefineLv($itemData ? $itemData->equip->refinelv : 0);
        $logInfo->setDamage($itemData ? $itemData->equip->damage : false);
        $logInfo->setTradetime($this->time);
        $logInfo->setCount($this->count);
        $logInfo->setPrice($this->price);
        $logInfo->setTax($this->tax);
        $logInfo->setGetmoney($this->price * $this->count - $this->tax);
        $logInfo->setIsManyPeople($this->isManyPeople);
        $logInfo->setStatus($this->takeStatus);
        $logInfo->setLogtype(EOperType::EOperType_NormalSell);
        if ($this->buyerInfo)
        {
            $logInfo->setNameInfo($this->buyerInfo);
        }

        return $logInfo;
    }

    /**
     * 更新状态
     *
     * @param $id
     * @param $status
     * @return bool|int
     */
    public static function setStatus($id, $status)
    {
        $id     = self::_quoteValue($id);
        $status = self::_quoteValue($status);
        $rs     = Server::$mysqlMaster->query("UPDATE `". self::$TableName ."` SET `status` = {$status} WHERE `id` = ". $id);
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
     * 更新操作状态
     *
     * @param $id
     * @param $status
     * @return bool|int
     */
    public static function setTakeStatus($id, $status)
    {
        $id     = self::_quoteValue($id);
        $status = self::_quoteValue($status);
        $rs     = Server::$mysqlMaster->query("UPDATE `". self::$TableName ."` SET `take_status` = {$status} WHERE `id` = ". $id);
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
     * 获取可领取数
     *
     * @param $charId
     * @return bool
     */
    public static function getCanTakeCount($charId)
    {
        $charId = self::_quoteValue($charId);
        $sql    = "SELECT COUNT(1) AS `count` FROM " . self::$TableName . " WHERE `char_id` = {$charId} AND `status` = " . self::STATUS_ADDING;
        $rs     = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            $data = $rs->fetch_object();
            $rs->free();
            return $data->count;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error);
        }

        return false;
    }

    /**
     * 获取卖家记录
     *
     * @param $charId
     * @return array|bool
     */
    public static function getSellList($charId)
    {
        $charId = self::_quoteValue($charId);
        $sql    = "SELECT * FROM " . self::$TableName . " WHERE `char_id` = {$charId}";
        $rs     = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            $list = [];
            $goodIds = new Set();
            while($data = $rs->fetch_object(__CLASS__))
            {
                $list[] = $data;
                $goodIds->add($data->goodsId);
            }
            $rs->free();

            $goods = Goods::getByIds($goodIds->toArray());

            /** @var RecordBought $item */
            foreach ($list as $item)
            {
                if ($item->goodsId)
                {
                    $item->goods = $goods[$item->goodsId];
                }
            }

            return $list;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error);
        }

        return false;
    }

    /**
     * 获取物品出售数量
     * 当填写近期时间, 则代表获取近期时间内的物品销量
     *
     * @param $itemId
     * @param int $currentTime  当前时间
     * @param int $nearTime     近期时间
     * @return int|bool
     */
    public static function getSoldCount($itemId, $currentTime = 0, $nearTime = 0)
    {
        if ($currentTime === 0) $currentTime = time();
        $itemId = self::_quoteValue($itemId);
        $time   = intval($currentTime - $nearTime);
        $sql    = "SELECT SUM(`count`) AS `soldCount` FROM `".self::$TableName."` WHERE `item_id` = {$itemId} AND `time` >= '{$time}'";
        $rs     = Server::$mysqlMaster->query($sql);

        if ($rs && ($data = $rs->fetch_object()))
        {
            return (int)$data->soldCount;
        }
        else
        {
            Server::$instance->warn('Query Sql: ' . $sql . '; Error:' . Server::$mysqlMaster->error);
            return false;
        }
    }

    /**
     * 删除过期记录
     *
     * @param \mysqli $mysql
     */
    public static function archiveExpireRecord($mysql)
    {
        $tookExpiredTime    = time() - (3 * 86400);
        $takeStatus         = Record::TAKE_STATUS_TOOK;
        $tookLogWhere       = "(`take_status` = '{$takeStatus}' AND `time` < '{$tookExpiredTime}')";

        $notTakeExpiredTime = time() - (30 * 86400);
        $notTakeStatus      = Record::TAKE_STATUS_CAN_TAKE_GIVE;
        $notTakeLogWhere    = "(`take_status` = '{$notTakeStatus}' AND `time` < '{$notTakeExpiredTime}')";

        $sql = "SELECT `id`, `time` FROM `" . self::$TableName . "` WHERE {$tookLogWhere} OR {$notTakeLogWhere} LIMIT 50000";
        $rs  = $mysql->query($sql);
        if ($rs)
        {
            $fields  = '`'. implode('`, `', self::$Fields) . '`';
            $tmpList = [];

            # 批量处理
            $multiQuery = function($month, $idArr) use ($mysql, $fields)
            {
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
            };

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
                    $multiRs         = $multiQuery($month, $tmpList[$month]);
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
                $multiRs = $multiQuery($month, $idsArr);
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

    /**
     * 获取交易所税率
     *
     * @param int $price 单价
     * @return int 税率
     */
    public static function getExchangeRate($price)
    {
        if ($price <= 100 * 10000)
        {
            return 0.09;
        }
        if ($price <= 500 * 10000)
        {
            return 0.10;
        }
        if ($price <= 1000 * 10000)
        {
            return 0.11;
        }
        if ($price <= 2500 * 10000)
        {
            return 0.12;
        }
        if ($price <= 5000 * 10000)
        {
            return 0.13;
        }
        if ($price <= 10000 * 10000)
        {
            return 0.14;
        }
        return 0.15;
    }
}