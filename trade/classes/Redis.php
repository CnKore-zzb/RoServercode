<?php
namespace RO;

class Redis
{
    /**
     * @var \Redis
     */
    protected $_redis;
    protected $_host;
    protected $_port;
    protected $_timeout;
    protected $_retry;
    protected $_clusterHosts;
    protected $_options = [];

    public function __construct($arg1 = null, $hosts = null)
    {
        if (is_array($hosts))
        {
            # 集群
            $this->_redis        = new \RedisCluster($arg1, $hosts);
            $this->_clusterHosts = $hosts;
        }
        else
        {
            $this->_redis = new \Redis();
        }

        $this->_resetOpt();
    }

    /**
     * @param string $host
     * @param int    $port
     * @param float  $timeout
     * @param int    $retryInterval
     * @return bool
     */
    public function connect($host, $port = 6379, $timeout = 0.0, $retryInterval = 0)
    {
        $this->_host    = $host;
        $this->_port    = $port;
        $this->_timeout = $timeout;
        $this->_retry   = $retryInterval;

        $rs = $this->_redis->connect($host, $port, $timeout, $retryInterval);
        if ($rs)
        {
            $this->_resetOpt();
        }
        return $rs;
    }

    public function close()
    {
        $this->_redis->close();
    }

    /**
     * @param string $key
     * @return bool|string
     * @throws \RedisException
     */
    public function get($key)
    {
        try
        {
            $rs = $this->_redis->get($key);
            return $rs;
        }
        catch (\RedisException $e)
        {
            if ($this->_reConnect())
            {
                return $this->_redis->get($key);
            }
            else
            {
                throw $e;
            }
        }
    }

    /**
     * @param string $key
     * @param string $value
     * @param int    $timeout
     * @return bool
     * @throws \RedisException
     */
    public function set($key, $value, $timeout = 0)
    {
        try
        {
            $time = microtime(true);
            $rs   = $this->_redis->set($key, $value, $timeout);
            if (($useTime = microtime(true) - $time) > 1)
            {
                \MyQEE\Server\Server::$instance->warn("Redis set $key, $value, $timeout 时间过长，耗时: {$useTime}s");
            }

            return $rs;
        }
        catch (\RedisException $e)
        {
            if ($this->_reConnect())
            {
                return $this->_redis->set($key, $value, $timeout);
            }
            else
            {
                throw $e;
            }
        }
    }

    /**
     * @param string $key
     * @param string $hashKey
     * @param int    $value
     * @return int
     * @throws \RedisException
     */
    public function hIncrBy($key, $hashKey, $value = 1)
    {
        try
        {
            $time = microtime(true);
            $rs   = $this->_redis->hIncrBy($key, $hashKey, $value);
            if (($useTime = microtime(true) - $time) > 1)
            {
                \MyQEE\Server\Server::$instance->warn("Redis hIncrBy $key, $hashKey, $value 时间过长，耗时: {$useTime}s");
            }
            return $rs;
        }
        catch (\RedisException $e)
        {
            if ($this->_reConnect())
            {
                return $this->_redis->hIncrBy($key, $hashKey, $value);
            }
            else
            {
                throw $e;
            }
        }
    }

    /**
     * @param string $key
     * @param string $hashKey
     * @return string
     * @throws \RedisException
     */
    public function hGet($key, $hashKey)
    {
        try
        {
            $rs = $this->_redis->hGet($key, $hashKey);

            return $rs;
        }
        catch (\RedisException $e)
        {
            if ($this->_reConnect())
            {
                return $this->_redis->hGet($key, $hashKey);
            }
            else
            {
                throw $e;
            }
        }
    }

    /**
     * @param $key
     * @return array
     * @throws \RedisException
     */
    public function hGetAll($key)
    {
        try
        {
            $rs = $this->_redis->hGetAll($key);

            return $rs;
        }
        catch (\RedisException $e)
        {
            if ($this->_reConnect())
            {
                return $this->_redis->hGetAll($key);
            }
            else
            {
                throw $e;
            }
        }
    }

    /**
     * @param string $key
     * @param string $hashKey
     * @param string $value
     * @return int
     * @throws \RedisException
     */
    public function hSet($key, $hashKey, $value)
    {
        try
        {
            $time = microtime(true);
            $rs   = $this->_redis->hSet($key, $hashKey, $value);
            if (($useTime = microtime(true) - $time) > 1)
            {
                \MyQEE\Server\Server::$instance->warn("Redis hSet $key, $hashKey, $value 时间过长，耗时: {$useTime}s");
            }

            return $rs;
        }
        catch (\RedisException $e)
        {
            if ($this->_reConnect())
            {
                return $this->_redis->hSet($key, $hashKey, $value);
            }
            else
            {
                throw $e;
            }
        }
    }

    /**
     * @param string $key
     * @param int    $ttl
     * @return bool
     * @throws \RedisException
     */
    public function expire($key, $ttl)
    {
        try
        {
            $rs = $this->_redis->expire($key, $ttl);

            return $rs;
        }
        catch (\RedisException $e)
        {
            if ($this->_reConnect())
            {
                return $this->_redis->expire($key, $ttl);
            }
            else
            {
                throw $e;
            }
        }
    }

    /**
     * @param string $name
     * @param string $value
     * @return bool
     * @throws \RedisException
     */
    public function setOption($name, $value)
    {
        $this->_options[$name] = $value;

        return $this->_redis->setOption($name, $value);
    }

    protected function _reConnect()
    {
        if (null !== $this->_clusterHosts)
        {
            try
            {
                $redis        = new \RedisCluster(null, $this->_clusterHosts);
                $this->_redis = $redis;
                $this->_resetOpt();
                return true;
            }
            catch (\RedisException $e)
            {
                \MyQEE\Server\Server::$instance->warn('连接 Redis 失败 :'. json_encode(\MyQEE\Server\Server::$instance->config['redis']) .'. err: '. $e->getMessage());
                return false;
            }
        }
        else
        {
            $rs = $this->_redis->connect($this->_host, $this->_port, $this->_timeout, $this->_retry);
            if ($rs)
            {
                $this->_resetOpt();
            }
            else
            {
                \MyQEE\Server\Server::$instance->warn('连接 Redis 失败 :'. json_encode(\MyQEE\Server\Server::$instance->config['redis']));
            }

            return $rs;
        }
    }

    public function __get($name)
    {
        return $this->_redis->$name;
    }

    public function __call($name, $arguments)
    {
        try
        {
            if (count($arguments) == 0)
            {
                return $this->_redis->$name();
            }

            return call_user_func_array([$this->_redis, $name], $arguments);
        }
        catch (\RedisException $e)
        {
            if ($this->_reConnect())
            {
                if (count($arguments) == 0)
                {
                    return $this->_redis->$name();
                }

                return call_user_func_array([$this->_redis, $name], $arguments);
            }
            else
            {
                throw $e;
            }
        }
    }

    public function ping()
    {
        return $this->_redis->ping();
    }

    protected function _resetOpt()
    {
        if ($this->_options)foreach ($this->_options as $k => $v)
        {
            $this->_redis->setOption($k, $v);
        }
    }
}