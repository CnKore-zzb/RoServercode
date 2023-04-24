<?php
namespace {
    class RedisCluster extends Redis{}

    class Lua {
        public function eval($str)
        {
            return '';
        }
    }
}

namespace RO {
    class MySQLi extends \mysqli {}
}