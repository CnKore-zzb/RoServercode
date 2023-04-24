<?php
namespace RO\Trade;

/**
 * 任务进程推送对象
 *
 * @package RO\Trade
 */
class ItemTask extends \stdClass
{
    /**
     * 物品 itemListId
     *
     * @var int
     */
    public $id;

    /**
     * 类型
     *
     * @var int
     */
    public $type;

    const TYPE_BUY           = 1;        // 购买请求
    const TYPE_BUY_ON_RETURN = 2;        // 购买请求服务器返回
    const TYPE_GOODS_CANCEL  = 3;        // 下架

    const TYPE_CHANNEL_GOODS_NEW       = 5;    // 上架
    const TYPE_CHANNEL_GOODS_RESELL    = 6;    // 重新上架
    const TYPE_CHANNEL_OFF_GOODS       = 7;    // 下架物品
    const TYPE_CHANNEL_RESET_STOCK     = 8;    // 重新加载库存
    const TYPE_CHANNEL_DEL_STOCK_CACHE = 9;    // 移除库存缓存

    /**
     * 根据物品ID投递
     *
     * @param $listId
     * @return int|false
     */
    public function send()
    {
        $this->id = (int)$this->id;
        if (!$this->id > 0)return false;
        return Server::$instance->worker->task($this, Server::getTaskIdById($this->id));
    }


    /**
     * 根据物品ID投递到列队里
     *
     * @param $listId
     * @return bool
     */
    public function sendToChannel()
    {
        $this->id = (int)$this->id;
        $channel  = Server::getChannelByListId($this->id);
        while (true)
        {
            if (true === $channel->push($this))
            {
                break;
            }
            usleep(10);
        }

        return true;
    }
}