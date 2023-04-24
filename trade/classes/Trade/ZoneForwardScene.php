<?php
namespace RO\Trade;

use RO\Booth\BoothOrderService;
use RO\Cmd\BoothOpenTradeCmd;
use RO\Cmd\EOperType;
use RO\Cmd\EPermission;
use RO\Cmd\ETradeType;
use RO\Cmd\ExtraPermissionSceneTradeCmd;
use RO\Cmd\GiveCheckMoneySceneTradeCmd;
use RO\Cmd\ItemData;
use RO\Cmd\LogItemInfo;
use RO\Cmd\NtfCanTakeCountTradeCmd;
use RO\Cmd\RecordServerTradeParam;
use RO\Cmd\ReduceItemRecordTrade;
use RO\Cmd\ReduceMoneyRecordTradeCmd;
use RO\Cmd\TakeLogCmd;
use RO\Cmd\TradePriceQueryTradeCmd;
use RO\Trade\Dao\Record;
use RO\Trade\Dao\RecordBought;
use RO\Trade\Dao\RecordSold;

/**
 * 来自Session服务器的返回数据
 *
 * @package RO
 */
class ZoneForwardScene extends \RO\ZoneForward
{
    /**
     * 玩家角色名
     * 
     * @var string
     */
    public $name;

    function __construct($fd, $fromId, $obj)
    {
        parent::__construct($fd, $fromId, $obj);
        
        $this->name = $obj->name;
    }

    public function run()
    {
        switch ($this->param)
        {
            # 处理返回逻辑
            case \RO\Cmd\RecordServerTradeParam::REDUCE_MONEY_RECORDTRADE:
                # 买家，扣款
                $cmd = new ReduceMoneyRecordTradeCmd($this->proto);
                switch ($cmd->type)
                {
                    case ETradeType::ETRADETYPE_TRADE:
                        ActBuy::onPayReceive($this, $cmd);
                        break;
                    case ETradeType::ETRADETYPE_BOOTH:
                        BoothOrderService::onReduceMoneyCallback($this, $cmd);
                        break;
                }
                break;

                /*
                $obj = new \RO\Cmd\ReduceMoneyRecordTradeCmd($this->proto);
                if (IS_DEBUG)
                {
                    Server::$instance->debug('[交易-购买] scene扣钱返回: '. json_encode($obj, JSON_UNESCAPED_UNICODE));
                }
                ActBuy::factory($this)->onBuyRecord($obj);

                # 成功
                $ret = \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_FAIL;

                # 失败
                $ret = \RO\Cmd\ETRADE_RET_CODE::ETRADE_RET_CODE_SUCCESS;

                # 消息回给客户端
                $rs = new \RO\Cmd\BuyItemRecordTradeCmd($this->proto);
                $rs->setItemInfo($obj->item_info);
                $rs->setRet($ret);
                $this->sendToUser($rs);
                */
                break;

            case \RO\Cmd\RecordServerTradeParam::ADD_ITEM_RECORDTRADE:
                # 加装备，或者撤单
                # SceneServer/ProcessSessionCmd.cpp line 929
                $obj = new \RO\Cmd\AddItemRecordTradeCmd($this->proto);
                if (IS_DEBUG)
                {
                    Server::$instance->debug('[Session-加物品] 加装备，或者撤单返回: '. json_encode($obj, JSON_UNESCAPED_UNICODE));
                }

                break;

            case \RO\Cmd\RecordServerTradeParam::REDUCE_ITEM_RECORDTRADE:
                # 卖家，扣钱，扣装备
                # SceneServer/ProcessSessionCmd.cpp line 874
                $cmd = new ReduceItemRecordTrade($this->proto);
                switch ($cmd->type)
                {
                    case ETradeType::ETRADETYPE_TRADE:
                        ActSell::factory($this)->onSellRecord($cmd);
                        break;
                    case ETradeType::ETRADETYPE_BOOTH:
                        BoothOrderService::onReduceItemRecordCallback($this, $cmd);
                        break;
                }
                break;

            case \RO\Cmd\RecordServerTradeParam::ADD_MONEY_RECORDTRADE:
                # 卖家，加钱
                # SceneServer/ProcessSessionCmd.cpp line 949
                $obj = new \RO\Cmd\AddMoneyRecordTradeCmd($this->proto);
                if (IS_DEBUG)
                {
                    Server::$instance->debug('[Session-加钱] 卖家，加钱返回: '. json_encode($obj, JSON_UNESCAPED_UNICODE));
                }
                break;
            case \RO\Cmd\SessionParam::SESSIONPARAM_GET_TRADELOG:
                $obj = new \RO\Cmd\GetTradeLogSessionCmd($this->proto);
                if (IS_DEBUG)
                {
                    Server::$instance->debug('[Session-领取物品] 场景服务器领取物品返回: '. json_encode($obj, JSON_UNESCAPED_UNICODE));
                }
                $this->onTakeLogFromSession($obj);
                break;
            case \RO\Cmd\RecordServerTradeParam::GIVE_CHECK_MONEY_RECORDTRADE:
                $obj = new GiveCheckMoneySceneTradeCmd($this->proto);
                if (IS_DEBUG)
                {
                    Server::$instance->debug('[Session-请求额度] 场景服务器请求额度: '. json_encode($obj, JSON_UNESCAPED_UNICODE));
                }
                $this->onGiveCheckMoney($obj);
                break;
            case \RO\Cmd\RecordServerTradeParam::EXTRA_PERMISSION_RECORDTRADE:
                $obj = new ExtraPermissionSceneTradeCmd($this->proto);
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[Session-玩家许可信息]" . json_encode($obj, JSON_UNESCAPED_UNICODE));
                }
                $this->onPermission($obj);
                break;
            case \RO\Cmd\RecordServerTradeParam::TRADE_PRICE_QUERY_RECORDTRADE:
                $obj = new TradePriceQueryTradeCmd($this->proto);
                if (IS_DEBUG)
                {
                    Server::$instance->debug("[Session-请求价格]" . json_encode($obj, JSON_UNESCAPED_UNICODE));
                }
                $this->onQueryPrice($obj);
                break;
            case RecordServerTradeParam::BOOTH_OPEN_RECORDTRADE:
                $obj = new BoothOpenTradeCmd($this->proto);
                $this->onBoothOpen($obj);
                break;
            default:
                Server::$instance->warn("[Session-未知] RecordServerTradeParam : {$this->param}");
                break;
        }
    }

    protected function onQueryPrice(TradePriceQueryTradeCmd $msg) {
        $itemId = $msg->itemdata->base->id;
        $item   = Item::get($itemId, $msg->itemdata);
        if (!$item)
        {
            return;
        }

        $msg->price = $item->getPrice();
        $this->sendToZone($msg);
    }

    protected function onPermission(ExtraPermissionSceneTradeCmd $msg)
    {
        switch ($msg->permission)
        {
            case EPermission::EPERMISSION_MAX_PENDING_LIMIT:
                Player::setPendingLimit($this->charId, $msg->value);
                break;
            case EPermission::EPERMISSION_RETURN_PERCENT:
                Player::setReturnRate($this->charId, $msg->value);
                break;
            case EPermission::EPERMISSION_QUOTA:
                Player::setQuota($this->charId, $msg->value);
                break;
            case EPermission::EPERMISSION_MAX_BOOTH_LIMIT:
                Player::setBoothPendingLimit($this->charId, $msg->value);
                break;
        }

        if (IS_DEBUG)
        {
            Server::$instance->debug("[交易-玩家许可信息] 玩家:{$msg->charid} 许可数据:" . json_encode(Server::$players->get($msg->charid), JSON_UNESCAPED_UNICODE));
        }
    }

    protected function onBoothOpen(BoothOpenTradeCmd $msg)
    {
        if (IS_DEBUG)
        {
            Server::$instance->debug("[摆摊状态] char_id:$this->charId, open:{$msg->open}");
        }
        Player::setBoothOpenStatus($this->charId, $msg->open);
    }

    protected function onGiveCheckMoney(\RO\Cmd\GiveCheckMoneySceneTradeCmd $msg)
    {
        if (null === $msg->itemdata || '' === $msg->itemdata)
        {
            $msg->quota = 0;
        }

        try
        {
            $itemData = new ItemData($msg->itemdata);
            $item = Item::get($itemData->base->id, $itemData);
            if ($item)
            {
                $msg->quota = $item->getPrice() * $itemData->base->count;
            }

            if (IS_DEBUG)
            {
                Server::$instance->debug("[交易-请求额度] 物品id:{$item->id} key:{$item->getKey()} 获取交易额度为:{$msg->quota}");
            }
        }
        catch (\RuntimeException $e)
        {
            Server::$instance->warn("[交易-请求额度] 解析itemData数据失败");
        }

        $this->sendToZone($msg);
    }

    protected function onTakeLogFromSession(\RO\Cmd\GetTradeLogSessionCmd $msg)
    {
        switch ($msg->logtype)
        {
            case EOperType::EOperType_NormalSell:
            case EOperType::EOperType_PublicitySellSuccess:
            case EOperType::EOperType_PublicitySellFail:
                $record = RecordSold::getById($msg->id);
                $type   = 'sell';
                break;
            default:
                $record = RecordBought::getById($msg->id);
                $type   = 'buy';
        }

        $ret = false;

        do
        {
            if (!$record)
            {
                Server::$instance->warn('[协议处理-场景服务器回调领取操作] 获取交易记录失败, ' . $type . ' record id ' . $msg->id);
                $ret = false;
                break;
            }

            if ($record->charId != $msg->charid)
            {
                Server::$instance->warn("[协议处理-场景服务器回调领取操作] 用户id不一致, 场景服务器返回的用户id为:{$msg->charid}, " . $type . ' record id:' . $msg->id);
                $ret = false;
                break;
            }

            // 场景服务器返回失败, 则还原交易记录状态
            if (!$msg->success)
            {
                $newTakeStatus = Record::TAKE_STATUS_CAN_TAKE_GIVE;
            }
            else
            {
                $newTakeStatus = Record::TAKE_STATUS_TOOK;
                $ret = true;
                try
                {
                    Server::$redis->decr('take_count_' . $this->charId);
                }
                catch (\Exception $e)
                {
                    Server::$instance->warn('[协议处理-场景服务器回调领取操作] redis:' . $e->getMessage());
                }
            }

            if (!$record->setMyTakeStatus($newTakeStatus))
            {
                Server::$instance->warn('[协议处理-场景服务器回调领取操作] 修改状态失败, ' . $type . ' record id:' . $msg->id);
                $ret = false;
                break;
            }
        }
        while(0);

        $cmd = new TakeLogCmd();
        $log = new LogItemInfo();
        $log->setId($msg->id);
        $log->setLogtype($msg->logtype);
        $cmd->setLog($log);
        $cmd->setSuccess($ret);
        $this->sendToUser($cmd);

        $msg = new NtfCanTakeCountTradeCmd();
        $msg->count = ZoneForwardUser::getCanTakeCount($this->charId);
        $this->sendToUser($msg);

        self::unLockUser($this->charId);

        if (IS_DEBUG)
        {
            Server::$instance->debug('[协议处理-场景服务器回调领取操作] 领取' . ($ret ? '成功' : '失败'));
        }
    }
}