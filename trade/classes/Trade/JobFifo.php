<?php
namespace RO\Trade;

/**
 * 文件型FIFO队列
 *
 *
 */
class JobFifo
{
    /**
     * $_file_data, 数据文件的路径
     */
    private $file = '';

    private static $instance = [];

    const CALL_DELAY_UP_STOCK    = 1;
    const CALL_DELAY_INSERT_SOLD = 2;

    /**
     * @param $file
     * @return self
     */
    public static function instance($file)
    {
        if (!isset(self::$instance[$file]))
        {
            self::$instance[$file] = new self($file);
        }

        return self::$instance[$file];
    }

    public function __construct($file)
    {
        $this->file = Server::$dataDir. $file;
    }

    /**
     * pop, 先进先出顺序执行
     *
     * @param bool $unlinkBufFile
     */
    public function exec($unlinkBufFile = false)
    {
        if (!file_exists($this->file))
        {
            # 文件不存在
            return;
        }

        $fp = fopen($this->file, 'r+');
        if (false === $fp)
        {
            clearstatcache(false, $this->file);
            if (!file_exists($this->file))
            {
                # 文件不存在
                return;
            }
            Server::$instance->warn("打开JobFifo文件失败，{$this->file}");
            return;
        }

        $index = $this->file . '.idx';
        $pos   = file_exists($index) ? intval(file_get_contents($index)) : 0;
        $data  = '';

        fseek($fp, $pos);
        while (false !== ($tmp = fgets($fp)))
        {
            if ($data === '')
            {
                $data = $tmp;
            }
            else
            {
                $data .= $tmp;
                unset($tmp);
            }

            if (substr($data, -3) === "\0\r\n")
            {
                # 读取到结尾
                $arr = @json_decode(trim($data), true);
                if (false === $arr || null === $arr)
                {
                    Server::$instance->warn("解析JobFifo失败，$data");
                    # 遇到无法处理的错误，需要停机人工修复
                    Server::$instance->server->shutdown();
                    break;
                }

                switch ($arr['type'])
                {
                    case self::CALL_DELAY_UP_STOCK:
                        $rs = ActBuy::delayUpStock($arr['data']);
                        break;

                    case self::CALL_DELAY_INSERT_SOLD:
                        $rs = ActBuy::delayInsertSold($arr['data']);
                        break;

                    default:
                        Server::$instance->warn("忽略未知JobFifo类型数据，{$arr['type']}, data: $data");
                        $rs = true;
                        break;
                }
                unset($arr);

                if (false !== $rs)
                {
                    $data = '';
                    # 设置 pos
                    file_put_contents($index, $pos = ftell($fp));

                    # 等待一会再处理
                    usleep(6000);
                }
                else
                {
                    $pos2 = ftell($fp);
                    $err  = $this->file.'.error-context.txt';
                    file_put_contents($err, $data);
                    Server::$instance->warn("执行JobFifo失败，你可以运行 echo '". $pos2 ."' > {$this->file}.idx 忽略此任务, 查看执行错误的数据内容: cat {$err}");
                    break;
                }
            }
        }

        # 没有再读取到内容
        if ($data === '')
        {
            clearstatcache(false, $this->file);
            flock($fp, LOCK_EX);
            if ($pos == filesize($this->file))
            {
                # 已经操作完成
                if (true === $unlinkBufFile)
                {
                    # 删除文件
                    $this->del();
                }
                else
                {
                    # 清空文件
                    if (ftruncate($fp, 0) && file_exists($index))
                    {
                        unlink($index);
                    }
                }
            }
            flock($fp, LOCK_UN);
        }
    }

    /**
     * 入队列
     *
     * @param array $data
     * @return bool
     */
    public function push($data, $callbackType)
    {
        $str  = json_encode(['type' => $callbackType, 'data' => $data], JSON_UNESCAPED_UNICODE) . "\0\r\n";
        return file_put_contents($this->file, $str, FILE_APPEND | LOCK_EX) === strlen($str);
    }

    /**
     * del, 清空一个队列
     */
    public function del()
    {
        unlink($this->file);
        $fIndex = $this->file .'.idx';
        if (file_exists($fIndex))
        {
            unlink($fIndex);
        }
        $fErr = $this->file .'.error-context.txt';
        if (file_exists($fErr))
        {
            unlink($fErr);
        }

        return true;
    }
}