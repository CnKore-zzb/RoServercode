<?php
namespace RO\Trade\Process;

use RO\Trade\JobFifo;
use RO\Trade\Server;

/**
 * 延迟任务子进程对象
 *
 * @package RO\Trade\Process
 */
class DelayJob extends \MyQEE\Server\WorkerCustom
{
    public function onStart()
    {
        # 读取配置
        Server::loadTableConfig();

        # 处理缓写任务
        self::$Server->setProcessTag("delayJob");

        $delayJob = JobFifo::instance('delay-buy.job');
        swoole_timer_tick(1000, function() use ($delayJob)
        {
            $delayJob->exec();
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