<?php
namespace RO\Trade;

class WorkerAdmin extends \MyQEE\Server\WorkerHttp
{
    public function onRequest($request, $response)
    {
        $response->end('hello');
    }
}