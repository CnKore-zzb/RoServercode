drop table if exists `accbase`;
create table `accbase` (
  `accid` bigint(20) unsigned not null default 0,  #accid
  `charid` bigint(20) unsigned not null default 0,  #charid 使用该数据的角色 使用时设置 退出时重置
  `zoneid` int(10) unsigned not null default 0,
  `fourthchar` int(10) unsigned not null default 0,
  `fifthchar` int(10) unsigned not null default 0,
  `lastselect` bigint(20) unsigned not null default 0,
  `deletecharid` bigint(20) unsigned not null default 0,
  `deletetime` int(10) unsigned not null default 0,
  `lastdeletetime` int(10) unsigned not null default 0,
  `deletecount` int(10) unsigned not null default 0,
  `deletecountdays` int(10) unsigned not null default 0,
  `pwdresettime` int(10) unsigned not null default 0,
  `nologintime` int(10) unsigned not null default 0,
  `maincharid` bigint(20) unsigned not null default 0, #主角色id
  `password` blob,
  `credit` longblob,
  `quest` longblob,
  primary key (`accid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

drop table if exists `char_store`;
create table `char_store` (
  `accid` bigint(20) unsigned not null default 0,     #accid
  `itemid` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null,  #itemid
  `item` longblob,                                        #物品信息
  primary key (`accid`, `itemid`),
  index id_index (`accid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

