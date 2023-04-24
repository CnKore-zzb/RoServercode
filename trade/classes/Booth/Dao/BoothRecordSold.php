<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/6/29
 * Time: 下午4:07
 */

namespace RO\Booth\Dao;

use RO\Cmd\ItemData;
use RO\Trade\Dao\Dao;
use RO\Trade\Dao\RecordSold;
use RO\Trade\Server;

class BoothRecordSold extends Dao
{
    const STATUS_ADDING = 0;     # 待领取钱款

    protected static $TableName = 'booth_record_sold';

    protected static $Fields = [
        'id'            => 'id',
        'charId'        => 'char_id',         # 卖家ID
        'playerName'    => 'player_name',     # 卖家名称
        'playerZoneId'  => 'player_zone_id',  # 卖家ZoneId
        'orderId'       => 'order_id',        # 挂单ID
        'buyerId'       => 'buyer_id',        # 买家ID
        'buyerName'     => 'buyer_name',      # 买家名称
        'buyerZoneId'   => 'buyer_zone_id',   # 买家zone id
        'itemId'        => 'item_id',         # 物品ID
        'itemKey'       => 'item_key',        # 物品key
        'status'        => 'status',          # 状态，见下面状态常量设置
        'isPublicity'   => 'is_publicity',    # 是否公示购买记录
        'takeStatus'    => 'take_status',     # 领取状态
        'time'          => 'time',            # 销售时间
        'count'         => 'count',           # 销售数
        'price'         => 'price',           # 销售价格
        'quota'         => 'quota',           # 额度单价
        'refineLv'      => 'refine_lv',       # 精炼等级
        'isDamage'      => 'is_damage',       # 是否损坏
        'tax'           => 'tax',             # 手续费
        'itemData'      => 'item_data',       # 物品信息
        'buyersInfo'    => 'buyers_info'      # 多个购买人信息
    ];

    protected static $createArchiveTableSql = <<<EOF
CREATE TABLE IF NOT EXISTS `%table` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_name` varchar(128) NOT NULL DEFAULT '',
  `player_zone_id` int(10) UNSIGNED NOT NULL,
  `order_id` bigint(20) UNSIGNED NOT NULL,
  `buyer_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `buyer_name` varchar(128) NOT NULL DEFAULT '',
  `buyer_zone_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `item_id` int(10) UNSIGNED NOT NULL,
  `item_key` varchar(32) NOT NULL,
  `is_publicity` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `status` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `take_status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `price` bigint(20) UNSIGNED NOT NULL,
  `quota` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `refine_lv` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `is_damage` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `tax` int(10) UNSIGNED NOT NULL,
  `buyers_info` longblob,
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `order_id` (`order_id`),
  KEY `charId_time` (`char_id`,`time`) USING BTREE,
  KEY `takeStatus_time` (`take_status`,`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='摆摊出售交易记录存档表';
EOF;

    public $id;

    public $charId;

    public $playerName;

    public $playerZoneId;

    public $orderId;

    public $buyerId;

    public $buyerName;

    public $buyerZoneId;

    public $itemId;

    public $itemKey;

    /**
     * 交易记录状态
     *
     * @var int
     */
    public $status = self::STATUS_ADDING;

    public $takeStatus = BoothRecordTakeStatus::TAKE_STATUS_CAN_TAKE_GIVE;

    /**
     * 购买时间
     *
     * @var int
     */
    public $time;

    public $isPublicity;

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
     * 额度单价
     *
     * @var int
     */
    public $quota;

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
     * @var ItemData
     */
    public $itemData;

    public $buyersInfo;

    /**
     * 交易手续费
     *
     * @return int
     */
    public function getTax()
    {
        return intval($this->count * $this->price * RecordSold::getExchangeRate($this->price));
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
        if ($rs === false)
        {
            return 0;
        }
        else
        {
            $data = $rs->fetch_object();
            return (int)$data->soldCount;
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
        $takeStatus         = BoothRecordTakeStatus::TAKE_STATUS_TOOK;
        $tookLogWhere       = "(`take_status` = '{$takeStatus}' AND `time` < '{$tookExpiredTime}')";

        $notTakeExpiredTime = time() - (30 * 86400);
        $notTakeStatus      = BoothRecordTakeStatus::TAKE_STATUS_CAN_TAKE_GIVE;
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
}