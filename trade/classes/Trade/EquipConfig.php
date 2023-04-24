<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/3
 * Time: 上午10:09
 */

namespace RO\Trade;

use Ds\Map;

class EquipConfig
{
    public $itemId;

    public $equipType;

    public $refineComposeId;

    public $cardSlot;

    public $enchant;

    public $subId;

    public $upgId;

    public $vid;

    public $maxRefineLv;

    public $job = [];

    /** @var Map */
    private static $equipConfigMap = null;

    public static function getConfigs()
    {
        return self::$equipConfigMap;
    }

    /**
     * @param $config
     * @return Map|null
     */
    public static function loadConfig($config)
    {
        if (!$config)
        {
            return null;
        }

        $map = new Map();

        foreach ($config as $key => $val)
        {
            $equipConfig                  = new EquipConfig();
            $equipConfig->itemId          = $key;
            $equipConfig->equipType       = isset($val['EquipType']) ? $val['EquipType'] : 0;
            $equipConfig->refineComposeId = isset($val['RefineEffectCost']) ? $val['RefineEffectCost'] : 0;
            $equipConfig->cardSlot        = isset($val['CardSlot']) ? $val['CardSlot'] : 0;
            $equipConfig->enchant         = isset($val['IsEnchant']) ? $val['IsEnchant'] : 0;
            $equipConfig->subId           = isset($val['SubstituteID']) ? $val['SubstituteID'] : 0;
            $equipConfig->upgId           = isset($val['UpgradeID']) ? $val['UpgradeID'] : 0;
            $equipConfig->vid             = isset($val['VID']) ? $val['VID'] : 0;
            $equipConfig->maxRefineLv     = isset($val['RefineMaxlv']) ? $val['RefineMaxlv'] : 0;
            if(!empty($val['CanEquip']))
            {
                foreach ($val['CanEquip'] as $job)
                {
                    $equipConfig->job[$job] = true;
                }
            }
            $map->put($key, $equipConfig);
        }

        return self::$equipConfigMap = $map;
    }

    /**
     * @param $itemId
     * @return null|EquipConfig
     */
    public static function getEquip($itemId)
    {
        if (self::$equipConfigMap === null) return null;

        return self::$equipConfigMap->get($itemId);
    }

    public static function getLowVidItemId($itemId)
    {
        if (self::$equipConfigMap === null) return 0;

        $equip = self::$equipConfigMap->get($itemId, null);
        if ($equip === null) return 0;

        $vid = $equip->vid;

        if ($vid == 0)
            return 0;

        $t = (int)($vid / 1000);
        $t = $t % 10;
        if ($t == 0)
            return 0;

        $newVid = $vid - 1000;

        foreach (self::$equipConfigMap as $id => $val)
        {
            if ($val->vid == $newVid)
            {
//                if (IS_DEBUG)
//                {
//                    Server::$instance->debug("[精炼-低洞] 物品ID:{$id}  high vid:{$vid} low vid:{$newVid}");
//                }
                return $id;
            }
        }
        return 0;
    }

    /**
     * 是否该职业可以装备
     *
     * @param $itemId
     * @param $job
     * @return bool
     */
    public static function isJob($itemId, $job)
    {
        if (self::$equipConfigMap === null) return false;

        /** @var EquipConfig $equip */
        $equip = self::$equipConfigMap->get($itemId, null);
        if ($equip === null) return false;

        // 表示所有职业都可以装备
        if (isset($equip->job[0]))
            return true;

        return isset($equip->job[$job]);
    }
}