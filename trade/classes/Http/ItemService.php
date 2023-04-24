<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/5/23
 * Time: 上午11:16
 */

namespace RO\Http;

use DrSlump\Protobuf\Exception;
use RO\Cmd\ETradeType;
use RO\Trade\Command;
use RO\Trade\Dao\ItemInfo;
use RO\Trade\Dao\Prohibition;
use RO\Trade\Server;

class ItemService
{
    public static function get($params)
    {
        $itemId = isset($params['item_id']) ? (int) $params['item_id'] : null;
        if (null === $itemId)
        {
            throw new \InvalidArgumentException('无效参数');
        }

        $rs = Server::$item->get($itemId);
        if (!$rs)
        {
            throw new Exception('物品不存在');
        }

        $rs['id'] = $itemId;
        return $rs;
    }

    public static function getList($params)
    {
        if (!is_array($params) || empty($params))
        {
            throw new \InvalidArgumentException('无效参数');
        }

        $list = ItemInfo::getList($params);
        $count = ItemInfo::count();

        if (false === $list || false === $count)
        {
            throw new \Exception('获取失败');
        }

        return [
            'list' => $list,
            'count' => $count
        ];
    }

    public static function setConfig($params)
    {
        if (!is_array($params) || empty($params))
        {
            throw new \InvalidArgumentException('无效参数');
        }

        $itemId    = !empty($params['item_id']) ? (int)$params['item_id'] : 0;
        $upRatio   = !empty($params['up_ratio']) ? (float)$params['up_ratio'] : 1;
        $downRatio = !empty($params['down_ratio']) ? (float)$params['down_ratio'] : 1;
        $maxPrice  = !empty($params['max_price']) ? (int)$params['max_price'] : 0;

        $itemInfo = ItemInfo::getById($itemId);
        $item = Server::$item->get($itemId);
        if (!$itemInfo)
        {
            throw new Exception('物品不存在');
        }

        if (!$item)
        {
            throw new Exception('物品在内存中不存在');
        }

        $itemInfo->upRatio   = $upRatio;
        $itemInfo->downRatio = $downRatio;
        $itemInfo->maxPrice  = $maxPrice;
        if (false === $itemInfo->update())
        {
            throw new Exception('修改失败');
        }

        return Server::$item->set($itemId, [
            'upRatio'   => $upRatio,
            'downRatio' => $downRatio,
            'maxPrice'  => $maxPrice,
        ]);
    }

    public static function setPrice($itemId, $price)
    {
        $itemId = (int)$itemId;
        $price  = (int)$price;
        if (!Server::$item->get($itemId))
        {
            throw new Exception('物品不存在');
        }

        if ($price <= 0)
        {
            throw new Exception('价格必须大于0');
        }

        $rs = Server::$instance->worker->sendMessageToCustomWorker(sprintf("setPrice %d %d", $itemId, $price), 'PriceManager');
        if (!$rs)
        {
            throw new Exception('设置失败');
        }

        return true;
    }

    public static function prohibit($params)
    {
        if (!is_array($params) || empty($params))
        {
            throw new \InvalidArgumentException('无效参数');
        }

        $params = array_map(function($val)
        {
            return (int)$val;
        }, $params);

        $type          = isset($params['type']) ? $params['type'] : Prohibition::ITEM_TYPE;
        $itemId        = isset($params['item_id']) ? $params['item_id'] : 0;
        $refineLv      = isset($params['refine_lv']) ? $params['refine_lv'] : 0;
        $enchantId     = isset($params['enchant_id']) ? $params['enchant_id'] : 0;
        $enchantBuffId = isset($params['enchant_buff_id']) ? $params['enchant_buff_id'] : 0;
        $tradeType     = isset($params['trade_type']) ? $params['trade_type'] : ETradeType::ETRADETYPE_TRADE;

        /** @var \RO\Trade\WorkerMain $worker */
        switch ($type)
        {
            case Prohibition::ITEM_TYPE:
                $cmd                = new Command();
                $cmd->type          = Prohibition::ITEM_TYPE;
                $cmd->itemId        = $itemId;
                $cmd->refineLv      = $refineLv;
                $cmd->enchantId     = $enchantId;
                $cmd->enchantBuffId = $enchantBuffId;
                $cmd->tradeType     = $tradeType;
                $worker             = Server::$instance->worker;
                $worker->tradeSecurityCommand($cmd);
                break;
            case Prohibition::ENCHANT_TYPE:
                $cmd                = new Command();
                $cmd->type          = Prohibition::ENCHANT_TYPE;
                $cmd->enchantId     = $enchantId;
                $cmd->enchantBuffId = $enchantBuffId;
                $cmd->tradeType     = $tradeType;
                $worker             = Server::$instance->worker;
                $worker->tradeSecurityCommand($cmd);
                break;
        }

        if (IS_DEBUG)
        {
            Server::$instance->debug('[安全指令API] 执行成功');
        }

        return 'success';
    }

    public static function getProhibitions()
    {
        $rs = Prohibition::getAll();
        if ($rs !== false)
        {
            return $rs;
        }
        else
        {
            throw new Exception('获取交易安全指令列表失败');
        }
    }
}