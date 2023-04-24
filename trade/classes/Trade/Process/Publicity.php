<?php
namespace RO\Trade\Process;

use RO\Trade\Dao\ItemList;
use RO\Trade\Server;

/**
 * 处理公示的子进程对象
 *
 * @package RO\Trade\Process
 */
class Publicity extends \MyQEE\Server\WorkerCustom
{
    public function onStart()
    {
        # reload 可能导致锁没有解掉，所以启动时直接解锁先
        Server::$publicityLock->unlock();

        Server::initConnection();

        # 读取配置
        Server::loadTableConfig();

        swoole_timer_tick(1000, function()
        {
            if (true === Server::$publicityLock->trylock())
            {
                ItemList::timerTaskOffShelf();
                Server::$publicityLock->unlock();
            }
        });

        # 处理公示延迟
        swoole_timer_tick(1000 * 60 * 1, function()
        {
            Server::doDelayPublicity(Server::$mysqlMaster);
        });
    }

    public function onPipeMessage($server, $fromWorkerId, $message, $fromServerId = -1)
    {
        switch ($message)
        {
            case 'reloadConfig':
                Server::loadTableConfig();
                break;
        }
    }
}