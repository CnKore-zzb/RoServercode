<?php
namespace RO\Trade;

class WorkerLocal extends \MyQEE\Server\WorkerTCP
{
    public function onReceive($server, $fd, $fromId, $data)
    {
        switch ($data)
        {
            case 'stop':
                # 停止服务
                Server::closeTrade();

                for($i = 0; $i < $this->server->setting['task_worker_num']; $i++)
                {
                    while (true)
                    {
                        $rs2 = $this->taskWait('stop', 0.5, $i);
                        if ($rs2 == 'ok')
                        {
                            break;
                        }
                        else
                        {
                            usleep(300);
                        }
                    }
                }
                $server->send($fd, "交易所已设为关闭状态，新的购买请求将被拒绝，10秒后关闭", $fromId);
                static $i = 10;
                swoole_timer_tick(1000, function() use (& $i, $fd, $fromId)
                {
                    $i--;

                    if ($i == 0)
                    {
                        $this->server->send($fd, "交易所服务器关闭", $fromId);
                        $this->server->shutdown();
                    }
                    else
                    {
                        $this->server->send($fd, "$i 秒后将关闭", $fromId);
                    }
                });

                break;

            default:
                $server->send($fd, "未知指令", $fromId);
                usleep(20000);
                $server->close($fd, $fromId);
        }
    }
}