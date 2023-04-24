drop table if exists `charge`;
create table `charge` (
  `id`  bigint(20) unsigned not null auto_increment,
  `orderid` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null,
  primary key(`id`),
  unique index orderid (`orderid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;


#月卡购买次数
drop table if exists `charge_card`;
create table `charge_card` (
  `charid` bigint(20) unsigned not null default 0,              #id
  `ym` int(10) unsigned not null default 0,                     #year*100+month
  `count` int(10) unsigned not null default 0,                  #次数
  primary key(`charid`, `ym`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#充值-账号去重
drop table if exists `charge_accid`;
create table `charge_accid` (
  `accid` bigint(20) unsigned not null default 0,              #id
  `ym` int(10) unsigned not null default 0,                     #year*100+month
  `fdcnt` int(10) unsigned not null default 0,                  #福袋次数
  `modern` int(10) unsigned not null default 0,                 #摩登盒子
  primary key(`accid`, `ym`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#充值-ep特典
drop table if exists `charge_epcard`;
create table `charge_epcard` (
  `accid` bigint(20) unsigned not null default 0,              
  `dataid` int(10) unsigned not null default 0,                 #充值id 折扣的算原价id
  `epcard` int(10) unsigned not null default 0,                 #次数
  primary key(`accid`, `dataid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#充值-首充
drop table if exists `charge_virgin`;
create table `charge_virgin` (
  `accid` bigint(20) unsigned not null default 0,
  `tag` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,    #标识
  primary key(`accid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
