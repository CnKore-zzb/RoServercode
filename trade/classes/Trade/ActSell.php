<?php
namespace RO\Trade;

use RO\Cmd\ETRADE_RET_CODE;
use RO\Cmd\ETradeType;
use RO\Cmd\ItemData;
use RO\Cmd\ReduceItemRecordTrade;
use RO\Cmd\StateType;
use RO\Trade\Dao\Goods;
use RO\Trade\Dao\ItemList;
use RO\Trade\Dao\Prohibition;
use RO\Trade\Dao\RecordBought;

/**
 * 出售物品动作对象
 *
 * @package RO\Trade
 */
class ActSell
{
    /**
     * 卖家charId
     *
     * @var int
     */
    public $charId;

    /**
     * 需要购买的物品ID
     *
     * @var int
     */
    public $itemId;

    /**
     * 购买数量
     *
     * @var int
     */
    public $count;

    /**
     * 请求的购买价格
     *
     * @var int
     */
    public $price;

    /**
     * 挂单ID
     *
     * @var int
     */
    public $pendingId;

    /**
     * 场景服务器发送的对象
     *
     * @var \RO\Trade\ZoneForwardUser|\RO\Trade\ZoneForwardScene
     */
    protected $forward;

    /**
     * 数据对象
     *
     * @var \RO\Cmd\BuyItemRecordTradeCmd
     */
    protected $cmd;

    /**
     * 禁止出售某物品
     *
     * @var []
     */
    protected static $prohibitItem = [];

    /**
     * 禁止出售某附魔
     *
     * @var []
     */
    protected static $prohibitEnchant;

    const SYS_MSG_CFG_ERROR             = 10000;         //策划表不一致
    const SYS_MSG_DB_ERROR              = 10001;         //数据库操作失败
    const SYS_MSG_SYS_ERROR             = 10002;         //系统异常

    const SYS_MSG_PUBLICITY_NOTICE      = 10400;         //世界消息 10400 [63cd4e] % s[-]进入了公示期，冒险者们快来抢购！

    const SYS_MSG_SELL_INVALID_COUNT    = 10100;         //出售-出售的个数不正确
    const SYS_MSG_SELL_INVALID_PRICE    = 10101;         //出售-出售的价格不正确
    const SYS_MSG_SELL_INVALID_ITEMID   = 10102;         //出售-不可交易的物品
    const SYS_MSG_SELL_INVALID_PARAMS   = 10103;         //出售-无法出售，请求参数有误
    const SYS_MSG_SELL_IS_FULL          = 10104;         //出售-无法出售，出售挂单已达上限
    const SYS_MSG_SELL_CANNOT_SELL1     = 10105;         //出售-无法出售，插卡物品无法出售
    const SYS_MSG_SELL_CANNOT_SELL2     = 10107;         //出售-无法出售，背包找不到该物品
    const SYS_MSG_SELL_CANNOT_SELL3     = 10108;         //出售-无法出售，物品数据同步异常
    const SYS_MSG_SELL_CANNOT_SELL4     = 10106;         //出售-强化的装备不可出售，前往分解
    const SYS_MSG_SELL_MONEY_NOT_ENOUGH = 10109;         //出售-无法出售，金钱不足

    const SYS_MSG_CANCEL_ALREADY_SELLED = 10200;         //撤单-无法撤单，已经出售
    const SYS_MSG_CANCEL_WAS_LOCKED     = 10201;         //撤单-无法撤单，该挂单已被锁定
    const SYS_MSG_ITEM_CANNOT_TRADE     = 10202;         //物品无法交易: [63cd4e]%s[-]现在已经无法在交易所中出售

    const SYS_MGS_SELL_SUCCESS          = 10250;         // 成功出售：恭喜！您成功地售出了[63cd4e]%s[-]个[63cd4e]%s[-]，扣税 %s, 共获得了[63cd4e]%s[-]金币

    const SYS_MGS_PROHIBIT_SELL         = 10253;         //禁止出售该商品
    const SYS_MGS_PUBLICITY_LOCK        = 10407;         //你所上架的物品正在结算公示，请稍后上架


    public function __construct(\RO\ZoneForward $session)
    {
        $this->forward = $session;
        $this->charId  = $this->forward->charId;
    }

    /**
     * 获取一个实例化对象
     *
     * @param \RO\ZoneForward $cmd
     * @return ActSell
     */
    public static function factory(\RO\ZoneForward $forward)
    {
        return new ActSell($forward);
    }

    /**
     * 出售上架
     *
     * @param \RO\Cmd\SellItemRecordTradeCmd|\RO\Cmd\ResellPendingRecordTrade $cmd
     * @param bool|Dao\Goods $goods
     * @return bool
     */
    public function reqSell($cmd, $goods = null)
    {
        # see TradeManager.cpp line 2860, 3504
        if (IS_DEBUG)
        {
            Server::$instance->debug("[交易所出售] cmd:" . json_encode($cmd));
        }

        # 检查是否同一个人
        if ($cmd->charid != $this->charId)
        {
            $this->forward->sendSysMsgToUser(self::SYS_MSG_SELL_INVALID_PARAMS);
            if (IS_DEBUG)
            {
                Server::$instance->debug("[交易所出售] 用户id不相同");
            }
            return false;
        }

        # 是否重新上架
        if (null !== $goods)
        {
            $reSell = true;
        }
        else
        {
            $reSell = false;
        }

        if (false === $reSell && $cmd->item_info->count <= 0)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[交易所出售] 错误出售参数");
            }
            $this->forward->sendSysMsgToUser(self::SYS_MSG_SELL_INVALID_COUNT);
            return false;
        }

        $itemInfo = $cmd->item_info;
        if ($itemInfo->item_data)
        {
            if (!$itemInfo->item_data->base || !$itemInfo->item_data->equip)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[交易所出售] 装备参数不正确");
                }
                $this->forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }

            if ($itemInfo->item_data->base->id != $itemInfo->itemid)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[交易所出售] 请求出售,item信息不一致");
                }
                $this->forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }

            if ($itemInfo->item_data->equip->lv > 0)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[交易所出售] 请求出售,升级装备不能进入交易所");
                }
                $this->forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }
        }

        $itemId   = $itemInfo->itemid;
        $item     = Item::get($itemId, $itemInfo->item_data);
        if (!$item)
        {
            # 不可以交易或物品不存在
            $this->forward->sendSysMsgToUser(self::SYS_MSG_SELL_INVALID_ITEMID);
            return false;
        }

        if ($item->equipType > 0 && !$item->itemData instanceof ItemData)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[交易所出售] 请求出售,属于装备物品,但没有传item_data");
            }
            $this->forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_ITEMID);
            return false;
        }

        if (!$item->canTrade())
        {
            # 不可交易的物品
            $this->forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_ITEMID);
            if (IS_DEBUG)
            {
                Server::$instance->debug("[交易所出售] 请求出售,不可交易的物品。 isTrade:{$item->isTrade}, tradeTime:{$item->tradeTime}, unTradeTime:{$item->unTradeTime}");
            }
            return false;
        }

        $itemKey = $item->getKey();
        if (true === $reSell && $itemKey != $goods->itemKey)
        {
            # 重新上架的物品 itemKey 和当前的 itemKey 不一致
            $this->forward->sendSysMsgToUser(self::SYS_MSG_SELL_INVALID_PARAMS);
            Server::$instance->warn("[交易所重新上架] itemKey 和当前 itemKey 不一致, goodsId: {$goods->id}, goodsItemKey:{$goods->itemKey} itemKey: {$itemKey}");
            return false;
        }

        if (false === $reSell && !$item->isOverlap)
        {
            # 不可堆叠的物品一次只能卖一个
            if ($itemInfo->count != 1)
            {
                $this->forward->sendSysMsgToUser(self::SYS_MSG_SELL_INVALID_COUNT);
                return false;
            }
        }

        if (!Prohibition::check(ETradeType::ETRADETYPE_TRADE, $itemInfo))
        {
            $this->forward->sendSysMsgToUser(self::SYS_MGS_PROHIBIT_SELL);
            if (IS_DEBUG)
            {
                Server::$instance->debug("[交易所出售] 安全指令禁止上架的物品. item_info:" . json_encode($itemInfo));
            }
            return false;
        }

        if (false === $reSell)
        {
            # 验证 挂了几单，最多只能挂8单
            if (Dao\Goods::isSellFull($cmd->charid))
            {
                $this->forward->sendSysMsgToUser(self::SYS_MSG_SELL_IS_FULL);
                return false;
            }
        }

        $itemList  = $item->getItemList();
        $stateType = Item::getPublicityState($item, $itemList);

        # 玩家发送请求时公示状态是否一致
        if ($stateType === StateType::St_InPublicity || $stateType === StateType::St_WillPublicity)
        {
            if ($itemInfo->publicity_id <= 0)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[交易所出售] 请求出售,玩家发送请求时公示状态与服务端不一致, 服务端当前正在公示");
                }
                $this->forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }
        }
        else
        {
            if ($itemInfo->publicity_id > 0)
            {
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[交易所出售] 请求出售,玩家发送请求时公示状态与服务端不一致, 服务端当前非公示");
                }
                $this->forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PARAMS);
                return false;
            }
        }

        # 一个已经过了公示期但还没有处理发货的，禁止上架
        if ($stateType === StateType::St_InPublicity && $itemList !== null && $itemList->getEndTime() <= time())
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[交易所出售] 请求出售,一个已经过了公示期但还没有处理发货的，禁止上架");
            }
            return false;
        }

        if ($stateType === StateType::St_InPublicity)
        {
            $price = $itemList ? $itemList->getPrice() : $item->getPrice();
        }
        else
        {
            $price = $item->getPrice();
        }

        if ($itemInfo->price != $price)
        {
            if (IS_DEBUG)
            {
                Server::$instance->debug("[交易所出售] 请求出售,价格不一致, 服务器价格:{$price}, 请求价格:{$itemInfo->price}");
            }
            $this->forward->sendSysMsgToUser(ActSell::SYS_MSG_SELL_INVALID_PRICE);
            return false;
        }

        if (false === $reSell)
        {
            # 锁用户操作，重新上架在 reqReSell 方法里已经有设置过
            if (!$this->forward->tryLockUser($this->charId))
            {
                Server::$instance->warn("[上架] 设置玩家锁失败, charId: {$this->charId}");
                return false;
            }

            $goods               = new Goods();
            $goods->itemId       = $itemId;
            $goods->itemListId   = $itemId;
            $goods->itemKey      = $item->getKey();
            $goods->charId       = $this->charId;
            $goods->playerName   = '';                      # 获取不到名称
            $goods->playerZoneId = $this->forward->zoneId;
            $goods->status       = Goods::STATUS_PENDING;
            $goods->count        = $itemInfo->count;
            $goods->stock        = $itemInfo->count;
            $goods->isPublicity  = 0;
            $goods->endTime      = 0;
            $goods->isOverlap    = $item->isOverlap;
            $goods->itemData     = null;
            $goods->time         = time();

            if (!$goods->insert())
            {
                $this->forward->sendSysMsgToUser(self::SYS_MSG_SELL_CANNOT_SELL3);
                $this->forward->unLockUser($this->charId);
                return false;
            }
        }

        $msg = new \RO\Cmd\ReduceItemRecordTrade();
        $msg->setItemInfo($itemInfo);
        $msg->setCharid($this->charId);
        $msg->setBoothfee(self::getBoothFee($itemInfo->price, $itemInfo->count));
        $msg->setIsResell($reSell);
        $msg->setOrderid($goods->id);
        $this->forward->sendToZone($msg);

        if (IS_DEBUG)
        {
            Server::$instance->debug("申请交易发送内容: " . json_encode($msg));
        }

        return true;
    }

    /**
     * 请求重新上架
     *
     * @param \RO\Cmd\ResellPendingRecordTrade $cmd
     */
    public function reqReSell(\RO\Cmd\ResellPendingRecordTrade $cmd)
    {
        if (IS_DEBUG)
        {
            Server::$instance->debug("交易-重新上架:" . json_encode($cmd));
        }

        $err = false;
        do
        {
            if (!$cmd->order_id)
            {
                $err = self::SYS_MSG_SELL_INVALID_PARAMS;
                break;
            }


            # 检查是否同一个人
            if ($cmd->charid != $this->charId)
            {
                $err = self::SYS_MSG_SELL_INVALID_PARAMS;
                break;
            }

            # 锁用户操作
            if (!$this->forward->tryLockUser($this->charId))
            {
                return;
            }

            $goods = Dao\Goods::getById($cmd->order_id);
            if ($goods)
            {
                if ($goods->charId != $this->charId)
                {
                    # 非本人订单
                    $err = self::SYS_MSG_SELL_INVALID_PARAMS;
                    break;
                }

                if ($cmd->item_info->itemid != $goods->itemId)
                {
                    $err = self::SYS_MSG_SELL_INVALID_PARAMS;
                    break;
                }

                if ($goods->stock == 0)
                {
                    # 已卖完毕
                    $err = self::SYS_MSG_CANCEL_ALREADY_SELLED;
                    break;
                }

                # 判断物品状态
                switch ($goods->status)
                {
                    case Dao\Goods::STATUS_EXPIRED:
                        # 通过
                        break;

                    case Dao\Goods::STATUS_SELLING:
                        if ($goods->isExpired())
                        {
                            # 通过
                            break;
                        }
                        $err = self::SYS_MSG_SELL_INVALID_PARAMS;
                        break 2;

                    default:
                        # 不通过
                        $err = self::SYS_MSG_SELL_INVALID_PARAMS;
                        break 2;
                }

                if ($goods->isPublicity == 1 && $goods->endTime > 0)
                {
                    # 公示物品还没有由系统处理完毕
                    $err = self::SYS_MSG_CANCEL_WAS_LOCKED;
                    break;
                }
            }
            elseif (false === $goods)
            {
                $err = self::SYS_MSG_DB_ERROR;
                break;
            }
            else
            {
                $err = self::SYS_MSG_CANCEL_ALREADY_SELLED;
                break;
            }

            $cmd->item_info->count     = $goods->stock;
            $cmd->item_info->item_data = $goods->getItemData();

            # 执行上架逻辑
            if (false === $this->reqSell($cmd, $goods))
            {
                $this->forward->unLockUser($this->charId);
                return;
            }
        }
        while (false);

        if ($err)
        {
            $this->forward->sendSysMsgToUser($err);
            $this->forward->unLockUser($this->charId);
            return;
        }
    }

    /**
     * 卖物品，扣物品后返回
     *
     * @param \RO\Cmd\ReduceItemRecordTrade $msg
     */
    public function onSellRecord(ReduceItemRecordTrade $msg)
    {
        $tag      = $msg->is_resell ? '重新上架' : '上架';
        $goodsId  = $msg->orderid;
        $itemInfo = $msg->item_info;
        $goods    = Dao\Goods::getById($goodsId);
        if ($goods === false)
        {
            Server::$instance->warn("[{$tag}] goods_id:{$goodsId}查找失败");
            return;
        }
        else if ($goods === null)
        {
            Server::$instance->warn("[{$tag}] goods_id:{$goodsId}不存在");
            return;
        }

        if (!$msg->is_resell && $goods->status !== Goods::STATUS_PENDING)
        {
            Server::$instance->warn("[{$tag}] 上架的订单, 处于非法状态! goods_id:{$goodsId}, status:{$goods->status}");
            return;
        }

        if ($msg->ret == ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS)
        {
            # 更新订单状态为扣除物品成功
            $mysql    = Server::$mysqlMaster;
            $item     = Item::get($itemInfo->itemid, $itemInfo->item_data);
            $itemKey  = $item->getKey();
            $isNewPublicity = false;
            $itemList       = ItemList::getByKey($itemKey);
            if ($itemList)
            {
                $itemList->item = $item;
                if ($itemList->isPublicity != 1 && $itemInfo->publicity_id > 0)
                {
                    # 重新开启一个公示
                    $itemList->startTime   = time();
                    $itemList->endTime     = time() + $item->publicityShowTime;
                    $itemList->isPublicity = 1;
                    $itemList->pubPrice    = $itemInfo->price;
                    $itemList->delayTime   = 0;
                    $itemList->update();
                    $isNewPublicity = true;
                }
            }
            else
            {
                // 只有在内存表找不到时且数据库查找返回false出现
                if ($itemList === false)
                {
                    Server::$instance->warn("[{$tag}] 查找ItemList失败, item_id:{$itemInfo->itemid}, item_key:{$itemKey}");
                }

                $itemList              = new ItemList([], $itemKey);
                $itemList->item        = $item;
                $itemList->itemId      = $item->id;
                $itemList->isOverlap   = $item->isOverlap;
                $itemList->itemData    = $itemInfo->item_data && $item->equipType > 0 ? $itemInfo->item_data : null;
                $itemList->refineLv    = $itemInfo->item_data && $item->equipType > 0 ? $itemInfo->item_data->equip->refinelv : 0;
                $itemList->isDamage    = $itemInfo->item_data && $item->equipType > 0 ? intval($itemInfo->item_data->equip->damage) : 0;
                $itemList->isPublicity = $itemInfo->publicity_id > 0 ? 1 : 0;

                if ($itemList->isPublicity == 1)
                {
                    $itemList->startTime = time();
                    $itemList->endTime   = time() + $item->publicityShowTime;
                    $itemList->pubPrice  = $itemInfo->price;
                    $itemList->delayTime = 0;
                    $isNewPublicity      = true;
                }

                if (!$itemList->insert())
                {
                    Server::$instance->warn("[{$tag}] 添加新的ItemList失败, item_id:{$itemInfo->itemid}, item_key:{$itemKey}");
                }

                Server::$instance->server->task('list.reload', 0);
            }

            if ($isNewPublicity)
            {
                # 公示数递增
                Server::$item->incr($goods->itemId, 'publicityNum', 1);

                # 移除库存缓存
                $task       = new \RO\Trade\ItemTask();
                $task->id   = $itemList->id;
                $task->type = \RO\Trade\ItemTask::TYPE_CHANNEL_DEL_STOCK_CACHE;
                $task->sendToChannel();

                Server::$instance->info("[{$tag}] 开启新公示, goods_id:{$goods->id}, char_id:{$goods->charId}, item_id:{$goods->itemId}, item_key:{$goods->itemKey}");

                # 推送世界消息
                //ZoneForwardUser::sendWorldMsg(self::SYS_MSG_PUBLICITY_NOTICE, [$item->name]);

                Server::$item->set($item->id, ['lastPubEndTime' => $itemList->endTime]);
            }

            $goods->playerZoneId = $this->forward->zoneId;
            $goods->playerName   = $this->forward->name;
            $goods->time         = time();
            $goods->isPublicity  = $itemList->isPublicity;
            $goods->endTime      = $itemList->isPublicity == 1 ? $itemList->endTime : 0;
            $goods->pubPrice     = $itemList->isPublicity == 1 ? $itemList->pubPrice : 0;
            $goods->status       = Dao\Goods::STATUS_SELLING;

            if ($msg->is_resell)
            {
                # 总数调整成和库存数一样的值
                $goods->count = $goods->stock;
                # 商人技能退回上架费
                $percent = Player::getReturnRate($goods->charId);
                if ($percent)
                {
                    $boothFee = self::getBoothFee($itemInfo->price, $goods->count);
                    $retMoney = intval($boothFee * $percent / 1000);
                    if ($retMoney)
                    {
                        $retBoothFeeCmd = self::getReturnBoothFeeCmd($goods->charId, $retMoney);
                        if ($this->forward->sendToZone($retBoothFeeCmd))
                        {
                            Server::$instance->info("[{$tag}] 商人技能,上架费用返还. 上架费:{$boothFee}, 返还费:{$retMoney}, 商人上架费比率:{$percent}, goods id:{$goods->id}, charId: {$goods->charId}, itemId: {$goods->itemId}, msg: " . json_encode($retBoothFeeCmd));
                        }
                        else
                        {
                            Server::$instance->warn("[{$tag}] 商人技能,上架费用返还失败,因发送消息失败. 上架费:{$boothFee}, 返还费:{$retMoney}, 商人上架费比率:{$percent}, goods id:{$goods->id}, charId: {$goods->charId}, itemId: {$goods->itemId}, msg: " . json_encode($retBoothFeeCmd));
                        }
                    }
                }
            }
            else
            {
                // 理论不存在该情况, 扣物品已经校验过物品的合法性
                if ($goods->itemKey != $itemKey)
                {
                    Server::$instance->warn("[{$tag}] goods_id:{$goods->id}, 原物品itemKey:{$goods->itemKey}, 扣物品返回itemKey:{$itemKey}, 两者不一致");
                }

                $itemData = null;
                if ($item->equipType > 0)
                {
                    $itemInfo->item_data->base->guid = $itemInfo->guid;
                    $itemData                        = $itemInfo->item_data;
                }
                $goods->itemListId   = $itemList->id;
                $goods->itemKey      = $itemKey;
                $goods->itemData     = $itemData;
            }

            if (false === $goods->update())
            {
                Server::saveDelaySQL($mysql->last_query);
            }

            // 推送fluent
            $obj             = new \RO\Cmd\TradeLogCmd();
            $obj->pid        = $msg->charid;
            $obj->time       = time();
            $obj->type       = $goods->isPublicity == 0 ? \RO\ZoneForward::ETRADETYPE_SELL : \RO\ZoneForward::ETRADETYPE_PUBLICITY_SELL;
            $obj->itemid     = $goods->itemId;
            $obj->count      = $goods->count;
            $obj->price      = $itemInfo->price;
            $obj->tax        = self::getBoothFee($obj->price, $goods->count);
            $obj->moneycount = 0;
            $obj->logid      = $goods->isPublicity == 0 ? "goods-add-$goodsId" : "pub-goods-add-$goodsId";
            if (!$goods->isOverlap && $goods->itemId != $goods->itemKey)
            {
                $obj->iteminfo = json_encode($itemInfo->getItemData());
            }
            ZoneForwardScene::pushToFluent($obj);

            if ($goods->isPublicity == 1 && $goods->endTime > 0)
            {
                # 公示则直接更新库存
                Dao\ItemList::incrItemStock($goods->itemId, $goods->itemKey, $goods->itemListId, $goods->stock);
            }
            else
            {
                # 非公示，通知任务进程增加库存
                $task         = new ItemTask();
                $task->type   = $msg->is_resell ? ItemTask::TYPE_CHANNEL_GOODS_RESELL : ItemTask::TYPE_CHANNEL_GOODS_NEW;
                $task->id     = (int)$goods->itemListId;
                $task->itemId = (int)$goods->itemId;
                $task->gid    = (int)$goodsId;
                $task->stock  = (int)$goods->stock;
                $task->sendToChannel();
            }

            if ($msg->is_resell)
            {
                Server::$instance->info("[{$tag}] 上架成功,扣手续费返回. char_id:{$goods->charId}, goods_id:{$goods->id}, is_publicity:{$goods->isPublicity}, item_list_stock:{$itemList->stock}, cmd:" . json_encode($msg));
            }
            else
            {
                Server::$instance->info("[{$tag}] 上架成功,扣手续费、装备返回. char_id:{$goods->charId}, goods_id:{$goods->id}, is_publicity:{$goods->isPublicity}, item_list_stock:{$itemList->stock}, cmd:" . json_encode($msg));
            }

            # 通知客户端
            $return            = new \RO\Cmd\SellItemRecordTradeCmd();
            $return->item_info = $itemInfo;
            $return->ret       = \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS;
            $this->forward->sendToUser($return);
        }
        else
        {
            # 重新上架的物品不用管
            if (!$msg->is_resell)
            {
                Dao\Goods::removeById($msg->orderid);
            }
        }

        switch ($msg->ret)
        {
            case ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS:
                # 公示物品，插入到公示表
                break;

            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_MONEY_IS_NOT_ENOUGH:
                # 金额不足
                break;

            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_ITEM_IS_FROSTED:
                # 找不到物品
                $this->forward->sendSysMsgToUser(self::SYS_MSG_SELL_CANNOT_SELL2);
                break;

            case ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL:
                #出售失败
                break;

            case ETRADE_RET_CODE::ETRADE_RET_CODE_CANNOT_CANNOT_SELL:
                #不能出售,可能是背包里没这个装备，要么就是检测不通过
                break;

            default:
                # 其它错误
                $this->forward->sendSysMsgToUser(self::SYS_MSG_SELL_CANNOT_SELL3);
                Server::$instance->warn("[{$tag}] 场景服务器返回了一个未知返回码:{$msg->ret} 卖家id:{$msg->charid} 挂单id:{$msg->orderid} 物品id:{$msg->item_info->itemid} 价格:{$msg->item_info->price} 数量:{$msg->item_info->count} msg:" . json_encode($msg));
                break;
        }

        unLock:
        $this->forward->unLockUser($this->charId);
    }

    /**
     * 取消订单
     *
     * @param \RO\Cmd\CancelItemRecordTrade $cmd
     */
    public function reqCancel(\RO\Cmd\CancelItemRecordTrade $cmd)
    {
        # see TradeManager.cpp line 2914
        if (IS_DEBUG)
        {
            Server::$instance->debug("[下架] " . json_encode($cmd));
        }

        $goodsId = intval($cmd->order_id);
        if (!$goodsId)
        {
            return;
        }

        if ($cmd->charid != $this->charId)
        {
            $this->forward->sendSysMsgToUser(self::SYS_MSG_CANCEL_WAS_LOCKED);
            Server::$instance->warn("[下架] charId 不一致: {$cmd->charid} != {$this->charId}, orderId: $cmd->order_id");
            return;
        }

        if (Server::$updatingGoodsStock->exist($goodsId))
        {
            $this->forward->sendSysMsgToUser(self::SYS_MSG_CANCEL_WAS_LOCKED);
            Server::$instance->warn("[下架] 尝试下架一个还在异步更新库存的挂单 charId: {$cmd->charid}, orderId: $cmd->order_id");
            return;
        }

        if (false === $this->forward->tryLockUser($cmd->charid))
        {
            # 没有抢到锁，可能是上一个操作还在执行中，避免用户并发操作
            $this->forward->sendSysMsgToUser(self::SYS_MSG_CANCEL_WAS_LOCKED);
            return;
        }

        $info = null;
        do
        {
            $goods = Dao\Goods::getById($goodsId);

            if (false === $goods)
            {
                # 读取数据失败
                $this->forward->sendSysMsgToUser(self::SYS_MSG_DB_ERROR);
                $info = '订单: '. $goodsId .' 查询失败在';
                break;
            }
            elseif (null === $goods)
            {
                $this->forward->sendSysMsgToUser(self::SYS_MSG_CANCEL_ALREADY_SELLED);
                $info = '订单: '. $goodsId .' 不存在';
                break;
            }

            if ($goods->charId != $cmd->charid)
            {
                $info = "非法下架，指定的订单 $goodsId 不是当前用户: {$cmd->charid} != {$goods->charId}";
                break;
            }

            if ($goods->stock == 0)
            {
                $this->forward->sendSysMsgToUser(self::SYS_MSG_CANCEL_ALREADY_SELLED);
                $info = '订单: '. $goodsId .' 已经卖光';
                break;
            }

            # 非在售状态
            if ($goods->status != Dao\Goods::STATUS_SELLING && $goods->status != Dao\Goods::STATUS_EXPIRED)
            {
                $this->forward->sendSysMsgToUser(self::SYS_MSG_CANCEL_WAS_LOCKED);
                $info = '玩家(CharId:'. $cmd->charid .')请求下架，但是订单 '. $goodsId .' 状态不是在售状态，是 '. $goods->status;
                break;
            }

            if ($goods->isPublicity == 1 && $goods->endTime > 0)
            {
                # 公示期物品不能下架
                $this->forward->sendSysMsgToUser(self::SYS_MSG_CANCEL_WAS_LOCKED);
                $info = "公示期物品不可下架, orderId: {$goodsId}, charId: {$cmd->charid}, endTime: {$goods->endTime}, now: ". time();
                break;
            }

            # 还是在售状态，投递到对应的任务进程中下架
            $task         = new ItemTask();
            $task->id     = (int)$goods->itemListId;
            $task->type   = ItemTask::TYPE_GOODS_CANCEL;
            $task->itemId = (int)$goods->itemId;
            $task->fd     = (int)$this->forward->fd;
            $task->fromId = (int)$this->forward->fromId;
            $task->gid    = (int)$goods->id;

            # 投递成功后将由Task进程执行 doCancelInTask($task) 方法
            if (false === $task->send())
            {
                # 投递失败
                $this->forward->sendSysMsgToUser(self::SYS_MSG_SYS_ERROR);
                $info = "投递任务失败, orderId: {$goodsId}, charId: {$cmd->charid}";
                break;
            }

            return;
        } while (false);

        if (IS_DEBUG && $info)
        {
            Server::$instance->debug('[下架] '. $info);
        }

        # 解除用户锁
        $this->forward->unLockUser($cmd->charid);
    }

    /**
     * 在Task进程里处理下架
     *
     * 此方法里不需要处理库存，返回 true 后会自动处理库存
     *
     * @param ItemTask $task 任务对象
     * @return bool
     */
    public static function doCancelInTask($goodsId, $fd, $fromId)
    {
        $ret   = false;
        $goods = Dao\Goods::getById($goodsId);

        if (null === $goods)
        {
            # 重新尝试
            $goods = Dao\Goods::getById($goodsId);
        }

        if (null === $goods)
        {
            return false;
        }
        elseif (false === $goods)
        {
            # 数据库异常
            ZoneForwardUser::sendSysMsgToUserByCharId($fd, $fromId, $goods->charId, self::SYS_MSG_DB_ERROR);
            goto unlock;
        }

        # 已经下架过了
        if ($goods->status === Dao\Goods::STATUS_CANCELED)
        {
            ZoneForwardUser::sendSysMsgToUserByCharId($fd, $fromId, $goods->charId, self::SYS_MSG_CANCEL_WAS_LOCKED);
            goto unlock;
        }

        if ($goods->stock == 0)
        {
            ZoneForwardUser::sendSysMsgToUserByCharId($fd, $fromId, $goods->charId, self::SYS_MSG_CANCEL_ALREADY_SELLED);
            goto unlock;
        }

        # 即将到期的挂单，改成锁定状态
        if ($goods->status == Dao\Goods::STATUS_SELLING && $goods->time + Server::$configExchange['ExchangeHour'] - 100 <= time())
        {
            ZoneForwardUser::sendSysMsgToUserByCharId($fd, $fromId, $goods->charId, self::SYS_MSG_CANCEL_WAS_LOCKED);
            goto unlock;
        }

        # 再次检查是否有异步更新库存数
        if (Server::$updatingGoodsStock->exist($goodsId))
        {
            ZoneForwardUser::sendSysMsgToUserByCharId($fd, $fromId, $goods->charId, self::SYS_MSG_CANCEL_WAS_LOCKED);
            Server::$instance->warn("[下架] 尝试下架一个还在异步更新库存的挂单 charId: {$goods->charId}, orderId: $goods->id");
            goto unlock;
        }

        $cmd           = new \RO\Cmd\CancelItemRecordTrade();
        $cmd->charid   = $goods->charId;
        $cmd->order_id = $goods->id;

        # 原始状态
        $mysql = Server::$mysqlMaster;

        try
        {
            # 执行加装备
            $itemInfo           = new \RO\Cmd\TradeItemBaseInfo();
            $itemInfo->order_id = $goods->id;
            $itemInfo->itemid   = $goods->itemId;
            $itemInfo->count    = $goods->stock;
            if ($itemData = $goods->getItemData())
            {
                $itemData->base->count = $goods->stock;
                $itemInfo->item_data   = $itemData;
            }

            $obj            = new \RO\Cmd\AddItemRecordTradeCmd();
            $obj->charid    = $goods->charId;
            $obj->addtype   = \RO\Cmd\EAddItemType::EADDITEMTYP_RETURN;
            $obj->item_info = $itemInfo;

            # 提交事务
            $rs = $goods->setStatus(Dao\Goods::STATUS_CANCELED);
            if ($rs > 0)
            {
                # 退回物品

                # 推送 fluent log, see TradeManager.cpp line 3698
                $fluentLog             = new \RO\Cmd\TradeLogCmd();
                $fluentLog->pid        = $goods->charId;
                $fluentLog->time       = time();
                $fluentLog->type       = \RO\ZoneForward::ETRADETYPE_CANCEL;
                $fluentLog->itemid     = $goods->itemId;
                $fluentLog->count      = $goods->stock;
                $fluentLog->price      = $goods->getPrice();
                $fluentLog->tax        = 0;
                $fluentLog->moneycount = 0;
                $fluentLog->logid      = "goods-cancel-$goodsId";
                if (!$goods->isOverlap && $goods->itemId != $goods->itemKey)
                {
                    $fluentLog->iteminfo = json_encode($goods->getItemData());
                }
                ZoneForwardScene::pushToFluent($fluentLog);

                Server::$instance->info("[下架] 执行成功：$goods->id, charId: {$goods->charId}, itemId: {$goods->itemId}, msg: " . json_encode($obj));

                if (ZoneForwardScene::sendToZoneByFd($obj, $fd, $fromId))
                {
                    # 回消息给玩家
                    $cmd->ret = \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS;
                    ZoneForwardUser::sendToUserByFd($cmd, $goods->charId, $fd, $fromId);

                    if ($item = Item::get($goods->itemId))
                    {
                        # 若不能交易,需要退上架费用
                        if (!$item->isTrade)
                        {
                            $retMoney = self::getBoothFee($fluentLog->price, $goods->stock);
                            $retBoothFeeCmd = self::getReturnBoothFeeCmd($goods->charId, $retMoney);
                            if (ZoneForwardScene::sendToZoneByFd($retBoothFeeCmd, $fd, $fromId))
                            {
                                Server::$instance->info("[下架] 退回不能交易物品的上架费用成功. goods id:{$goods->id}, charId: {$goods->charId}, itemId: {$goods->itemId}, msg: " . json_encode($retBoothFeeCmd));
                            }
                        }
                        else
                        {
                            $percent = Player::getReturnRate($goods->charId);
                            if ($percent)
                            {
                                $boothFee = self::getBoothFee($fluentLog->price, $goods->stock);
                                $retMoney = intval($boothFee * $percent / 1000);
                                if ($retMoney)
                                {
                                    $retBoothFeeCmd = self::getReturnBoothFeeCmd($goods->charId, $retMoney);
                                    if (ZoneForwardScene::sendToZoneByFd($retBoothFeeCmd, $fd, $fromId))
                                    {
                                        Server::$instance->info("[下架] 商人技能,上架费用返还. 上架费:{$boothFee}, 返还费:{$retMoney}, 商人上架费比率:{$percent}, goods id:{$goods->id}, charId: {$goods->charId}, itemId: {$goods->itemId}, msg: " . json_encode($retBoothFeeCmd));
                                    }
                                    else
                                    {
                                        Server::$instance->warn("[下架] 商人技能,上架费用返还失败,因发送消息失败. 上架费:{$boothFee}, 返还费:{$retMoney}, 商人上架费比率:{$percent}, goods id:{$goods->id}, charId: {$goods->charId}, itemId: {$goods->itemId}, msg: " . json_encode($retBoothFeeCmd));
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        # 如果不存在是获取不到物品的价格且没法计算要返还的上架费用
                        Server::$instance->warn("[下架] 下架的商品itemId: {$goods->itemId}不存在! 请检查配置表是否删除了");
                    }

                    $ret = true;
                }
                else
                {
                    throw new \Exception("[下架] 物品发送Zone服务器失败, fd: $fd, fromId: $fromId, goodsId: $goods->id");
                }
            }
            elseif ($rs == 0)
            {
                # 没有更新到数据
                $ret = true;
            }
            else
            {
                Server::$instance->warn("[下架] 事务提交失败：$goods->id, charId: {$goods->charId}, itemId: {$goods->itemId}, msg: " . json_encode($obj));
            }
        }
        catch (\Exception $e)
        {
            throw $e;
        }

        # 解除用户锁
        unlock:
        ZoneForwardUser::unLockUser($goods->charId);

        return $ret;
    }

    public static function getReturnBoothFeeCmd($charId, $money)
    {
        $itemInfo = new \RO\Cmd\TradeItemBaseInfo();
        $itemInfo->itemid = 100;
        $itemInfo->count  = $money;
        $obj            = new \RO\Cmd\AddItemRecordTradeCmd();
        $obj->charid    = $charId;
        $obj->addtype   = \RO\Cmd\EAddItemType::EADDITEMTYP_RETURN;
        $obj->item_info = $itemInfo;
        return $obj;
    }

    /**
    * 交易费
    *
    * @param $price
    * @param $count
    * @return int
    */
    public static function getBoothFee($price, $count)
    {
        return intval($count * min(Server::$configExchange['MaxBoothfee'], ceil($price * Server::$configExchange['SellCost'])));
    }

    /**
     * 禁止某项物品出售
     *
     * @param $itemId
     * @param int $refineLv
     * @param int $enchantId
     * @param int $buffId
     */
    public static function prohibitItem($itemId, $refineLv = 0, $enchantId = 0, $buffId = 0)
    {
        if (!isset(self::$prohibitItem[$itemId]))
        {
            self::$prohibitItem[$itemId] = [];
        }

        if ($refineLv == 0 && $enchantId == 0 && $buffId == 0)
        {
            $condition = [];
            $key = '0,0,0';
        }
        else
        {
            $condition = [$refineLv, $enchantId, $buffId];
            $key = "$refineLv,$enchantId,$buffId";
        }

        self::$prohibitItem[$itemId][$key] = $condition;
    }

    public static function cancelOffByItem($itemId, $refineLv = 0, $enchantId = 0, $buffId = 0)
    {
        self::cancelOffByCondition($itemId, $refineLv, $enchantId, $buffId);
    }

    public static function cancelOffByEnchant($enchantId = 0, $buffId = 0)
    {
        self::cancelOffByCondition(null, null, $enchantId, $buffId);
    }

    /**
     * 根据条件下架物品
     * 被下架的商品会被设为过期,让玩家自行取回
     *
     * 1. refineLv,enchantId和buffId为0时, 仅对itemId的物品下架
     * 2. 下列参数除itemId,其余为0时代表不作为条件筛选
     * 3. 当itemId为null时, 仅根据enchantId和buffId作为条件
     *
     *
     * @param int $itemId
     * @param int $refineLv
     * @param int $enchantId 附魔id
     * @param int $buffId    附魔buffId（如斗志、专注）
     */
    private static function cancelOffByCondition($itemId, $refineLv, $enchantId, $buffId)
    {
        $item = null;
        if ($itemId !== null)
        {
            $item = Item::get($itemId);
            if (!$item)
            {
                Server::$instance->info("[交易所-条件下架] itemId:{$itemId} 物品不存在!");
                return;
            }
        }

        $itemListIds = [];

        # 公示上锁,避免公示处理进程抢占
        Server::$publicityLock->lock();
        Server::$instance->info("[交易所-条件下架] 正在执行下架操作. itemId:{$itemId}, refineLv:{$refineLv}, enchantId:{$enchantId}, buffId:{$buffId}");

        if ($itemId === null || $item->equipType > 0)
        {
            foreach (Server::$itemList as $data)
            {
                $dataItemId   = $data['item_id'];
                $dataRefineLv = $data['refine_lv'];
                $id           = $data['id'];
                $isPublicity  = $data['is_publicity'];

                if ($itemId != null)
                {
                    if ($itemId != $dataItemId)
                        continue;

                    if ($refineLv == 0 && $enchantId == 0 && $buffId == 0)
                    {
                        $itemListIds[$id] = $isPublicity;
                        continue;
                    }

                    if ($refineLv > 0 && $refineLv == $dataRefineLv)
                    {
                        $itemListIds[$id] = $isPublicity;
                        continue;
                    }
                }

                # 不管itemId,只要符合附魔条件,都处理下架
                if ($data['item_data'])
                {
                    $itemData = new ItemData($data['item_data']);

                    if ($enchantId)
                    {
                        if (isset($itemData->enchant->attrs))
                        {
                            foreach ($itemData->enchant->attrs as $attr)
                            {
                                if ($enchantId == $attr->type)
                                {
                                    $itemListIds[$id] = $isPublicity;
                                    continue;
                                }
                            }
                        }
                    }

                    if ($buffId)
                    {
                        if (isset($itemData->enchant->extras))
                        {
                            foreach ($itemData->enchant->extras as $extra)
                            {
                                if ($buffId == $extra->buffid)
                                {
                                    $itemListIds[$id] = $isPublicity;
                                    continue;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            $itemList = Server::$itemList->get($itemId);
            if ($itemList)
                $itemListIds[$itemId] = $itemList['is_publicity'];
        }

        if (!empty($itemListIds))
        {
            foreach ($itemListIds as $itemListId => $isPublicity)
            {
                $mysql = Server::$mysqlMaster;

                # 处理为公示结束
                if ($isPublicity)
                {
                    $itemList = ItemList::getById($itemListId);
                    $itemList->isPublicity = 0;
                    $itemList->endTime = 0;
                    $itemList->pubBuyPeople = 0;
                    $itemList->pubPrice = 0;

                    if ($itemList->update())
                    {
                        try
                        {
                            Server::$redis->del('pub_buy_count_'. $itemList->id);
                        }
                        catch (\Exception $e)
                        {
                            Server::$instance->warn("[交易所-条件下架] pub_buy_count_{$itemList->id} 缓存删除失败 error:{$e->getMessage()}");
                        }

                        $rs = Server::$item->decr($itemList->itemId, 'publicityNum', 1);
                        if ($rs !== false && $rs < 0)
                        {
                            Server::$item->set($itemList->itemId, ['publicityNum' => 0]);
                            Server::$instance->warn("[交易所-条件下架] Item 公示购买数异常, ItemId: {$itemList->itemId}, publicity number: {$rs}");
                        }
                    }
                    else
                    {
                        Server::$instance->warn('[交易所-条件下架] itemList更新失败 id为:' . $itemList->id);
                    }

                    $sql = 'UPDATE `trade_record_bought` SET `status` = ' . RecordBought::STATUS_PUBLICITY_CANCEL . " WHERE `count` = 0 AND `publicity_id` = '{$itemList->id}' AND `status` = " . RecordBought::STATUS_PUBLICITY_PAY_SUCCESS;
                    $rs = $mysql->query($sql);
                    if (!$rs)
                    {
                        Server::$instance->warn("[交易所-条件下架] publicity_id:{$itemList->id} 抢购记录退款失败");
                    }

                    $sql = "UPDATE `trade_goods` SET `is_publicity` = 0, `end_time` = 0 WHERE `status` = '". Dao\Goods::STATUS_SELLING ."' AND `item_list_id` = '{$itemListId}'";
                    $rs = $mysql->query($sql);
                    if (!$rs)
                    {
                        Server::$instance->warn("[交易所-条件下架] publicity_id:{$itemList->id} 更新公示挂单记录失败!");
                    }
                }

                $goodIds = [];
                $sql  = "SELECT `id` FROM `trade_goods` WHERE `status` = '". Dao\Goods::STATUS_SELLING ."' AND `item_list_id` = '{$itemListId}'";
                $rs = $mysql->query($sql);

                if ($rs && $rs->num_rows)
                {
                    while($data = $rs->fetch_object())
                    {
                        $goodIds[] = (int) $data->id;
                    }

                    $rs->free();
                }

                if (empty($goodIds))
                    continue;

                $count  = count($goodIds);
                $limit  = 100;
                $page   = ceil($count / $limit);
                for ($i = 1; $i <= $page; $i++)
                {
                    $offset = $limit * ((int)$i - 1);
                    $goodIdsSlice = array_slice($goodIds, $offset, $limit);
                    $task       = new ItemTask();
                    $task->type = ItemTask::TYPE_CHANNEL_OFF_GOODS;
                    $task->id  = (int) $itemListId;
                    $task->ids = $goodIdsSlice;    # 注意goodsId必须是int
                    $task->sendToChannel();
                }
            }
        }

        Server::$publicityLock->unlock();
        Server::$instance->info("[交易所-条件下架] 执行完毕. itemId:{$itemId}, refineLv:{$refineLv}, enchantId:{$enchantId}, buffId:{$buffId}");
    }
}