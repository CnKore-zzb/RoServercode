<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/3
 * Time: 下午7:18
 */

namespace RO\Trade;

use Exception;
use Ds\Map;
use RO\Cmd\EItemType;
use RO\Utils\ROConfig;

class EnchantPriceConfig
{
    /** @var Map */
    public $value;

    /** @var Map */
    private static $enchantPriceConfigMap = null;

    public static function loadConfig($config)
    {
        if (!$config)
        {
            return null;
        }

        $map = new Map();
        foreach ($config as $key => $val)
        {
            $attr = RoleDataConfig::getIdByName($val['AttrType']);
            if ($attr == 0)
            {
                throw new Exception("[附魔价格配置-加载] id:{$key} AttrType: {$val['AttrType']} 不合法");
            }

            $it = $map->get($attr, null);
            if ($it !== null)
            {
                throw new Exception("[附魔价格配置-加载] id:{$key} AttrType: {$val['AttrType']} 重复了");
            }

            /** @var EnchantPriceConfig $cfg */
            if (!$map->hasKey($attr))
            {
                $cfg = new EnchantPriceConfig();
                $cfg->value = new Map();
                $map->put($attr, $cfg);
            }
            $cfg = $map->get($attr);

            $equipName = [
                "SpearRate", "SwordRate", "StaffRate", "KatarRate", "BowRate", "MaceRate", "AxeRate", "BookRate", "KnifeRate", "InstrumentRate", "LashRate", "PotionRate",
                "GloveRate", "ArmorRate", "ShieldRate", "RobeRate", "ShoeRate", "AccessoryRate", "OrbRate", "EikonRate", "BracerRate", "BraceletRate", "TrolleyRate",
                "HeadRate", "FaceRate", "MouthRate", "TailRate", "WingRate",
            ];

            foreach ($equipName as $name)
            {
                $itemType = ROConfig::getItemType($name);
                if ($itemType == EItemType::EITEMTYPE_MIN)
                {
                    throw new Exception("[附魔价格配置-加载] id:{$key} {$name} 类型不合法");
                }

                $it = $cfg->value->get($itemType, null);
                if ($it !== null)
                {
                    throw new Exception("[附魔价格配置-加载] id:{$key} {$name} 重复的道具类型");
                }
                $cfg->value->put($itemType, isset($val[$name]) ? $val[$name] : 0);
            }
        }

        return self::$enchantPriceConfigMap = $map;
    }

    public static function getEnchantPriceRate($type, $itemId)
    {
        $item = Server::$item->get($itemId);
        if (!$item)
        {
            return 1;
        }

        /** @var EnchantPriceConfig $it */
        $it = self::$enchantPriceConfigMap->get($type, null);
        if ($it === null)
        {
            return 1;
        }

        $subIt = $it->value->get($item['itemType'], null);
        if ($subIt === null)
        {
            return 1;
        }

        return (float) $subIt;
    }

}