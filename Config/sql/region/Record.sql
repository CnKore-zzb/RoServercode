#点唱机数据
drop table if exists `music`;
create table `music` (
  `zoneid` int(10) unsigned not null default 0,     # 区id
  `charid` bigint(20) unsigned not null default 0,  # 玩家guid
  `demandtime` int(10) unsigned not null default 0, # 点播时间
  `status` int(10) unsigned not null default 0,     # 状态
  `mapid` int(10) unsigned not null default 0,      # mapid
  `npcid` int(10) unsigned not null default 0,      # npcid
  `musicid` int(10) unsigned not null default 0,    # 音乐id
  `starttime` int(10) unsigned not null default 0,  # 播放时间
  `endtime` int(10) unsigned not null default 0,    # 结束时间
  `name` varchar(64) character set utf8mb4 COLLATE utf8mb4_bin not null,   # 角色名
  primary key(`zoneid`, `charid`, `demandtime`),
  index zoneid_status_index(`zoneid`, `status`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#多职业备份
drop table if exists `char_profession`;
create table `char_profession` (
  `charid` bigint(20) unsigned not null default 0,  # 玩家guid
  `branch` int(10) unsigned not null default 0,     # 分支
  `data`   blob,                                    # 分支数据
  primary key(`charid`, `branch`),
  index char_index (`charid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#数据库更新
CREATE TABLE `updatesql` (
  `fileList` varchar(256) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE utf8mb4_bin;

#Lottery数据
drop table if exists `lottery`;
create table `lottery` (
  `id` bigint(20) unsigned not null AUTO_INCREMENT,                            # 唯一id
  `zoneid` int(10) unsigned not null default 0,                                # 区id
  `charid` bigint(20) unsigned not null default 0,                             # 玩家guid
  `name` varchar(64) character set utf8mb4 COLLATE utf8mb4_bin not null,       # 角色名
  `type` int(10) unsigned not null default 0,                                  # 类型
  `itemid` int(10) unsigned not null default 0,                                # 物品id
  `itemname` varchar(64) character set utf8mb4 COLLATE utf8mb4_bin not null,   # 物品名
  `rate` int(10) unsigned not null default 0,                                  # 概率
  `timestamp` int(10) unsigned not null default 0,                             # 抽取时间
  primary key(`id`),
  index timestamp (`timestamp`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

