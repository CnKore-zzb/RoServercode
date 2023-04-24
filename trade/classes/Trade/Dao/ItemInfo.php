<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/9
 * Time: 下午5:18
 */

namespace RO\Trade\Dao;

use RO\Trade\Server;

class ItemInfo extends Dao
{
    public $itemId;

    public $lastServerPrice;

    public $lastCalcPriceTime;

    public $cycle;

    public $upRatio;

    public $downRatio;

    public $maxPrice;

    protected static $IdField = 'itemid';

    protected static $TableName = 'trade_item_info';

    protected static $Fields = [
        'itemId'            => 'itemid',
        'lastServerPrice'   => 'last_server_price',
        'lastCalcPriceTime' => 'last_calc_price_time',
        'cycle'             => 't',
        'upRatio'           => 'up_ratio',
        'downRatio'         => 'down_ratio',
        'maxPrice'          => 'max_price',
    ];

    public static function getList($params)
    {
        $paging = '';
        if (isset($params['page']))
        {
            $page = (int) $params['page'];
            $limit = 30;
            if (isset($params['limit'])) $limit = (int) $params['limit'];
            $limit  = is_numeric($limit) ? $limit : 30;
            $page   = $page == 0 ? 1 : (int) $page;
            $offset = $limit * ($page - 1);
            $paging .= " LIMIT $limit OFFSET $offset";
        }

        $sql = "SELECT * FROM `" . self::$TableName . "`";
        $sql = $paging ? $sql . $paging : $sql;

        $rs = Server::$mysqlMaster->query($sql);
        $ret = [];
        if ($rs && $rs->num_rows)
        {
            while ($data = $rs->fetch_assoc())
            {
                $ret[] = $data;
            }
        }
        else
        {
            return false;
        }

        return $ret;
    }

    public static function count()
    {
        $sql = "SELECT COUNT(1) AS `count` FROM `" . self::$TableName . "`";
        $rs = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            $ret = $rs->fetch_assoc();
        }
        else
        {
            return false;
        }

        $count = intval($ret['count']);
        return $count;
    }
}