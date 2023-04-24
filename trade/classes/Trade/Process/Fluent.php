<?php
namespace RO\Trade\Process;

use RO\Trade\FluentClient;
use RO\Trade\Server;

/**
 * Fluent 推送子进程对象
 *
 * @package RO\Trade\Process
 */
class Fluent extends \MyQEE\Server\WorkerCustom
{
    /**
     * @var FluentClient
     */
    protected static $fluent;

    /** @var FluentClient */
    protected static $adjustFluent;

    public function onStart()
    {
        $config = Server::$instance->config['fluent'] ?: ['127.0.0.1:13100'];
        list($host, $port) = explode(':', $config[0]);

        $plat = 'ro';
        if (Server::$platName !== 'xd' && Server::$platName !== 'autumn')
        {
            $plat = 'ro_' . Server::$platName;
        }

        self::$fluent = new FluentClient($host, $port, "xd.game.{$plat}.trade");

        self::$adjustFluent = new FluentClient($host, $port, "xd.game.{$plat}.trade_adjust");

        swoole_timer_tick(300, function()
        {
            # 读取
            while (false !== ($tmp = Server::$fluentChannel->pop()))
            {
                self::$fluent->push($tmp);
            };
        });

        swoole_timer_tick(1000, function()
        {
            # 读取
            while (false !== ($tmp = Server::$adjustFluentChannel->pop()))
            {
                self::$adjustFluent->push($tmp);
            };
        });
    }

    public function onStop()
    {
        $i = 0;
        while (false !== ($tmp = Server::$fluentChannel->pop()))
        {
            if ($i == 0)
            {
                self::$fluent->saveBufferRuntime(true);
                $i = 1;
            }
            self::$fluent->push($tmp);
        }
    }
}