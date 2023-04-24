<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/2
 * Time: 下午7:55
 */

namespace RO\Trade;

use Ds\Vector;

class RefineTradeConfig
{
    public $itemType;

    public $refineLv;

    public $equipRate;

    public $itemId;

    public $itemRate;

    public $lvRate;

    public $riskRate;

    public $equipRateB;

    public $itemRateB;

    public $lvRateB;

    private static $refineTrades = null;

    public static function getConfigs()
    {
        return self::$refineTrades;
    }

    public static function loadConfig($config)
    {
        if (!$config)
        {
            return null;
        }

        $refineTrades = new Vector();
        foreach ($config as $key => $value)
        {
            $itemTypes = [];
            foreach ($value['EuqipType'] as $type)
            {
                $itemTypes[] = $type;
            }

            foreach ($itemTypes as $type)
            {
                $refineTrade             = new RefineTradeConfig();
                $refineTrade->itemId     = $value['ItemID'];
                $refineTrade->itemType   = $type;
                $refineTrade->refineLv   = $value['RefineLv'];
                $refineTrade->equipRate  = (float)$value['EquipRate'];
                $refineTrade->equipRateB = (float)$value['EquipRate_1'];
                $refineTrade->itemRate   = (int)$value['ItemRate'];
                $refineTrade->itemRateB  = (int)$value['ItemRate_1'];
                $refineTrade->lvRate     = (int)$value['LvRate'];
                $refineTrade->lvRateB    = (int)$value['LvRate_1'];
                $refineTrade->riskRate   = (float)$value['RiskRate'];

                $refineTrades->push($refineTrade);
            }
        }
        return self::$refineTrades = $refineTrades;
    }

    /**
     * @param $itemType
     * @param $refineLv
     * @return RefineTradeConfig
     */
    public static function getRefineTrade($itemType, $refineLv)
    {
        if (self::$refineTrades === null)
        {
            return null;
        }

        foreach (self::$refineTrades as $r)
        {
            if ($r->itemType == $itemType && $r->refineLv == $refineLv)
            {
                return $r;
            }
        }

        return null;
    }
}