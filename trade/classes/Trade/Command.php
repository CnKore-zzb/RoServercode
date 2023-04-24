<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/4/17
 * Time: 下午3:38
 */

namespace RO\Trade;

class Command extends \stdClass
{
    public $cmd;

    const PROHIBIT_ITEM_CMD    = 1;
    const PROHIBIT_ENCHANT_CMD = 2;
    const PERMIT_ITEM_CMD      = 3;
    const PERMIT_ENCHANT_CMD   = 4;
}