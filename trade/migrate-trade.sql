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
  KEY `charId_stock` (`char_id`,`stock`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='挂单表';

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
  PRIMARY KEY (`id`),
  UNIQUE KEY `item_key` (`item_key`)
) ENGINE=InnoDB AUTO_INCREMENT=1000000001 DEFAULT CHARSET=utf8mb4 COMMENT='物品列表';

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
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `pubId_status` (`publicity_id`,`status`),
  KEY `charId_time` (`char_id`,`time`) USING BTREE
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
  PRIMARY KEY (`id`),
  KEY `time` (`time`),
  KEY `charId_time` (`char_id`,`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='出售交易记录表';

ALTER TABLE `trade_item_info` ADD `is_trade` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `t`;