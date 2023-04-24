<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/7/12
 * Time: 下午4:41
 */

namespace RO\Trade;

class Player
{
    private static $fields = [
        'zoneId'            => 'zoneid',
        'pendingLimit'      => 'pendinglimit',
        'returnRate'        => 'returnrate',
        'quota'             => 'quota',
        'boothPendingLimit' => 'boothPendinglimit',
        'boothOpenStatus'   => 'boothOpenStatus',
    ];

    protected static function init(int $charId, array $value = [])
    {
        $charData = ROCache::getCharData($charId);
        if (!$charData)
            return false;

        $data = [];
        foreach (self::$fields as $field => $key)
        {
            if (isset($value[$field]))
            {
                $data[$field] = $value[$field];
            }
            else if (isset($charData[$key]))
            {
                $data[$field] = $charData[$key];
            }
        }

        if (IS_DEBUG)
        {
            Server::$instance->debug("[玩家表] 初始化数据. char_id:{$charId}, data:" . json_encode($data) . ", char_data:" . json_encode($charData));
        }
        Server::$players->set($charId, $data);
        return $data;
    }

    public static function setValue($charId, array $value)
    {
        if (Server::$players->exist($charId))
        {
            return Server::$players->set($charId, $value);
        }
        else
        {
            return self::init($charId, $value) === false ? false : true;
        }
    }

    public static function getValue($charId, $field, $default = null)
    {
        $value = Server::$players->get($charId, $field);
        if ($value === false)
        {
            $data = self::init($charId);
            if ($data === false)
            {
                return $default;
            }

            return $data[$field] ?? $default;
        }

        return $value;
    }

    /**
     * 获取返回上架费率（千分比）
     *
     * @param $charId
     * @return int
     */
    public static function getReturnRate($charId)
    {
        return intval(self::getValue($charId, 'returnRate'));
    }

    public static function setReturnRate($charId, int $value)
    {
        return self::setValue($charId, ['returnRate' => $value]);
    }

    /**
     * 获取额外挂单数
     *
     * @param $charId
     * @return int
     */
    public static function getPendingLimit($charId)
    {
        return intval(self::getValue($charId, 'pendingLimit', 0));
    }

    public static function setPendingLimit($charId, int $value)
    {
        return self::setValue($charId, ['pendingLimit' => $value]);
    }

    /**
     * 获取摆摊额外挂单数
     *
     * @param $charId
     * @return int
     */
    public static function getBoothPendingLimit($charId)
    {
        return intval(self::getValue($charId, 'boothPendingLimit', 0));
    }

    public static function setBoothPendingLimit($charId, int $value)
    {
        return self::setValue($charId, ['boothPendingLimit' => $value]);
    }

    /**
     * 获取额度
     *
     * @param $charId
     * @return int
     */
    public static function getQuota($charId)
    {
        return intval(self::getValue($charId, 'quota', 0));
    }

    public static function setQuota($charId, int $value)
    {
        return self::setValue($charId, ['quota' => $value]);
    }

    /**
     * 获取zone_id
     *
     * @param $charId
     * @param bool $actual 是否准确获取zoneId
     * @return int
     */
    public static function getZoneId($charId, $actual = false)
    {
        if ($actual)
        {
            $value = ROCache::getZoneId($charId);
            if ($value)
            {
                self::setZoneId($charId, $value);
                return $value;
            }
        }

        return intval(self::getValue($charId, 'zoneId', 0));
    }

    public static function setZoneId($charId, int $value)
    {
        return self::setValue($charId, ['zoneId' => $value]);
    }

    /**
     * 获取开启状态
     *
     * @param $charId
     * @return int
     */
    public static function getBoothOpenStatus($charId)
    {
        return intval(self::getValue($charId, 'boothOpenStatus', 0));
    }

    public static function setBoothOpenStatus($charId, int $value)
    {
        return self::setValue($charId, ['boothOpenStatus' => $value]);
    }
}