/* 最新完整版 */

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";

DROP TABLE IF EXISTS `trade_convert_record`;
CREATE TABLE IF NOT EXISTS `trade_convert_record` (     /* 创建时间 */
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,     /* 玩家ID */
  `time` int(10) UNSIGNED NOT NULL,                     /* 类型, 参阅 Dao 的 ConvertRecord 中类型 */
  `charid` bigint(20) UNSIGNED NOT NULL,                /* 状态 0:处理中,1:成功,2:有错误 */
  `type` smallint(4) UNSIGNED NOT NULL DEFAULT '0',     /* 当前类型的一些其它参数 */
  `status` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',    /* 请求的命令，类型不同数据不同 */
  `ret` mediumint(4) UNSIGNED NOT NULL DEFAULT '0',
  `vars` varchar(4096) DEFAULT NULL,
  `cmd` blob,
  PRIMARY KEY (`id`),
  KEY `time` (`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='兑换处理请求记录';

DROP TABLE IF EXISTS `trade_gift`;
CREATE TABLE IF NOT EXISTS `trade_gift` (
  `record_id` bigint(20) UNSIGNED NOT NULL,
  `receiver_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `receiver_name` varchar(128) NOT NULL DEFAULT '',
  `receiver_zone_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `expire_time` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `anonymous` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `background` int(11) UNSIGNED NOT NULL DEFAULT '0',
  `content` varchar(512) NOT NULL DEFAULT '',
  PRIMARY KEY (`record_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='赠送表';

DROP TABLE IF EXISTS `trade_goods`;
CREATE TABLE IF NOT EXISTS `trade_goods` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `item_id` int(10) UNSIGNED NOT NULL,
  `item_list_id` bigint(20) UNSIGNED NOT NULL,
  `item_key` varchar(32) NOT NULL,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_name` varchar(128) CHARACTER SET utf8mb4 DEFAULT '',
  `player_zone_id` int(10) UNSIGNED NOT NULL,
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `status` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `stock` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `is_overlap` tinyint(1) UNSIGNED NOT NULL,
  `is_publicity` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `end_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `pub_price` int(10) UNSIGNED NOT NULL,
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `itemKey_stock` (`item_key`,`stock`) USING BTREE,
  KEY `itemListId_status` (`item_list_id`,`status`) USING BTREE,
  KEY `charId_stock` (`char_id`,`stock`) USING BTREE,
  KEY `status_time` (`status`, `time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='挂单表';

DROP TABLE IF EXISTS `trade_item_info`;
CREATE TABLE IF NOT EXISTS `trade_item_info` (
  `itemid` int(10) UNSIGNED NOT NULL,
  `last_server_price` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `last_calc_price_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `t` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `up_ratio` float NOT NULL DEFAULT '1' COMMENT '涨幅系数',
  `down_ratio` float NOT NULL DEFAULT '1' COMMENT '跌幅系数',
  `max_price` int(10) DEFAULT '0' COMMENT '最高价格',
  `is_trade` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  UNIQUE KEY `itemid` (`itemid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='物品价格表';

DROP TABLE IF EXISTS `trade_item_list`;
CREATE TABLE IF NOT EXISTS `trade_item_list` (
  `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `item_key` varchar(32) NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL,
  `is_publicity` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `pub_buy_people` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `pub_price` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `start_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `end_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `refine_lv` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `is_damage` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `is_good_enchant` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `stock` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `item_data` blob,
  `delay_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `is_hot` tinyint(1) UNSIGNED NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`),
  UNIQUE KEY `item_key` (`item_key`)
) ENGINE=InnoDB AUTO_INCREMENT=1000000001 DEFAULT CHARSET=utf8mb4 COMMENT='物品列表';

DROP TABLE IF EXISTS `trade_price_adjust_log`;
CREATE TABLE IF NOT EXISTS `trade_price_adjust_log` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `itemid` int(10) UNSIGNED NOT NULL,
  `refinelv` int(10) UNSIGNED NOT NULL,
  `last_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `t` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `d` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `k` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `qk` float UNSIGNED NOT NULL DEFAULT '0',
  `p0` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `oldprice` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `newprice` int(10) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COMMENT='调价记录表';

DROP TABLE IF EXISTS `trade_prohibition`;
CREATE TABLE IF NOT EXISTS `trade_prohibition` (
  `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `unique_id` varchar(255) NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `refine_lv` int(11) NOT NULL DEFAULT '0',
  `enchant_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `enchant_buff_id` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `type` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `status` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `created_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `unique_id` (`unique_id`(128))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='安全指令表';

DROP TABLE IF EXISTS `trade_record_bought`;
CREATE TABLE IF NOT EXISTS `trade_record_bought` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_name` varchar(128) NOT NULL DEFAULT '',
  `player_zone_id` bigint(20) UNSIGNED NOT NULL,
  `goods_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `take_status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `can_give` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `item_id` int(10) UNSIGNED NOT NULL,
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `publicity_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `end_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `total_count` int(10) UNSIGNED NOT NULL,
  `price` int(10) UNSIGNED NOT NULL,
  `refine_lv` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `is_damage` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `is_many_people` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `seller_info` tinyblob,
  `sellers_info` longblob,
  `item_data` blob,
  `log_id` VARCHAR(32) NOT NULL COMMENT '唯一id防止重复插入',
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `pubId_status` (`publicity_id`,`status`),
  KEY `charId_time` (`char_id`,`time`) USING BTREE,
  KEY `takeStatus_time` (`take_status`,`time`) USING BTREE,
  UNIQUE KEY (`log_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='购买交易记录表';

DROP TABLE IF EXISTS `trade_record_sold`;
CREATE TABLE IF NOT EXISTS `trade_record_sold` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_zone_id` int(10) UNSIGNED NOT NULL,
  `goods_id` bigint(20) UNSIGNED NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL,
  `status` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `take_status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `price` int(10) UNSIGNED NOT NULL,
  `refine_lv` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `is_damage` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `tax` int(10) UNSIGNED NOT NULL,
  `is_many_people` tinyint(1) UNSIGNED NOT NULL,
  `buyer_info` tinyblob,
  `buyers_info` longblob,
  `item_data` blob,
  `log_id` VARCHAR(32) NOT NULL COMMENT '唯一id防止重复插入',
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `goods_id` (`goods_id`),
  KEY `charId_time` (`char_id`,`time`) USING BTREE,
  KEY `takeStatus_time` (`take_status`,`time`) USING BTREE,
  UNIQUE KEY (`log_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='出售交易记录表';

DROP TABLE IF EXISTS `trade_finance`;
CREATE TABLE `trade_finance` (
  `id` int(10) UNSIGNED NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL COMMENT '物品id',
  `price` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '当前价格',
  `last_price` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '上次最后价格',
  `deal_count` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '成交量',
  `last_deal_count` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '上次成交量',
  `price_ratio` float NOT NULL DEFAULT '0' COMMENT '涨跌幅',
  `deal_count_ratio` float NOT NULL DEFAULT '0' COMMENT '成交量涨跌幅',
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '统计时间',
  `created_time` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '创建记录时间'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `trade_finance`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `itemid_time` (`item_id`,`time`),
  ADD KEY `time` (`time`);

ALTER TABLE `trade_finance`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;


# 摆摊数据表
DROP TABLE IF EXISTS `booth_order`;
CREATE TABLE IF NOT EXISTS `booth_order` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `item_id` int(10) UNSIGNED NOT NULL,
  `item_key` varchar(32) NOT NULL,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_name` varchar(128) CHARACTER SET utf8mb4 DEFAULT '',
  `player_zone_id` int(10) UNSIGNED NOT NULL,
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `created_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `status` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `stock` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `lock_stock` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `origin_count` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `is_publicity` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `end_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `pub_price` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `pub_buy_people` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `up_rate` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `down_rate` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `booth_fee` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `quota` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `item_id_is_publicity` (`item_id`, `is_publicity`) USING BTREE,
  KEY `char_id_status` (`char_id`, `status`) USING BTREE,
  KEY `status_time` (`status`, `time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='摆摊订单表';

DROP TABLE IF EXISTS `booth_record_bought`;
CREATE TABLE IF NOT EXISTS `booth_record_bought` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_name` varchar(128) NOT NULL DEFAULT '',
  `player_zone_id` bigint(20) UNSIGNED NOT NULL,
  `order_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `seller_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `seller_name` varchar(128) NOT NULL DEFAULT '',
  `seller_zone_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `status` smallint(4) UNSIGNED NOT NULL DEFAULT '10',
  `take_status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `can_give` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `item_id` int(10) UNSIGNED NOT NULL,
  `item_key` varchar(32) NOT NULL,
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `is_publicity` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `end_time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `total_count` int(10) UNSIGNED NOT NULL,
  `price` bigint(20) UNSIGNED NOT NULL,
  `real_price` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `spend_quota` bigint(20) UNSIGNED NOT NULL,
  `refine_lv` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `is_damage` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `charId_time` (`char_id`,`time`) USING BTREE,
  KEY `takeStatus_time` (`take_status`,`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='摆摊购买交易记录表';

DROP TABLE IF EXISTS `booth_record_sold`;
CREATE TABLE IF NOT EXISTS `booth_record_sold` (
  `id` bigint(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` bigint(20) UNSIGNED NOT NULL,
  `player_name` varchar(128) NOT NULL DEFAULT '',
  `player_zone_id` int(10) UNSIGNED NOT NULL,
  `order_id` bigint(20) UNSIGNED NOT NULL,
  `buyer_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `buyer_name` varchar(128) NOT NULL DEFAULT '',
  `buyer_zone_id` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `item_id` int(10) UNSIGNED NOT NULL,
  `item_key` varchar(32) NOT NULL,
  `is_publicity` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `status` tinyint(1) UNSIGNED NOT NULL DEFAULT '0',
  `take_status` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `count` int(10) UNSIGNED NOT NULL,
  `price` bigint(20) UNSIGNED NOT NULL,
  `quota` bigint(20) UNSIGNED NOT NULL DEFAULT '0',
  `refine_lv` smallint(4) UNSIGNED NOT NULL DEFAULT '0',
  `is_damage` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `tax` int(10) UNSIGNED NOT NULL,
  `buyers_info` longblob,
  `item_data` blob,
  PRIMARY KEY (`id`),
  KEY `order_id` (`order_id`),
  KEY `charId_time` (`char_id`,`time`) USING BTREE,
  KEY `takeStatus_time` (`take_status`,`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='摆摊出售交易记录表';