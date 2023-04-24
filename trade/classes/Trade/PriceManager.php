<?php
namespace RO\Trade;

use Ds\Set;
use RO\Booth\Dao\BoothRecordSold;
use RO\Cmd\EEnchantType;
use RO\Cmd\EEquipType;
use RO\Cmd\EPriceStatus;
use RO\Cmd\ItemData;
use RO\Cmd\TradeAdjustPriceLogCmd;
use RO\Trade\Dao\ItemInfo;
use RO\Trade\Dao\RecordSold;
use RO\Utils\Common;
use RO\Utils\ROConfig;

/**
 * 价格管理对象
 *
 * @package RO
 */
class PriceManager
{
    private $cycleBegin;

    private $cycleEnd;

    private $cycleKT;

    private $upRate;

    private $downRate;

    private $noDealDropRatio;

    /**
     * @var ExchangeItemNode[]
     */
    private $itemNodes = [];

    private $updateTime = 1000 * 60 * 10;
//    private $updateTime = 1000 * 5;

    protected $updateTimeTick = null;

    // 装备价格缓存key
    const EQUIP_PRICE_KEY = 'p_equip_';

    // 物品价格缓存key
    const ITEM_PRICE_KEY  = 'price';

    function __construct()
    {
    }

    public function init()
    {
        $this->cycleBegin      = (int)Server::$configExchange['Cycle_T'][1] ?: 14400;
        $this->cycleEnd        = (int)Server::$configExchange['Cycle_T'][2] ?: 28800;
        $this->cycleKT         = (int)Server::$configExchange['Cycle_KT'] ?: 86400;
        $this->upRate          = (int)Server::$configExchange['UpRate'] ?: 10;
        $this->downRate        = (int)Server::$configExchange['DownRate'] ?: 50;
        $this->noDealDropRatio = (float)Server::$configExchange['NoDealDropRatio'] ?: 0.05;

        $this->initItem();
    }

    /**
     * 自动重新加载库存
     */
    public function reloadStock()
    {
        $list  = new \SplQueue();
        $total = 0;
        $time  = time();
        foreach (Server::$itemList as $itemKey => $value)
        {
            $list->enqueue($itemKey);
        }

        swoole_timer_tick(1000, function($timerId) use (& $total, & $time, & $list)
        {
            $now = microtime(1);
            while (true)
            {
                if (null === $list || $list->isEmpty())
                {
                    swoole_timer_clear($timerId);

                    Server::$instance->info("[重置库存] 批量更新了 $total 个库存，历时: ". (time() - $time) .'s');

                    # 清除对象
                    $total = $time = $list = null;
                    unset($total, $time, $list);

                    # 增加调价定时器
                    if (!$this->updateTimeTick)$this->updateTimeTick = swoole_timer_tick($this->updateTime, function()
                    {
                        try
                        {
                            $this->updateItemPrice();
                        }
                        catch (\Exception $e)
                        {
                            Server::$instance->warn($e->getFile().'('.$e->getLine().') '.$e->getMessage());
                        }
                    });
                    # 立即执行调价方法
                    $this->updateItemPrice();

                    break;
                }

                $itemKey = $list->dequeue();
                $item    = Dao\ItemList::getByKey($itemKey);
                
                if (false === $item)
                {
                    $list->enqueue($itemKey);
                    Server::$instance->warn("[重置库存] 读取ItemList内存表失败, itemKey: {$itemKey}");
                }
                elseif ($item->isResetStock == 0)
                {
                    # 充值库存
                    $stock = $item->stock;
                    if ($item->resetStockFromDB())
                    {
                        $total++;
                        if (IS_DEBUG)Server::$instance->debug("[自动重置库存] itemKey: {$itemKey}, itemListId: {$item->id} from: {$stock}, to {$item->stock}");
                    }
                    else
                    {
                        # 更新失败重新加入队列
                        $list->enqueue($itemKey);
                    }
                }

                if (microtime(1) - $now > 0.01)
                {
                    break;
                }
            }
        });
    }


    /**
     * 初始化物品
     *
     * 加载物品内存数据, 并构造物品价格依赖关系图的数据结构
     * 完成构造后, 全部更新一次价格。
     * 1. 保证有部分物品依赖的物品不上交易所但需要它的价格得到正确的初始化
     * 2. 保证策划表更新后,能重新加载策划表的信息
     * 3. 出现回路的物品,从内存中删除,避免错误价格购买
     * 4. 检测策划表的配置是否正确,发现错误写日志,并从内存中删除
     */
    public function initItem()
    {
        $exchangeConfig        = Server::loadConfig('Exchange');
        $composeConfig         = Server::loadConfig('Compose');
        $equipConfig           = Server::loadConfig('Equip');
        $equipUpgradeConfig    = Server::loadConfig('EquipUpgrade');
        $equipComposeConfig    = Server::loadConfig('EquipCompose');
        $this->itemNodes = [];

        $buildItemNode = function($val) use ($exchangeConfig, $composeConfig,
                $equipConfig, $equipUpgradeConfig, $equipComposeConfig, &$buildItemNode)
        {
            $itemId = (int)$val['id'];
            if (isset($this->itemNodes[$itemId])) {
                return $this->itemNodes[$itemId];
            }

            $itemNode                  = new ExchangeItemNode();
            $itemNode->id              = $itemId;
            $itemNode->isTrade         = (int)$val['Trade'];
            $item                      = Item::get($itemNode->id);
            $itemNode->realPrice       = $item ? $item->getPrice() : 0;
            $itemNode->cycle           = $item ? $item->cycle : 0;
            $itemNode->priceUpdateTime = $item ? $item->priceUpdateTime : 0;
            $itemNode->isEquip         = isset($equipConfig[$itemNode->id]) ? 1 : 0;
            $itemNode->setPrice($val['Price']);
            $itemNode->setMinPrice($val['MinPrice']);
            $this->itemNodes[$itemNode->id] = $itemNode;


            $node = [];
            if ($itemNode->price['type'] == ExchangeItemNode::PRICE_TYPE_SUM)
            {
                $node = $itemNode->price['pricesum'];
            }
            else if (!empty($itemNode->minPrice)
                && $itemNode->minPrice['type'] != ExchangeItemNode::MIN_PRICE_TYPE_SELF)
            {
                if ($itemNode->minPrice['type'] == ExchangeItemNode::MIN_PRICE_TYPE_EQUIP_UPGRADE)
                {
                    // 升级装备
                    $equipUpgradeId = isset($itemNode->minPrice['equip_upgrade_id']) ? $itemNode->minPrice['equip_upgrade_id'] : 0;
                    if (empty($equipUpgradeConfig[$equipUpgradeId]))
                    {
                        throw new \Exception('[交易所-初始物品价格] [物品配置-EquipUpgrade.json] 策划表中找不到升级装备配置ID:' . $equipUpgradeId);
                    }

                    $equipUpgrade = $equipUpgradeConfig[$equipUpgradeId];
                    foreach ($equipUpgrade as $key => $configData)
                    {
                        if ($mLv = strstr($key, 'Material_'))
                        {
                            if (empty($equipUpgrade[$key]))
                                continue;

                            foreach ($equipUpgrade[$key] as $data)
                            {
                                $node[] = [
                                    'itemid' => $data['id'],
                                    'num' => $data['num']
                                ];
                            }
                        }
                    }

                    // 材料
                    $itemNode->minPrice['material'] = $node;

                    // 原装备
                    $itemNode->minPrice['equipId'] = $equipUpgradeId;
                    $node[]                        = [
                        'itemid' => $equipUpgradeId,
                        'num' => 1
                    ];
                }
                else if ($itemNode->minPrice['type'] == ExchangeItemNode::MIN_PRICE_TYPE_EQUIP_NEW_COMPOSE)
                {
                    // 装备合成
                    $equipComposeId         = $itemNode->minPrice['equipcomposeid'] ?? 0;
                    $equipComposeConfigItem = $equipComposeConfig[$equipComposeId] ?? null;
                    if (!$equipComposeConfigItem)
                    {
                        throw new \Exception('[交易所-初始物品价格] EquipCompose配置表中找不到装备配置, ID:' . $equipComposeId);
                    }

                    if (!isset($equipComposeConfigItem['Material']))
                    {
                        throw new \Exception('[交易所-初始物品价格] EquipCompose配置表中找不到Material配置, ID:' . $equipComposeId);
                    }

                    foreach ($equipComposeConfigItem['Material'] as $data)
                    {
                        $node[] = [
                            'itemid' => $data['id'],
                            'lv'     => $data['lv'],
                            'num'    => 1
                        ];
                    }

                    foreach ($equipComposeConfigItem['MaterialCost'] as $data)
                    {
                        $node[] = [
                            'itemid' => $data['id'],
                            'num' => $data['num']
                        ];
                    }

                    $itemNode->minPrice['rob']      = $equipComposeConfig['Cost'] ?? 0;
                    $itemNode->minPrice['material'] = $node;
                }
                else
                {
                    // 图纸合成和装备制作
                    $composeId = isset($itemNode->minPrice['composeid']) ? $itemNode->minPrice['composeid'] : 0;
                    if (!isset($composeConfig[$composeId]))
                    {
                        throw new \Exception('[交易所-初始物品价格] [物品配置-Compose.json] 策划表中找不到组合ID:' . $composeId);
                    }

                    $composeItem = $composeConfig[$composeId];
                    $node        = [];
                    foreach ($composeItem['BeCostItem'] ?: [] as $n)
                    {
                        $node[] = [
                            'itemid' => $n['id'],
                            'num' => $n['num']
                        ];
                    }
                    $itemNode->minPrice['rob']      = $composeItem['ROB'] ?? 0;
                    $itemNode->minPrice['material'] = $node;
                }
            }

            if (!empty($node))
            {
                foreach ($node as $child)
                {
                    if (!isset($child['itemid']))
                    {
                        throw new \Exception('[交易所-初始物品价格] 组合物品: ' . $itemNode->id . ' 组合价格无对应的item id');
                    }

                    $id        = $child['itemid'];
                    $childNode = null;
                    if (isset($this->itemNodes[$id]))
                    {
                        $childNode               = $this->itemNodes[$id];
                    }
                    else
                    {
                        if (!isset($exchangeConfig[$id]))
                        {
                            throw new \Exception('[交易所-初始物品价格] [物品配置-Exchange.json] 策划表中物品:' . $itemId . ', 找不到依赖价格物品, ID:' . $id);
                        }

                        $childConfigItem = $exchangeConfig[$id];
                        $childNode = $buildItemNode($childConfigItem);
                    }

                    if ($childNode){
                        $itemNode->children[$id] = $childNode;
                        $childNode->parents[$itemNode->id] = $itemNode;
                    }
                }
            }

            return $itemNode;
        };

        try
        {
            //构造物品价格依赖关系图的数据结构, 以便递归更新价格
            foreach ($exchangeConfig as $itemId => $val)
            {
                $buildItemNode($val);
            }

            $rs   = [];
            foreach ($this->itemNodes as $id => $node)
            {
                if (!$node)
                {
                    continue;
                }

                // 检查回路会,若出现会导致价格计算死循环,为避免回路,需要检查每个节点
                if ($this->checkLoop($node) === false)
                {
                    throw new \Exception("[交易所-初始物品价格] 发现物品id {$node->id}出现回路,请检查策划表配置是否有误!");
                }
                else
                {
                    $rs = array_merge($rs, $node->updatePrice());
                }
            }

            // 初始化时全部更新一次价格。
            $this->updateItemsPrice(array_unique($rs));
        }
        catch (\Exception $e)
        {
            Server::$instance->warn($e->getMessage());
            Server::$instance->server->shutdown();
        }
    }

    /**
     * 直接修改价格
     *
     * @param int $itemId
     * @param int $price
     * @throws \Exception
     */
    public function setPrice($itemId, $price)
    {
        if (!is_int($itemId) || !is_int($price))
        {
            Server::$instance->warn('[设置价格] 非法参数');
            return;
        }

        if (!isset($this->itemNodes[$itemId]))
        {
            Server::$instance->warn("[设置价格] id: {$itemId}物品不存在");
            return;
        }

        $node            = $this->itemNodes[$itemId];
        if ($node->price['type'] == ExchangeItemNode::PRICE_TYPE_SUM)
            return;

        $node->realPrice = $price;
        $node->cycle     = Common::randBetween($this->cycleBegin, $this->cycleEnd);
        $ids             = $node->updatePrice();
        $this->updateItemsPrice($ids);

        Server::$instance->info("[设置价格] 设置成功, id: {$itemId}, price:{$price}");
    }

    /**
     * 一个专门的进程每隔一段事件来更新下部分物品的价格的方法
     */
    public function updateItemPrice()
    {
        $itemSet     = [];
        /** @var ExchangeItemNode[] $changedItem */
        $changedItem = [];

        foreach (Server::$item as $itemId => $val)
        {
            # 有物品正在公示不进行定时调价
            if (isset($val['publicityNum']) && $val['publicityNum'] > 0)
            {
                continue;
            }

            if (!isset($this->itemNodes[$itemId]))
            {
                continue;
            }

            $item     = $this->itemNodes[$itemId];
            $isChange = false;
            if ($item->isTrade)
            {
                $stock = $val['stock'] + $val['boothStock'];
                $isChange = $this->adjustPrice($item, $stock, $val['maxPrice'], $val['upRatio'], $val['downRatio']);
            }

            if ($isChange)
            {
                $changedItem[$itemId] = $item;
            }
        }

        if (!empty($changedItem))
        {
            foreach ($changedItem as $itemId => $val)
            {
                $itemSet = array_merge($itemSet, $val->updatePrice());
            }

            $itemSet = array_unique($itemSet);
            $this->updateItemsPrice($itemSet);
        }
    }

    /**
     * 立即调价
     * 只能在PriceManager进程执行
     *
     * @param $itemId
     */
    public function instantAdjustPrice($itemId)
    {
        $item = Server::$item->get($itemId);
        if (!$item)
        {
            return;
        }

        if (!isset($this->itemNodes[$itemId]))
        {
            return;
        }

        $itemNode = $this->itemNodes[$itemId];
        $stock = $item['stock'] + $item['boothStock'];
        $isChange = $this->adjustPrice($itemNode, $stock, $item['maxPrice'], $item['upRatio'], $item['downRatio']);
        if ($isChange)
        {
            $itemSet = $itemNode->updatePrice();
            $this->updateItemsPrice($itemSet);
        }
    }

    /**
     * 批量更新物品价格
     * 更新的物品是装备需要删除redis中的缓存,避免价格不是最新的
     *
     * @param $itemIds
     */
    private function updateItemsPrice($itemIds)
    {
        foreach ($itemIds as $id)
        {
            if (isset($this->itemNodes[$id]))
            {
                $item = $this->itemNodes[$id];
                if (!$item->isTrade)
                {
                    continue;
                }

                if ($this->changePrice($id, $item->realPrice, $item->priceUpdateTime, $item->cycle))
                {
                    if ($item->isEquip)
                    {
                        try
                        {
                            Server::$redis->del(self::EQUIP_PRICE_KEY . $id);
                        }
                        catch (\Exception $e)
                        {
                            Server::$instance->warn("[价格管理-批量更新价格] 物品id:{$id} 缓存清理失败 错误信息:{$e->getMessage()}");
                        }
                    }
                }
                else
                {
                    Server::$instance->warn("[价格管理-批量更新价格] 物品id:{$id}的价格更新失败");
                }
            }
            else
            {
                if (IS_DEBUG)
                {
                    Server::$instance->warn("[价格管理-批量更新价格] itemNodes不存在该物品id:{$id}");
                }
            }
        }

        if (IS_DEBUG)
        {
//            Server::$instance->debug("[价格管理-批量更新价格] 批量更新完毕, 物品id:", $itemIds);
        }
    }

    /**
     * 物品调价
     *
     * @param ExchangeItemNode $item
     * @param int $stock 物品库存
     * @param int $maxPrice 最高价格, 0和负数代表不设置
     * @param float $upRatio 涨幅系数
     * @param float $downRatio 跌幅系数
     * @return bool
     */
    private function adjustPrice(ExchangeItemNode $item, $stock = 0, $maxPrice = 0, $upRatio = 1.0, $downRatio = 1.0)
    {
        if ($stock < 0)
        {
            Server::$instance->warn("[交易所-价格调整] 库存为负数 物品id:{$item->id}");
            return false;
        }

        if ($item->price['type'] == ExchangeItemNode::PRICE_TYPE_SUM || $item->price['type'] == ExchangeItemNode::PRICE_TYPE_MIN)
        {
            return false;
        }

        $currentTime = time();

        if ($item->priceUpdateTime && $currentTime < ($item->priceUpdateTime + $item->cycle))
        {
//            Server::$instance->info('[价格管理-' . __METHOD__ . "] 物品ID:{$item->id} 当前时间:{$currentTime} 更新时间:" . ($item->priceUpdateTime + $item->cycle));
            return false;
        }

        $priceChanged = false;

        if ($item->cycle == 0)
        {
            $item->cycle = Common::randBetween($this->cycleBegin, $this->cycleEnd);
        }

        if ($item->priceUpdateTime == 0)
        {
            $item->priceUpdateTime = $currentTime - $item->cycle;
        }

        // 期望库存周期
        $KT   = $this->cycleKT;
        $up   = $this->upRate;
        $down = $this->downRate;

        $newPrice = $oldPrice = $item->realPrice;

        # 当前库存量
        $K = $stock;
        # 本期成交量
        $D = RecordSold::getSoldCount($item->id, $item->priceUpdateTime) + BoothRecordSold::getSoldCount($item->id, $item->priceUpdateTime);
//        $K = mt_rand(1, 100);
//        $D = mt_rand(1, 100);

        # 期望库存量
        $QK = 0.0;
        $R  = 0;

        # 调价状态
        $status = EPriceStatus::EPriceStatus_NoChange;

        do
        {
            if ($maxPrice > 0 && $oldPrice > $maxPrice)
            {
                # 当前价格比最高价格高,则跌10%
                $newPrice = (int)ceil($oldPrice * (1 - 0.1));
                $status = EPriceStatus::EPriceStatus_Premium;
                $R = -0.1;
                break;
            }

            if ($D === false)
            {
                $priceChanged = false;
                break;
            }

            //无任何成交和挂单则不用改变价格
            if ($D == 0 && $K == 0)
            {
                $priceChanged = false;
                break;
            }

            if ($D == 0 && $K > 0)
            {
                $isUp = false;
                $R    = $this->noDealDropRatio;
            }
            else
            {
                $a  = $item->cycle / $KT;
                $QK = round($D / $a, 4);

                if ($QK <= 0)
                {
                    Server::$instance->warn(sprintf("[交易所-价格调整] QK少于等于0 物品ID:%s, 周期cycle:%s, 库存K:%s, 成交量D:%s, KT:%s, QK:%s, a:%s",
                                                     $item->id, $item->cycle, $K, $D, $KT, $QK, $a));
                    $priceChanged = false;
                    break;
                }

                if ($K < $QK)
                {
                    $isUp = true;
                    $R    = $K / $QK;
                    $R    = (1 - $R) / $up;
                    $R    = $R * $upRatio;
                    $R    = round($R > 0 ? min(0.1, $R) : max(-0.1, $R), 4);
                }
                else
                {
                    $isUp = false;
                    $R    = $K / $QK;
                    $R    = ($R - 1) / $down;
                    $R    = $R * $downRatio;
                    $R    = round($R > 0 ? min(0.1, $R) : max(-0.1, $R), 4);
                }
            }

            if ($isUp)
            {
                $newPrice = (int)ceil($oldPrice * (1 + $R));
            }
            else
            {
                $newPrice = (int)ceil($oldPrice * (1 - $R));
                $R = -$R;
            }

            if ($maxPrice > 0)
            {
                # 若新价格比最高价格大,则取最高价格,反之亦然
                $newPrice = min($maxPrice, $newPrice);
            }

            if ($status === EPriceStatus::EPriceStatus_NoChange)
            {
                if ($newPrice === $maxPrice)
                    $status = EPriceStatus::EPriceStatus_MaxPrice;
                else
                    $status = $isUp ? EPriceStatus::EPriceStatus_Up : EPriceStatus::EPriceStatus_Down;
            }
        }
        while (0);

        if ($oldPrice != $newPrice)
        {
            $minPrice = $item->getMinPrice();
            // 若新价格比物品最低价格还低,则价格不变。
            if ($newPrice > $minPrice)
            {
                $item->realPrice = $newPrice;
                $priceChanged    = true;
            }
            else
            {
                // 如果原价格已经等于最低价格了,则价格不需要改变
                if ($oldPrice == $minPrice)
                {
                    $priceChanged = false;
                }
                else
                {
                    $item->realPrice = $minPrice;
                    $priceChanged    = true;
                }

                $status = EPriceStatus::EPriceStatus_Bottom;
            }
        }

        if (IS_DEBUG)
        {
            switch ($status)
            {
                case EPriceStatus::EPriceStatus_Up:
                    $statusLabel = '上涨';
                    break;
                case EPriceStatus::EPriceStatus_Down:
                    $statusLabel = '下跌';
                    break;
                case EPriceStatus::EPriceStatus_Bottom:
                    $statusLabel = '最低价';
                    break;
                case EPriceStatus::EPriceStatus_Premium:
                    $statusLabel = '溢价回落';
                    break;
                case EPriceStatus::EPriceStatus_MaxPrice:
                    $statusLabel = '最高价';
                    break;
                default:
                    $statusLabel = '无变化';
                    break;
            }

            $log = sprintf("[交易所-价格调整] 物品ID:%s, 周期T:%s, 库存K:%s, 成交量D:%s, 期望库存周期KT:%s, 期望库存量QK:%s, 涨幅R:%s, 涨幅系数:%s, 跌幅系数:%s, 上次价格:%s, 新价格:%s, 实际价格:%s, 涨跌情况:%s, 上次调价时间:%s",
                             $item->id,
                             $item->cycle,
                             $K,
                             $D,
                             $KT,
                             $QK,
                             $R,
                             $upRatio,
                             $downRatio,
                             $oldPrice,
                             $newPrice,
                             $item->realPrice,
                             $statusLabel,
                             date('Y-m-d H:i:s', $item->priceUpdateTime));
            Server::$instance->debug($log);
        }

        // 记录调价日志
        // $log           = new PriceAdjustLog();
        // $log->itemId   = $item->id;
        // $log->lastTime = $currentTime;
        // $log->t        = $item->cycle;
        // $log->d        = $D;
        // $log->k        = $K;
        // $log->qk       = $QK;
        // $log->rawPrice = $item->getRawPrice();
        // $log->oldPrice = $oldPrice;
        // $log->newPrice = $newPrice;
        // $log->insert();

        # 推送调价日志
        $cmd = [
            'item_id'    => (int) $item->id,
            't'          => (int) $item->cycle,
            'k'          => (int) $K,
            'sold_count' => (int) $D,
            'kt'         => (int) $KT,
            'qk'         => (float) $QK,
            'r'          => (float) $R,
            'up_ratio'   => (float) $upRatio,
            'down_ratio' => (float) $downRatio,
            'old_price'  => (int) $oldPrice,
            'new_price'  => (int) $newPrice,
            'real_price' => (int) $item->realPrice,
            'status'     => $status,
            'logid'      => "adjust-price-{$item->id}-{$currentTime}",
            'last_time'  => (int) $item->priceUpdateTime,
            'time'       => $currentTime,
        ];
        ZoneForwardScene::pushToAdjustFluent($cmd);

        $item->cycle = Common::randBetween($this->cycleBegin, $this->cycleEnd);
        $item->priceUpdateTime = $currentTime;

        // 价格无发生改变, 需要记录下一轮调价周期
        if ($priceChanged === false)
        {
            $itemInfo = new ItemInfo();
            $itemInfo->itemId            = $item->id;
            $itemInfo->lastCalcPriceTime = $item->priceUpdateTime;
            $itemInfo->cycle             = $item->cycle;
            $rs = $itemInfo->update();
            if ($rs)
            {
                Server::$item->set($item->id, [
                    'priceUpdateTime' => $item->priceUpdateTime,
                    'cycle'           => $item->cycle,
                ]);
            }
            else
            {
                Server::$instance->warn("[交易所-价格调整] 更新调价周期失败");
            }
        }

        return $priceChanged;
    }


    /**
     * 获取价格
     * 无精炼附魔物品直接从内存中获取价格, 若含精炼附魔则优先从redis获取, 取不到则计算价格,并写入缓存。
     * 装备依赖的材料价格发生变化,redis缓存会被清除
     *
     * @param Item $item
     * @return int
     */
    public static function getServerItemPrice($item)
    {
        if (!$item) return 0;

        do
        {
            $price = $item->price ?: 0;
            $itemData = $item->itemData;

            if ($itemData === null || !$itemData->hasBase() || $item->equipType === 0)
            {
                break;
            }

            // 对精炼或附魔装备尝试从缓存中取装备价格
            if ($itemData->equip->refinelv > 0 || ($itemData->hasEnchant() && !empty($itemData->enchant->attrs)) || $itemData->equip->lv > 0)
            {
                try
                {
                    $key = self::generateHash($itemData);

                    try
                    {
                        $cachePrice = Server::$redis->hGet(self::EQUIP_PRICE_KEY . $item->id, $key);
                    }
                    catch (\Exception $e)
                    {
                        Server::$instance->warn("[获取装备价格缓存] 发生异常, 错误信息: {$e->getMessage()}");
                        $cachePrice = false;
                    }

                    if ($cachePrice !== false)
                    {
                        $price = (int)$cachePrice;
                        break;
                    }

                    $refinePrice  = self::calcItemRefinePrice($item);
                    $enchantPrice = self::calcEnchantPrice($itemData);

                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[交易所-价格计算] 装备基础价格:{$price} 精炼价格:{$refinePrice} 附魔价格: {$enchantPrice}");
                    }

                    $price += $refinePrice + $enchantPrice;
                    Server::$redis->hSet(self::EQUIP_PRICE_KEY . $item->id, $key, $price);
                    Server::$redis->expire(self::EQUIP_PRICE_KEY . $item->id, 3600);
                }
                catch (\Exception $e)
                {
                    Server::$instance->warn("[交易所-价格获取] 装备ID:{$item->id} 发生异常 错误信息: {$e->getMessage()}");
                    break;
                }
            }
        } while (0);

        $maxPrice = 4000000000;
        if ($price > $maxPrice)
            $price = $maxPrice;

        return $price;
    }

    /**
     * 生成唯一hash
     *
     * @param ItemData $itemData
     * @param bool $isGoodEnchant
     * @return string
     */
    public static function generateHash(ItemData $itemData, $isGoodEnchant = false)
    {
        return md5(self::generateKey($itemData, $isGoodEnchant));
    }

    /**
     * 生成唯一字符串
     *
     * @param ItemData $itemData
     * @param bool $isGoodEnchant
     * @return string
     */
    public static function generateKey(ItemData $itemData, $isGoodEnchant = false)
    {
        $key = "itemid:{$itemData->base->id}refinelv:{$itemData->equip->refinelv}damage:{$itemData->equip->damage}";

        if (true === $isGoodEnchant || ROConfig::isGoodEnchant($itemData))
        {
            if (!empty($itemData->enchant->attrs))
            {
                $key .= 'enchant:';
                $map = [];
                foreach ($itemData->enchant->attrs as $attr)
                {
                    $map[$attr->getType()] = $attr->getValue();
                }

                foreach ($map as $type => $value)
                {
                    $key .= "{$type},{$value}";
                }
            }

            if (!empty($itemData->enchant->extras))
            {
                $key .= "extras:";
                $set = new Set();
                foreach ($itemData->enchant->extras as $extra)
                {
                    $set->add($extra->buffid);
                }
                $key .= implode(',', $set->toArray());
            }
        }

        if ($itemData->equip->lv > 0)
        {
            $key .= 'lv:' . $itemData->equip->lv;
        }

        return $key;
    }

    /**
     * 计算附魔属性
     *
     * @param ItemData $itemData
     * @return int
     */
    public static function calcEnchantPrice(ItemData $itemData)
    {
        $price = 0;
        do
        {
            if (ROConfig::isGoodEnchant($itemData) === false)
            {
                break;
            }

            $itemId = $itemData->base->id;
            $item = Server::$item->get($itemId);
            if (!$item)
            {
                break;
            }

            $enchantCfg = EnchantConfig::getEnchantConfig(EEnchantType::EENCHANTTYPE_SENIOR);
            if (!$enchantCfg)
            {
                break;
            }

            $enchantEquip = $enchantCfg->getEnchantEquipByType($item['itemType']);
            if ($enchantEquip === null)
            {
                break;
            }

            $enchantData = $itemData->enchant;

            $all = $enchantEquip->maxWeight;
            $rate = 1.0;
            $ret = 1.0; //平均附魔次数

            foreach ($enchantData->attrs as $attr)
            {
                $type = $attr->type;
                //$value = (float)$attr->value;

                $equipItem = $enchantEquip->getEnchantEquipItem($type);
                if ($equipItem === null)
                {
                    continue;
                }

                $baseRate = $equipItem->p->value == 0 ? 100 : $equipItem->p->value;

                $attrCfg = $enchantCfg->getEnchantAttrByAttrTypeAndItemType($type, $item['itemType']);
                if ($attrCfg === null)
                {
                    continue;
                }

                // 计算点赞概率,点赞概率=单项属性的后两个权重值之和/该属性的总权重值
                $f1 = $attrCfg->getRandomRate();

                // 附魔属性概率,该属性在该装备上的权重值/该装备的权重值之和
                $f2 = 0;
                if ($all > 0)
                {
                    $f2 = $baseRate / $all;
                }

                if ($all > $baseRate)
                {
                    $all -= $baseRate;
                }

                // 属性必定为赞的概率 = 点赞概率(f1) * 附魔属性概率(f2)
                $f3 = (1 - $f1 * $f2);
                $ret *= $f3;

                if ($equipItem->p->value)
                {
                    $rate *= EnchantPriceConfig::getEnchantPriceRate($type, $itemId);
                }
            }

            // 平均附魔次数=1/(1-(1-属性1必定为赞概率)*(1-属性2必定为赞概率)*(1-属性3必定为赞概率))
            $ret = 1 - $ret;
            if ($ret > 0)
            {
                $ret = floor(1 / $ret);
            }
            else
            {
                $ret = 1;
            }

            $tradeConfig = Server::$configExchange;

            $extraCount = 1;
            foreach ($itemData->enchant->extras as $extra)
            {
                $enchantCfg = EnchantConfig::getEnchantConfig($itemData->enchant->type);
                if ($enchantCfg === null)
                    break;

                $attr = $enchantCfg->getEnchantAttrById($extra->configid);
                if ($attr === null)
                {
                    break;
                }

                $level = 1;
                foreach ($attr->extraBuff as $buff)
                {
                    if ($buff->key == $extra->buffid)
                        break;

                    $level++;
                }

                if ($level <= count($tradeConfig['LvRate']))
                {
                    $extraCount *= $tradeConfig['LvRate'][$level];
                }
            }

            // 莫拉币价格
            $mobPrice = (int) $tradeConfig['MobPrice'] ?: 0;
            // 通货膨胀系数
            $inflation = (float) $tradeConfig['Inflation'] ?: 0.0;
            $typeRate = isset($tradeConfig['TypeRate'][$itemData->enchant->type]) ? $tradeConfig['TypeRate'][$itemData->enchant->type] : 0;
            $headRatio = $tradeConfig['HeadRatio'][$item['itemType']] ?? 1;
            $price = intval($ret * $extraCount * $mobPrice * $inflation * $rate * $typeRate * $headRatio);
        } while(0);

        return $price;
    }

    /**
     * 计算装备物品精炼价格
     *
     * @param Item $item
     * @return int
     */
    public static function calcItemRefinePrice(Item $item)
    {
        if ($item->minPriceType === ExchangeItemNode::MIN_PRICE_TYPE_EQUIP_UPGRADE)
        {
            return self::calcUpgradeRefinePrice($item);
        }
        else if ($item->minPriceType === ExchangeItemNode::MIN_PRICE_TYPE_EQUIP_NEW_COMPOSE)
        {
            $mainEquipId = $item->mainEquipId;
            $item = Item::get($mainEquipId, $item->itemData);
            return self::calcItemRefinePrice($item);
        }
        else
        {
            return self::calcRefinePrice($item);
        }
    }

    /**
     * 计算精炼价格
     *
     * @param Item $item
     * @param int $refineLv 不设则读Item对象的itemData
     * @param bool $damage 强制计算是否损坏
     * @return int
     */
    public static function calcRefinePrice(Item $item, $refineLv = null, $damage = null)
    {
        $itemData = $item->itemData;
        $itemId   = $item->id;
        $price    = 0;

        $damage   = $damage === null ? ($itemData ? $itemData->equip->damage : false) : $damage;
        $refineLv = $refineLv === null ? ($itemData ? $itemData->equip->refinelv : 0) : $refineLv;

        do
        {
            if ($refineLv > 0)
            {
                if ($item->equipType <= EEquipType::EEQUIPTYPE_MIN || $item->equipType >= EEquipType::EEQUIPTYPE_MAX)
                {
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[交易所-精炼计算] 计算精炼价格失败, 装备类型错误 物品ID:{$itemId} equip_type:{$item->equipType}");
                    }
                    break;
                }

                $refineTradeConfig = RefineTradeConfig::getRefineTrade($item->itemType, $refineLv);
                if ($refineTradeConfig === null)
                {
                    if (IS_DEBUG)
                    {
                        Server::$instance->debug("[交易所-精炼计算] 获取refineTradeConfig失败 item_type:{$item->itemType} refineLv:{$refineLv}");
                    }
                    break;
                }

                // 精炼材料的id
                $otherItemId = $refineTradeConfig->itemId;
                $lowItemId   = EquipConfig::getLowVidItemId($itemId);

                // 不是打洞装备
                if ($lowItemId == 0)
                {
                    $lowItemId = $itemId;
                    $lowItem   = Item::get($lowItemId);
                }
                else
                {
                    $lowItem = Item::get($lowItemId);
                    if ($lowItem)
                    {
                        // 如果是升级装备, 则按低洞升级装备精炼价格计算
                        if ($lowItem->equipUpgradeId)
                        {
                            $lowItem->itemData = $item->itemData;
                            return self::calcUpgradeRefinePrice($lowItem);
                        }
                    }
                    else
                    {
                        // 兼容头饰精炼1洞找不到低洞装备的价格, 使用自身作为低洞装备参与计算
                        $lowItemId = $itemId;
                        $lowItem   = Item::get($itemId);
                    }
                }

                $otherItem  = Item::get($otherItemId);
                $lowPrice   = self::getServerItemPrice($lowItem);
                $otherPrice = self::getServerItemPrice($otherItem);
                $price  = $lowPrice * $refineTradeConfig->equipRate;
                $price += $otherPrice * $refineTradeConfig->itemRate;
                $price += $refineTradeConfig->lvRate;
                $price = intval($price * $refineTradeConfig->riskRate);
                if ($itemData && $damage)
                {
                    $price -= $lowPrice;
                }

//                if ($item->refineCycle == 0)
//                {
//                    if (IS_DEBUG) {
//                        Server::$instance->debug("[交易所-精炼计算] 不计算planB价格 物品:{$itemId}, 精炼等级:{$refineLv}, planA价格:{$price}, lowItemId: {$lowItemId}, lowPrice: {$lowPrice}, otherPrice:{$otherPrice}");
//                    }
//                    break;
//                }

                $priceB = $lowPrice * $refineTradeConfig->equipRateB;
                $priceB += $otherPrice * $refineTradeConfig->itemRateB;
                $priceB += $refineTradeConfig->lvRateB;
                $priceB = intval($priceB * $refineTradeConfig->riskRate);
                if ($itemData && $damage)
                {
                    $priceB -= $lowPrice;
                }

//                 不算planA价格了
//                 $pricePlanRefineRate = $item->refineCycle == 1 ? isset(Server::$configExchange['WeekRefineRate']) ? Server::$configExchange['WeekRefineRate'] : 0
//                    : isset(Server::$configExchange['MonthRefineRate']) ? Server::$configExchange['MonthRefineRate'] : 0;
                $pricePlanRefineRate = 1000;

                if (IS_DEBUG)
                {
                    Server::$instance->debug("[交易所-精炼计算] 物品:{$itemId}, 精炼等级:{$refineLv}, planA价格:{$price}, planB价格:{$priceB}, refineRate: {$pricePlanRefineRate}, lowItemId: {$lowItemId}, lowPrice: {$lowPrice}, otherPrice:{$otherPrice}");
                }

                $price = intval($price + ($priceB - $price) * ($pricePlanRefineRate / 1000));
            }
        } while (0);

        return $price;
    }

    /**
     * 计算升级装备精炼价格
     *
     * @param Item $itemB
     * @return int
     */
    public static function calcUpgradeRefinePrice(Item $itemB)
    {
        $price = 0;
        do
        {
            if (!isset($itemB->itemData) && $itemB->itemData->equip->refinelv <= 0)
            {
                break;
            }

            $equipUpgradeId = $itemB->equipUpgradeId;
            $itemA = Item::get($equipUpgradeId);
            if (!$itemA)
            {
                Server::$instance->warn("[交易所-精炼计算] 计算精炼升级装备发现升级装备id不存在 id:{$equipUpgradeId}");
                break;
            }

            $equipConf = EquipConfig::getEquip($equipUpgradeId);
            if (!$equipConf)
            {
                Server::$instance->warn("[交易所-精炼计算] 计算精炼升级装备发现升级装备id:{$equipUpgradeId}在配置Equip表中不存在");
                break;
            }

            $m = $itemB->itemData->equip->refinelv;
            $n = $equipConf->maxRefineLv;

            # 记录原装备是否破损,计算精炼价格时先当不破损计算,后面再扣掉
            $isDamageItemB = $itemB->itemData->equip->damage;

            # (m)装备B精炼价格
            $mPriceB = self::calcRefinePrice($itemB, null, false);
            $priceB = 0;

            if ($m + 2 <= $n)
            {
                # (m + 2)装备A精炼价格
                $m2PriceA = self::calcRefinePrice($itemA, $m + 2);
                $priceB = min($mPriceB, $m2PriceA);
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[交易所-精炼升级装备计算] (m + 2)装备A精炼价格: {$m2PriceA}");
                }
            }
            else if ($m + 2 > $n)
            {
                # (n - 2)装备B精炼价格
                $n2PriceB = self::calcRefinePrice($itemB, $n - 2, false);
                # (n)装备A精炼价格
                $nPriceA  = self::calcRefinePrice($itemA, $n);
                $priceB = min($n2PriceB, $nPriceA) + $mPriceB - $n2PriceB;
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[交易所-精炼升级装备计算] (n - 2)装备B精炼价格: {$n2PriceB}, (n)装备A精炼价格: {$nPriceA}");
                }
            }

            $equipUpRate = isset(Server::$configExchange['EquipUpRate']) ? Server::$configExchange['EquipUpRate'] : 0;

            $price = intval($mPriceB + ($priceB - $mPriceB) * ($equipUpRate / 1000));
            if ($isDamageItemB)
            {
                $lowItemId    = EquipConfig::getLowVidItemId($itemB->id);
                $lowItemId    = $lowItemId === 0 ? $itemB->id : $lowItemId;
                $lowItem      = Item::get($lowItemId);
                $lowItemPrice = self::getServerItemPrice($lowItem);
                $price -= $lowItemPrice;
            }

            if (IS_DEBUG)
            {
                Server::$instance->debug("[交易所-精炼升级装备计算] 是否破损:" . ($isDamageItemB ? '是' : '否') . ", 装备B:{$itemB->id}, 精炼等级:{$m}, 装备A:{$itemA->id}, 最大精炼:{$n}, 实际价格:{$price}, planA价格:{$mPriceB}, planB价格:{$priceB}, equipUpRate:{$equipUpRate}");
            }

        } while (0);

        return $price;
    }

    public static function calcLowPrice($itemId)
    {
        $lowItemId   = EquipConfig::getLowVidItemId($itemId);
        $lowItem = Item::get($lowItemId);
        return self::getServerItemPrice($lowItem);
    }

    /**
     * 计算升级装备的价格（不含装备价格）
     *
     * @param int $itemId
     * @param int $lv
     * @return float|int
     */
    public static function calcUpgradePrice(int $itemId, int $lv)
    {
        $price = 0;
        if ($lv < 1)
        {
            return $price;
        }

        $config = EquipUpgradeConfig::getConfigs();
        /** @var EquipUpgradeConfig $equipUpgrade */
        $equipUpgrade = $config[$itemId];

        for($i = 1; $i <= $lv; $i++)
        {
            $material = $equipUpgrade->materials[$i];
            foreach ($material as $data)
            {
                $item = Item::get($data['id']);
                if (!$item) continue;
                $price += $item->getPrice() * $data['num'] ?? 0;
            }
        }

        return $price;
    }

    /**
     * 删除物品
     * 删除内存中的物品数据
     *
     * @param $itemId
     */
    private function delItem($itemId)
    {
        Server::$item->del($itemId);
        unset($this->itemNodes[$itemId]);
    }

    /**
     * 检查是否出现回路
     * 若出现回路会导致价格计算死循环,为避免回路,需要检查每个节点
     *
     * @param $node
     * @param array $visit
     * @param array $father
     * @return bool 出现回路返回false
     */
    function checkLoop($node, &$visit = [], &$father = [])
    {
        $visit[$node->id] = 1;
        foreach ($node->parents as $id => $n)
        {
            if (!isset($visit[$id]))
            {
                $visit[$id] = 0;
            }
            if (!isset($father[$id]))
            {
                $father[$id] = -1;
            }

            if ($visit[$id] == 1 && $id != $father[$id])
            {
                return false;
            }
            else if ($visit[$id] == 0)
            {
                $father[$id] = $node->id;
                return $this->checkLoop($n, $visit, $father);
            }
        }
        $visit[$node->id] = 2;
        return true;
    }

    /**
     * 修改物品价格
     * 检查价格是否相同,若相同则不进行修改。反之先修改数据库的价格,若成功则修改内存中的价格,否则返回失败
     *
     * @param $itemId
     * @param int $price
     * @param int $time
     * @param int $cycle
     * @return bool
     */
    private function changePrice($itemId, $price, $time = null, $cycle = null)
    {
        $item = Server::$item->get($itemId);
        if (!$item)
        {
            return false;
        }

        try
        {
            // 为每一个基础物品设置一个redis缓存（不包括精炼和附魔的装备）
            Server::$redis->hSet(self::ITEM_PRICE_KEY, $itemId, $price);
        }
        catch (\Exception $e)
        {
            Server::$instance->warn('[交易所-价格调整] Redis发生异常。 错误信息:' . $e->getMessage());
        }

//        if (IS_DEBUG)
//        {
//            Server::$instance->debug("[交易所-改变物品价格] 物品ID: {$itemId} 内存价格:{$item['price']} 更新价格:{$price}");
//        }

        // 价格相同则不执行更新
        if ($item['price'] == $price)
        {
            return true;
        }

        if ($time === null)
        {
            $time = time();
        }

        if ($cycle === null)
        {
            $cycle = Common::randBetween($this->cycleBegin, $this->cycleEnd);
        }

        $itemInfo = new ItemInfo();
        $itemInfo->itemId            = $itemId;
        $itemInfo->lastServerPrice   = $price;
        $itemInfo->lastCalcPriceTime = $time;
        $itemInfo->cycle             = $cycle;
        $rs = $itemInfo->update();

        if ($rs !== false)
        {
            Server::$item->set($itemId, [
                'price'           => $price,
                'priceUpdateTime' => $time,
                'cycle'           => $cycle,
            ]);

            return true;
        } else {
            return false;
        }
    }
}