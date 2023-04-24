<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/9
 * Time: 下午1:50
 */

namespace RO\Trade\Dao;

class PriceAdjustLog extends Dao
{
    public $id;

    public $itemId;

    public $refineLv = 0;

    public $lastTime;

    public $t;

    public $d;

    public $k;

    public $qk;

    /**
     * @var int 策划表配置的原始价格
     */
    public $rawPrice;

    public $oldPrice;

    public $newPrice;

    protected static $TableName = 'trade_price_adjust_log';

    protected static $Fields = [
        'id'       => 'id',
        'itemId'   => 'itemid',
        'refineLv' => 'refinelv',
        'lastTime' => 'last_time',
        't'        => 't',
        'd'        => 'd',
        'k'        => 'k',
        'qk'       => 'qk',
        'rawPrice' => 'p0',
        'oldPrice' => 'oldprice',
        'newPrice' => 'newprice',
    ];
}