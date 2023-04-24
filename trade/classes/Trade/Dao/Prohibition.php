<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/4/11
 * Time: 上午11:09
 */

namespace RO\Trade\Dao;

use RO\Cmd\ETradeType;
use RO\Cmd\TradeItemBaseInfo;
use RO\Trade\ActSell;
use RO\Trade\Server;

class Prohibition extends Dao
{
    // 禁止上架某物品
    const ITEM_TYPE = 0;

    //  禁止上架某附魔
    const ENCHANT_TYPE = 1;

    const STATUS_ACTIVE = 1;

    const STATUS_DELETED = 0;

    public $id;

    public $uniqueId;

    public $itemId = 0;

    public $refineLv = 0;

    public $enchantId = 0;

    public $enchantBuffId = 0;

    public $type = self::ITEM_TYPE;

    public $tradeType = ETradeType::ETRADETYPE_TRADE;

    public $status;

    public $createdTime;

    public $remake;

    protected static $TableName = 'trade_prohibition';

    protected static $Fields = [
            'id'            => 'id',
            'uniqueId'      => 'unique_id',
            'itemId'        => 'item_id',
            'refineLv'      => 'refine_lv',
            'enchantId'     => 'enchant_id',
            'enchantBuffId' => 'enchant_buff_id',
            'type'          => 'type',
            'tradeType'     => 'trade_type',
            'status'        => 'status',
            'createdTime'   => 'created_time',
            'remake'        => 'remake',
        ];

    /**
     * 禁止出售某物品
     *
     * @var []
     */
    protected static $prohibitItem = [];

    /**
     * 禁止出售某附魔
     *
     * @var []
     */
    protected static $prohibitEnchant = [];


    public function insert()
    {
        $this->uniqueId = self::generateUniqueId($this->tradeType, $this->type, [
            $this->itemId, $this->refineLv, $this->enchantId, $this->enchantBuffId
        ]);

        if (!$this->createdTime)
        {
            $this->createdTime = time();
        }

        return parent::insert();
    }

    /**
     * @param $tradeType
     * @param $type
     * @param array $data
     * @return string
     */
    public static function generateUniqueId($tradeType, $type, $data = [])
    {
        $key = implode('-', $data);
        switch ($tradeType)
        {
            case ETradeType::ETRADETYPE_ALL:
                $tradeTypeStr = 'all';
                break;
            case ETradeType::ETRADETYPE_TRADE:
                $tradeTypeStr = 'trade';
                break;
            case ETradeType::ETRADETYPE_BOOTH:
                $tradeTypeStr = 'booth';
                break;
            default:
                $tradeTypeStr = 'all';
        }
        return $tradeTypeStr . '-' . ($type == self::ITEM_TYPE ? 'item' : 'enchant') . "-{$key}";
    }

    public static function getByUniqueId($uniqueId)
    {
        $sql = "SELECT * FROM " . self::$TableName . " WHERE `unique_id` = '{$uniqueId}'";
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            if ($rs->num_rows > 0)
            {
                $obj = new Prohibition($rs->fetch_assoc());

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

    /**
     * @return Prohibition[]|bool|null
     */
    public static function getAllActiveRecord()
    {
        $sql = 'SELECT * FROM `'. self::$TableName .'` WHERE `status` = ' . self::STATUS_ACTIVE;
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            $ret = [];
            while($data = $rs->fetch_object(static::class))
            {
                $ret[] = $data;
            }

            return $ret;
        }
        else
        {
            return false;
        }
    }

    public static function getAll()
    {
        $sql = 'SELECT * FROM `'. self::$TableName .'`';
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            return $rs->fetch_all(MYSQLI_ASSOC);
        }
        else
        {
            return false;
        }
    }

    /**
     * 初始化需要禁止上架的物品
     */
    public static function initProhibitItem()
    {
        $prohibitions = self::getAllActiveRecord();
        foreach ($prohibitions as $prohibition)
        {
            switch ($prohibition->type)
            {
                case self::ENCHANT_TYPE:
                    $enchantId = $prohibition->enchantId;
                    $isBuff = false;
                    if($prohibition->enchantBuffId > 0)
                    {
                        $isBuff = true;
                        $enchantId = $prohibition->enchantBuffId;
                    }
                    self::prohibitEnchant($prohibition->tradeType, $enchantId, $isBuff);
                    break;
                case self::ITEM_TYPE:
                    self::prohibitItem($prohibition->tradeType, $prohibition->itemId, $prohibition->refineLv, $prohibition->enchantId, $prohibition->enchantBuffId);
                    break;
            }
        }
    }

    /**
     * 禁止某项附魔属性
     *
     * @param int $tradeType
     * @param int $enchantId
     * @param bool $isBuff
     */
    public static function prohibitEnchant(int $tradeType, int $enchantId = 0, bool $isBuff = false)
    {
        $prohibitEnchant = &self::$prohibitEnchant[$tradeType];
        if ($isBuff)
        {
            $enchantId = 'buff' . $enchantId;
        }

        $prohibitEnchant[$enchantId] = true;
    }

    /**
     * 允许某项附魔属性上架
     *
     * @param int $tradeType
     * @param int $enchantId
     * @param bool $isBuff
     */
    public static function permitEnchant(int $tradeType, int $enchantId, bool $isBuff = false)
    {
        $prohibitEnchant = &self::$prohibitEnchant[$tradeType];
        if ($isBuff)
        {
            $enchantId = 'buff' . $enchantId;
        }

        unset($prohibitEnchant[$enchantId]);
    }

    /**
     * 禁止某项物品出售
     *
     * @param int $tradeType
     * @param $itemId
     * @param int $refineLv
     * @param int $enchantId
     * @param int $buffId
     */
    public static function prohibitItem(int $tradeType, int $itemId, int $refineLv = 0, int $enchantId = 0, int $buffId = 0)
    {
        $prohibitItem = &self::$prohibitItem[$tradeType];
        if (!isset($prohibitItem[$itemId]))
        {
            $prohibitItem[$itemId] = [];
        }

        if ($refineLv == 0 && $enchantId == 0 && $buffId == 0)
        {
            $condition = [];
            $key = '0,0,0';
        }
        else
        {
            $condition = [$refineLv, $enchantId, $buffId];
            $key = "$refineLv,$enchantId,$buffId";
        }

        $prohibitItem[$itemId][$key] = $condition;
    }

    /**
     * 允许商品上架
     *
     * @param int $tradeType
     * @param $itemId
     * @param int $refineLv
     * @param int $enchantId
     * @param int $buffId
     */
    public static function permitItem(int $tradeType, int $itemId, int $refineLv = 0, int $enchantId = 0, int $buffId = 0)
    {
        $prohibitItem = &self::$prohibitItem[$tradeType];

        if ($refineLv == 0 && $enchantId == 0 && $buffId == 0)
        {
            $key = '0,0,0';
        }
        else
        {
            $key = "$refineLv,$enchantId,$buffId";
        }

        unset($prohibitItem[$itemId][$key]);

        if (empty($prohibitItem[$itemId]))
        {
            unset($prohibitItem[$itemId]);
        }
    }

    /**
     * 检查物品属性是否通过
     *
     * @param int $tradeType
     * @param TradeItemBaseInfo $itemInfo
     * @return bool
     */
    public static function checkItem(int $tradeType, TradeItemBaseInfo $itemInfo)
    {
        $prohibitItem = &self::$prohibitItem[$tradeType];
        if (!empty($prohibitItem) && isset($prohibitItem[$itemInfo->itemid]))
        {
            $conditions = $prohibitItem[$itemInfo->itemid];

            if (isset($conditions['0,0,0']))
            {
                return false;
            }

            foreach ($conditions as $key => $condition)
            {
                if ($key == '0,0,0') continue;

                list($refineLv, $enchantId, $buffId) = $condition;

                if ($refineLv == $itemInfo->item_data->equip->refinelv)
                {
                    return false;
                }

                if ($itemInfo->item_data->hasEnchant())
                {
                    $enchantData = $itemInfo->item_data->enchant;
                    foreach ($enchantData->attrs as $attr)
                    {
                        if ($enchantId == $attr->type)
                        {
                            return false;
                        }
                    }

                    foreach ($enchantData->extras as $extra)
                    {
                        if ($buffId == $extra->configid)
                        {
                            return false;
                        }
                    }
                }
            }
        }

        return true;
    }

    /**
     * 检查附魔属性是否通过
     *
     * @param int $tradeType
     * @param TradeItemBaseInfo $itemInfo
     * @return bool
     */
    public static function checkEnchant(int $tradeType, TradeItemBaseInfo $itemInfo)
    {
        $prohibitEnchant = &self::$prohibitEnchant[$tradeType];
        $itemId = $itemInfo->itemid;
        $equipType = Server::$item->get($itemId, 'equipType');
        if ($equipType && !empty($prohibitEnchant))
        {
            if ($itemInfo->item_data->hasEnchant())
            {
                $enchantData = $itemInfo->item_data->enchant;
                foreach ($enchantData->attrs as $attr)
                {
                    if (isset($prohibitEnchant[$attr->type]) && $prohibitEnchant[$attr->type])
                    {
                        return false;
                    }
                }

                foreach ($enchantData->extras as $extra)
                {
                    $configIdKey = 'buff' . $extra->buffid;
                    if (isset($prohibitEnchant[$configIdKey]) && $prohibitEnchant[$configIdKey])
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    /**
     * 检查是否装备通过
     *
     * @param int $tradeType
     * @param TradeItemBaseInfo $itemInfo
     * @return bool
     */
    public static function check(int $tradeType, TradeItemBaseInfo $itemInfo)
    {
        return self::checkItem($tradeType, $itemInfo) && self::checkEnchant($tradeType, $itemInfo);
    }

    public static function getProhibitEnchant()
    {
        return self::$prohibitEnchant;
    }

    public static function getProhibitItem()
    {
        return self::$prohibitItem;
    }
}