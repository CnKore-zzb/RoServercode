<?php
namespace RO\Trade;

use RO\Booth\Dao\BoothOrder;
use RO\Cmd\ETradeType;
use RO\Cmd\ItemData;
use RO\Cmd\QueryItemCountTradeCmd;
use RO\Cmd\TradeItemBaseInfo;
use RO\Trade\Dao\ItemList;

class ActList
{
    /**
     * 按分类分的列表
     *
     * @var \Ds\Map
     */
    protected static $simpleList;

    /**
     * 按物品ID分的列表
     *
     * @var \Ds\Map
     */
    protected static $itemIdList;

    /**
     * 已经加载的数量
     *
     * @var int
     */
    protected static $loadedCount = 0;

    protected static $updateTick;

    /**
     * 输出列表（在 TaskId=0的进程里执行）
     */
    public static function outList()
    {
        # 读取通道中的数据
        $listSimple = new \Ds\Map();
        $listItem   = new \Ds\Map();
        $userMap    = new \Ds\Map();

        $i = 0;
        $c = null;
        while (false !== ($data = Server::$listChannel->pop()))
        {
            $i++;
            if (null === $c)
            {
                $c = Server::$listChannel->stats()['queue_num'] + 1;
            }

            $type = @unpack('C', $data[0]);
            switch ($type[1])
            {
                case 1:
                    # 分类列表
                    $arr = @unpack('Ctype/QcharId/Qfd/SfromId/Scategory/Sjob/Cfashion', $data);
                    if ($arr)
                    {
                        $charId        = $arr['charId'];
                        $fd            = $arr['fd'];
                        $fromId        = $arr['fromId'];
                        $category      = $arr['category'];
                        $jobAndFashion = $arr['job'] << 8 | $arr['fashion'];
                        if (isset($listSimple[$category]))
                        {
                            $list = $listSimple[$category];
                        }
                        else
                        {
                            $list = $listSimple[$category] = new \Ds\Map();
                        }

                        if (isset($list[$jobAndFashion]))
                        {
                            $listJobAndFashion = $list[$jobAndFashion];
                        }
                        else
                        {
                            $listJobAndFashion = $list[$jobAndFashion] = new \Ds\Map();
                        }

                        if (isset($userMap[$charId]))
                        {
                            # 同一个玩家有多次请求，只给最后一个，将之前的删除
                            $t = $userMap[$charId][0] ?? 0;
                            if ($t == 1)
                            {
                                $a = $userMap[$charId][1];
                                $b = $userMap[$charId][2];
                                unset($listSimple[$a][$b][$charId]);
                            }
                            else if ($t == 2)
                            {
                                $a = $userMap[$charId][1];
                                $b = $userMap[$charId][2];
                                $c = $userMap[$charId][3];
                                unset($listItem[$a][$b][$c][$charId]);
                            }
                        }

                        $listJobAndFashion[$charId] = new \Ds\Pair($fd, $fromId);
                        $userMap[$charId]           = new \Ds\Vector([1, $category, $jobAndFashion]);
                    }
                    break;
                case 2:
                    # 物品列表
                    $arr = @unpack('Ctype/QcharId/Qfd/SfromId/LitemId/CrankType/Lindex/LtradeType', $data);
                    if ($arr)
                    {
                        $charId   = $arr['charId'];
                        $fd       = $arr['fd'];
                        $fromId   = $arr['fromId'];
                        $itemId   = $arr['itemId'];
                        $rankType = $arr['rankType'];
                        $index    = $arr['index'];
                        $tradeType = $arr['tradeType'];

                        if (isset($listItem[$itemId]))
                        {
                            $list = $listItem[$itemId];
                        }
                        else
                        {
                            $list = $listItem[$itemId] = new \Ds\Map();
                        }

                        if (isset($list[$tradeType]))
                        {
                            $listTradeType = $list[$tradeType];
                        }
                        else
                        {
                            $listTradeType = $list[$tradeType] = new \Ds\Map();
                        }

                        if (isset($listTradeType[$rankType]))
                        {
                            $listType = $listTradeType[$rankType];
                        }
                        else
                        {
                            $listType = $listTradeType[$rankType] = new \Ds\Map();
                        }

                        if (isset($userMap[$charId]))
                        {
                            # 同一个玩家有多次请求，只给最后一个，将之前的删除
                            $t = $userMap[$charId][0] ?? 0;
                            if ($t == 1)
                            {
                                $a = $userMap[$charId][1];
                                $b = $userMap[$charId][2];
                                unset($listSimple[$a][$b][$charId]);
                            }
                            else if ($t == 2)
                            {
                                $a = $userMap[$charId][1];
                                $b = $userMap[$charId][2];
                                $c = $userMap[$charId][3];
                                unset($listItem[$a][$b][$c][$charId]);
                            }
                        }

                        $listType[$charId] = [$index, $fd, $fromId];
                        $userMap[$charId]  = new \Ds\Vector([2, $itemId, $tradeType, $rankType]);
                    }
                    break;
                case 3:
                    try
                    {
                        $arr    = @unpack('Ctype/Qfd/SfromId/a*cmd', $data);
                        $fd     = $arr['fd'];
                        $fromId = $arr['fromId'];
                        $cmd    = new QueryItemCountTradeCmd($arr['cmd']);
                        self::getMinPriceItems($cmd, $cmd->charid, $fd, $fromId);
                    }
                    catch (\Exception $e)
                    {
                        Server::$instance->warn("[获取最低价格物品] 发生异常: {$e->getMessage()}");
                    }
                    break;
            }

            if ($i == $c)break;
        }

        unset($userMap);

        # 处理分类列表
        if ($listSimple->count() > 0)
        {
            $cmd  = new \RO\Cmd\BriefPendingListRecordTradeCmd();
            $time = microtime(true);
            foreach ($listSimple as $category => $list)
            {
                $cmd->category  = $category;
                $cmd->lists     = [];
                $cmd->pub_lists = [];

                foreach ($list as $jobAndFashion => $listJobAndFashion)
                {
                    $job     = $jobAndFashion >> 8;
                    $fashion = $jobAndFashion & 0xff;
                    $list    = self::getIdsForSimpleList($category, $job, $fashion);

                    $cmd->job       = $job;
                    $cmd->fashion   = $fashion;
                    $cmd->pub_lists = $list[0];
                    $cmd->lists     = $list[1];

                    foreach ($listJobAndFashion as $charId => $obj)
                    {
                        $cmd->charid = $charId;
                        ZoneForwardUser::sendToUserByFd($cmd, $charId, $obj->key, $obj->value);
                    }
                }
            }

            if (($useTime = microtime(true) - $time) > 1)
            {
                Server::$instance->warn("批量输出分类列表到场景服务器耗时较长，用时: " . $useTime);
            }

            if (IS_DEBUG)
            {
                Server::$instance->debug("输出分类列表: " . json_encode($cmd));
            }
        }

        # 处理物品列表
        if ($listItem->count() > 0)
        {
            $cmd              = new \RO\Cmd\DetailPendingListRecordTradeCmd();
            $search           = new \RO\Cmd\SearchCond();
            $cmd->search_cond = $search;
            $cmd->search_cond->page_index = 0;
            $cmd->total_page_count        = 1;

            # 每个分页显示数量
            $countPerPage = 16;
            $time         = microtime(true);

            foreach ($listItem as $itemId => $list)
            {
                $cmd->lists = [];
                $cmd->search_cond->item_id = $itemId;

                foreach ($list as $tradeType => $listRankType)
                {
                    $items = new \Ds\Vector();
                    $items->allocate(128);
                    $boothList = [];
                    $tradeList = [];
                    switch ($tradeType)
                    {
                        case ETradeType::ETRADETYPE_ALL:
                            $boothList = self::getBoothPubList($itemId);
                            $tradeList = self::getItemsForList($itemId);
                            break;
                        case ETradeType::ETRADETYPE_BOOTH:
                            $boothList = self::getBoothPubList($itemId);
                            break;
                        case ETradeType::ETRADETYPE_TRADE:
                            $tradeList = self::getItemsForList($itemId);
                            break;
                    }

                    foreach ($boothList as $info)
                    {
                        $items[] = $info;
                    }

                    foreach ($tradeList as $info)
                    {
                        $items[] = $info;
                    }

                    foreach ($listRankType as $rankType => $listType)
                    {
                        # 所有分页列表
                        $pageList = [[]];
                        $cmd->search_cond->rank_type = $rankType;
                        if (($count = $items->count()) > 0)
                        {
                            self::sortList($items, $rankType);
                            foreach ($items as $i => $val)
                            {
                                /**
                                 * @var TradeItemBaseInfo $val
                                 */
                                $page = intval($i / $countPerPage);
                                $pageList[$page][] = $val;
                            }
                        }
                        $cmd->total_page_count = ceil($count / $countPerPage);

                        foreach ($listType as $charId => $tmp)
                        {
                            list($index, $fd, $fromId) = $tmp;
                            if (isset($pageList[$index]))
                            {
                                $cmd->charid = $charId;
                                $cmd->lists  = $pageList[$index];
                                $cmd->search_cond->page_index = $index;
                                ZoneForwardUser::sendToUserByFd($cmd, $charId, $fd, $fromId);
                            }
                        }
                    }
                }
            }

            if (($useTime = microtime(true) - $time) > 1)
            {
                Server::$instance->warn("批量发送物品列表到场景服务器耗时较长，用时: " . $useTime);
            }
        }
    }

    /**
     * 批量获取物品最低价格
     *
     * @param QueryItemCountTradeCmd $cmd
     * @param $charId
     * @param $fd
     * @param $fromId
     */
    public static function getMinPriceItems(QueryItemCountTradeCmd $cmd, $charId, $fd, $fromId)
    {
        $tradeItemBaseInfoList = [];
        foreach ($cmd->items as $itemCount)
        {
            $items = self::getItemsForList($itemCount->itemid);

            $index = 0;
            while ($index < $items->count())
            {
                # 移除超过精炼+5以上的装备
                /** @var TradeItemBaseInfo $info */
                $info = $items[$index];
                if ($info->refine_lv > 5)
                {
                    $items->remove($index);
                }
                $index++;
            }

            if ($items->isEmpty())
            {
                continue;
            }

            /** @var TradeItemBaseInfo $a */
            /** @var TradeItemBaseInfo $b */
            # 按价格最低排序,公示物品放在最后
            $items->sort(function($a, $b)
            {
                if ($a->publicity_id == 0 && $b->publicity_id > 0)
                {
                    return -1;
                }
                elseif ($a->publicity_id > 0 && $b->publicity_id == 0)
                {
                    return 1;
                }
                elseif ($a->price > $b->price)
                {
                    return 1;
                }
                elseif ($a->price == $b->price)
                {
                    return 0;
                }
                else
                {
                    return -1;
                }
            });

            $count = $itemCount->count;
            do
            {
                /** @var TradeItemBaseInfo $tmpItemList */
                $tmpItemList = $items->first();
                if ($tmpItemList->count > 0)
                {
                    $count -= $tmpItemList->count;
                    $tradeItemBaseInfoList[] = $tmpItemList;
                }

                if ($count > 0)
                {
                    $items->shift();
                    if ($items->isEmpty())
                    {
                        break;
                    }
                }
            }
            while ($count > 0);
        }

        $cmd->res_items = $tradeItemBaseInfoList;
        if (IS_DEBUG)
        {
            Server::$instance->debug("[请求批量获取物品最低价格] 返回baseInfoList:" . json_encode($cmd->res_items));
        }
        ZoneForwardUser::sendToUserByFd($cmd, $charId, $fd, $fromId);
    }

    /**
     * 清除缓存
     */
    public static function cleanCache()
    {
        if (is_object(self::$itemIdList))
        {
            self::$itemIdList->clear();
        }

        if (is_object(self::$simpleList))
        {
            self::$simpleList->clear();
        }

        self::$itemIdList  = null;
        self::$simpleList  = null;
        self::$loadedCount = 0;
    }

    /**
     * @param ItemList $itemList
     * @return TradeItemBaseInfo
     */
    protected static function getListMsg($itemList)
    {
        $info               = new \RO\Cmd\TradeItemBaseInfo();
        $info->price        = $itemList->getPrice();
        $info->count        = $itemList->stock;
        $info->itemid       = $itemList->itemId;
        $info->order_id     = $itemList->id;
        $info->key          = $itemList->itemKey;
        $info->overlap      = $itemList->isOverlap;
        $info->refine_lv    = $itemList->refineLv;

        if ($itemList->isPublicity == 1)
        {
            $info->publicity_id = $itemList->id;
            $info->end_time     = $itemList->getEndTime();
        }

        if ($itemList->itemData)
        {
            $info->item_data = $itemList->getItemData();
            $info->item_data->base->count = $itemList->stock;       # 非堆叠物品强制显示数量
        }

        return $info;
    }

    protected static function sortList(\Ds\Vector $data, $rankType)
    {
        if ($data->count() == 0)return;
        switch ($rankType)
        {
            case \RO\Cmd\RankType::RANKTYPE_ITEM_PRICE_INC:
                $data->sort(function ($a, $b)
                {
                    /**
                     * @var TradeItemBaseInfo $a
                     * @var TradeItemBaseInfo $b
                     */
                    $aPrice = BoothOrder::calcAdjustPrice($a->price, $a->up_rate, $a->down_rate);
                    $bPrice = BoothOrder::calcAdjustPrice($b->price, $b->up_rate, $b->down_rate);
                    if ($a->publicity_id == 0 && $b->publicity_id > 0)
                    {
                        return 1;
                    }
                    elseif ($a->publicity_id > 0 && $b->publicity_id == 0)
                    {
                        return -1;
                    }
                    elseif ($aPrice > $bPrice)
                    {
                        return 1;
                    }
                    elseif ($aPrice == $bPrice)
                    {
                        if ($a->end_time > $b->end_time)
                        {
                            return 1;
                        }
                        return 0;
                    }
                    else
                    {
                        return -1;
                    }
                });
                break;

            case \RO\Cmd\RankType::RANKTYPE_ITEM_PRICE_DES:
                $data->sort(function ($a, $b)
                {
                    /**
                     * @var TradeItemBaseInfo $a
                     * @var TradeItemBaseInfo $b
                     */
                    $aPrice = BoothOrder::calcAdjustPrice($a->price, $a->up_rate, $a->down_rate);
                    $bPrice = BoothOrder::calcAdjustPrice($b->price, $b->up_rate, $b->down_rate);
                    if ($a->publicity_id == 0 && $b->publicity_id > 0)
                    {
                        return 1;
                    }
                    elseif ($a->publicity_id > 0 && $b->publicity_id == 0)
                    {
                        return -1;
                    }
                    elseif ($aPrice < $bPrice)
                    {
                        return 1;
                    }
                    elseif ($aPrice == $bPrice)
                    {
                        if ($a->end_time > $b->end_time)
                        {
                            return 1;
                        }
                        return 0;
                    }
                    else
                    {
                        return -1;
                    }
                });
                break;

            case \RO\Cmd\RankType::RANKTYPE_REFINE_LV_DES:
                $data->sort(function ($a, $b)
                {
                    /**
                     * @var TradeItemBaseInfo $a
                     * @var TradeItemBaseInfo $b
                     */
                    if ($a->publicity_id == 0 && $b->publicity_id > 0)
                    {
                        return 1;
                    }
                    elseif ($a->publicity_id > 0 && $b->publicity_id == 0)
                    {
                        return -1;
                    }
                    elseif ($a->refine_lv < $b->refine_lv)
                    {
                        return 1;
                    }
                    elseif ($a->refine_lv == $b->refine_lv)
                    {
                        if ($a->end_time > $b->end_time)
                        {
                            return 1;
                        }
                        return 0;
                    }

                    else
                    {
                        return -1;
                    }
                });
                break;

            case \RO\Cmd\RankType::RANKTYPE_REFINE_LV_INC:
                $data->sort(function ($a, $b)
                {
                    /**
                     * @var TradeItemBaseInfo $a
                     * @var TradeItemBaseInfo $b
                     */
                    if ($a->publicity_id == 0 && $b->publicity_id > 0)
                    {
                        return 1;
                    }
                    elseif ($a->publicity_id > 0 && $b->publicity_id == 0)
                    {
                        return -1;
                    }
                    elseif ($a->refine_lv > $b->refine_lv)
                    {
                        return 1;
                    }
                    elseif ($a->refine_lv == $b->refine_lv)
                    {
                        if ($a->end_time > $b->end_time)
                        {
                            return 1;
                        }
                        return 0;
                    }
                    else
                    {
                        return -1;
                    }
                });
                break;

            default:
                $data->sort(function ($a, $b)
                {
                    /**
                     * @var TradeItemBaseInfo $a
                     * @var TradeItemBaseInfo $b
                     */
                    if ($a->publicity_id == 0 && $b->publicity_id > 0)
                    {
                        return 1;
                    }
                    elseif ($a->publicity_id > 0 && $b->publicity_id == 0)
                    {
                        return -1;
                    }
                    elseif ($a->end_time > $b->end_time)
                    {
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                });
                break;
        }
    }


    /**
     * 获取一个简单列表
     *
     * @param $category
     * @param $job
     */
    public static function getIdsForSimpleList($category, $job, $fashion)
    {
        if (!isset(self::$simpleList))
        {
            self::reloadItem();
        }

        if (is_array(self::$simpleList))
        {
            Server::$instance->warn("为什么会是个数组?", self::$simpleList);
            self::reloadItem();
        }

        $list1 = new \Ds\Set();
        $list2 = new \Ds\Set();
        $curTime = time();
        foreach (self::$simpleList->get($category, []) as $itemId)
        {
            if ($job > 0 && false === EquipConfig::isJob($itemId, $job))
            {
                continue;
            }

            $arr = Server::$item->get($itemId);

            if ($fashion > 0 && $arr['fashionType'] != $fashion)
            {
                continue;
            }

            if ($arr['isTrade'] == 0)
            {
                continue;
            }

            if ($arr['stock'] > 0)
            {
                $list2->add($itemId);
            }

            if ($arr['lastPubEndTime'] > $curTime)
            {
                $list1->add($itemId);
            }
        }
        $list2 = $list2->diff($list1)->toArray();
        $list1 = $list1->toArray();

        return [$list1, $list2];
    }

    protected static function reloadItem()
    {
        self::$simpleList = new \Ds\Map();
        self::$simpleList->allocate(2048);

        $stock = [];
        foreach (Server::$item as $itemId => $data)
        {
            if (0 == $data['isTrade'])continue;
            $itemId         = (int)$itemId;
            $category       = $data['category'];
            $stock[$itemId] = $data['stock'];

            if (!isset(self::$simpleList[$category]))
            {
                self::$simpleList[$category]   = new \Ds\Vector();
                self::$simpleList[$category]->allocate(256);
            }

            self::$simpleList[$category][] = $itemId;
        }

        # 将库存最低的排前面
        $sort  = function($a, $b) use ($stock)
        {
            if ($stock[$a] == $stock[$b])
            {
                return 0;
            }

            return ($stock[$a] < $stock[$b]) ? -1 : 1;
        };

        foreach (self::$simpleList as $i => $list)
        {
            /**
             * @var \Ds\Vector $list
             */
            # 按在卖的数量排序
            $list->sort($sort);
        }

        if (null === self::$updateTick)
        {
            # 每10分钟重新排序一次
            self::$updateTick = swoole_timer_tick(1000 * 60 * 10, function() {
                $stock = [];
                foreach (Server::$item as $itemId => $data)
                {
                    if (0 == $data['isTrade'])continue;

                    $itemId         = (int)$itemId;
                    $stock[$itemId] = $data['stock'];
                    $category       = $data['category'];
                    if (isset(self::$simpleList) && !isset(self::$simpleList[$category]))
                    {
                        self::$simpleList[$category]   = new \Ds\Vector();
                        self::$simpleList[$category]->allocate(256);
                        self::$simpleList[$category][] = $itemId;
                    }
                }

                $sort  = function($a, $b) use ($stock)
                {
                    if ($stock[$a] == $stock[$b])
                    {
                        return 0;
                    }

                    return ($stock[$a] < $stock[$b]) ? -1 : 1;
                };

                if (isset(self::$simpleList))
                {
                    foreach (self::$simpleList as $i => $list)
                    {
                        /**
                         * @var \Ds\Vector $list
                         */
                        # 按在卖的数量排序
                        $list->sort($sort);
                    }
                }
            });
        }
    }

    /**
     * 获取列表
     *
     * @param $itemId
     * @return \Ds\Vector|null
     */
    public static function getItemsForList($itemId)
    {
        if (self::$loadedCount != Server::$itemList->count() || null === self::$itemIdList)
        {
            self::reloadItemList();
        }

        $t  = microtime(1);
        $rs = new \Ds\Vector();
        $rs->allocate(128);
        if (isset(self::$itemIdList[$itemId]))
        {
            $time = time();
            foreach (self::$itemIdList[$itemId] as $itemKey)
            {
                /**
                 * @var mixed|Dao\ItemList $tmp
                 */
                $tmp = Dao\ItemList::getByKey($itemKey, true);
                if (!$tmp)continue;
                if ($tmp->stock() == 0)continue;

                if ($tmp->isPublicity == 1 && $tmp->getEndTime() <= $time)
                {
                    continue;
                }

                $rs[] = self::getListMsg($tmp);
            }
        }

        Server::$instance->debug("读取列表耗时: ". (microtime(1) - $t). ', 物品数: '. $rs->count());
        return $rs;
    }

    public static function getBoothPubList($itemId)
    {
        $t  = microtime(1);

        $itemId = (int)$itemId;
        $item = Server::$item->get($itemId);
        if (!$item)
            return [];

        $curTime = time();
        if ($item['lastPubEndTime'] <= $curTime)
        {
            return [];
        }

        $rs = new \Ds\Vector();
        $rs->allocate(128);

        $sql = "SELECT * FROM `" . BoothOrder::getTableName() . "` WHERE `item_id` = '{$itemId}' AND `is_publicity` = 1 AND `status` = '" . BoothOrder::STATUS_SELLING . "'";
        $db = Server::$mysqlMaster;
        $rs = $db->query($sql);
        $list = [];
        if ($rs === false)
        {
            return [];
        }
        else if ($rs->num_rows > 0)
        {
            while ($row = $rs->fetch_object())
            {
                if ($row->end_time <= $curTime) {
                    continue;
                }

                $itemInfo               = new TradeItemBaseInfo();
                $itemInfo->order_id     = $row->id;
                $itemInfo->charid       = $row->char_id;
                $itemInfo->name         = $row->player_name;
                $itemInfo->count        = $row->stock;
                $itemInfo->price        = $row->pub_price;
                $itemInfo->itemid       = $row->item_id;
                $itemInfo->up_rate      = $row->up_rate;
                $itemInfo->down_rate    = $row->down_rate;
                $itemInfo->type         = ETradeType::ETRADETYPE_BOOTH;
                $itemInfo->overlap      = $item['isOverlap'] == 1 ? true : false;
                $itemInfo->publicity_id = $row->id;
                $itemInfo->end_time     = $row->end_time;
                $itemData               = $row->item_data === null ? null : new ItemData($row->item_data);
                if ($itemData !== null)
                {
                    $itemInfo->item_data = $itemData;
                    $itemInfo->refine_lv = $itemData->equip->refinelv;
                }

                $list[] = $itemInfo;
            }
        }

        Server::$instance->debug("[摆摊列表] 读取公示列表耗时: ". (microtime(1) - $t) . " SQL:{$sql}");

        return $list;
    }


    protected static function reloadItemList()
    {
        self::$itemIdList = new \Ds\Map();

        $count = Server::$itemList->count();
        foreach (Server::$itemList as $itemKey => $data)
        {
            $itemId = $data['item_id'];

            if (!isset(self::$itemIdList[$itemId]))
            {
                self::$itemIdList[$itemId] = new \Ds\Vector();
                self::$itemIdList[$itemId]->allocate(128);
            }

            self::$itemIdList[$itemId][] = $itemKey;
        }

        self::$loadedCount = $count;
    }
}