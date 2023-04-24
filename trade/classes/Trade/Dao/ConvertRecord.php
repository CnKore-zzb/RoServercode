<?php
namespace RO\Trade\Dao;

use RO\Trade\Server;

/**
 * 处理服务器交互数据
 *
 * @package RO\Trade\Dao
 */
class ConvertRecord extends Dao
{
    public $id;

    public $time;

    public $charId;

    public $type;

    public $status = self::STATUS_DOING;

    /**
     * 当前类型用到的一些数据参数
     *
     * @var string
     */
    public $vars = '';

    /**
     * 发送的命令的ProtoBuf字符串
     *
     * @var string
     */
    public $cmd = '';

    protected static $TableName = 'trade_convert_record';

    protected static $Fields = [
        'id'     => 'id',
        'time'   => 'time',
        'charId' => 'charid',
        'type'   => 'type',
        'status' => 'status',
        'vars'   => 'vars',
        'cmd'    => 'cmd',
    ];

    //const TYPE_SELL_ADD = 101;      # 发布物品请求服务器扣除物品

    const TYPE_REDUCE_MONEY = 201;      # 购买物品请求服务器扣除Zeny

    const STATUS_DOING = 0;             # 正在处理中

    function __construct($data = null)
    {
        parent::__construct($data);

        if ($this->vars)
        {
            if ($this->vars[0] == '{' || $this->vars[0] == '[')
            {
                $tmp = @json_decode($this->vars, true);
                if ($tmp)
                {
                    $this->vars = $tmp;
                }
            }
            else
            {
                $tmp = @unserialize($this->vars);
                if (false !== $tmp)
                {
                    $this->vars = $tmp;
                }
            }
        }
    }

    public function insert()
    {
        if (!$this->time)
        {
            $this->time = time();
        }

        return parent::insert();
    }

    /**
     * 更新返回信息
     *
     * @param $id
     * @param $ret
     * @return bool|int
     */
    public static function updateRst($id, $charId, $ret)
    {
        $id     = self::_quoteValue($id);
        $ret    = self::_quoteValue($ret);
        $charId = self::_quoteValue($charId);
        $time = time();
        $sql  = "UPDATE `". static::$TableName ."` SET `ret` = {$ret}, `time` = '{$time}' WHERE `". static::$IdField ."` = $id AND `charid` = {$charId}";
        $rs   = Server::$mysqlMaster->query($sql);
        if ($rs)
        {
            # 更新进去
            return Server::$mysqlMaster->affected_rows;
        }
        else
        {
            Server::$instance->warn($sql .' '. Server::$mysqlMaster->error);
            return false;
        }
    }
}