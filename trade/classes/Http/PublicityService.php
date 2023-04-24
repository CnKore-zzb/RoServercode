<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/17
 * Time: 下午7:45
 */

namespace RO\Http;

use Exception;
use RO\Trade\Server;

class PublicityService
{
//    public static function delayPublicity($begin, $end)
//    {
////        if ($begin < time())
////        {
////            throw new Exception("维护开始时间不能少于当前时间");
////        }
//
//        $begin = abs($begin);
//        $end   = abs($end);
//
//        if ($begin > $end)
//        {
//            throw new Exception("维护开始时间不能大于维护结束时间");
//        }
//
//        $data = json_encode(['begin' => $begin, 'end' => $end]);
//
//        $file = Server::$dataDir . Server::$platName . '_' . Server::$regionName . '_maintain.json';
//
//        $fp   = fopen($file, 'w');
//        if (flock($fp, LOCK_EX | LOCK_NB))
//        {
//            if (!file_put_contents($file, $data))
//            {
//                throw new Exception('操作失败, 写入文件发生错误!');
//            }
//        }
//        else
//        {
//            throw new Exception('系统正在处理公示延期中,请稍候操作');
//        }
//
//        flock($fp, LOCK_UN);
//        fclose($fp);
//
//        if (IS_DEBUG)
//        {
//            Server::$instance->debug('[公示延期API] 执行成功, 维护时间:' . date('Y-m-d H:i:s', $begin) . ' 至 ' . date('Y-m-d H:i:s', $end));
//        }
//
//        return 'ok';
//    }

//    public static function publicize($itemKey, $price = 0, $endTime = null)
//    {
//        $publicity = Publicity::getByKey($itemKey);
//        $logTime   = time() - Server::$configExchange['LogTime'];
//
//        if ($publicity === false)
//        {
//            echo "itemKey:{$itemKey}}物品公式失败";
//            return;
//        }
//
//        if ($publicity && $publicity->endTime > $logTime)
//        {
//            echo "itemKey:{$itemKey}}物品还处于公式中";
//            return;
//        }
//
//        $mysql = Server::$mysqlMaster;
//        if (!$mysql->begin_transaction())
//        {
//            echo '开启事务失败, ' . $mysql->error;
//            return;
//        }
//
//        try
//        {
//            $itemList = ItemList::getByKey($itemKey);
//            if (!$itemList && $itemList->stock > 0)
//            {
//                throw new Exception("itemKey:{$itemKey}}物品, 没有玩家挂单");
//            }
//
//            $item = Item::get($itemList->itemId, $itemList->getItemData());
//            if ($publicity === null)
//            {
//                $publicity            = new Publicity();
//                $publicity->price     = $price ?: $item->getPrice();
//                $publicity->endTime   = $endTime ?: time();
//                $publicity->buyPeople = 0;
//                $publicity->key       = $itemKey;
//                if ($publicity->insert())
//                {
//                    throw new Exception('公式表插入失败');
//                }
//            }
//            else
//            {
//                $publicity->price     = $price ?: $item->getPrice();
//                $publicity->endTime   = $endTime ?: time();
//                $publicity->buyPeople = 0;
//                if ($publicity->update() === false)
//                {
//                    throw new Exception('公式表更新失败');
//                }
//            }
//
//            $itemList->isPublicity = $publicity->id;
//            $itemList->endTime     = $publicity->endTime;
//
//            if ($itemList->update() === false)
//            {
//                throw new Exception('物品表更新失败');
//            }
//
//            $sql = "UPDATE `trade_goods` SET `publicity_id` = '{$publicity->id}', `end_time` = '{$publicity->endTime}' WHERE `itemKey` = '{$itemKey}' AND `status` = " . Goods::STATUS_SELLING;
//            $rs  = Server::$mysqlMaster->query($sql);
//            if ($rs)
//            {
//                echo "公式成功!";
//                $mysql->commit();
//            }
//            else
//            {
//                throw new Exception('挂单表更新失败');
//            }
//        }
//        catch (\Exception $e)
//        {
//            $mysql->rollback();
//            echo $e->getMessage() . PHP_EOL;
//        }
//    }

//    public static function editPublicity($itemKey, $endTime)
//    {
//        if ($endTime <= time())
//        {
//            echo "修改公式结束时间无效,必须设置大于当前时间的时间戳";
//            return;
//        }
//
//        $publicity = Publicity::getByKey($itemKey);
//
//        if (!$publicity)
//        {
//            echo "itemKey:{$itemKey}}物品从未公式";
//            return;
//        }
//
//        if ($publicity->endTime <= (time() - 2 * 60))
//        {
//            echo  "itemKey:{$itemKey}}物品离公式结束还有2分钟,不允许修改结束时间";
//            return;
//        }
//
//        $itemList = ItemList::getByKey($itemKey);
//        if (!$itemList && $itemList->stock > 0)
//        {
//            echo "itemKey:{$itemKey}}物品, 没有玩家挂单";
//            return;
//        }
//
//        $mysql = Server::$mysqlMaster;
//        if (!$mysql->begin_transaction())
//        {
//            echo '开启事务失败, ' . $mysql->error;
//            return;
//        }
//
//        try
//        {
//            $publicity->endTime   = $endTime;
//            if ($publicity->update() === false)
//            {
//                throw new Exception('公式表更新失败');
//            }
//
//            $itemList->endTime     = $publicity->endTime;
//            if ($itemList->update() === false)
//            {
//                throw new Exception('物品表更新失败');
//            }
//
//            $sql = "UPDATE `trade_goods` SET `publicity_id` = '{$publicity->id}', `end_time` = '{$publicity->endTime}' WHERE `itemKey` = '{$itemKey}' AND `status` = " . Goods::STATUS_SELLING;
//            $rs  = Server::$mysqlMaster->query($sql);
//            if ($rs)
//            {
//                echo "修改公式时间成功!";
//                $mysql->commit();
//            }
//            else
//            {
//                throw new Exception('挂单表更新失败');
//            }
//        }
//        catch (\Exception $e)
//        {
//            $mysql->rollback();
//            echo $e->getMessage() . PHP_EOL;
//        }
//
//    }
//
//    public static function editPublicityPrice($itemKey, $price)
//    {
//        if ($price < 0)
//        {
//            echo "价格必须大于0";
//            return;
//        }
//
//        $publicity = Publicity::getByKey($itemKey);
//
//        if (!$publicity)
//        {
//            echo "itemKey:{$itemKey}}物品从未公式";
//            return;
//        }
//
//        if ($publicity->endTime <= (time() - 2 * 60))
//        {
//            echo  "itemKey:{$itemKey}}物品离公式结束还有2分钟,不允许修改价格";
//            return;
//        }
//
//        $itemList = ItemList::getByKey($itemKey);
//        if (!$itemList && $itemList->stock > 0)
//        {
//            echo "itemKey:{$itemKey}}物品, 没有玩家挂单";
//            return;
//        }
//
//        $publicity->price = (int) $price;
//        if ($publicity->update() === false)
//        {
//            echo "修改价格失败!";
//        }
//        else
//        {
//            echo "修改价格成功!";
//        }
//    }
}