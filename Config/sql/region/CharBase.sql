set names utf8;
#玩家基本信息表
drop table if exists `charbase`;
create table `charbase` (
  `platformid` int(10) unsigned not null default 0,             #平台id
  `zoneid` int(10) unsigned not null default 0,                 #区id
  `destzoneid` int(10) unsigned not null default 0,             #区id
  `originalzoneid` int(10) unsigned not null default 0,         #区id
  `accid` bigint(20) unsigned not null default 0,               #游戏账号id
  `charid` bigint(20) unsigned not null auto_increment,         #角色id
  `guildid` bigint(20) unsigned not null default 0,             #公会id(不是实时)
  `sequence` int(10) unsigned not null default 0,
  `name` varchar(64) character set utf8mb4 COLLATE utf8mb4_bin not null,    #角色名
  `mapid` int(10) unsigned not null default 0,                  #地图id
  `gender` int(10) unsigned not null default 0,                 #性别
  `profession` int(10) unsigned not null default 0,             #职业
  `destprofession` int(10) unsigned not null default 0,         #目标职业
  `rolelv` int(10) unsigned not null default 0,                 #角色等级
  `roleexp` bigint(20) unsigned not null default 0,             #角色经验
  `charge` int(10) unsigned not null default 0,                 #充值
  `diamond` int(10) unsigned not null default 0,                #钻石
  `silver` bigint(20) not null default 0,                       #银币
  `gold` int(10) unsigned not null default 0,                   #金币
  `garden` int(10) unsigned not null default 0,                 #乐园币
  `friendship` int(10) unsigned not null default 0,
  `createtime` int(10) unsigned not null default 0,             #创建时间
  `onlinetime` int(10) unsigned not null default 0,             #上线时间
  `offlinetime` int(10) unsigned not null default 0,            #离线时间
  `addict` int(10) unsigned not null default 0,                 #防沉迷
  `battletime` int(10) unsigned not null default 0,             #战斗时长
  `rebattletime` int(10) unsigned not null default 0,
  `usedbattletime` int (10) unsigned not null default 0,
  `body` int(10) unsigned not null default 0,
  `hair` int(10) unsigned not null default 0,
  `haircolor` int(10) unsigned not null default 0,
  `lefthand` int(10) unsigned not null default 0,
  `righthand` int(10) unsigned not null default 0,
  `head` int(10) unsigned not null default 0,
  `back` int(10) unsigned not null default 0,
  `face` int(10) unsigned not null default 0,
  `tail` int(10) unsigned not null default 0,
  `mount` int(10) unsigned not null default 0,
  `title` int(10) unsigned not null default 0,
  `eye` int(10) unsigned not null default 0,
  `partnerid` int(10) unsigned not null default 0,
  `mouth` int(10) unsigned not null default 0,
  `portrait` int(10) unsigned not null default 0,
  `clothcolor` int(10) unsigned not null default 0,
  `addicttipstime` int(10) unsigned not null default 0,         #防沉迷提示
  `gagtime` int(10) unsigned not null default 0,                #禁言时间
  `nologintime` int(10) unsigned not null default 0,            #封号时间
  `data`   longblob,                                            #各种数据
  primary key (`charid`),
  unique index charbase_name (`name`),
  index charbase_accid (`accid`),
  index index_offlinetime(`offlinetime`),
  index index_onlinetime(`onlinetime`),
  index index_zoneid(`zoneid`),
  unique index charbase_accid_sequence (`accid`, `sequence`)
) engine=InnoDB default charset=utf8mb4 COLLATE=utf8mb4_bin auto_increment=4294967296;

