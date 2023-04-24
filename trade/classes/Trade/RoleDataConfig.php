<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/3
 * Time: 下午4:01
 */

namespace RO\Trade;

use Ds\Map;

class RoleDataConfig
{
    public $percent = false;

    public $sync = false;

    public $id;

    public $name;

    public $prop;

    /** @var Map */
    public static $roleDataMap = null;

    public static function loadConfig($config)
    {
        if (!$config)
        {
            return null;
        }
        $map = new Map();
        foreach ($config as $id => $val)
        {
            $role          = new RoleDataConfig();
            $role->id      = $id;
            $role->name    = $val['VarName'];
            $role->prop    = $val['PropName'];
            $role->percent = $val['IsPercent'] == 1;
            $role->sync    = isset($val['Sync']) ? $val['Sync'] == 1 : false;
            $map->put($id, $role);
        }

        return self::$roleDataMap = $map;
    }

    public static function getConfig()
    {
        return self::$roleDataMap;
    }

    public static function getIdByName($name)
    {
        if (self::$roleDataMap === null) return 0;
        foreach (self::$roleDataMap as $role)
        {
            if ($role->name == $name)
            {
                return $role->id;
            }
        }

        return 0;
    }

    /**
     * @param $id
     * @return RoleDataConfig|null
     */
    public static function getRoleData(int $id)
    {
        return self::$roleDataMap->get($id, null);
    }
}