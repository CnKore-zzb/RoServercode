<?php
/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/3/17
 * Time: 下午2:35
 */

namespace RO\Http;

use MyQEE\Server\WorkerHttp;
use RO\Trade\Item;
use RO\Trade\Server;

class WorkerMain extends WorkerHttp
{
    private $token = '78eea3435d5c7caa8ca6d239ecde84ebb2232304';

    public function onRequest($request, $response)
    {
        $uri = $request->server['request_uri'];
        $method = isset($request->server['request_method']) ? strtoupper($request->server['request_method']) : 'GET';
        $result = null;

        $request->post = isset($request->post) ? $request->post : [];
        $request->get  = isset($request->get) ? $request->get : [];

        if (IS_DEBUG)
        {
            $postStr = empty($request->post) ? '' : 'POST:'. json_encode($request->post);
            $getStr  = empty($request->get) ? '' : 'GET:'. json_encode($request->get);
            Server::$instance->debug("当前{$method}请求 URI:{$uri} {$postStr} {$getStr}");
        }

        try
        {
            if (!isset($request->get['token']) || $request->get['token'] !== $this->token)
            {
                throw new \Exception('非法token值');
            }

            switch ($method)
            {
                case 'GET':
                    switch ($uri)
                    {
                        case '/item':
                            $result = ItemService::get($request->get);
                            break;
                        case '/item/list':
                            $result = ItemService::getList($request->get);
                            break;
                        case '/prohibit':
                            $result = ItemService::getProhibitions();
                            break;
                        case '/status':
                            $result = 'ok';
                            break;
                        default:
                            $result = '';
                            break;
                    }
                    break;
                case 'POST':
                    switch ($uri)
                    {
                        case '/item/price':
                            $itemId = isset($request->post['item_id']) ? $request->post['item_id'] : 0;
                            $price  = isset($request->post['price']) ? $request->post['price'] : 0;
                            $result = ItemService::setPrice($itemId, $price);
                            break;
                        case '/item/config':
                            $result = ItemService::setConfig($request->post);
                            break;
                        case '/prohibit':
                            $result = ItemService::prohibit($request->post);
                            break;
                        case '/reload':
                            $result = StatusService::reloadConfig();
                            break;
                        case '/open':
                            $result = StatusService::openTrade();
                            break;
                        case '/close':
                            $result = StatusService::closeTrade();
                            break;
                        default:
                            $result = '';
                            break;
                    }
                    break;
                default:
                    throw new \HttpException('404 not found');
            }
        }
        catch (\Exception $e)
        {
            $response->end($this->error($e->getCode(), $e->getMessage()));
            return;
        }

        if (IS_DEBUG)
        {
            Server::$instance->debug('result:', is_array($result) ? $result : [$result]);
        }
        $response->end($this->success($result));
    }

    public function success($data)
    {
        return json_encode(['status' => 1, 'data' => $data], JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
    }

    public function error($code, $msg)
    {
        return json_encode(['status' => $code, 'message' => $msg], JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
    }
}