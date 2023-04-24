<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/5/22
 * Time: 下午12:03
 */

namespace RO\Trade;

use RO\Redis;

class ROCache
{
    /** @var Redis */
    private static $redis;

    public static function getRedis()
    {
        if (!isset(self::$redis) || self::$redis === false)
        {
            self::$redis = Server::createRedis();
            if (self::$redis)
            {
                self::$redis->setOption(\Redis::OPT_PREFIX, Server::$regionId . ':');
            }
        }

        return self::$redis;
    }

    /**
     * 获取玩家信息
     *
     * @param $charId
     * @return bool|string
     */
    public static function getCharData($charId)
    {
        try
        {
            return self::getRedis()->hGetAll("gchardata:{$charId}:default:data");
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[获取玩家信息] 发生异常, 错误信息: {$e->getMessage()}");
            return false;
        }
    }

    /**
     * 获取返回上架费率（千分比）
     *
     * @param $charId
     * @return int
     */
    public static function getReturnRate($charId)
    {
        try
        {
            return intval(self::getRedis()->hGet("gchardata:{$charId}:default:data", 'returnrate'));
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[获取返回上架费率] 发生异常, 错误信息: {$e->getMessage()}");
            return 0;
        }
    }

    /**
     * 获取额外挂单数
     *
     * @param $charId
     * @return int
     */
    public static function getPendingLimit($charId)
    {
        try
        {
            return intval(self::getRedis()->hGet("gchardata:{$charId}:default:data", 'pendinglimit'));
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[获取额外挂单数] 发生异常, 错误信息: {$e->getMessage()}");
            return 0;
        }
    }

    public static function getBoothPendingLimit($charId)
    {
        try
        {
            return intval(self::getRedis()->hGet("gchardata:{$charId}:default:data", 'boothPendingLimit'));
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[获取额外挂单数] 发生异常, 错误信息: {$e->getMessage()}");
            return 0;
        }
    }

    /**
     * 获取额度
     *
     * @param $charId
     * @return int
     */
    public static function getQuota($charId)
    {
        try
        {
            return intval(self::getRedis()->hGet("gchardata:{$charId}:default:data", 'quota'));
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[获取额度] 发生异常, 错误信息: {$e->getMessage()}");
            return 0;
        }
    }

    /**
     * 获取zone_id
     *
     * @param $charId
     * @return bool|int
     */
    public static function getZoneId($charId)
    {
        try
        {
            return intval(self::getRedis()->hGet("gchardata:{$charId}:default:data", 'zoneid'));
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[获取zone_id] 发生异常, 错误信息: {$e->getMessage()}");
            return 0;
        }
    }

    /**
     * 获取开启状态
     *
     * @param $charId
     * @return bool|int
     */
    public static function getBoothOpenStatus($charId)
    {
        try
        {
            return intval(self::getRedis()->hGet("gchardata:{$charId}:default:data", 'boothOpenStatus'));
        }
        catch (\Exception $e)
        {
            Server::$instance->warn("[获取摆摊开启状态] 发生异常, 错误信息: {$e->getMessage()}");
            return 0;
        }
    }
}