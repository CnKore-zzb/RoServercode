<?php
namespace RO\Trade\Process;

use RO\Trade\Server;

/**
 * 价格管理子进程对象
 *
 * @package RO\Trade\Process
 */
class PriceManager extends \MyQEE\Server\WorkerCustom
{
    /** @var \RO\Trade\PriceManager */
    private $priceManager;

    public function onStart()
    {
        Server::initConnection();
        # 读取配置
        Server::loadTableConfig();
        $this->priceManager  = new \RO\Trade\PriceManager();
        $this->priceManager->init();
        $this->priceManager->reloadStock();
    }

    public function onPipeMessage($server, $fromWorkerId, $message, $fromServerId = -1)
    {
        $params = @explode(' ', $message);
        if (!$params) return;
        $cmd = $params[0];
        switch ($cmd)
        {
            case 'reloadConfig':
                Server::loadTableConfig();
                $this->priceManager->init();
                break;
            case 'setPrice':
                if(!isset($params[1])) break;
                if(!isset($params[2])) break;
                $this->priceManager->setPrice((int)$params[1], (int)$params[2]);
                break;
            case 'adjustPrice':
                if(!isset($params[1])) break;
                $this->priceManager->instantAdjustPrice((int)$params[1]);
                break;
        }
    }
}