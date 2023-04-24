<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/6/22
 * Time: 下午2:25
 */

namespace RO\Booth\Tasks;

use RO\Trade\Server;

abstract class BoothTask extends \stdClass
{
    abstract public function execute($taskId);

    /**
     * 根据玩家id投递到指定task进程
     *
     * @param int $charId
     * @param callable $finishCallback
     * @return bool|int
     */
    public function deliver(int $charId, callable $finishCallback = null)
    {
        $workerId = Server::getTaskIdById($charId);
        return Server::$instance->worker->task($this, $workerId, $finishCallback);
    }
}