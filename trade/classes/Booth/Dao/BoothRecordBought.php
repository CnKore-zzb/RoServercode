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
use RO\Trade\Player;
use RO\Trade\ROCache;
use RO\Trade\Server;

class BoothRecordBought extends Dao
{
    const STATUS_PAY_PENDING              = 0;     // 待扣款
    const STATUS_PAY_SUCCESS              = 2;     // 扣款成功
    const STATUS_PUBLICITY_PAY_SUCCESS    = 8;     // 公示物品预先支付成功;
    const STATUS_PUBLICITY_SUCCESS        = 6;     // 公示抢购成功，等待领取物品;
    const STATUS_PUBLICITY_CANCEL         = 7;     // 公示订单没有抢购到，无效的订单，等待玩家退款;
    const STATUS_PAY_FAIL                 = 10;    // 扣款失败
    const STATUS_PAY_TIMEOUT              = 11;    // 扣款超时

    protected static $TableName = 'booth_record_bought';

    protected static $Fields = [
        'id'           => 'id',
        'charId'       => 'char_id',         // 买家ID
        'playerName'   => 'player_name',     // 买家角色名
        'playerZoneId' => 'player_zone_id',  // 买家ZoneId
        'orderId'      => 'order_id',        // 挂单ID
        'sellerId'     => 'seller_id',       // 卖家ID
        'sellerName'   => 'seller_name',     // 卖家角色名
        'sellerZoneId' => 'seller_zone_id',  // 卖家ZoneId
        'status'       => 'status',          // 状态
        'takeStatus'   => 'take_status',     // 领取状态
        'canGive'      => 'can_give',        // 是否能赠送
        'itemId'       => 'item_id',         // 物品ID
        'itemKey'      => 'item_key',        // 物品key
        'time'         => 'time',            // 购买时间
        'isPublicity'  => 'is_publicity',    // 是否公示购买记录
        'endTime'      => 'end_time',        // 公示结束时间
        'count'        => 'count',           // 购买成功数
        'totalCount'   => 'total_count',     // 全部购买数（抢购公示期物品数）
        'price'        => 'price',           // 购买价格
        'realPrice'    => 'real_price',      // 物品当时实际价格
        'spendQuota'   => 'spend_quota',     // 花费的额度
        'refineLv'     => 'refine_lv',       // 精炼等级
        'isDamage'     => 'is_damage',       // 是否损坏
        'itemData'     => 'item_data',       // 物品信息
    ];

    /**
     * 存档记录SQL语句

     * @var string
     */
    protected static $createArchiveTableSql = <<<EOF
CREATE TABLE IF NOT EXISTS `%table` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_name` varchar(128) NOT NULL DEFAULT '',
  `player_zone_id` bigint(20) UNSIGNED NOT NULL,
  `order_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `seller_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `seller_name` varchar(128) NOT NULL DEFAULT '',
  `seller_zone_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `status` smallint(4) UNSIGNED NOT NULL DEFAULT '10',
  `take_status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `can_give` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `item_id` int(10) UNSIGNED NOT NULL,
  `item_key` varchar(32) NOT NULL,
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `is_publicity` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `end_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `total_count` int(10) UNSIGNED NOT NULL,
  `price` bigint(20) UNSIGNED NOT NULL,
  `real_price` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `spend_quota` int(10) UNSIGNED NOT NULL,
  `refine_lv` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `is_damage` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `charId_time` (`char_id`,`time`) USING BTREE,
  KEY `takeStatus_time` (`take_status`,`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='摆摊购买交易存档表';
EOF;

    public $id;

    public $charId;

    public $playerName;

    public $playerZoneId;

    public $orderId;

    public $sellerId;

    public $sellerName;

    public $sellerZoneId;

    /**
     * 交易记录状态
     *
     * @var int
     */
    public $status = self::STATUS_PAY_PENDING;

    public $takeStatus = BoothRecordTakeStatus::TAKE_STATUS_CAN_TAKE_GIVE;

    /**
     * 是否能赠送
     *
     * @var int
     */
    public $canGive;

    public $itemId;

    public $itemKey;

    /**
     * 购买时间
     *
     * @var int
     */
    public $time;

    public $isPublicity;

    /**
     * 公示结束时间
     *
     * @var int
     */
    public $endTime;

    /**
     * 成功购买物品数
     *
     * @var int
     */
    public $count;

    /**
     * 购买总数
     *
     * @var int
     */
    public $totalCount;

    /**
     * 购买价格
     *
     * @var int
     */
    public $price;

    /**
     * 物品当时实际价格
     *
     * @var int
     */
    public $realPrice;

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
     * 花费额度
     *
     * @var int
     */
    public $spendQuota;

    /**
     * 物品信息
     *
     * @var ItemData
     */
    public $itemData;

    /**
     * 获取 ItemData
     * @return null|\RO\Cmd\ItemData
     */
    public function getItemData()
    {
        if (null === $this->itemData || '' === $this->itemData) return null;

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
     * 计算花费额度
     * @param int $price 物品单价
     * @return mixed
     */
    public function getSpendQuota($price)
    {
        if ($this->isPublicity)
        {
            $rate = Server::$configBooth['quota_exchange_rate_pub'] ?? 1;
        }
        else
        {
            $rate = Server::$configBooth['quota_exchange_rate'] ?? 1;
        }
        $max   = Server::$configBooth['quota_cost_max'] ?? PHP_INT_MAX;
        return min($max, intval($price * (1 / $rate)));
    }

    public function getDiscountPrice($price)
    {
        return self::calcDiscountPrice($price);
    }

    /**
     * 是否能赠送
     *
     * @param $curQuota
     * @return int
     */
    public function isCanGive(&$curQuota = null)
    {
        $total = $this->price * $this->count;
        return self::canGive($this->charId, $total, $curQuota);
    }

    /**
     * 是否能赠送
     *
     * @param $charId
     * @param $totalPrice
     * @param null $curQuota
     * @return int
     */
    public static function canGive($charId, $totalPrice, &$curQuota = null)
    {
        if ($curQuota === null)
        {
            $curQuota = Player::getQuota($charId);
        }

        $sendMoneyLimit = Server::$configExchange['SendMoneyLimit'] ?? 100000;
        return $totalPrice >= $sendMoneyLimit && $curQuota >= $totalPrice ? 1 : 0;
    }

    /**
     * 删除过期记录
     *
     * @param \mysqli $mysql
     */
    public static function archiveExpireRecord($mysql)
    {
        $tookExpiredTime = time() - (3 * 86400);
        $takeStatus      = BoothRecordTakeStatus::TAKE_STATUS_TOOK;
        $tookLogWhere    = "(`take_status` = '{$takeStatus}' AND `time` < '{$tookExpiredTime}')";

        $notTakeExpiredTime = time() - (30 * 86400);
        $notTakeStatus      = BoothRecordTakeStatus::TAKE_STATUS_CAN_TAKE_GIVE;
        $notTakeLogWhere    = "(`take_status` = '{$notTakeStatus}' AND `time` < '{$notTakeExpiredTime}')";

        $statusExpiredTime = time() - 3600;
        $orWhere = "(`status` IN (" . BoothRecordBought::STATUS_PAY_FAIL . ", " . BoothRecordBought::STATUS_PAY_TIMEOUT . ") AND `time` < '$statusExpiredTime')";

        $sql = "SELECT `id`, `time` FROM `" . self::$TableName . "` WHERE {$tookLogWhere} OR {$notTakeLogWhere} OR {$orWhere} LIMIT 50000";
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
                        $rs = $mysql->query("REPLACE INTO `{$tb}` ({$fields}) SELECT {$fields} FROM " . self::$TableName . " WHERE `id` IN ({$ids})");
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
     * 获取买家公示购买数量
     *
     * @param $orderId
     * @param $charId
     * @return bool|int
     */
    public static function getPubBuyCount($orderId, $charId)
    {
        $sql = "SELECT SUM(`total_count`) as `count` FROM `" . BoothRecordBought::getTableName() . "` WHERE `order_id` = '{$orderId}' AND `char_id` = '{$charId}' AND `status` = '" . BoothRecordBought::STATUS_PUBLICITY_PAY_SUCCESS . "'";
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs === false)
        {
            return false;
        }
        else if ($row = $rs->fetch_assoc())
        {
            return $row['count'] ?? 0;
        }

        return false;
    }

    public static function calcDiscountPrice($price, $quota = null)
    {
        if ($quota === null)
        {
            $quota = $price;
        }
        $discount = Server::$configBooth['quota_zeny_discount'] ?? 4;
        return $price - intval($quota * 1 / $discount);
    }
}