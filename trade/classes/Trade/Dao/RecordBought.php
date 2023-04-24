<?php
namespace RO\Trade\Dao;

use Ds\Set;
use RO\Cmd\EOperType;
use RO\Cmd\LogItemInfo;
use RO\Trade\Player;
use RO\Trade\Server;

/**
 * 购买记录表
 *
 * @package RO\Trade\Dao
 */
class RecordBought extends Record
{
    public $id;

    public $charId;

    public $playerName;

    public $playerZoneId;

    public $goodsId;

    /**
     * 交易记录状态
     *
     * @var int
     */
    public $status = self::STATUS_PAY_SUCCESS;

    public $itemId;

    /**
     * 购买时间
     *
     * @var int
     */
    public $time;

    /**
     * 公示期物品ID
     *
     * @var int
     */
    public $publicityId;

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
     * 是否来自多个卖家
     *
     * 如果是购买堆叠物品，一次购买多个时，会出现多个卖家的情况，会产生1条购买记录，n条销售记录
     *
     * @var int
     */
    public $isManyPeople = 0;

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
     * 物品信息
     *
     * @var \RO\Cmd\ItemData
     */
    public $itemData;

    /**
     * @var \RO\Cmd\NameInfo
     */
    public $sellerInfo;

    /**
     * @var \RO\Cmd\NameInfoList
     */
    public $sellersInfo;

    /**
     * 是否能赠送
     *
     * @var int
     */
    public $canGive;

    /**
     * 赠送额度
     *
     * @var int
     */
    public $quota;

    public $logId;

    /** @var Goods */
    protected $goods;

    protected static $TableName = 'trade_record_bought';

    protected static $Fields = [
        'id'           => 'id',
        'logId'        => 'log_id',          # 防止重复插入
        'charId'       => 'char_id',         # 买家ID
        'playerName'   => 'player_name',     # 买家角色名
        'playerZoneId' => 'player_zone_id',  # 买家ZoneId
        'goodsId'      => 'goods_id',        # 挂单ID，堆叠物品涉及到多个的话则为0
        'status'       => 'status',          # 状态，见下面状态常量设置
        'takeStatus'   => 'take_status',     # 领取赠送状态, 常量见Record父类
        'canGive'      => 'can_give',        # 是否能赠送, 不能作为实际赠送时候依据的字段
        'itemId'       => 'item_id',         # 物品ID
        'time'         => 'time',            # 购买时间
        'publicityId'  => 'publicity_id',    # 公示物品Id
        'endTime'      => 'end_time',        # 公示结束时间
        'count'        => 'count',           # 购买成功数
        'totalCount'   => 'total_count',     # 全部购买数（抢购公示期物品数）
        'price'        => 'price',           # 购买价格
        'refineLv'     => 'refine_lv',       # 精炼等级
        'isDamage'     => 'is_damage',       # 是否损坏
        'isManyPeople' => 'is_many_people',  # 是否来自多个玩家
        'sellerInfo'   => 'seller_info',     # 卖家信息（如果物品来自多个卖家的话）
        'sellersInfo'  => 'sellers_info',    # 多个卖家信息（如果物品来自多个卖家的话）
        'itemData'     => 'item_data',       # 物品信息
    ];

    /**
     * 存档记录SQL语句

     * @var string
     */
    protected static $createArchiveTableSql = <<<EOF
CREATE TABLE IF NOT EXISTS `%table` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `log_id` char(32) NOT NULL,
  `player_name` varchar(128) NOT NULL DEFAULT '',
  `player_zone_id` bigint(20) UNSIGNED NOT NULL,
  `goods_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `take_status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `can_give` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `item_id` int(10) UNSIGNED NOT NULL,
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `publicity_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `end_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `total_count` int(10) UNSIGNED NOT NULL,
  `price` int(10) UNSIGNED NOT NULL,
  `refine_lv` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `is_damage` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `is_many_people` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `seller_info` tinyblob,
  `sellers_info` longblob,
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `char_id` (`char_id`),
  KEY `time` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='购买交易记录存档表';
EOF;
    const STATUS_PAY_SUCCESS              = 0;     //扣款成功;
//    const STATUS_FINISH                   = 1;     //购买成功并且已经领取到相应物品（含公示购买）;
    const STATUS_PUBLICITY_PAY_SUCCESS    = 2;     //公示物品预先支付成功;
    const STATUS_PUBLICITY_SUCCESS        = 3;     //公示抢购成功，等待领取物品;
    const STATUS_PUBLICITY_CANCEL         = 4;     //公示订单没有抢购到，无效的订单，等待玩家退款;
//    const STATUS_PUBLICITY_CANCEL_SUCCESS = 5;     //公示订单退款成功;

    /**
     * 单个人最多上架数目
     *
     * @var int
     */
    const FULL_NUM = 8;

    function __construct($data = null)
    {
        parent::__construct($data);

        if ($this->sellerInfo && is_string($this->sellerInfo))
        {
            try
            {
                $this->sellerInfo = new \RO\Cmd\NameInfo($this->sellerInfo);
            }
            catch (\RuntimeException $e)
            {
                $this->sellerInfo = null;
                Server::$instance->warn("解析 RecordBought: {$this->id} sellerInfo 数据失败");
            }
        }
    }

    /**
     * 获取一个新的logId
     *
     * @return string
     */
    public function createLogId($isPublicity = false)
    {
        if ($isPublicity)
        {
            return md5($this->charId . $this->itemId . $this->price . $this->publicityId . $this->endTime . $this->status);
        }
        else
        {
            return md5($this->charId . $this->itemId . $this->time . $this->price . $this->count . $this->totalCount . mt_rand(0, 999999) . microtime());
        }
    }

    public function insert()
    {
        if (!$this->time)
        {
            $this->time = time();
        }

        if (!$this->canGive)
        {
            $this->canGive = $this->isCanGive();
            Server::$instance->info("[是否能赠送] charId:{$this->charId}, 玩家:{$this->playerName}, 订单id:{$this->goodsId}, 公示id:{$this->publicityId}, "
                                              . "itemId:{$this->itemId}, 价格:{$this->price}, 购买的数量:{$this->totalCount}, 赠送情况:" . ($this->canGive ? '能' : '不能') . ", 额度:{$this->quota}");
        }

        if (!$this->logId)
        {
            $this->logId = $this->createLogId();
        }

        return parent::insertIgnore();
    }

    /**
     * 插入公示购买记录
     *
     * @return bool
     */
    public function insertForPublicity()
    {
        if (!($this->publicityId > 0 && $this->endTime > 0))
        {
            return $this->insert();
        }

        if (!$this->time)
        {
            $this->time = time();
        }

        if (!$this->logId)
        {
            $this->logId = $this->createLogId(true);
        }

        $fields = [];
        $values = [];
        foreach (static::$Fields as $key => $field)
        {
            if (isset($this->$key))
            {
                $values[]     = self::_quoteValue($this->$key);
                $fields[]     = $field;
            }
        }

        $sql = "INSERT INTO `". static::$TableName ."` (`". implode('`, `', $fields) ."`) VALUES (" . implode(", ", $values) . ") ON DUPLICATE KEY UPDATE `total_count` = `total_count` + " . intval($this->totalCount);

        if (IS_DEBUG)
        {
            Server::$instance->debug($sql);
        }

        if (Server::$mysqlMaster->query($sql))
        {
            if (Server::$mysqlMaster->affected_rows > 0)
            {
                $id = static::_idKey();
                foreach (static::$Fields as $key => $field)
                {
                    if (isset($this->$key))
                    {
                        $this->_old[$field] = $this->$key;
                    }
                }

                if ($this->$id)
                {
                    # 非自增
                    $this->_old[static::$IdField] = $this->$id;
                }
                else
                {
                    $this->_old[static::$IdField] = $this->$id = Server::$mysqlMaster->insert_id;
                }
            }

            return true;
        }
        else
        {
            Server::$instance->warn($sql .'; error: '. Server::$mysqlMaster->error);
            return false;
        }
    }

    /**
     * 获取挂单对象
     *
     * @return false|null|Goods
     */
    public function getGoods()
    {
        if (null === $this->goods)
        {
            if ($this->goodsId)
            {
                $this->goods = Goods::getById($this->goodsId);
            }
            else
            {
                return null;
            }
        }

        return $this->goods;
    }

    public function setGoods($goods)
    {
        $this->goods = $goods;
    }

    /**
     * 是否能赠送
     *
     * @return bool
     */
    public function isCanGive()
    {
        $sendMoneyLimit = isset(Server::$configExchange['SendMoneyLimit']) ? Server::$configExchange['SendMoneyLimit'] : 100000;
        $this->quota = $quota = Player::getQuota($this->charId);
        $total = $this->price * $this->count;
        return $total >= $sendMoneyLimit && $quota >= $total;
    }

    /**
     * 获取卖家信息
     * isManyPeople = 1时返回\RO\Cmd\NameInfoList,反之\RO\Cmd\NameInfo
     *
     * @return null|\RO\Cmd\NameInfo|\RO\Cmd\NameInfoList
     */
    public function getSellersInfo()
    {
        if ($this->sellersInfo && is_string($this->sellersInfo))
        {
            try
            {
                $this->sellersInfo = new \RO\Cmd\NameInfoList($this->sellersInfo);
            }
            catch (\RuntimeException $e)
            {
                $this->sellersInfo = null;
                Server::$instance->warn("解析 RecordBought: {$this->id} sellersInfo 数据失败");
            }
        }

        return $this->sellersInfo;
    }

    /**
     * 获取 ItemData
     *
     * @return null|\RO\Cmd\ItemData
     */
    public function getItemData()
    {
        if ($this->itemData)
        {
            if (is_string($this->itemData))
            {
                try
                {
                    $this->itemData = new \RO\Cmd\ItemData($this->itemData);
                }
                catch (\RuntimeException $e)
                {
                    $this->itemData = null;
                    Server::$instance->warn("解析 RecordBought: {$this->id} itemData 数据失败");
                }
            }
        }

        return $this->itemData;
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
     * 获取一个LogItemInfo对象
     *
     * @return LogItemInfo
     */
    public function getLogItemInfo()
    {
        $logInfo = new LogItemInfo();
        $logInfo->setId($this->id);
        $logInfo->setItemid($this->itemId);
        $logInfo->setRefineLv($this->getItemData() ? $this->getItemData()->equip->refinelv : 0);
        $logInfo->setDamage($this->getItemData() ? $this->getItemData()->equip->damage : false);
        $logInfo->setTradetime($this->time);
        $logInfo->setCount($this->count);
        $logInfo->setPrice($this->price);
        $logInfo->setCostmoney($this->price * $this->count);
        $logInfo->setRetmoney(($this->totalCount - $this->count) * $this->price);
        $logInfo->setFailcount($this->totalCount - $this->count);
        $logInfo->setIsManyPeople($this->isManyPeople);
        $logInfo->setTotalcount($this->totalCount);
        $logInfo->setStatus($this->takeStatus);
        $logInfo->setLogtype(EOperType::EOperType_NoramlBuy);
        if ($this->sellerInfo)
        {
            $logInfo->setNameInfo($this->sellerInfo);
        }

        if ($this->getItemData())
        {
            $logInfo->setItemdata($this->getItemData());
        }

        switch ($this->status)
        {
            case RecordBought::STATUS_PUBLICITY_PAY_SUCCESS:
                $logInfo->setCostmoney($this->price * $this->totalCount);
                $logInfo->setCount($this->totalCount);
                $logInfo->setEndtime($this->endTime);
                $logInfo->setLogtype(EOperType::EOperType_PublicityBuying);
                break;
            case RecordBought::STATUS_PUBLICITY_SUCCESS:
                $logInfo->setLogtype(EOperType::EOperType_PublicityBuySuccess);
                break;
            case RecordBought::STATUS_PUBLICITY_CANCEL:
                $logInfo->setLogtype(EOperType::EOperType_PublicityBuyFail);
                break;
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
     * 获取领取数
     *
     * @param $charId
     * @return bool
     */
    public static function getCanTakeCount($charId)
    {
        $charId = self::_quoteValue($charId);
        $sql    = "SELECT COUNT(1) AS `count` FROM " . self::$TableName . " WHERE `char_id` = {$charId} AND `status` IN (" . self::STATUS_PAY_SUCCESS . ", ". self::STATUS_PUBLICITY_PAY_SUCCESS .", ". self::STATUS_PUBLICITY_CANCEL .")";
        $rs     = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            $data = $rs->fetch_object();
            $rs->free();
            Server::$redis->hSet('bought_take_count', $charId, $data->count);
            return $data->count;
        }
        else
        {
            Server::$instance->warn('Sql: '. $sql .'; Error: '. Server::$mysqlMaster->error);
        }

        return false;
    }

    /**
     * 获取买家记录
     *
     * @param $charId
     * @return array|bool
     */
    public static function getBuyList($charId)
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
                if ($data->goodsId) $goodIds->add($data->goodsId);
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
     * 删除过期记录
     *
     * @param \mysqli $mysql
     */
    public static function archiveExpireRecord($mysql)
    {
        $tookExpiredTime = time() - (3 * 86400);
        $takeStatus      = Record::TAKE_STATUS_TOOK;
        $tookLogWhere    = "(`take_status` = '{$takeStatus}' AND `time` < '{$tookExpiredTime}')";

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
     * 获取一段范围内的成交量
     *
     * @param $itemId
     * @param int $beginTime
     * @param null $endTime
     * @return bool|int
     */
    public static function getSoldCount($itemId, $beginTime = 0, $endTime = null)
    {
        $itemId = self::_quoteValue($itemId);
        $status = self::STATUS_PAY_SUCCESS . ', ' . self::STATUS_PUBLICITY_SUCCESS;
        $sql    = "SELECT SUM(`count`) AS `soldCount` FROM `".self::$TableName."` WHERE `item_id` = {$itemId} AND `status` IN ({$status}) AND `time` >= '{$beginTime}'";
        if (is_int($endTime))
        {
            $sql .= " AND `time` <= '{$endTime}'";
        }
        $rs = Server::$mysqlMaster->query($sql);
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

}