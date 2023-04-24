<?php
namespace RO\Trade;

/**
 * 异步 Fluent 客户端
 *
 * @package RO\Trade
 */
class FluentClient
{
    public $tag;
    
    public $host;

    public $port;

    /**
     * @var \Swoole\Client
     */
    public $client;

    /**
     * 文件队列
     * 
     * @var \SplQueue
     */
    protected $bufFiles;

    /**
     * Buffer内容
     * 
     * @var string
     */
    protected $buffer = '';

    /**
     * 当前 buffer 对应的文件
     *
     * @var string
     */
    protected $bufferFile;

    /**
     * 当前 buffer 对应文件大小
     *
     * @var int
     */
    protected $bufferSize = 0;

    /**
     * 当前 buffer 对应的文件指针
     *
     * @var resource
     */
    protected $bufferResource;

    /**
     * 是否开启buffer实时保存
     *
     * @var bool
     */
    protected $isSaveBufferRuntime = false;

    /**
     * ACK
     * 
     * @var string
     */
    protected $taskAck;

    protected $taskData = null;

    protected $taskBufferFile = null;

    protected $taskTryNum = 0;

    protected $taskIsWaiting = false;

    /**
     * 发送信息时间
     * 
     * @var int
     */
    protected $taskSendTime = 0;

    /**
     * 超时定时器
     * 
     * @var int
     */
    protected static $timeTickTimeout;

    protected static $timeTick;

    protected static $bufDir;
    
    /**
     * ACK 超时时间
     * 
     * @var int
     */
    protected static $ackTimeOut = 180;

    /**
     * 单个buffer长度
     * 
     * @var int
     */
    protected static $maxLenPerBuffer = 4194304;

    protected static $instance = [];
    
    function __construct($host, $port, $tag)
    {
        if (isset(self::$instance[$tag]))throw new \Exception("已经定义过一个相同Task的Fluent对象");

        if (null === self::$bufDir)
        {
            self::$bufDir = Server::$dataDir;        # 文件目录
        }
        $this->host = $host;
        $this->port = $port;
        $this->tag  = $tag;

        $this->bufFiles = new \SplQueue();

        # 读取所有 buffer 文件
        foreach (glob(self::$bufDir ."fluent.{$this->tag}.*.buf") as $item)
        {
            if (filesize($item) === 0)
            {
                unlink($item);
                continue;
            }
            $this->bufFiles->enqueue($item);
        }

        if (null === $this::$timeTickTimeout)
        {
            self::$timeTickTimeout = swoole_timer_tick(self::$ackTimeOut * 1000 / 2, function()
            {
                foreach (self::$instance as $fluent)
                {
                    /**
                     * @var FluentClient $fluent
                     */
                    if (true === $fluent->taskIsWaiting && time() - $fluent->taskSendTime >= self::$ackTimeOut)
                    {
                        # 超时，断开重新连接
                        $fluent->client->close();
                        $fluent->connect();
                    }
                }
            });

            # 每秒钟定时发送
            self::$timeTick = swoole_timer_tick(1000, function()
            {
                foreach (self::$instance as $fluent)
                {
                    /**
                     * @var FluentClient $fluent
                     */
                    $fluent->send();
                }
            });
        }

        $this->client = new \Swoole\Client(SWOOLE_SOCK_TCP, SWOOLE_SOCK_ASYNC);  # 客户端对象
        $this->client->on("connect", [$this, 'onConnect']);
        $this->client->on("receive", [$this, 'onReceive']);
        $this->client->on("error",   [$this, 'onError']);
        $this->client->on("close",   [$this, 'onClose']);
        $this->client->set(['socket_buffer_size' => 1024 * 1024 * 20, 'open_tcp_nodelay' => true]);
        $this->connect();

        self::$instance[$tag] = $this;
    }

    /**
     * 销毁当前对象
     */
    public function destroy()
    {
        $this->saveAllBuffer();

        unset(self::$instance[$this->tag]);

        if ($this->client->isConnected())
        {
            $this->client->close();
        }

        if (null !== $this->bufferResource)
        {
            $this->bufferResource = null;
            $this->bufferFile     = null;
            $this->bufferSize     = 0;
            fclose($this->bufferResource);
        }

        # 没有任何Fluent，移除监听
        if (count(self::$instance) == 0)
        {
            if (null !== self::$timeTick)
            {
                self::$timeTick = null;
                swoole_timer_clear(self::$timeTick);
            }

            if (null !== self::$timeTickTimeout)
            {
                self::$timeTickTimeout = null;
                swoole_timer_clear(self::$timeTickTimeout);
            }
        }
    }

    /**
     * 是否开启立即保存buffer到文件
     *
     *  `$this->saveBufferRuntime()`      返回当前状态
     *  `$this->saveBufferRuntime(true)`  设置开启, 成功返回 true
     *  `$this->saveBufferRuntime(false)` 设置关闭, 成功返回 true
     *
     * @param null $isOpen
     * @return bool 状态
     */
    public function saveBufferRuntime($isOpen = null)
    {
        if (null === $isOpen)
        {
            return $this->isSaveBufferRuntime;
        }
        elseif (true === $isOpen)
        {
            if (null === $this->bufferFile)
            {
                $this->createNewRunTimeBuffer();
            }

            $this->isSaveBufferRuntime = true;
            return true;
        }
        else
        {
            $this->isSaveBufferRuntime = false;
            if ($this->bufferFile)
            {
                fclose($this->bufferResource);
                unlink($this->bufferFile);
                $this->bufferFile = null;
            }
            return true;
        }
    }

    /**
     * 入队列
     * 
     * @param string|array $data 内容
     * @return bool
     */
    public function push($data)
    {
        if (is_array($data))
        {
            $time = isset($data['time']) && $data['time'] ? $data['time'] : time();
            $data = msgpack_pack([$time, $data]);
        }

        # 立即保存到buffer中
        if (true === $this->isSaveBufferRuntime)
        {
            if (null === $this->bufferResource)
            {
                # 创建新的buffer
                if (false === $this->createNewRunTimeBuffer())
                {
                    return false;
                }
            }

            $wLen = fwrite($this->bufferResource, $data, $len = strlen($data));
            if ($wLen < $len)
            {
                # 写入长度不够，硬盘满了？
                Server::$instance->warn("写入buffer文件长度有误，文件长度 $len, 写入: $wLen");

                # 重新写入空内容，避免
                fseek($this->bufferResource, $this->bufferSize);
                fwrite($this->bufferResource, str_pad('', $wLen, "\0"), $wLen);
                return false;
            }

            $this->bufferSize += $wLen;
            $this->buffer     .= $data;

            if ($this->bufferSize >= self::$maxLenPerBuffer)
            {
                # 超过最大容量
                if (null === $this->taskData && $this->bufFiles->isEmpty())
                {
                    # 列队里没有要处理的可以立即发送数据
                    $this->send();
                }
                else
                {
                    $this->createNewRunTimeBuffer();
                }
            }
            return true;
        }

        $this->buffer .= $data;

        # 一次性写入
        if (strlen($this->buffer) >= self::$maxLenPerBuffer)
        {
            file_put_contents($file = $this->bufferFile ?: $this->getBufferFileName(), $this->buffer);
            $this->bufFiles->enqueue($file);
            $this->buffer     = '';
            $this->bufferSize = 0;
            $this->bufferFile = null;
        }

        return true;
    }

    /**
     * 创建一个新的buffer
     *
     * @return bool
     */
    protected function createNewRunTimeBuffer()
    {
        $file = $this->getBufferFileName();
        $fp   = @fopen($file, 'w');
        if (false === $this->bufferResource)
        {
            Server::$instance->warn("创建Fluent buffer文件失败, {$file}");
            return false;
        }

        # 关闭旧的buffer
        if (null !== $this->bufferResource)
        {
            $this->buffer     = '';
            $this->bufferSize = 0;
            $this->bufFiles->enqueue($this->bufferFile);
            fclose($this->bufferResource);
        }

        $this->bufferFile     = $file;
        $this->bufferResource = $fp;
        if ($len = strlen($this->buffer))
        {
            $this->bufferSize = fwrite($fp, $this->buffer, $len);
        }

        return true;
    }

    /**
     * 获取一个新的buffer文件名
     *
     * @return string
     */
    protected function getBufferFileName()
    {
        return self::$bufDir ."fluent.{$this->tag}.". intval(microtime(1) * 1000) . mt_rand(1000, 9999) .'.buf';
    }

    /**
     * 执行发送数据任务
     */
    public function send()
    {
        if (null !== $this->taskData)
        {
            if (false === $this->client->isConnected())return;

            if (false === $this->taskIsWaiting)
            {
                if ($this->doSend())
                {
                    $this->taskTryNum++;
                }
            }

            return;
        }

        # 从文件队列里发送
        if (!$this->bufFiles->isEmpty())
        {
            if (false === $this->client->isConnected())return;

            $file = $this->bufFiles->dequeue();
            $data = file_get_contents($file);
            if (false === $data)
            {
                Server::$instance->warn("读取buffer文件失败 $file");
            }
            $this->taskData       = $data;
            $this->taskBufferFile = $file;

            $this->doSend();

            return;
        }

        # 从当前buffer里发送
        if ('' !== $this->buffer)
        {
            if (false === $this->client->isConnected())return;

            $this->taskData       = $this->buffer;
            $this->taskBufferFile = $this->bufferFile;
            $this->buffer         = '';
            if (null !== $this->bufferResource)
            {
                fclose($this->bufferResource);
                $this->bufferResource = null;
                $this->bufferFile     = null;
                $this->bufferSize     = 0;
            }

            $this->doSend();
        }
    }

    /**
     * 发送数据
     *
     * @param $data
     * @return bool
     */
    protected function doSend()
    {
        if (null === $this->taskData)return false;

        $ack = microtime(1);
        $rs = $this->client->send(msgpack_pack([$this->tag, $this->taskData, ['chunk' => $ack]]));
        if ($rs)
        {
            $this->taskAck       = $ack;
            $this->taskIsWaiting = true;
            $this->taskSendTime  = time();
        }

        return $rs;
    }

    public function connect()
    {
        if (false == $this->client->isConnected())
        {
            $this->client->connect($this->host, $this->port);
        }
    }

    /**
     * 将buffer保存到文件里
     */
    public function saveAllBuffer()
    {
        if (true === $this->isSaveBufferRuntime && null !== $this->bufferResource)
        {
            # 开启了实时保存 buffer 功能, 不用在保存了
            return true;
        }

        if (null !== $this->taskData)
        {
            # 将没有保存的buf文件保存
            $buf = $this->taskData;
            if ('' !== $this->buffer && strlen($this->buffer) + strlen($buf) < 4194304)
            {
                # 合并到一起
                $buf         .= $this->buffer;
                $this->buffer = '';
            }
            $this->taskBufferFile = $this->taskBufferFile ?: $this->getBufferFileName();
            $rs1 = strlen($buf) === file_put_contents($this->taskBufferFile, $buf);

            if (false === $rs1)return false;
        }

        if ('' !== $this->buffer)
        {
            return strlen($this->buffer) === file_put_contents($this->bufferFile = $this->bufferFile ?: $this->getBufferFileName(), $this->buffer);
        }
        else
        {
            return true;
        }
    }

    public function onConnect($cli)
    {
        Server::$instance->debug("Fluent {$this->host}:{$this->port} is connected.");
        $this->taskIsWaiting = false;
        $this->send();
    }
    
    public function onReceive($cli, $data)
    {
        $this->client->wait = false;
        $tmp = @msgpack_unpack($data);

        if (($tmp && $tmp['ack'] == $this->taskAck) || $this->taskTryNum > 3)
        {
            # 投递成功, 清理数据
            if ($file = $this->taskBufferFile)
            {
                unlink($file);
            }
            $this->taskTryNum     = 0;
            $this->taskAck        = null;
            $this->taskData       = null;
            $this->taskBufferFile = null;
            $this->taskIsWaiting  = false;

            # 接着发送
            $this->send();
        }
        else
        {
            # 投递失败，重新投递
            Server::$instance->warn("解析 msgPack 失败: ". $data);
            $this->client->close();
            $this->connect();
        }
    }
    
    public function onClose($cli)
    {
        Server::$instance->debug("Fluent {$this->host}:{$this->port} is close, retry connect.");
        swoole_timer_after(1000, function()
        {
            $this->connect();
        });
    }

    public function onError($cli)
    {
        /**
         * @var \Swoole\Client $cli
         */
        Server::$instance->debug("Fluent {$this->host}:{$this->port} error, code: {$cli->errCode}, retry connect.");
        swoole_timer_after(3000, function()
        {
            $this->connect();
        });
    }
}