#交易所_当前物品的服务器价格

drop table if exists `trade_item_info`;
create table `trade_item_info` (
  `itemid` int(10) unsigned not null,                  #item type id
  `last_server_price` int(10) unsigned not null default 0,
  `last_calc_price_time` int(10) unsigned not null default 0,
  `t` int(10) unsigned not null default 0,
   unique index itemid (`itemid`)
) engine=InnoDB default charset=utf8;

drop table if exists `trade_price_adjust_log`;
create table `trade_price_adjust_log` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `itemid` int(10) unsigned not null,
  `refinelv` int(10) unsigned not null,
  `last_time` int(10) unsigned not null default 0,
  `t` int(10) unsigned not null default 0,
  `d` int(10) unsigned not null default 0,
  `k` int(10) unsigned not null default 0,
  `qk` float unsigned not null default 0,
  `p0` int(10) unsigned not null default 0,
  `oldprice` int(10) unsigned not null default 0,
  `newprice` int(10) unsigned not null default 0
) engine=InnoDB default charset=utf8;

drop table if exists `trade_pending_list`;
create table `trade_pending_list` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `itemid` int(10) unsigned not null default 0,
  `price`  int(10) unsigned not null default 0,
  `count`  int(10) unsigned not null default 0,
  `sellerid` bigint(20) unsigned not null default 0,
  `name` varchar(512) default "",
  `pendingtime` int(10) unsigned not null default 0,
  `refine_lv` int(10) not null default 0,
  `itemdata` blob,
  `is_overlap` bool not null default true,
  `publicity_id` int(10) unsigned not null default 0,
  `endtime` int(10) unsigned not null default 0,
  index itemid_pendingtime_lv_index(`itemid`,`pendingtime`,`refine_lv`),
  index index_sellerid(`sellerid`),
  index index_a(`itemid`,`sellerid`,`pendingtime`),
  index index_publicity_id(`publicity_id`)
) engine=InnoDB default charset=utf8;


drop table if exists `trade_saled_list`;
create table `trade_saled_list` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `itemid` int(10) unsigned not null,
  `price`  int(10) unsigned not null,
  `count`  int(10) unsigned not null,
  `sellerid` bigint(20) unsigned not null,
  `buyerid` bigint(20) unsigned not null,
  `pendingtime` int(10) unsigned not null,
  `tradetime` int(10) unsigned not null,
  `refine_lv` int(10) not null default 0,
  `itemdata` blob,
  `logtype` int(10) unsigned not null default 1,
  `tax` int(10) unsigned not null default 0,
  `status` int(10) unsigned default 1,
  `damage` int(10) unsigned default 0,
  `buyerinfo` blob,
  index tradetime_index (`tradetime`),
  index sellerid_index (`sellerid`),
  index sellerid_tradetime_index (`sellerid`, `tradetime`),
  index status_tradetime_index (status,tradetime),
  index i_itemid_time_refine_lv_count(`itemid`, `tradetime`,`refine_lv`,`count`)
) engine=InnoDB default charset=utf8;

drop table if exists `trade_buyed_list`;
create table `trade_buyed_list` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `publicity_id` int(10) unsigned not null default 0,
  `itemid` int(10) unsigned not null,
  `price`  int(10) unsigned not null,
  `count`  int(10) unsigned not null,
  `sellerid` bigint(20) unsigned not null default 0,
  `buyerid` bigint(20) unsigned not null,
  `buyername` varchar(512) default "",
  `pendingtime` int(10) unsigned not null default 0,
  `tradetime` int(10) unsigned not null,
  `refine_lv` int(10) not null default 0,
  `itemdata` blob,
  `logtype` int(10) unsigned not null default 2,
  `totalcount` int(10) unsigned not null default 0,
  `failcount` int(10) unsigned not null default 0,
  `status` int(10) unsigned default 1,
  `endtime` int(10) unsigned default 0,
  `damage` int(10) unsigned default 0,
  `sellerinfo` blob,
  `receiveid` bigint(20) unsigned not null default 0,
  `receivername` varchar(128) default "", 
  `receiverzoneid` int(10) unsigned not null default 0,
  `expiretime` int(10) unsigned default 0,
  `anonymous` bool not null default false,
  `background` int(10) unsigned default 0,
  `content` varchar(512) default "",
  index tradetime_index (`tradetime`),
  index buyerid_index (`buyerid`),
  index status_tradetime_index (status,tradetime),
  index buyerid_tradetime_index (`buyerid`, `tradetime`),
  index idx_receiveid (`receiveid`),
  index idx_receiveid_expiretime (`receiveid`, `expiretime`),
  index index_publicity_id_endtime(`publicity_id`, `endtime`)
) engine=InnoDB default charset=utf8;
alter table trade_buyed_list add index index_itemid_logtype(`itemid`, `logtype`);

drop table if exists `trade_publicity`;
create table `trade_publicity` (
  `id` int(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `uniqueid` varchar(255) not null,
  `itemid`  int(10) unsigned not null default 0,
  `starttime`  int(10) unsigned not null default 0,
  `endtime`  int(10) unsigned not null default 0,
  `price` int(20) unsigned not null default 0,
  `buy_people` int(10) unsigned not null default 0,
  `last_stoptime` int(10) unsigned not null default 0,
  unique index i_uniqueid (`uniqueid`)
) engine=InnoDB default charset=utf8;

drop table if exists `trade_publicity_buy`;
create table `trade_publicity_buy` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `publicity_id` int(10) unsigned not null default 0,
  `buyerid` bigint(20) unsigned not null,
  `count`  int(10) unsigned not null default 0
) engine=InnoDB default charset=utf8;

drop table if exists `trade_give`;

drop table if exists `trade_security`;
create table `trade_security` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `charid` bigint(20) unsigned not null default 0,
  `type` int(10) unsigned not null,
  `itemid`  int(10) unsigned not null default 0,
  `refinelv`  int(10) unsigned not null default 0,
  `uniquekey` varchar(512) default "",
  `time`  int(10) unsigned not null default 0
) engine=InnoDB default charset=utf8;

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
  KEY `status_time` (`status`, `time`) USING BTREE
  KEY `char_id_status` (`char_id`, `status`) USING BTREE,
  KEY `item_id_is_publicity` (`item_id`, `is_publicity`) USING BTREE,
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

