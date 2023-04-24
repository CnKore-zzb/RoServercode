<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/6/29
 * Time: 下午4:20
 */

namespace RO\Booth\Dao;

class BoothRecordTakeStatus
{
    const TAKE_STATUS_CAN_TAKE_GIVE = 0;       # 可领取
    const TAKE_STATUS_TOOK          = 1;       # 已领取
    const TAKE_STATUS_TAKING        = 2;       # 领取中
    const TAKE_STATUS_GIVING        = 3;       # 赠送中
    const TAKE_STATUS_ACCEPTING     = 4;       # 赠送正在领取中
    const TAKE_STATUS_ACCEPTED_1    = 5;       # 赠送已经领取额度未知（已赠送）
    const TAKE_STATUS_ACCEPTED_2    = 6;       # 赠送已经领取额度已知（已赠送）
}