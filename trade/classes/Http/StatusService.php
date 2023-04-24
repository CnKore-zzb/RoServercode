<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/17
 * Time: 下午7:33
 */

namespace RO\Http;

use RO\Trade\Server;

class StatusService
{
    public static function getTaskStatus()
    {
        $data = [];
        foreach (Server::$taskStatus as $taskId => $status)
        {
            $data[$taskId] = $status['queue'];
        }

        return $data;
    }

    public static function openTrade()
    {
        $open = Server::isOpen();
        if(!Server::isOpen())
        {
            Server::$instance->info('[开放交易所API] 执行成功');
            Server::openTrade();
        }
        else
        {
            Server::$instance->info('[开放交易所API] 交易所已开启');
        }

        return ['is_open' => $open];
    }

    public static function closeTrade()
    {
        $open = Server::isOpen();
        if(Server::isOpen())
        {
            Server::$instance->info('[关闭交易所API] 执行成功');
            Server::closeTrade();
        }
        else
        {
            Server::$instance->info('[关闭交易所API] 交易所已关闭');
        }

        return ['is_open' => $open];
    }

    public static function reloadConfig()
    {
        /** @var \RO\Trade\WorkerMain $worker */
        $worker = Server::$instance->worker;
        $worker->reloadConfig();

        if (IS_DEBUG)
        {
            Server::$instance->debug('[重新加载配置API] 执行成功');
        }

        return 'success';
    }
}