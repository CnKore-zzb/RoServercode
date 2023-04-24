<?php
namespace RO\Trade\Process;

use RO\Trade\Server;

/**
 * 交易所管理子进程对象
 *
 * @package RO\Trade\Process
 */
class TradeManager extends \MyQEE\Server\WorkerCustom
{

    public function onStart()
    {
        \RO\Trade\TradeManager::create();
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