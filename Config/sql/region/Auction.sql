#拍卖行配置

drop table if exists `auction_config`;
create table `auction_config` (
  `id` bigint(20) unsigned not null PRIMARY KEY,
  `state` int(10) unsigned not null default 0,        #拍卖行阶段
  `begin_time` int(10) unsigned not null default 0,
  `verify_time` int(10) unsigned not null default 0,
  `end_time` int(10) unsigned not null default 0,
  `create_time` int(10) unsigned not null default 0
) engine=InnoDB default charset=utf8;

drop table if exists `auction_item`;
create table `auction_item` (
  `batchid` bigint(20) unsigned not null,
  `itemid` int(10) unsigned not null,
  `base_price` bigint(20) unsigned not null,
  `zeny_price` bigint(20) unsigned not null,
  `choose` bool not null default false,
  `orderid` int(10) unsigned not null default 0,
  `signup_id` bigint(20) unsigned not null default 0,
  `offerprice_id` bigint(20)  unsigned not null default 0,
  `status` int(10) unsigned not null default 0,         #竞拍状态  
  `trade_price` bigint(20)  unsigned not null default 0,
  `time` int(10) unsigned not null default 0,
  `auction` int(10) unsigned not null default 0,         #拍卖类型
  `istemp` int(10) unsigned not null default 0,          #是否临时
  `isread` int(10) unsigned not null default 0,          #是否已读
  unique key `i_batchid_itemid_signupid` (`batchid`, `itemid`, `signup_id`),
  index i_batchid_istemp_isread(`batchid`, `istemp`, `isread`)
) engine=InnoDB default charset=utf8;

drop table if exists `auction_item_signup`;
create table `auction_item_signup` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `batchid` bigint(20) unsigned not null,
  `itemid` int(10) unsigned not null,
  `charid` bigint(20) unsigned not null,
  `name` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `zoneid` int(10) unsigned not null,
  `type` int(10) unsigned not null,
  `take_status` int(10) unsigned not null default 0,
  `buyer_name` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `buyer_zoneid` int(10) unsigned not null default 0,
  `verify_time` int(10) unsigned not null default 0,
  `earn` bigint(20) unsigned not null default 0,            #税后赚取的钱
  `tax` bigint(20) unsigned not null default 0,             #扣税
  `time` int(10) unsigned not null default 0,
  `base_price` bigint(20) unsigned not null default 0,
  `zeny_price` bigint(20) unsigned not null default 0,
  `fm_buff` int(10) unsigned not null default 0,
  `fm_point` int(10) unsigned not null default 0,
  `itemdata` blob,                                    # 物品数据
  index i_batchid(`batchid`),
  index i_batchid_itemid(`batchid`, `itemid`),  
  index i_batchid_charid(`batchid`, `charid`)
) engine=InnoDB default charset=utf8;

drop table if exists `auction_offerprice`;
create table `auction_offerprice` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `batchid` bigint(20) unsigned not null,
  `itemid` int(10) unsigned not null,
  `charid` bigint(20) unsigned not null,
  `name` varchar(512)  character set utf8mb4 COLLATE utf8mb4_bin  default "",
  `zoneid` int(10) unsigned not null,
  `price` bigint(20) unsigned not null,
  `type` int(10) unsigned not null,
  `take_status` int(10) unsigned not null default 0,
  `verify_time` int(10) unsigned not null default 0,
  `seller_name`  varchar(512)  character set utf8mb4 COLLATE utf8mb4_bin  default "",
  `seller_zoneid` int(10) unsigned not null default 0,
  `time` int(10) unsigned not null default 0,
  `signup_id` bigint(20) unsigned not null default 0,
  `itemdata` blob,                                    # 物品数据
  index i_batchid_itemid(`batchid`, `itemid`),
  index i_batchid_charid(`batchid`, `charid`),
  index i_batchid_type(`batchid`, `type`),
  index i_id_type(`id`, `type`)
) engine=InnoDB default charset=utf8;

drop table if exists `auction_event`;
create table `auction_event` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `batchid` bigint(20) unsigned not null,
  `itemid` int(10) unsigned not null,
  `event` int(10) unsigned not null,
  `price` bigint(20) unsigned not null,
  `name` varchar(512)   character set utf8mb4 COLLATE utf8mb4_bin default "",
  `zoneid` int(10) unsigned not null,
  `max_price` int(10) unsigned not null default 0,
  `time` int(10) unsigned not null,
  `player_id` bigint(20) unsigned not null default 0,
  `signup_id` bigint(20) unsigned not null default 0,
  index i_batchid_itemid(`batchid`, `itemid`)
) engine=InnoDB default charset=utf8;

#可否上架
drop table if exists `auction_can_signup`;  
create table `auction_can_signup` (
  `itemid` int(10) unsigned not null,
  `auction` int(10) unsigned not null,
  primary key i_itemid(`itemid`)
) engine=InnoDB default charset=utf8;
