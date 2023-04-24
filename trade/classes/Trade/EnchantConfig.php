<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/3
 * Time: 下午1:57
 */

namespace RO\Trade;

use Ds\Map;
use Ds\Pair;
use Ds\Vector;
use RO\Cmd\EEnchantType;
use RO\Cmd\EItemType;
use RO\Utils\ROConfig;

class EnchantConfig
{
    public $enchantType;

    public $maxNum;

    public $rob;

    public $attrType;

    /** @var Vector */
    public $attrVec;

    /** @var Vector */
    public $equips;

    private static $enchantConfigMap = [];

    public static function loadConfig($config)
    {
        if (!$config)
        {
            return null;
        }

        $map = [];
        for ($i = EEnchantType::EENCHANTTYPE_MIN + 1; $i < EEnchantType::EENCHANTTYPE_MAX; $i++)
        {
            $enchantCfg             = new EnchantConfig();
            $enchantCfg->attrVec    = new Vector();
            $enchantCfg->equips     = new Vector();
            $map[$i] = $enchantCfg;
        }

        foreach ($config as $key => $val)
        {
            unset($enchant);
            $type                    = $val['EnchantType'] ?: 0;
            /** @var EnchantConfig $enchant */
            $enchant                 = &$map[$type];
            $enchant->enchantType    = $type;
            $enchant->maxNum         = $val['MaxNum'] ?: 0;
            $enchant->rob            = $val['ZenyCost'] ?: 0;
            $dwAttr                  = RoleDataConfig::getIdByName($val['AttrType']);
            $attr                    = new EnchantAttr();
            $attr->equipsRate        = [];
            $attr->configId          = $val['UniqID'] ?: 0;
            $attr->type              = $dwAttr;
            $attr->pairType          = new Vector();
            $attr->extraBuff         = new Vector();
            $attr->extraCondition    = new Vector();
            $attr->items             = new Vector();
            $attr->noExchangeEnchant = new Map();

            $attr2                   = $val['AttrType2'];
            foreach ($attr2 as $data)
            {
                $attrType = RoleDataConfig::getIdByName($data);
                $attr->pairType->push($attrType);
            }
            $extraBuff = $val['AddAttr'];
            foreach ($extraBuff as $data)
            {
                $p = new Pair($data[1], $data[2]);
                $attr->extraBuff->push($p);
            }

            $noExchangeEnchant = $val['NoExchangeEnchant'];
            foreach ($noExchangeEnchant as $noExchangeEnchantKey => $noExchangeEnchantVal)
            {
                $attr->noExchangeEnchant->put(ROConfig::getItemType($noExchangeEnchantKey), (int) $noExchangeEnchantVal);
            }

            if (!empty($val['Condition']))
            {
                $attr->extraCondition->push($val['Condition']['type']);
                // EENCHANTEXTRACON_REFINELV
                if ($attr->extraCondition[0] == 1)
                {
                    $attr->extraCondition->push($val['Condition']['refinelv']);
                }
            }

            $attr->min = (float) isset($val['AttrBound']['1']['1']) ? $val['AttrBound']['1']['1'] : -1;
            $attr->max = (float) isset($val['AttrBound']['1']['2']) ? $val['AttrBound']['1']['2'] : -1;
            if ($attr->min === -1 || $attr->max === -1)
            {
                throw new \Exception("[附魔配置-加载] id:{$key} AttrBound 不存在");
            }

            $attr->expressionOfMaxUp = (float) $val['ExpressionOfMaxUp'];

            if (!empty($val['MaxAttrRate']))
            {
                $lastValue = $attr->min;
                $rate = $val['MaxAttrRate'];
                foreach ($rate as $data)
                {
                    $item            = new EnchantAttrItem();
                    $item->min       = $lastValue;
                    $item->max       = (float)$data['1'];
                    $item->weight    = (int)$data['2'];
                    $item->rawWeight = $item->weight;
                    if ($item->min > $item->max)
                    {
                        throw new \Exception("[附魔配置-加载] id:{$key} min:{$item->min} 大于 max:{$item->max}");
                    }

                    $attr->items->push($item);
                    $lastValue = $item->max;
                }
            }

            $enchant->attrVec->push($attr);

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
                    throw new \Exception("[附魔配置-加载] id:{$key} {$name} 类型不合法");
                }

                /** @var EnchantEquip $v */
                $v = null;
                foreach ($enchant->equips as $equip)
                {
                    if ($equip->itemType == $itemType)
                    {
                        $v = $equip;
                    }
                }
                if ($v === null)
                {
                    $enchantEquip           = new EnchantEquip();
                    $enchantEquip->items    = new Vector();
                    $enchantEquip->itemType = $itemType;
                    $enchant->equips->push($enchantEquip);
                    $v = $enchantEquip;
                }

                if ($v === null)
                {
                    throw new \Exception("[附魔配置-加载] id:{$key} 创建equip配置失败");
                }

                $item    = new EnchantEquipItem();
                $item->p = new Pair($dwAttr, isset($val[$name]) ? $val[$name] : 0);
                $v->items->push($item);
                $attr->equipsRate[$itemType] = isset($val[$name]) ? $val[$name] : 0;
            }
        }

        unset($enchant);
        for ($i = EEnchantType::EENCHANTTYPE_MIN + 1; $i < EEnchantType::EENCHANTTYPE_MAX; $i++)
        {
            $enchant = $map[$i];
            foreach ($enchant->attrVec as $attr)
            {
                foreach ($attr->extraBuff as $extra)
                {
                    $extra->value += $attr->extraMaxWeight;
                    $attr->extraMaxWeight = $extra->value;
                }

                foreach ($attr->items as $item)
                {
                    $item->weight += $attr->maxWeight;
                    $attr->maxWeight = $item->weight;
                }
            }

            /** @var EnchantEquip $equip */
            foreach ($enchant->equips as $equip)
            {
                $maxWeight = 0;
                foreach ($equip->items as $item)
                {
                    if ($item->p->value == 0)
                    {
                        continue;
                    }
                    $maxWeight += $item->p->value;
                }
                $equip->maxWeight = $maxWeight;
            }
        }

        return self::$enchantConfigMap = $map;
    }

    /**
     * @return EnchantConfig[]
     */
    public static function getConfigs()
    {
        return self::$enchantConfigMap;
    }

    /**
     * @param $type
     * @return EnchantConfig|null
     */
    public static function getEnchantConfig($type)
    {

        if ($type <= EEnchantType::EENCHANTTYPE_MIN || $type >= EEnchantType::EENCHANTTYPE_MAX || ROConfig::isValidEnchantType($type) === false)
            return null;

        return self::$enchantConfigMap[$type];
    }

    /**
     * @param $type
     * @return null|EnchantAttr
     */
    public function getEnchantAttrByType($type)
    {
        foreach ($this->attrVec as $attr)
        {
            if ($attr->type == $type)
            {
                return $attr;
            }
        }

        return null;
    }

    /**
     * 根据附魔类型和装备类型查找对应词条
     * 找到的词条必须符合装备类型出现在该词条的概率大于0的情况
     * 多个相同等级相同属性的词条在同一个装备中不会出现多个概率的情况
     *
     * @param $itemType
     * @param $attrType
     * @return null|EnchantAttr
     */
    public function getEnchantAttrByAttrTypeAndItemType($attrType, $itemType)
    {
        /** @var EnchantAttr $attr */
        foreach ($this->attrVec as $attr)
        {
            if ($attr->type == $attrType && isset($attr->equipsRate[$itemType]) && $attr->equipsRate[$itemType] > 0)
            {
                return $attr;
            }
        }

        return null;
    }

    /**
     * 根据configId获取附魔属性
     *
     * @param $id
     * @return null|EnchantAttr
     */
    public function getEnchantAttrById($id)
    {
        /** @var EnchantAttr $attr */
        foreach ($this->attrVec as $attr)
        {
            if ($attr->configId == $id)
            {
                return $attr;
            }
        }

        return null;
    }

    /**
     * 根据buffId获取configId
     *
     * @param $id
     * @return int|null
     */
    public function getEnchantConfigIdByBuffId($id)
    {
        /** @var EnchantAttr $attr */
        foreach ($this->attrVec as $attr)
        {
            /** @var Pair $buff */
            foreach ($attr->extraBuff as $buff)
            {
                if ($buff->key == $id)
                {
                    return $attr->configId;
                }
            }
        }

        return null;
    }

    /**
     * 根据buffId获取附魔属性
     *
     * @param $id
     * @return null|EnchantAttr
     */
    public function getEnchantAttrByBuffId($id)
    {
        /** @var EnchantAttr $attr */
        foreach ($this->attrVec as $attr)
        {
            /** @var Pair $buff */
            foreach ($attr->extraBuff as $buff)
            {
                if ($buff->key == $id)
                {
                    return $attr;
                }
            }
        }

        return null;
    }

    /**
     * @param $type
     * @return null|EnchantEquip
     */
    public function getEnchantEquipByType($type)
    {
        foreach ($this->equips as $equip)
        {
            if ($equip->itemType == $type)
            {
                return $equip;
            }
        }

        return null;
    }


}