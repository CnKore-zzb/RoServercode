###update_20180727_1
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
  KEY `time` (`time`),
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
  KEY `time` (`time`),
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
  KEY `time` (`time`),
  KEY `order_id` (`order_id`),
  KEY `charId_time` (`char_id`,`time`) USING BTREE,
  KEY `takeStatus_time` (`take_status`,`time`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='摆摊出售交易记录表';
###update_20180727_1
###update_20180730_1
  alter table `booth_order` drop KEY time;
  alter table `booth_order` add KEY `char_id_status` (`char_id`, `status`) USING BTREE;
  alter table `booth_order` add KEY `item_id_is_publicity` (`item_id`, `is_publicity`) USING BTREE;

  alter table `booth_record_bought` drop KEY time;
  alter table `booth_record_sold` drop KEY time;
###update_20180730_1
###update_20180813_1
  ALTER TABLE `trade_item_list` ADD `is_hot` TINYINT UNSIGNED NOT NULL DEFAULT '1' COMMENT '是否热数据' AFTER `delay_time`;
###update_20180813_1
###update_20180731_1
alter table `auction_item` add `istemp` int(10) unsigned not null default 0;
alter table `auction_item` add `isread` int(10) unsigned not null default 0;
alter table `auction_item` add index i_batchid_istemp_isread(`batchid`, `istemp`, `isread`);
###update_20180731_1
