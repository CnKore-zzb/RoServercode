<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/2
 * Time: 下午7:02
 */

namespace RO\Utils;

use RO\Cmd\EEnchantType;
use RO\Cmd\EItemType;
use RO\Cmd\ItemData;
use RO\Trade\EnchantConfig;
use RO\Trade\EnchantPriceConfig;
use RO\Trade\RoleDataConfig;
use RO\Trade\Server;

class ROConfig
{
    public static $EItemTypeArr;

    public static $EEnchantTypeArr;

    public static $roleData;

    # 临时处理极品属性
    public static $notGoodBuffId = [
        500061, 500062, 500063, 500064,
        500001, 500002, 500003, 500004
    ];

    public static function isValidItemType($type)
    {
        if(!isset(self::$EItemTypeArr))
        {
            $type = new EItemType();
            self::$EItemTypeArr = $type->toArray();
        }

        return in_array($type, self::$EItemTypeArr);
    }

    public static function isValidEnchantType($type)
    {
        if (!isset(self::$EEnchantTypeArr))
        {
            self::$EEnchantTypeArr = (new EEnchantType())->toArray();
        }

        return in_array($type, self::$EEnchantTypeArr);
    }

    /**
     * @param ItemData $itemData
     * @return bool
     */
    public static function isGoodEnchant($itemData)
    {
        if ($itemData === null)
        {
            return false;
        }

        if (!$itemData->hasEnchant())
        {
            return false;
        }

        if (isset($itemData->isGoodEnchant) && $itemData->isGoodEnchant)
        {
            return true;
        }

        // 有额外属性的都认为是好的附魔
        if (count($itemData->enchant->extras) >= 1)
        {
            foreach ($itemData->enchant->extras as $extra)
            {
                if (in_array($extra->buffid, self::$notGoodBuffId))
                {
                    return false;
                }

                $enchantCfg = EnchantConfig::getEnchantConfig($itemData->enchant->type);
                if ($enchantCfg === null)
                    return false;

                $attr = $enchantCfg->getEnchantAttrByBuffId($extra->buffid);
                if ($attr === null)
                    return false;

                if ($attr->noExchangeEnchant->hasKey(self::getItemTypeById($itemData->base->id)))
                {
                    return false;
                }
            }

//            if (IS_DEBUG)
//            {
//                Server::$instance->debug('[装备检测] 该装备是极品附魔, itemData->enchant:' . json_encode($itemData->enchant));
//            }

            $itemData->isGoodEnchant = true;
            return true;
        }

//        // 初级和中级不算好的附魔
//        if ($itemData->enchant->type == EEnchantType::EENCHANTTYPE_PRIMARY || $itemData->enchant->type == EEnchantType::EENCHANTTYPE_MEDIUM)
//        {
//            return false;
//        }
//
//        $tradeConfig = Server::$configExchange;
//
//        foreach ($itemData->enchant->attrs as $attr)
//        {
//            $enchantConfig = EnchantConfig::getEnchantConfig($itemData->enchant->type);
//            if (!$enchantConfig)
//            {
//                continue;
//            }
//
//            $attrConfig = $enchantConfig->getEnchantAttrByType($attr->type);
//            if (!$attrConfig)
//            {
//                continue;
//            }
//
//            $rate = EnchantPriceConfig::getEnchantPriceRate($attr->type, $itemData->base->id);
//            if ($rate < $tradeConfig['GoodRate'])
//            {
//                continue;
//            }
//
//            $roleData =  RoleDataConfig::getRoleData($attrConfig->type);
//            if ($roleData == null)
//            {
//                Server::$instance->warn("附魔随机属性 {$attr->type} 不合法");
//                continue;
//            }
//
//            $newValue = $attr->value;
//            if ($roleData->percent)
//                $newValue = $newValue / 1000;
//
//            // 指附魔属性出现高于该属性最大值的XX百分比时会出现赞的表情,也就算极品附魔
//            if ($newValue >= $attrConfig->max * $attrConfig->expressionOfMaxUp)
//                return true;
//        }

        return false;
    }

    public static function getItemTypeById($itemId)
    {
        if ($item = Server::$item->get($itemId))
        {
            return $item['itemType'];
        }

        return 0;
    }

    public static function getItemType($str)
    {
        if ($str == "SpearRate")
            return EItemType::EITEMTYPE_WEAPON_LANCE;
        if ($str == "SwordRate")
            return EItemType::EITEMTYPE_WEAPON_SWORD;
        if ($str == "StaffRate")
            return EItemType::EITEMTYPE_WEAPON_WAND;
        if ($str == "KatarRate")
            return EItemType::EITEMTYPE_WEAPON_KNIFE;
        if ($str == "BowRate")
            return EItemType::EITEMTYPE_WEAPON_BOW;
        if ($str == "MaceRate")
            return EItemType::EITEMTYPE_WEAPON_HAMMER;
        if ($str == "AxeRate")
            return EItemType::EITEMTYPE_WEAPON_AXE;
        if ($str == "BookRate")
            return EItemType::EITEMTYPE_WEAPON_BOOK;
        if ($str == "KnifeRate")
            return EItemType::EITEMTYPE_WEAPON_DAGGER;
        if ($str == "InstrumentRate")
            return EItemType::EITEMTYPE_WEAPON_INSTRUMEMT;
        if ($str == "LashRate")
            return EItemType::EITEMTYPE_WEAPON_WHIP;
        if ($str == "PotionRate")
            return EItemType::EITEMTYPE_WEAPON_TUBE;
        if ($str == "GloveRate")
            return EItemType::EITEMTYPE_WEAPON_FIST;
        if ($str == "ArmorRate")
            return EItemType::EITEMTYPE_ARMOUR;
        if ($str == "ShieldRate")
            return EItemType::EITEMTYPE_SHIELD;
        if ($str == "RobeRate")
            return EItemType::EITEMTYPE_ROBE;
        if ($str == "ShoeRate")
            return EItemType::EITEMTYPE_SHOES;
        if ($str == "AccessoryRate")
            return EItemType::EITEMTYPE_ACCESSORY;
        if ($str == "OrbRate")
            return EItemType::EITEMTYPE_PEARL;
        if ($str == "EikonRate")
            return EItemType::EITEMTYPE_EIKON;
        if ($str == "BracerRate")
            return EItemType::EITEMTYPE_BRACER;
        if ($str == "BraceletRate")
            return EItemType::EITEMTYPE_BRACELET;
        if ($str == "TrolleyRate")
            return EItemType::EITEMTYPE_TROLLEY;
        if ($str == "HeadRate")
            return EItemType::EITEMTYPE_HEAD;
        if ($str == "FaceRate")
            return EItemType::EITEMTYPE_FACE;
        if ($str == "MouthRate")
            return EItemType::EITEMTYPE_MOUTH;
        if ($str == "TailRate")
            return EItemType::EITEMTYPE_TAIL;
        if ($str == "WingRate")
            return EItemType::EITEMTYPE_BACK;

        return EItemType::EITEMTYPE_MIN;
    }

}