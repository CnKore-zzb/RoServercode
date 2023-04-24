<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/31
 * Time: 上午10:12
 */

namespace RO\Trade\Dao;

use RO\Cmd\LogItemInfo;

abstract class Record extends Dao
{
    const TAKE_STATUS_CAN_TAKE_GIVE   = 0;       # 可领取可赠送
    const TAKE_STATUS_TOOK            = 1;       # 已领取
    const TAKE_STATUS_TAKING          = 2;       # 领取中
    const TAKE_STATUS_GIVING          = 3;       # 赠送中
    const TAKE_STATUS_ACCEPTING       = 4;       # 赠送正在领取中
    const TAKE_STATUS_ACCEPTED_1      = 5;       # 赠送已经领取额度未知（已赠送）
    const TAKE_STATUS_ACCEPTED_2      = 6;       # 赠送已经领取额度已知（已赠送）

    /**
     * 操作状态
     *
     * @var int
     */
    public $takeStatus = self::TAKE_STATUS_CAN_TAKE_GIVE;

    /**
     * 获取一个LogItemInfo对象
     *
     * @return LogItemInfo
     */
    abstract function getLogItemInfo();

    /**
     * 设置数据库的操作状态
     *
     * @param $status
     * @return mixed
     */
    abstract function setMyTakeStatus($status);
}