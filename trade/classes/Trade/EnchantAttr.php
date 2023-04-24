<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/3
 * Time: 下午3:29
 */

namespace RO\Trade;

use Ds\Map;
use Ds\Vector;
use RO\Cmd\EAttrType;

class EnchantAttr
{
    public $configId = 0;

    public $type = EAttrType::EATTRTYPE_MIN;

    /** @var Vector */
    public $equipsRate;

    public $min = 0.0;

    public $max = 0.0;

    public $expressionOfMaxUp = 0.0;

    /** @var Vector */
    public $extraCondition;

    /** @var Vector */
    public $items;

    /** @var Vector */
    public $pairType;

    /** @var Vector */
    public $extraBuff;

    /** @var Map */
    public $noExchangeEnchant;

    public $maxWeight = 0;

    public $extraMaxWeight = 0;

    /**
     * 获取点赞概率
     * @return float
     */
    public function getRandomRate()
    {
        if ($this->items->count() < 2 || $this->maxWeight == 0)
            return 0.0;

        $weight = 0;
        $allWeight = $this->maxWeight;
        // 后两项权重和
        $pos = $this->items->count() - 2;
        $i = 1;
        foreach ($this->items as $item)
        {
            if ($i > $pos)
            {
                $weight += $item->rawWeight;
            }
            $i++;
        }

        return $weight / $allWeight;
    }
}