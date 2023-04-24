<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/4/18
 * Time: 下午8:11
 */

namespace RO\Trade;

class EquipUpgradeConfig
{
    public $buffIds = [];

    public $materials = [];

    public $npcId;

    // 升级后装备id
    public $product;

    // 原始装备id
    public $rawId;

    private static $equipUpgradeConfigMap = [];

    public static function loadConfig($config)
    {
        if (!$config)
        {
            return null;
        }

        $map = [];
        $preBuff = 'BuffID_';
        $preMaterial = 'Material_';
        foreach ($config as $id => $data)
        {
            $cfg = new EquipUpgradeConfig();

            foreach ($data as $key => $item)
            {
                if (strpos($key, $preBuff) !== false) {
                    $cfg->buffIds[] = $item;
                    continue;
                }

                if (strpos($key, $preMaterial) !== false)
                {
                    $num = (int) ltrim($key, $preMaterial);
                    if (!empty($item) && is_array($item))
                    {
                        foreach ($item as $m)
                        {
                            if (Item::get($m['id']))
                            {
                                $cfg->materials[$num][] = $m;
                            }
                            else
                            {
                                throw new \Exception("[装备升级策划表-加载] 物品id:{$id} " . 'Material_' . $num . "中的id:{$m['id']} 在交易所Item内存表中不存在");
                            }
                        }
                    }
                }
            }

            $cfg->npcId = $data['NpcId'] ?? null;
            $cfg->product = $data['Product'] ?? null;
            $cfg->rawId = $id;

            $map[$id] = $cfg;
        }

        return self::$equipUpgradeConfigMap = $map;
    }

    public static function getConfigs()
    {
        return self::$equipUpgradeConfigMap;
    }
}