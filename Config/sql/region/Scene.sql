#任务配置
drop table if exists `questconfig`;
create table `questconfig` (
  `questid` bigint(20) unsigned not null default 0, #id
  `version` int(10) unsigned not null default 0,    #版本号
  `txt` blob,                                       #内容字符串
  primary key (`questid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin ;

#公会留声机
drop table if exists `guildmusic`;
create table `guildmusic` (
  `guildid` bigint(20) unsigned not null default 0,  # 公会id
  `charid` bigint(20) unsigned not null default 0,   # 玩家guid
  `demandtime` int(10) unsigned not null default 0,  # 点播时间
  `status` int(10) unsigned not null default 0,      # 状态
  `mapid` int(10) unsigned not null default 0,       # mapid
  `npcid` int(10) unsigned not null default 0,       # npcid
  `musicid` int(10) unsigned not null default 0,     # 音乐id
  `starttime` int(10) unsigned not null default 0,   # 播放时间
  `endtime` int(10) unsigned not null default 0,     # 结束时间
  `name` varchar(64) character set utf8mb4 COLLATE utf8mb4_bin not null, # 角色名
  primary key (`guildid`,`charid`,`demandtime`),
  key `guildid_status_index` (`guildid`,`status`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin ;

CREATE TABLE `quest_patch_2` (
  `accid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `quest_39300` int(10) unsigned NOT NULL DEFAULT '0',
  `quest_39301` int(10) unsigned NOT NULL DEFAULT '0',
  `quest_39309` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`accid`),
  KEY `id_index` (`accid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
