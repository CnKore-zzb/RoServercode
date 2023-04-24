<?php
namespace RO\Booth;

use DrSlump\Protobuf;
use RO\Cmd\RegistProxySystemCmd;
use RO\Cmd\RegistRegionSystemCmd;
use RO\Cmd\SessionForwardScenecmdTrade;
use Swoole\Client;

/**
 * Created by PhpStorm.
 * User: rain
 * Date: 18/6/19
 * Time: 下午6:11
 */
class TradeClient
{
    const TIME_OUT = 0.5;

    const RETRY_COUNT = 1;

    private $host;

    private $port;

    private $retryCount = 0;

    /**
     * @var \Swoole\Client
     */
    private $client;

    public function __construct(string $host, int $port)
    {
        $this->host   = $host;
        $this->port   = $port;
        $this->client = new Client(SWOOLE_TCP | SWOOLE_KEEP, SWOOLE_SOCK_SYNC);
        $this->client->set([
                               'open_eof_check'        => false,
                               'open_eof_split'        => false,
                               'open_length_check'     => true,
                               'package_length_type'   => 'S',
                               'package_length_offset' => 1,
                               'package_body_offset'   => 3,
                               'package_max_length'    => 65535,
                           ]);
        $this->connect();
    }

    private function connect():bool
    {
        do
        {
            if ($this->client->isConnected()) {
                $this->client->close(true);
            }
            $rs = $this->client->connect($this->host, $this->port, self::TIME_OUT);
            if ($rs === false)
            {
                $this->retryCount++;
            }
            else
            {
                break;
            }
        }
        while($this->retryCount >= self::RETRY_COUNT);

        if ($rs) {
            $msg = new RegistRegionSystemCmd();
            $msg->servertype = 11;
            $this->client->send($this->createMsgData($msg));
        }

        return $rs;
    }

    public function send($data):bool
    {
        $msg = $this->createForwardMsg($data);
        if ($msg === null) {
            return false;
        }

        if ($this->client->send($msg) === false) {
            if ($this->connect()){
                $rs = $this->client->send($msg);
                if ($rs) {
                    return $rs;
                } else {
                    BoothServer::$instance->warn(sprintf("[trade-client] send failed, err:%s", socket_strerror($this->client->errCode)));
                }
            }
        }

        return true;
    }

    /**
     * @param mixed $msg
     * @return string
     */
    public function createForwardMsg($msg):string
    {
        $forward = new SessionForwardScenecmdTrade();
        $forward->data = pack('CCa*', $msg->cmd, $msg->param, $msg->serialize());
        return $this->createMsgData($forward);
    }

    /**
     * @param mixed $msg
     * @return string
     */
    public function createMsgData($msg):string
    {
        try
        {
            $flags = 0;
            $body  = pack('CCa*', $msg->cmd, $msg->param, $msg->serialize());
            $len   = strlen($body);
            return pack('CS', $flags, $len) . $body;
        }
        catch (\Exception $e)
        {
            BoothServer::$instance->warn('[解析失败]' . get_class($msg).', '.$e->getMessage());
            return null;
        }
    }
}