drop table if exists `stat_normal`;
create table `stat_normal` (
  `zoneid` int(10) unsigned not null,               #zoneid
  `type` int(10) unsigned not null,                 #type`
  `skey`  bigint(20) unsigned not null,             #key 会造成关键字冲突
  `subkey`  bigint(20) unsigned not null default 0,
  `subkey2`  bigint(20) unsigned not null default 0,
  `level`  int(10) unsigned not null default 0,
  `time` int(10) unsigned not null,
  `count`  int(10) unsigned not null default 0,
  `value1`  bigint(20) unsigned not null default 0,              #如果是浮点数就是 浮点数*1000 后取整
  `value2`  int(10) unsigned not null default 0,              #如果是浮点数就是 浮点数*1000 后取整
  `value3`  int(10) unsigned not null default 0,              #如果是浮点数就是 浮点数*1000 后取整
  `isfloat`  bool not null default false,                     #
   primary key (`zoneid`,`type`,`time`,`skey`, `subkey`, `subkey2`,`level`)
) engine=InnoDB default charset=utf8;

drop table if exists `stat_result`;
create table `stat_result` (
  `type` int(10) unsigned not null,                 #type`
  `time` int(10) unsigned not null,
  `skey`  bigint(20) unsigned not null,             #key 会造成关键字冲突
  `subkey`  bigint(20) unsigned not null default 0,
  `q1`  bigint(20) unsigned not null default 0,     
  `q2`  bigint(20) unsigned not null default 0,        
  `q3`  bigint(20) unsigned not null default 0,        
  `q4`  bigint(20) unsigned not null default 0,        
  `q5`  bigint(20) unsigned not null default 0,        
  `q6`  bigint(20) unsigned not null default 0,        
  `q7`  bigint(20) unsigned not null default 0,        
  `q8`  bigint(20) unsigned not null default 0,        
  `q9`  bigint(20) unsigned not null default 0,        
  `q10`  bigint(20) unsigned not null default 0,        
  `q11`  bigint(20) unsigned not null default 0,        
  `q12`  bigint(20) unsigned not null default 0,        
  `q13`  bigint(20) unsigned not null default 0,        
  `q14`  bigint(20) unsigned not null default 0,        
  `q15`  bigint(20) unsigned not null default 0,   
   primary key (`type`,`time`,`skey`, `subkey`)
) engine=InnoDB default charset=utf8;
