<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/3
 * Time: 下午5:19
 */

namespace RO\Trade;

use Ds\Vector;
use RO\Cmd\EItemType;

class EnchantEquip
{
    public $itemType = EItemType::EITEMTYPE_MIN;

    /** @var Vector */
    public $items;

    public $maxWeight = 0;

    /**
     * @param $type
     * @return EnchantEquipItem|null
     */
    public function getEnchantEquipItem($type)
    {
        foreach ($this->items as $item)
        {
            if ($item->p->key == $type && $item->p->value > 0)
            {
                return $item;
            }
        }

        return null;
    }
}