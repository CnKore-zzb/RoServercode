set names utf8;
#玩家基本信息表
drop table if exists `log_usr`;
create table `log_usr` (
  `id` bigint(20) unsigned not null auto_increment,             # id
  `accid` bigint(20) unsigned not null default 0,               # 游戏账号id
  `charid` bigint(20) unsigned not null default 0,              # 角色id
  `timestamps` int(10) unsigned not null default 0,             # 时间戳
  `datatype` int(10) unsigned not null default 0,               # 数据类型 accbase:1 charbase:2
  `data`   longblob,                                            #各种数据
  primary key (`id`),
  index index_accid (`accid`),
  index index_charid(`charid`),
  index index_timestamps(`timestamps`)
) engine=InnoDB default charset=utf8mb4 COLLATE=utf8mb4_bin;
