<?php
namespace RO\Trade;

use RO\Cmd\StateType;
use RO\Trade\Dao\ItemList;
use RO\Utils\ROConfig;

/**
 * 多进程内存共享表对象
 *
 * @package RO\Trade
 */
class Item
{
    /**
     * 物品ID
     *
     * @var int
     */
    public $id;

    /**
     * 名称
     *
     * @var string
     */
    public $name;

    /**
     * 是否堆叠
     *
     * @var int
     */
    public $isOverlap;

    /**
     * 正在购买
     *
     * @var int
     */
    public $buying;

    /**
     * 已售数量
     *
     * @var int
     */
    public $soldNum;

    /**
     * 价格
     *
     * 堆叠物品可以直接获取此值，非堆叠物品使用 `$this->getPrice()` 方法获取
     *
     * @var int
     */
    public $price;

    /**
     * 最高价格
     * 0和负数代表不设置
     *
     * @var int
     */
    public $maxPrice;

    /**
     * 涨幅系数
     *
     * @var float
     */
    public $upRatio;

    /**
     * 跌幅系数
     *
     * @var float
     */
    public $downRatio;

    /**
     * 总库存
     *
     * @var int
     */
    public $stock;

    /**
     * 摆摊总库存
     *
     * @var int
     */
    public $boothStock;

    /**
     * 是否售卖
     *
     * @var int
     */
    public $isTrade;

    /**
     * 价格更新时间
     *
     * @var int
     */
    public $priceUpdateTime;

    /**
     * 价格更新的随机时间
     *
     * @var int
     */
    public $cycle;

    /**
     * 精炼调价周期
     *
     * @var
     */
    public $refineCycle;

    /**
     * 兑换数
     *
     * @var int
     */
    public $exchangeNum;

    /**
     * 装备类型
     *
     * @var int
     */
    public $equipType;

    /**
     * 最低价格计算类型
     *
     * @var int
     */
    public $minPriceType;

    /**
     * 升级装备id, 0为不是升级装备
     *
     * @var int
     */
    public $equipUpgradeId;

    /**
     * 主装备id
     *
     * @var int
     */
    public $mainEquipId;

    /**
     * 物品类型
     *
     * @var int
     */
    public $itemType;

    /**
     * 是否有公示
     *
     * @var int
     */
    public $publicityNum;

    /**
     * 最新公示结束时间
     * @var
     */
    public $lastPubEndTime;

    /**
     * 公示物品公示时间
     *
     * @var int
     */
    public $publicityShowTime;

    /**
     * 物品分类id
     *
     * @var int
     */
    public $category;

    /**
     * 时装分类，头饰分类中有
     *
     * @var int
     */
    public $fashionType;

    /**
     * 消费类型, Zeny,等等
     *
     * @var
     */
    public $moneyType;

    /**
     * 物品信息（如果没有设置的话是 null）
     *
     * @var \RO\Cmd\ItemData
     */
    public $itemData = null;

    /**
     * @var int
     */
    public $tradeTime;

    /**
     * @var int
     */
    public $unTradeTime;


    private $_key;

    protected $old = [];

    /** @var ItemList */
    protected $itemList;

    function __construct($id, array $data)
    {
        $this->id = (int)$id;

        foreach ($data as $k => $v)
        {
            $this->$k = $v;
        }

        $this->old = $data;
    }

    /**
     * 获取物品实时价格
     *
     * @return int
     */
    public function getPrice()
    {
        return PriceManager::getServerItemPrice($this);
    }

    /**
     * 物品是否可以交易
     *
     * @return bool
     */
    public function canTrade()
    {
        $rs = $this->isTrade == 1;
        if ($rs)
        {
            $cur = time();
            if ($this->tradeTime > 0)
            {
                $rs = $cur >= $this->tradeTime;
            }

            if ($rs && $this->unTradeTime > 0)
            {
                $rs = $cur <= $this->unTradeTime;
            }
        }

        return $rs;
    }

    /**
     * 给某个字段递增
     *
     * @param $column
     * @param $incrby
     * @return bool|int
     */
    public function incr($column, $incrby)
    {
        $rs = Server::$item->incr($this->id, $column, $incrby);
        if ($rs !== false)
        {
            $this->$column = $this->old[$column] = $rs;
        }

        return $rs;
    }

    /**
     * 给某个字段递减
     *
     * @param $column
     * @param $incrby
     * @return bool|int
     */
    public function decr($column, $incrby)
    {
        $rs = Server::$item->decr($this->id, $column, $incrby);
        if ($rs !== false)
        {
            $this->$column = $this->old[$column] = $rs;
        }

        return $rs;
    }

    /**
     * 拉取数据
     *
     * @return bool
     */
    public function pull()
    {
        $data = Server::$item->get($this->id);
        if ($data)
        {
            foreach ($data as $k => $v)
            {
                $this->$k = $v;
            }
            $this->old = $data;

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * 更新数据
     *
     * @return bool
     */
    public function update()
    {
        $change = [];
        foreach ($this as $k => $v)
        {
            if ($k != 'id' && $v != $this->old[$k])
            {
                $change[$k] = $v;
            }
        }

        if ($change)
        {
            return Server::$item->set($this->id, $change);
        }
        else
        {
            return true;
        }
    }

    /**
     * 获取唯一hash
     *
     * @param $isGoodEnchant
     * @return string
     */
    private function generateHash($isGoodEnchant = false)
    {
        if (isset($this->_key))
        {
            return $this->_key;
        }

        return $this->_key = PriceManager::generateHash($this->itemData, $isGoodEnchant);
    }

    /**
     * 获取唯一key
     *
     * 折叠物品,垃圾附魔,无itemData属性则返回物品id
     *
     * @return int|string
     */
    public function getKey()
    {
        if ($this->itemData === null || $this->isOverlap)
        {
            return $this->id;
        }

        if ($this->itemData->equip->refinelv > 0 || $this->itemData->equip->damage)
        {
            return $this->generateHash();
        }

        if (ROConfig::isGoodEnchant($this->itemData))
        {
            return $this->generateHash(true);
        }

        return $this->id;
    }

    /**
     * 是否处于公示期
     *
     *
     * @param Item $item
     * @param Dao\ItemList $itemList
     * @return bool|int
     * 返回StateType::St_WillPublicity表示即将公式
     * 返回StateType::St_InPublicity表示正在公式
     * 返回false代表不是公式物品
     */
    public static function getPublicityState($item, $itemList = null)
    {
        if ($itemList && $itemList->isPublicity == 1)
        {
            return StateType::St_InPublicity;
        }

        if (0 == $item->isOverlap && ROConfig::isGoodEnchant($item->itemData))
        {
            if ($itemList && $itemList->stock() > 0)
            {
                return false;
            }
            else
            {
                return StateType::St_WillPublicity;
            }
        }

        if ($item->soldNum < $item->exchangeNum)
        {
            return false;
        }

        if ($itemList && $itemList->stock() > 0)
        {
            return false;
        }

        return StateType::St_WillPublicity;
    }

    /**
     * 是否摆摊处于公示期
     *
     * @param Item $item
     * @return bool|int
     * 返回StateType::St_WillPublicity表示即将公式
     * 返回StateType::St_InPublicity表示正在公式
     * 返回false代表不是公式物品
     */
    public static function getBoothPublicityState(Item $item)
    {
        $itemList = $item->getItemList();
        // 出现从未上架过的物品, 直接判单交易所是否上公式
        if ($itemList === null)
        {
            return self::getPublicityState($item);
        }

        $state = self::getPublicityState($item, $itemList);
        if ($itemList->boothStock <= 0)
        {
            return $state;
        }
        return false;
    }

    /**
     * @return null|ItemList
     */
    public function getItemList()
    {
        if (isset($this->itemList))
        {
            return $this->itemList;
        }

        $itemList = ItemList::getByKey($this->getKey());
        if ($itemList)
        {
            $itemList->item = $this;
            return $this->itemList = $itemList;
        }

        return null;
    }

    /**
     * 重新加载
     *
     * @return $this
     */
    public function reload()
    {
        $data = Server::$item->get($this->id);
        if (!$data)return $this;

        foreach ($data as $k => $v)
        {
            $this->$k = $v;
        }

        $this->old = $data;
        return $this;
    }

    /**
     * 获取一个物品对象
     *
     * @param $itemId
     * @param \RO\Cmd\ItemData|null $itemData 物品信息，堆叠物品不需要设置
     * @return bool|Item
     */
    public static function get($itemId, \RO\Cmd\ItemData $itemData = null)
    {
        $rs = Server::$item->get($itemId);
        if (!$rs)return false;

        $item = new Item($itemId, $rs);

        if (!$item->isOverlap && $itemData)
        {
            $item->itemData = $itemData;
        }

        return $item;
    }

    /**
     * 重置Item的总库存数和公示挂单数
     *
     * @param $itemId
     * @return array
     */
    public static function resetStockAndPubNum($itemId)
    {
        $count = 0;
        $pubNum = 0;
        foreach (Server::$itemList as $k => $v)
        {
            if ($v['item_id'] != $itemId)continue;
            if ($v['stock'] > 0)
            {
                $count += $v['stock'];
            }
            if ($v['is_publicity'])
            {
                $pubNum++;
            }
        }
        Server::$item->set($itemId, ['stock' => $count]);

        return [$count, $pubNum];
    }

    /**
     * 获取库存
     * itemData为空则以itemId为key读取库存,反之亦然
     *
     * @return int
     */
    public function getItemListStock()
    {
        $itemList = $this->getItemList();
        return $itemList ? $itemList->stock : 0;
    }

    /**
     * 增加物品成交数量
     *
     * @param $count
     * @return bool|int
     */
    public function addSoldCount($count)
    {
        return TradeManager::addSoldCount($this->id, $count);
    }

    /**
     * 减少物品成交数量
     *
     * @param $count
     * @return bool|int
     */
    public function deductSoldCount($count)
    {
        return TradeManager::deductSoldCount($this->id, $count);
    }

    /**
     * 从redis中获取在售数
     *
     * @param int $itemId
     * @param $key
     * @return int
     */
    //public static function getRedisStock($itemId, $key)
    //{
    //    try
    //    {
    //        return (int) Server::$redis->hGet('stock_' . $itemId, $key) ?: 0;
    //    }
    //    catch (\Exception $e)
    //    {
    //        Server::$instance->warn('[交易所-获取库存] 发生异常。 错误信息:' . $e->getMessage());
    //        return 0;
    //    }
    //}

    //public static function getAllStock($itemId)
    //{
    //    try
    //    {
    //        return Server::$redis->hGetAll('stock_' . $itemId);
    //    }
    //    catch (\Exception $e)
    //    {
    //        Server::$instance->warn('[交易所-获取库存] 发生异常。 错误信息:' . $e->getMessage());
    //        return [];
    //    }
    //}

    /**
     * 从redis增加库存
     *
     * @param int $itemId
     * @param string $key
     * @param int $incrBy
     * @return int
     */
    //public static function incrRedisStock($itemId, $key, $incrBy = 1)
    //{
    //    try
    //    {
    //        return Server::$redis->hIncrBy('stock_' . $itemId, $key, $incrBy);
    //    }
    //    catch (\Exception $e)
    //    {
    //        Server::$instance->warn('[交易所-增加库存] 发生异常。 错误信息:' . $e->getMessage());
    //        return 0;
    //    }
    //}

    /**
     * 从redis减少库存
     *
     * @param int $itemId
     * @param string $key
     * @param int $incrBy
     * @return int
     */
    //public static function decrRedisStock($itemId, $key, $incrBy = 1)
    //{
    //    try
    //    {
    //        return Server::$redis->hIncrBy('stock_' . $itemId, $key, - $incrBy);
    //    }
    //    catch (\Exception $e)
    //    {
    //        Server::$instance->warn('[交易所-减少库存] 发生异常。 错误信息:' . $e->getMessage());
    //        return 0;
    //    }
    //}


    /**
     * 设置库存
     *
     * @param int    $itemId
     * @param string $itemKey
     * @param int    $incrBy
     * @return int
     */
    //public static function setRedisStock($itemId, $itemKey, $value)
    //{
    //    try
    //    {
    //        return Server::$redis->hSet('stock_' . $itemId, $itemKey, $value);
    //    }
    //    catch (\Exception $e)
    //    {
    //        Server::$instance->warn('[交易所-减少库存] 发生异常。 错误信息:' . $e->getMessage());
    //        return 0;
    //    }
    //}


}