<?php
namespace RO\Booth;

use RO\Trade\Server;

/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/6/21
 * Time: 下午6:08
 *
 * 单进程对象,尽量全局只保持一个
 */
class BoothPlayerLock
{
    private $timeoutMap = [];

    const BOOTH_LOCK_KEY = 'booth:lock:%s';

    public function tryLock($charId, $timeout = 30)
    {
        try
        {
            $timeoutTime = time() + $timeout;
            $rs = Server::$redis->set(sprintf(self::BOOTH_LOCK_KEY, $charId), $timeoutTime, ['nx', 'ex' => $timeout]);
            if ($rs) {
                $timeoutTime[$charId] = $timeoutTime;
            }
            return $rs;
        }
        catch (\Exception $e)
        {
            # Redis 连接失败
            Server::$instance->warn("[redis] 设置摆摊玩家锁失败, charId: $charId, msg: ". $e->getMessage());
            return false;
        }
    }

    public function unLock($charId)
    {
        // 同一进程中对该玩家上过锁
        if(isset($this->timeoutMap[$charId])) {
            if (time() < $this->timeoutMap[$charId]) {
                unset($this->timeoutMap[$charId]);
                try
                {
                    return Server::$redis->del(sprintf(self::BOOTH_LOCK_KEY, $charId)) ? true : false;
                }
                catch (\Exception $e)
                {
                    # Redis 连接失败
                    Server::$instance->warn("[redis] 删除摆摊玩家锁失败, charId: $charId, msg: ". $e->getMessage());
                    return false;
                }
            }
        }

        return true;
    }
}