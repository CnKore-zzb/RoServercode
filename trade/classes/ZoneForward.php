<?php
namespace RO;

use Exception;
use RO\Cmd\RegistRegionSystemCmd;
use RO\Trade\Server;

/**
 * Session 场景服务器到这里来的通讯对象
 *
 * @package RO
 */
abstract class ZoneForward
{
    public $fd;

    public $fromId;

    /**
     * Cmd ProtoBuf 字符串
     *
     * @var string
     */
    public $cmd;

    public $param;

    public $zoneId;

    public $charId;

    public $proto;

    const MIN_COMPRESS_SIZE    = 48; // 数据包最小压缩长度
    const PACKET_FLAG_COMPRESS = 1;  // 压缩
    const PACKET_FLAG_ENCRYPT  = 2;  // 加密
    const MAX_BUFSIZE = 65535;       // 包体最大长度

    const ETRADETYPE_SELL                      = 1;        //上架
    const ETRADETYPE_CANCEL                    = 2;        //撤单下架
    const ETRADETYPE_TRUEBUY                   = 3;        //成功购买
    const ETRADETYPE_TRUESELL                  = 4;        //成功出售
    const ETRADETYPE_RESELL                    = 5;        //重新上架
    const ETRADETYPE_PUBLICITY_SELL            = 6;        //公示上架
    const ETRADETYPE_RESELL_AUTO               = 7;        //公示后自动上架
    const ETRADETYPE_PUBLICITY_SEIZURE         = 8;        //公示期扣押钱
    const ETRADETYPE_PUBLICITY_RETURN          = 9;        //公示期返回钱
    const ETRADETYPE_PUBLICITY_BUY             = 10;       //公示期购买成功
    const ETRADETYPE_TAKE                      = 11;       //交易所领取
    const ETRADETYPE_TAKE_SELL_MONEY           = 12;       //领取出售所得银币
    const ETRADETYPE_TAKE_BUY_ITEM             = 13;       //领取购买所得道具
    const ETRADETYPE_TAKE_RETURN_MONEY         = 14;       //领取公示期返还所得银币
    const ETRADETYPE_TAKE_PUBLICITY_BUY_ITEM   = 15;       //领取公示期所得道具
    const ETRADETYPE_TAKE_PUBLICITY_SELL_MONEY = 16;       //领取公示期出售所得银币

    const ETRADETYPE_BOOTH_SELL                      = 18; //上架
    const ETRADETYPE_BOOTH_CANCEL                    = 19; //撤单下架
    const ETRADETYPE_BOOTH_TRUE_BUY                  = 20; //成功购买
    const ETRADETYPE_BOOTH_TRUE_SELL                 = 21; //成功出售
    const ETRADETYPE_BOOTH_RESELL                    = 22; //重新上架
    const ETRADETYPE_BOOTH_PUBLICITY_SELL            = 23; //公示上架
    const ETRADETYPE_BOOTH_RESELL_AUTO               = 24; //公示后自动上架
    const ETRADETYPE_BOOTH_PUBLICITY_SEIZURE         = 25; //公示期扣押钱
    const ETRADETYPE_BOOTH_PUBLICITY_RETURN          = 26; //公示期返回钱
    const ETRADETYPE_BOOTH_PUBLICITY_BUY             = 27; //公示期购买成功
    const ETRADETYPE_BOOTH_PUBLICITY_TRUE_SELL       = 28; //公示期出售成功
    const ETRADETYPE_BOOTH_TAKE                      = 29; //交易所领取
    const ETRADETYPE_BOOTH_TAKE_SELL_MONEY           = 30; //领取出售所得银币
    const ETRADETYPE_BOOTH_TAKE_BUY_ITEM             = 31; //领取购买所得道具
    const ETRADETYPE_BOOTH_TAKE_RETURN_MONEY         = 32; //领取公示期返还所得银币
    const ETRADETYPE_BOOTH_TAKE_PUBLICITY_BUY_ITEM   = 33; //领取公示期所得道具
    const ETRADETYPE_BOOTH_TAKE_PUBLICITY_SELL_MONEY = 34; //领取公示期出售所得银币


    /**
     * ZoneForward constructor.
     *
     * @param $fd
     * @param $fromId
     * @param \RO\Cmd\SessionForwardUsercmdTrade|\RO\Cmd\SessionForwardScenecmdTrade $obj
     */
    function __construct($fd, $fromId, $obj)
    {
        $this->fd     = $fd;
        $this->fromId = $fromId;
        $this->zoneId = $obj->zoneid;
        $this->charId = $obj->charid;
        $buf          = @unpack('Ccmd/Cparam/a*proto', $obj->data);
        if ($buf)
        {
            $this->cmd    = $buf['cmd'];
            $this->param  = $buf['param'];
            $this->proto  = $buf['proto'];
        }
    }

    /**
     * 尝试抢一个订单的锁，避免同时操作
     *
     * @param int $itemId
     * @param int $goodsId
     * @param int $timeout 自动释放时长
     * @return bool
     */
    public static function tryLockGoods($itemId, $goodsId = 0, $timeout = 60)
    {
        try
        {
            $key = self::getLockGoodsKey($itemId, $goodsId);
            return Server::$redis->set($key, 1, ['nx', 'ex' => $timeout]);   # 抢个网络锁
        }
        catch (Exception $e)
        {
            # Redis 连接失败
            Server::$instance->warn("[redis] 尝试设置物品锁失败, itemId: $itemId, goodsId: $goodsId, msg: ". $e->getMessage());
            return false;
        }
    }

    /**
     * 锁订单30天（系统存在异常会执行）
     *
     * @param int $itemId
     * @param int $goodsId
     * @return bool
     */
    public static function lockGoods($itemId, $goodsId = 0)
    {
        try
        {
            $key = self::getLockGoodsKey($itemId, $goodsId);
            return Server::$redis->set($key, 1, 86400 * 30);
        }
        catch (Exception $e)
        {
            # Redis 连接失败
            Server::$instance->warn("[redis] 设置物品锁失败, itemId: $itemId, goodsId: $goodsId, msg: ". $e->getMessage());
            return false;
        }
    }

    /**
     * 解除订单锁
     *
     * @param $itemId
     * @param $goodsId
     * @return bool
     */
    public static function unLockGoods($itemId, $goodsId = 0)
    {
        try
        {
            $key = self::getLockGoodsKey($itemId, $goodsId);
            return Server::$redis->del($key) ? true : false;
        }
        catch (Exception $e)
        {
            # Redis 连接失败
            Server::$instance->warn("[redis] 解除物品锁失败, itemId: $itemId, goodsId: $goodsId, msg: ". $e->getMessage());
            return false;
        }
    }

    /**
     * 获取订单锁Key
     *
     * @param $itemId
     * @param $pendingId
     * @return string
     */
    protected static function getLockGoodsKey($itemId, $pendingId)
    {
        return "lock_order_{$itemId}_{$pendingId}";
    }


    /**
     * 尝试抢一个用户操作的锁，避免同时操作
     *
     * @param int $charId
     * @param int $timeout 自动释放时长
     * @return bool
     */
    public static function tryLockUser($charId, $timeout = 30)
    {
        try
        {
            $key = self::getLockUserKey($charId);
            return Server::$redis->set($key, $charId, ['nx', 'ex' => $timeout]);   # 抢个网络锁
        }
        catch (Exception $e)
        {
            # Redis 连接失败
            Server::$instance->warn("[redis] 设置玩家锁失败, charId: $charId, msg: ". $e->getMessage());
            return false;
        }
    }

    /**
     * 解除用户锁
     *
     * @param $charId
     * @return bool
     */
    public static function unLockUser($charId)
    {
        try
        {
            $key = self::getLockUserKey($charId);
            return Server::$redis->del($key) ? true : false;
        }
        catch (Exception $e)
        {
            # Redis 连接失败
            Server::$instance->warn("[redis] 解除玩家锁失败, charId: $charId, msg: ". $e->getMessage());
            return false;
        }
    }

    /**
     * 获取用户操作锁Key
     *
     * @param $charId
     * @return string
     */
    protected static function getLockUserKey($charId)
    {
        return "lock_user_{$charId}";
    }

    const BOOTH_LOCK_KEY = 'booth:lock:%s';

    public static function tryBoothLock($charId, $timeout = 30)
    {
        try
        {
            $timeoutTime = time() + $timeout;
            $rs = Server::$redis->set(sprintf(self::BOOTH_LOCK_KEY, $charId), $timeoutTime, ['nx', 'ex' => $timeout]);
            return $rs;
        }
        catch (\Exception $e)
        {
            # Redis 连接失败
            Server::$instance->warn("[redis] 设置摆摊玩家锁失败, charId: $charId, msg: ". $e->getMessage());
            return false;
        }
    }

    public static function unBoothLock($charId)
    {
        try
        {
            return Server::$redis->del(sprintf(self::BOOTH_LOCK_KEY, $charId)) ? true : false;
        }
        catch (\Exception $e)
        {
            # Redis 连接失败
            Server::$instance->warn("[redis] 删除摆摊玩家锁失败, charId: $charId, msg: ". $e->getMessage());
            return false;
        }
    }

    /**
     * 回消息给 Session 服务器
     *
     * @param $msg
     * @return bool
     */
    public function sendToZone($msg)
    {
        return self::sendToZoneByFd($msg, $this->fd, $this->fromId);
    }

    /**
     * 回去一个消息
     *
     * @param  $msg
     * @return bool
     */
    public function sendToUser($msg)
    {
        return self::sendToUserByFd($msg, $this->charId, $this->fd, $this->fromId);
    }

    /**
     * 根据zoneId发送消息
     *
     * @param $msg
     * @param $charId
     * @param $zoneId
     * @return bool
     */
    public static function sendToUserByZoneId($msg, $charId, $zoneId)
    {
        $zone = Server::$zone->get($zoneId);
        if ($zone)
        {
            return self::sendToUserByFd($msg, $charId, $zone['fd'], $zone['fromId']);
        }

        return false;
    }

    /**
     * 发送系统消息给用户
     *
     * @param     $msgId
     * @param int $type
     * @param int $opt \RO\Cmd\EMessageActOpt
     * @return bool
     */
    public function sendSysMsgToUser($msgId, array $params = [], $type = \RO\Cmd\EMessageType::EMESSAGETYPE_FRAME, $opt = \RO\Cmd\EMessageActOpt::EMESSAGEACT_ADD)
    {
        return self::sendSysMsgToUserByCharId($this->fd, $this->fromId, $this->charId, $msgId, $params, $type, $opt);
    }

    /**
     * 给客户端发一个错误信息提示
     *
     * @param int $fd
     * @param int $fromId
     * @param int $charId
     * @param array $params
     * @param int $msgId
     * @param int $type
     * @param int $opt
     * @return bool
     */
    public static function sendSysMsgToUserByCharId($fd, $fromId, $charId, $msgId, array $params = [], $type = \RO\Cmd\EMessageType::EMESSAGETYPE_FRAME, $opt = \RO\Cmd\EMessageActOpt::EMESSAGEACT_ADD)
    {
        $obj = new \RO\Cmd\SysMsg();
        $obj->setId($msgId);
        $obj->setType($type);
        $obj->setAct($opt);
        if ($params)foreach($params as $v)
        {
            $p = new \RO\Cmd\MsgParam();
            $p->setParam($v);
            $obj->addParams($p);
        }
        return self::sendToUserByFd($obj, $charId, $fd, $fromId);
    }

    /**
     * 发送一个世界消息
     *
     * @param int $msgId
     * @param array $params
     * @param int $type
     * @param int $opt
     */
    public static function sendWorldMsg($msgId, array $params = [], $type = \RO\Cmd\EMessageType::EMESSAGETYPE_FRAME, $opt = \RO\Cmd\EMessageActOpt::EMESSAGEACT_ADD)
    {
        $obj = new \RO\Cmd\SysMsg();
        $obj->setId($msgId);
        $obj->setType($type);
        $obj->setAct($opt);

        if ($params)foreach($params as $v)
        {
            $p = new \RO\Cmd\MsgParam();
            $p->setParam($v);
            $obj->addParams($p);
        }

        try
        {
            $proto = pack('CC', $obj->cmd, $obj->param) . $obj->serialize();
        }
        catch (Exception $e)
        {
            Server::$instance->warn('[解析失败]' . get_class($obj).', '.$e->getMessage());
            return;
        }

        $cmd       = new \RO\Cmd\WorldMsgCmd();
        $cmd->data = $proto;
        $cmd->len  = strlen($proto);

        foreach (Server::$zone as $data)
        {
            self::sendToZoneByFd($cmd, $data['fd'], $data['fromId']);
        }
    }

    /**
     * 给客服端发送信息
     *
     * @param mixed $msg
     * @param int $fd
     * @param int $fromId
     * @return bool
     */
    public static function sendToUserByFd($msg, $charId, $fd, $fromId = 0)
    {
        try
        {
            $proto = pack('CC', $msg->cmd, $msg->param) . $msg->serialize();
        }
        catch (Exception $e)
        {
            Server::$instance->warn('[解析失败]' . get_class($msg).', '.$e->getMessage());
            return false;
        }
        $obj   = new \RO\Cmd\SessionToMeRecordTrade();
        $obj->setCharid($charId);
        $obj->setData($proto);
        $obj->setLen(strlen($proto));

        return self::sendToZoneByFd($obj, $fd, $fromId);
    }

    /**
     * 向服务器发送信息
     *
     * @param mixed $msg
     * @param int $fd
     * @param int $fromId
     * @return bool
     */
    public static function sendToZoneByFd($msg, $fd, $fromId = 0)
    {
        $data = self::createMsgData($msg);
        if (false === $data)return false;

        return Server::$instance->server->send($fd, $data, $fromId);
    }

    public static function createMsgData($msg)
    {
        /**
         * @var mixed $msg
         */
        # 结构体: flags len data，其中 data 由 cmd param proto 组成
        # flags: 标记位，1字节，0-无，1-压缩，2-加密
        # len:   data 长度，2字节
        # data内结构
        # cmd:   命令号，1字节
        # param: 参数，1字节
        # proto: 内容体，长度为 len - 2
        try
        {
            $flags = 0;
            $body  = pack('CCa*', $msg->cmd, $msg->param, $msg->serialize());
            $len   = strlen($body);
        }
        catch (Exception $e)
        {
            Server::$instance->warn('[解析失败]' . get_class($msg).', '.$e->getMessage() . ' cmd:' . json_encode($msg));
            return false;
        }

        if ($len > self::MIN_COMPRESS_SIZE)
        {
            # 数据压缩处理
            $flags = $flags | self::PACKET_FLAG_COMPRESS;
            $body  = gzcompress($body, 6);
            $len   = strlen($body);

            if ($len > self::MAX_BUFSIZE)
            {
                # 还超过长度，则采用最高等级压缩
                $body = gzencode($body, 9);
                $len  = strlen($body);
            }
        }

        # 非开发的、发送给玩家的数据
        //if ($msg instanceof \RO\Cmd\SessionToMeRecordTrade)
        //{
        //    $flags = $flags | self::PACKET_FLAG_ENCRYPT;
        //    $body  = ro_encrypt(rtrim($body, "\0"));
        //    $len   = strlen($body);
        //}

        if ($len + 3 > self::MAX_BUFSIZE)
        {
            Server::$instance->warn("发送包数据太大, len: $len, message: ". get_class($msg) .", cmd: {$msg->cmd}, param: {$msg->param}");
            return false;
        }

        return pack('CS', $flags, $len) . $body;
    }

    /**
     * 推送到fluent服务器
     *
     * @param \RO\Cmd\TradeLogCmd $msg
     */
    public static function pushToFluent($msg)
    {
        $msg->sid = Server::$regionId;
        $msg->cid = Server::$platId;
        $arr = $msg->asArray();

        # 将字段转为int
        array_walk($arr, function(&$val, $key) {
            if ($key != 'iteminfo' && $key != 'logid')
            {
                $val = (int) $val;
            }
        });
        unset($arr['cmd']);
        unset($arr['param']);
        $arr['ts']  = date('c');
        $str = msgpack_pack([time(), $arr]);

        Server::$fluentChannel->push($str);
    }

    /**
     * 推送adjust日志到fluent服务器
     *
     * @param array $msg
     */
    public static function pushToAdjustFluent($msg)
    {
        $msg['sid'] = Server::$regionId;
        $msg['cid'] = Server::$platId;
        $msg['ts']  = date('c');
        $str = msgpack_pack([time(), $msg]);
        Server::$adjustFluentChannel->push($str);
    }

    /**
     * 发送统计数据
     *
     * @param $type
     * @param $key
     * @param $subKey
     * @param $level
     * @param $value
     * @return bool
     */
    public static function sendStatLog($type, $key, $subKey, $level, $value)
    {
        static $client = null;
        $count = 0;

        reConnect:

        if (!isset($client) || null === $client)
        {
            $client = new \Swoole\Client(SWOOLE_TCP | SWOOLE_KEEP);
        }

        if (false === $client->isConnected())
        {
            list($host, $port) = Server::$statServer;
            $rs = $client->connect($host, $port);

            if (false === $rs)
            {
                $client->close(true);
                Server::$instance->warn("连接 Stat 服务器 {$host}:{$port} 失败");
                return false;
            }
            else
            {
                $cmd = new RegistRegionSystemCmd();
                $cmd->client = 1;
                $data = self::createMsgData($cmd);
                if (false === $client->send($data))
                {
                    $client->close(true);
                    unset($client);
                    goto reConnect;
                }

                try
                {
                    $rec = $client->recv();
                    if (false === $rec)
                    {
                        # 忽略系统中断错误
                        if ($client->errCode !== SOCKET_EINTR)
                        {
                            throw new Exception("注册Stat服务器 {$host}:{$port} 时等待返回结果失败" . ($client->errCode ? " Client错误信息:[{$client->errCode}] " . socket_strerror($client->errCode) : ""));
                        }
                    }
                    else if ('' === $rec)
                    {
                        # 连接被关闭
                        Server::$instance->warn("注册Stat服务器 {$host}:{$port} 时连接被关闭");
                        return false;
                    }
                }
                catch (Exception $e)
                {
                    Server::$instance->warn($e->getMessage());

                    # 尝试一次重试
                    $client->send($data);
                    $rec = $client->recv();
                    if (!$rec)
                    {
                        Server::$instance->warn("重试一次注册Stat服务器 {$host}:{$port} 时等待返回结果失败" . ($client->errCode ? " Client错误信息:[{$client->errCode}] " . socket_strerror($client->errCode) : ""));
                    }
                }
            }

            $count++;
        }

        $cmd          = new \RO\Cmd\StatCmd();
        $cmd->type    = $type;
        $cmd->key     = $key;
        $cmd->subkey  = $subKey;
        $cmd->level   = ((($level - 1) / 10) + 1) * 10;
        $cmd->value1  = $value;
        $cmd->isfloat = is_float($value);

        $data = self::createMsgData($cmd);

        if (false === $data)
        {
            return false;
        }

        $rs = $client->send($data);
        if (false === $rs)
        {
            if ($count > 1)
            {
                Server::$instance->warn("已重连Stat服务器后, 再次发送数据到 Stat 服务器". Server::$statServer[0] .":". Server::$statServer[1] ."失败,  错误信息:[{$client->errCode}]" . socket_strerror($client->errCode));
            }
            $client->close(true);
            unset($client);
            goto reConnect;
        }

        return $rs;
    }
}