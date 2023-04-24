<?php
namespace RO;

class MySQLi
{
    /**
     * @var \mysqli
     */
    protected $_mysql;

    protected $_config = [];

    protected $_isMaster;

    public $last_query;

    public $error;

    public $errno;

    public $error_list = [];

    protected $_retryNum = 0;

    public function __construct($config, $master = true)
    {
        $this->_config   = $config;
        $this->_isMaster = $master;
        $this->_connect();
    }

    public function query($sql, $resultMode = MYSQLI_STORE_RESULT)
    {
        $time = microtime(true);

        if (null === $this->_mysql && false === $this->_connect())
        {
            return false;
        }

        $this->errno      = 0;
        $this->error      = null;
        $this->error_list = [];

        $this->last_query = $sql;
        $rs = $this->_mysql->query($sql, $resultMode);
        if (($useTime = microtime(true) - $time) > 1)
        {
            \MyQEE\Server\Server::$instance->warn("MySQL慢查询，耗时: {$useTime}s, SQL: $sql");
        }

        if (false === $rs || null === $rs)
        {
            if ($this->_retryNum > 3)
            {
                $this->_retryNum = 0;
                @$this->_mysql->close();
                $this->_mysql = null;
                \MyQEE\Server\Server::$instance->warn("MySQL retry total {$this->_retryNum} : {$this->errno}, Error: {$this->error}, SQL: $sql");
                return false;
            }

            try
            {
                $this->errno      = $this->_mysql->errno;
                $this->error      = $this->_mysql->error;
                $this->error_list = $this->_mysql->error_list;
            }
            catch (\Exception $e)
            {
                $this->errno = 2099;
                $this->error = 'Code: '. $e->getCode() .', Msg: '. $e->getMessage();
            }
            \MyQEE\Server\Server::$instance->warn("MySQL ErrNO: {$this->errno}, Error: {$this->error}, SQL: $sql");
            if ($this->errno == 104 || ($this->errno >= 2000 && $this->errno < 2100) || false === $this->ping())
            {
                # 2006 server has gone away
                # 连接断开
                if ($this->_connect())
                {
                    $this->_retryNum++;
                    return $this->query($sql, $resultMode);
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        elseif ($this->_retryNum > 0)
        {
            $this->_retryNum = 0;
        }

        return $rs;
    }

    /**
     * 开启事务方法
     *
     * @param int  $flags
     * @param null $name
     * @return bool
     */
    public function begin_transaction($flags = null, $name = null)
    {
        if (null === $this->_mysql && false === $this->_connect())
        {
            return false;
        }

        $this->errno      = 0;
        $this->error      = null;
        $this->error_list = [];
        switch (func_num_args())
        {
            case 1:
                $rs = $this->_mysql->begin_transaction($flags);
                break;

            case 2:
                $rs = $this->_mysql->begin_transaction($flags, $name);
                break;

            default:
                $rs = $this->_mysql->begin_transaction();
                break;
        }
        if (true !== $rs)
        {
            $errno      = $this->_mysql->errno;
            $error      = $this->_mysql->error;
            $error_list = $this->_mysql->error_list;
            if ($this->_connect())
            {
                switch (func_num_args())
                {
                    case 1:
                        $rs = $this->_mysql->begin_transaction($flags);
                        break;

                    case 2:
                        $rs = $this->_mysql->begin_transaction($flags, $name);
                        break;

                    default:
                        $rs = $this->_mysql->begin_transaction();
                        break;
                }
                return $rs;
            }

            $this->errno      = $errno;
            $this->error      = $error;
            $this->error_list = $error_list;
        }

        return $rs;
    }

    public function __get($name)
    {
        if (null === $this->_mysql && false === $this->_connect())
        {
            return false;
        }

        return $this->_mysql->$name;
    }

    public function __call($name, $arguments)
    {
        if (null === $this->_mysql && false === $this->_connect())
        {
            return false;
        }

        if (count($arguments) == 0)
        {
            return $this->_mysql->$name();
        }

        return call_user_func_array([$this->_mysql, $name], $arguments);
    }

    public function ping()
    {
        return $this->_mysql->ping();
    }

    public function _connect()
    {
        if ($this->_isMaster)
        {
            $host = $this->_config['master'];
        }
        else
        {
            $rand = mt_rand(0, count($this->_config['slave']) - 1);
            $host = $this->_config['slave'][$rand];
        }

        if ($this->_mysql)
        {
            @$this->_mysql->close();
            $this->_mysql = null;
        }

        list($ip, $port) = explode(':', $host);
        $mysql = new \mysqli($ip, $this->_config['user'], $this->_config['pass'], $this->_config['db'], $port);

        if ($mysql->connect_errno)
        {
            return false;
        }
        
        $mysql->set_charset($this->_config['charset']);
        $mysql->options(MYSQLI_OPT_INT_AND_FLOAT_NATIVE, true);
        $this->_mysql = $mysql;

        return true;
    }

    /**
     * 根据一个数组构造出插入语句
     *
     * @param $db
     * @param array $data
     * @param bool $replace
     * @return string
     */
    public function composeInsertSql($db, array $data, $replace = false)
    {
        if (!$db)throw new \Exception('构造sql语句缺少 db 参数');
        $fields = [];
        $values = [];
        foreach ($data as $key => $value)
        {
            $fields[] = $key;
            $values[] = self::quote($value);
        }

        return ($replace ? 'REPLACE':'INSERT'). " INTO `{$db}` (`". implode('`, `', $fields) ."`) VALUES (" . implode(", ", $values) . ")";
    }

    /**
     * 根据一个数组构造出更新语句
     *
     * @param $db
     * @param array $data
     * @return string
     */
    public function composeUpdateSql($db, array $data)
    {
        if (!$db)throw new \Exception('构造sql语句缺少 db 参数');
        $values = [];
        foreach ($data as $key => $value)
        {
            $value = self::quote($value);

            $values[] = "`$key` = $value";
        }

        return "UPDATE `{$db}` SET ". implode(', ', $values);
    }

    /**
     * 转换为一个可用于SQL语句的字符串
     *
     * @param $value
     * @return int|null|string
     */
    public function quote($value)
    {
        $value = self::_getFieldTypeValue($value);
        if (is_string($value))
        {
            if (null === $this->_mysql && false === $this->_connect())
            {
                return "'". str_replace("'", "\\'", $value) . "'";
            }
            return "'". $this->_mysql->real_escape_string($value) . "'";
        }
        elseif (is_object($value))
        {
            if ($value instanceof \stdClass && isset($value->value))
            {
                return $value->value;
            }
            elseif (null === $this->_mysql && false === $this->_connect())
            {
                return "'". str_replace("'", "\\'", serialize($value)) . "'";
            }
            else
            {
                return "'". $this->_mysql->real_escape_string(serialize($value)) ."'";
            }
        }
        elseif (is_null($value))
        {
            return 'NULL';
        }
        else
        {
            # int, float
            return "'$value'";
        }
    }

    /**
     * 获取一个数据库存储的类型的数据
     *
     * @param $value
     * @return int|null|string
     */
    protected static function _getFieldTypeValue($value)
    {
        if (is_null($value))
        {
            return null;
        }

        if (is_numeric($value))
        {

        }
        elseif (is_bool($value))
        {
            $value = (int)$value;
        }
        elseif (is_array($value))
        {
            $value = json_encode($value, JSON_UNESCAPED_UNICODE);
        }
        elseif (is_object($value))
        {
            if ($value instanceof \DrSlump\Protobuf\Message)
            {
                try
                {
                    $tmp        = new \stdClass();
                    $tmp->value = '0x'. bin2hex($value->serialize());
                    $value      = $tmp;
                }
                catch (\Exception $e)
                {
                    return false;
                }
            }
            elseif ($value instanceof \stdClass && isset($value->value))
            {
                # 返回原来
                return $value;
            }
            else
            {
                return serialize($value);
            }
        }
        else
        {
            $value = (string)$value;
        }

        return $value;
    }
}