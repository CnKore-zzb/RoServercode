<?php
namespace RO\Utils;

/**
 * Created by PhpStorm.
 * User: rain
 * Date: 17/2/28
 * Time: 下午6:43
 */
class Common
{
    public static function randBetween($min, $max)
    {
        if ($max == $min) return $min;
        $gap = abs($max - $min);
        $ret = $max > $min ? $min : $max;
        $ret += rand()%($gap+1);
        return $ret;
    }

    /**
     * 计算涨幅
     *
     * @param $val
     * @param $lastVal
     * @return float|int
     */
    public static function calcRatio($val, $lastVal)
    {
        if ($lastVal == 0) {
            return 1;
        }

        $ratio = round(($val - $lastVal) / $lastVal, 3);
        if ($ratio > 10) {
            return 10;
        } else if ($ratio < -10) {
            return -10;
        } else {
            return $ratio;
        }
    }
}