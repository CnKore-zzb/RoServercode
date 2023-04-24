<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/1
 * Time: 下午7:26
 */

namespace RO\Trade;

use Exception;

class ExchangeItemNode
{
    const PRICE_TYPE_SELF = 1;      //原始价格直接取price
    const PRICE_TYPE_SUM = 2;       //原始价格需依赖其他物品价格
    const PRICE_TYPE_MIN = 3;       //原始价格直接读取minPrice, 不接受调价

    const MIN_PRICE_TYPE_SELF = 1;  //直接取price
    const MIN_PRICE_TYPE_CARD_COMPOSE = 2;  //图纸合成
    const MIN_PRICE_TYPE_EQUIP_COMPOSE = 3;  //装备制作
    const MIN_PRICE_TYPE_EQUIP_UPGRADE = 4;  //装备升级
    const MIN_PRICE_TYPE_EQUIP_NEW_COMPOSE = 5;  //装备合成

    public $id;

    /** @var array 物品原始价格配置 */
    public $price = [];

    /** @var int 策划表配置的原始价格 */
    public $rawPrice;

    /** @var array 物品最低价格配置 */
    public $minPrice = [];

    public $isTrade = 0;

    /** @var ExchangeItemNode[] 依赖该物品价格的节点 */
    public $parents = [];

    /** @var ExchangeItemNode[] 组成该物品价格的节点 */
    public $children = [];

    /** @var int 实际价格 */
    public $realPrice = 0;

    public $cycle = 0;

    public $priceUpdateTime;

    /** @var int 是否装备 */
    public $isEquip = 0;


    /**
     * 更新价格
     * 递归更新关联的物品价格
     *
     * @param int $time 更新时间
     * @return array 返回所有更新的物品id
     */
    public function updatePrice($time = null)
    {
        $rs = [$this->id];
        $this->realPrice = $this->getPrice();
        $this->priceUpdateTime = $time ?? $this->priceUpdateTime;
        foreach ($this->parents as $id => $val)
        {
            $rs = array_merge($rs, $val->updatePrice($time));
        }

        return $rs;
    }

    /**
     * 计算物品价格
     *
     * @return int
     */
    public function getPrice()
    {
        $retPrice = 0;
        do
        {
            if ($this->isTrade)
            {
                if ($this->price['type'] == self::PRICE_TYPE_SELF)
                {
                    $retPrice = $this->realPrice;
                    //第一次从数据库加载时 对最小价格进行验证
                    if ($retPrice == 0)
                    {
                        $retPrice = $this->price['price'];
                    }

                    $minPrice = $this->getMinPrice();
                    $retPrice = max($retPrice, $minPrice);
                    break;
                }
            }
            else
            {
                //交易所不能交易的，但是服务器价格会用到的物品。
                if ($this->price['type'] == self::PRICE_TYPE_SELF)
                {
                    $retPrice = $this->price['price'];
                    $minPrice = $this->getMinPrice();
                    $retPrice = max($retPrice, $minPrice);
                    break;
                }
            }

            if ($this->price['type'] == self::PRICE_TYPE_SUM)
            {
                foreach ($this->price['pricesum'] as $val)
                {
                    $id        = $val['itemid'];
                    $count     = $val['count'] ?: 1;
                    $retPrice += $this->children[$id]->getPrice() * $count;
                }
                break;
            }

            if ($this->price['type'] == self::PRICE_TYPE_MIN)
            {
                $retPrice = $this->getMinPrice();
                break;
            }
        } while (0);

        $maxPrice = 4000000000;
        if ($retPrice > $maxPrice)
        {
            $retPrice = $maxPrice;
        }

        return (int) $retPrice;
    }

    /**
     * 获取物品策划表原始价格
     *
     * @return int
     */
    public function getRawPrice()
    {
        if (isset($this->rawPrice)) return $this->rawPrice;

        $retPrice = 0;
        do
        {
            if ($this->price['type'] == self::PRICE_TYPE_SELF)
            {
                $retPrice = $this->price['price'];
                break;
            }

            if ($this->price['type'] == self::PRICE_TYPE_SUM)
            {
                foreach ($this->price['pricesum'] as $val)
                {
                    $id        = $val['itemid'];
                    $count     = $val['count'] ?: 1;
                    $retPrice += $this->children[$id]->getRawPrice() * $count;
                }
                break;
            }
        }
        while (0);

        return (int) $this->rawPrice = $retPrice;
    }

    /**
     * 获取物品最低价格
     *
     * @return int|mixed
     */
    public function getMinPrice()
    {
        if (empty($this->minPrice)) return 0;

        if ($this->minPrice['type'] == self::MIN_PRICE_TYPE_SELF)
        {
            return (int)$this->minPrice['price'];
        }

        $price = isset($this->minPrice['rob']) ? $this->minPrice['rob'] : 0;

        foreach ($this->minPrice['material'] as $val)
        {
            $id     = $val['itemid'];
            $count  = $val['num'];
            $upgradePrice = PriceManager::calcUpgradePrice($id, $val['lv'] ?? 0);
            $price += ($this->children[$id]->getPrice() + $upgradePrice) * $count;
        }

        $price = $price * $this->minPrice['ratio'];
        if ($this->minPrice['type'] == self::MIN_PRICE_TYPE_EQUIP_UPGRADE)
        {
            $price += $this->children[$this->minPrice['equipId']]->getPrice();
        }

        return (int) $price;
    }

    public function setPrice(array $price)
    {
        if (!isset($price['type']))
        {
            throw new Exception('[物品配置-Exchange.json] price 不存在 type. 物品ID:' . $this->id);
        }

        switch ($price['type'])
        {
            case self::PRICE_TYPE_SELF:
                if(!isset($price['price']))
                    throw new Exception('[物品配置-Exchange.json] price 不存在 price. 物品ID:'. $this->id);
                break;

            case self::PRICE_TYPE_SUM:
                if(!isset($price['pricesum']))
                    throw new Exception('[物品配置-Exchange.json] price 不存在 pricesum. 物品ID:'. $this->id);
                break;

            case self::PRICE_TYPE_MIN:
                break;

            default:
                throw new Exception('[物品配置-Exchange.json] price 错误 type. 物品ID:'. $this->id);
        }

        $this->price = $price;
    }

    public function setMinPrice(array $minPrice)
    {
        if (!empty($minPrice))
        {
            if (!isset($minPrice['type']))
            {
                throw new Exception('[物品配置-Exchange.json] min price 不存在  type. 物品ID:' . $this->id);
            }

            switch ($minPrice['type'])
            {
                case self::MIN_PRICE_TYPE_SELF:
                    if (!isset($minPrice['price']))
                        throw new Exception('[物品配置-Exchange.json] min price 不存在 price. 物品ID:'. $this->id);
                    break;

                case self::MIN_PRICE_TYPE_CARD_COMPOSE:
                case self::MIN_PRICE_TYPE_EQUIP_COMPOSE:
                    if(!isset($minPrice['composeid']))
                        throw new Exception('[物品配置-Exchange.json] min price 不存在 composeid. 物品ID:'. $this->id);
                    if(!isset($minPrice['ratio']))
                        throw new Exception('[物品配置-Exchange.json] min price 不存在 ratio. 物品ID:'. $this->id);
                    break;
                case self::MIN_PRICE_TYPE_EQUIP_UPGRADE:
                    if(!isset($minPrice['equip_upgrade_id']))
                        throw new Exception('[物品配置-Exchange.json] min price 不存在 equip_upgrade_id. 物品ID:'. $this->id);
                    if(!isset($minPrice['ratio']))
                        throw new Exception('[物品配置-Exchange.json] min price 不存在 ratio. 物品ID:'. $this->id);
                    break;
                case self::MIN_PRICE_TYPE_EQUIP_NEW_COMPOSE:
                    if(!isset($minPrice['equipcomposeid']))
                        throw new Exception('[物品配置-Exchange.json] min price 不存在 equipcomposeid. 物品ID:'. $this->id);
                    if(!isset($minPrice['ratio']))
                        throw new Exception('[物品配置-Exchange.json] min price 不存在 ratio. 物品ID:'. $this->id);
                    break;
                default:
                    throw new Exception('[物品配置-Exchange.json] min price 错误 type. 物品ID:'. $this->id);
            }
        }

        $this->minPrice = $minPrice;
    }

    /**
     * 打印计算过程
     */
    public function printCalcProcess()
    {
        $str = '';
        foreach ($this->children as $child)
        {
            $item = Item::get($child->id);
            $str .= "{$item->name}:{$child->realPrice} ";
        }

        Server::$instance->info($str);
    }
}

