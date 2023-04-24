<?php
namespace RO\Trade\Process;

class Logger extends \MyQEE\Server\ProcessLogger
{
    /**
     * 根据当前路径和时间key获取存档的log路径
     *
     * @param string $path
     * @param string $timeKey
     * @return string
     */
    protected function getActiveFilePath($path, $timeKey)
    {
        # 直接在路径后加时间戳
        return "{$path}.{$timeKey}";
    }
}