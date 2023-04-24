<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/13
 * Time: 下午8:58
 */

namespace RO\Trade\Dao;

use RO\Trade\Server;

class RecordNameInfo extends Dao
{
    public $id;

    public $recordId;

    public $name;

    public $zoneId;

    public $count;

    public $type;

    /* 类型: 0:购买交易记录的玩家信息 1:出售交易记录的玩家信息 */
    const TYPE_BUY = 0;
    const TYPE_SELL = 1;

    protected static $TableName = 'trade_record_name_info';

    protected static $Fields = [
        'id'       => 'id',
        'recordId' => 'record_id',
        'name'     => 'name',
        'zoneId'   => 'zone_id',
        'count'    => 'count',
        'type'     => 'type'
    ];

    /**
     *
     *
     * @param int $id
     * @param int $type
     * @return bool|null|RecordNameInfo
     */
    public static function getByRecordId($id, $type = self::TYPE_BUY)
    {
        if (!$id)return false;

        $id   = self::_quoteValue($id);
        $type = self::_quoteValue($type);

        $rs  = Server::$mysqlMaster->query("SELECT * FROM `" . static::$TableName . "` WHERE `record_id` = {$id} AND `type` = {$type} LIMIT 1");
        if ($rs)
        {
            if ($rs->num_rows)
            {
                $ret = $rs->fetch_object(static::class);
            }
            else
            {
                $ret = null;
            }
            $rs->free();

            /**
             * @var self $ret
             */
            return $ret;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error);

            return false;
        }
    }

    /**
     * 分页获取交易记录玩家信息
     *
     *
     * @param $recordId
     * @param $type
     * @param int $page
     * @return RecordNameInfo[]|false
     */
    public static function getNameInfoList($recordId, $type = self::TYPE_BUY, $page = 1)
    {
        $recordId = self::_quoteValue($recordId);
        $type     = self::_quoteValue($type);

        $page     = $page ?: 1;
        $listRows = Server::$configExchange['PageNumber'];
        $listRows = isset($listRows) ? $listRows : 20;
        $offset   = $listRows * ((int)$page - 1);

        $sql = "SELECT * FROM `" . static::$TableName . "` WHERE `record_id` = {$recordId} AND `type` = {$type} LIMIT {$offset}, {$listRows}";
        $rs  = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            $list = [];
            while ($data = $rs->fetch_object(static::class))
            {
                $list[] = $data;
            }
            $rs->free();

            /**
             * @var self $ret
             */
            return $list;
        }
        else
        {
            Server::$instance->warn(Server::$mysqlMaster->error . ' sql:' . $sql);

            return false;
        }
    }

    /**
     * 获取交易记录玩家信息总数
     *
     * @param $recordId
     * @param int $type
     * @return false|int
     */
    public static function getNameInfoCount($recordId, $type = self::TYPE_BUY)
    {
        $rs = Server::$mysqlMaster->query("SELECT COUNT(1) as `count` FROM `". self::$TableName ."` WHERE `record_id` = {$recordId} AND `type` = {$type}");
        if ($rs)
        {
            $q = $rs->fetch_assoc();
            $rs->free();
            return $q['count'];
        }

        return false;
    }
}