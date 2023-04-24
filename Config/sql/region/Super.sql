# 游戏网关列表
drop table if exists `gateinfo`;
create table `gateinfo` (
  `zoneid` int(10) unsigned not null,
  `ip` int(10) unsigned not null,
  `port` int(10) unsigned not null,
  `onlinenum` int(10) unsigned not null,
  `ipstr` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  index zoneid(zoneid),
  index onlinenum(onlinenum),
  primary key ip_port (`ip`, `port`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
