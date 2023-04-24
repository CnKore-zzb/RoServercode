#邮件列表
drop table if exists `mail`;
create table `mail` (
  `id` bigint(20) unsigned not null auto_increment,        #id
  `senderid` bigint(20) unsigned not null default 0,  #senderid
  `receiveid` bigint(20) unsigned not null default 0, #receiveid
  `receiveaccid` int(10) unsigned not null default 0,
  `sysid` bigint(20) unsigned not null default 0,     #sysid
  `mailtype` int(10) unsigned not null,               #类型
  `status` int(10) unsigned not null,                 #状态
  `time` int(10) unsigned not null,                   #时间
  `title` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,    #标题
  `sender` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,   #发送者
  `msg` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,    #内容
  `groupid` varchar(256) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `attach` blob,                                      #附件
  primary key (`id`),
  index receiveid_index (`receiveid`),
  index receiveaccid_sysid_index (`receiveaccid`, `sysid`),
  index sysid_index (`sysid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin auto_increment=10000;

#系统邮件列表
drop table if exists `sysmail`;
create table `sysmail` (
  `senderid` bigint(20) unsigned not null default 0,  #senderid
  `receiveid` bigint(20) unsigned not null default 0, #receiveid
  `receiveaccid` int(10) unsigned not null default 0,
  `sysid` bigint(20) unsigned not null auto_increment,     #sysid
  `mailtype` int(10) unsigned not null,               #类型
  `status` int(10) unsigned not null,                 #状态
  `time` int(10) unsigned not null,                   #时间
  `starttime` int(10) unsigned not null default 0,
  `endtime` int(10) unsigned not null default 0,
  `title` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,    #标题
  `sender` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,   #发送者
  `msg` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,    #内容
  `attach` blob,                                      #附件
  primary key (`sysid`),
  index sysid_index (`sysid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin auto_increment=10000;

#boss列表
drop table if exists `boss`;
create table `boss` (
  `zoneid` int(10) unsigned not null,                                           #zoneid
  `id` int(10) unsigned not null,                                               #bossid
  `refresh` int(10) unsigned not null,                                          #刷新时间
  `dietime` int(10) unsigned not null,                                          #死亡时间
  `settime` int(10) unsigned not null,                                          #点名时间
  `lv` int(10) unsigned not null,                                               #等级(亡者boss)
  `killer` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,      #上次击杀者
  `killerid` bigint(20) unsigned NOT NULL DEFAULT 0,
  `deadkiller` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,  #上次击杀者
  `deadkillerid` bigint(20) unsigned NOT NULL DEFAULT 0,
  `summontime` int(10) unsigned NOT NULL DEFAULT 0,
  primary key zoneid_id (`zoneid`, `id`),
  index zoneid(`zoneid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#道场留言
drop table if exists `dojo`;
create table `dojo` (
  `zoneid` int(10) unsigned not null,               #zoneid
  `id` bigint(20) unsigned not null,                #道场id
  `msg` blob,                                       #聊天信息
  primary key zoneid_id (`zoneid`, `id`),
  index zoneid(`zoneid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#离线消息
drop table if exists `offmsg`;
create table `offmsg` (
  `id` bigint(20) unsigned not null auto_increment,                #id
  `targetid` bigint(20) unsigned not null default 0,
  `senderid` bigint(20) unsigned not null default 0,
  `time` int(10) unsigned not null default 0,
  `type` int(10) unsigned not null default 0,
  `sendername` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `senderportrait` int(10) unsigned not null default 0,
  `senderframe` int(10) unsigned not null default 0,
  `profession` int(10) unsigned not null default 0,
  `msg` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `itemid` int(10) unsigned not null default 0,
  `price` int(10) unsigned not null default 0,
  `count` int(10) unsigned not null default 0,
  `givemoney` int(10) unsigned not null default 0,
  `moneytype` int(10) unsigned not null default 0,
  `gmcmd` varchar(256) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `chat` blob,
  primary key(`id`),
  index targetid_i(`targetid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#global数据
drop table if exists `globaldata`;
create table `globaldata` (
  `zoneid` int(10) unsigned not null default 0,         #区id
  `name` varchar(32) not null,                      #名
  `data` blob,                                      #成员
  primary key (`zoneid`, `name`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#社交列表
drop table if exists `social`;
create table `social` (
  `id` bigint(20) unsigned not null default 0,                #id
  `destid` bigint(20) unsigned not null default 0,            #destid
  `relation` int(10) unsigned not null default 0,             #关系
  `relationtime` blob,                                        #申请时间
  primary key(`id`, `destid`),
  index id_index (`id`),
  index destid_index (`destid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#召回列表
drop table if exists `social_recall`;
create table `social_recall` (
  `id` bigint(20) unsigned not null default 0,                #id
  `destid` bigint(20) unsigned not null default 0,            #destid
  primary key(`id`, `destid`),
  index id_index (`id`),
  index destid_index (`destid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#组队列表
drop table if exists `team`;
create table `team` (
  `zoneid` int(10) unsigned not null default 0,   #区id
  `id` bigint(20) unsigned not null auto_increment,      #id
  `name` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,   #名
  `teamtype` int(10) unsigned not null default 0,   #类型
  `minlv` int(10) unsigned not null default 0,      #最小等级
  `maxlv` int(10) unsigned not null default 0,      #最大等级
  `timer` int(10) unsigned not null default 0,
  `autoaccept` int(10) unsigned not null default 0,
  `pickupmode` int(10) unsigned not null default 0,
  `member` blob,                                    #成员
  `apply` blob,                                     #申请
  `wantedquest` blob,                               #看板任务
  primary key (`id`),
  index zoneid_index(`zoneid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin auto_increment=10000;

#公会基本信息表
drop table if exists `guild`;
create table `guild` (
  `zoneid` int(10) unsigned not null default 0,                   #区id
  `id` bigint(20) unsigned not null auto_increment,                              #id
  `status` int(10) unsigned not null default 0,                   #状态
  `lv` int(10) unsigned not null default 0,                       #等级
  `questtime` int(10) unsigned not null default 0,                #任务重置时间
  `conlv` int(10) unsigned not null default 0,                    #建筑等级
  `dismisstime` int(10) unsigned not null default 0,              #解散申请时间
  `createtime` int(10) unsigned not null default 0,               #公会创建时间
  `nextzone` int(10) unsigned not null default 0,
  `zonetime` int(10) unsigned not null default 0,
  `name` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null default "",      #名
  `board` varchar(1024) character set utf8mb4 COLLATE utf8mb4_bin not null default "",   #公告板
  `recruit` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin  not null default "",  #招募信息
  `portrait` varchar(1024) character set utf8mb4 COLLATE utf8mb4_bin not null default "",#公会头像
  `member` longblob,                                                  #成员
  `apply` blob,                                                   #申请
  `misc` longblob,                                                    #其他
  `pack` longblob,                                                    #仓库
  `event` longblob,                                                   #事件
  `photo` longblob,                                                   #照片
  primary key (`id`),
  index zoneid_status_index (`zoneid`, `status`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin auto_increment=10000;

#公会离线信息表
drop table if exists `guildoffline`;
create table `guildoffline` (
  `zoneid` int(10) unsigned not null default 0,         #区id
  `charid` bigint(20) unsigned not null,                #charid
  `guildid` bigint(20) unsigned not null,               #guildid
  `freeze` int(10) unsigned not null default 0,         #冻结状态
  `contribute` int(10) unsigned not null default 0,     #贡献
  `totalcontribute` int(10) unsigned not null default 0,#累积贡献
  `giftpoint` int(10) unsigned not null default 0,      #天赋点
  `pray` blob,                                          #祈福数据
  `donate` blob,
  `var` blob,                                           #var
  `building` blob,                                      #公会建筑
  `exittime` int(10) unsigned not null default 0,       #退会时间
  primary key (`zoneid`, `charid`),
  index zoneid_index(`zoneid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#公会城池信息表
drop table if exists `guild_city`;
create table `guild_city` (
  `zoneid` int(10) unsigned not null default 0,         #区id
  `city_1001` bigint(20) unsigned not null,                 #城池占领guildid
  `city_1002` bigint(20) unsigned not null,                 #城池占领guildid
  `city_1003` bigint(20) unsigned not null,                 #城池占领guildid
  `city_2001` bigint(20) unsigned not null,                 #城池占领guildid
  `city_2002` bigint(20) unsigned not null,                 #城池占领guildid
  `city_2003` bigint(20) unsigned not null,                 #城池占领guildid
  `city_3001` bigint(20) unsigned not null,                 #城池占领guildid
  `city_3002` bigint(20) unsigned not null,                 #城池占领guildid
  `city_3003` bigint(20) unsigned not null,                 #城池占领guildid
  `city_4001` bigint(20) unsigned not null,                 #城池占领guildid
  `city_4002` bigint(20) unsigned not null,                 #城池占领guildid
  `city_4003` bigint(20) unsigned not null,                 #城池占领guildid
  primary key (`zoneid`),
  index zoneid_index(`zoneid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#公会离线操作
drop table if exists `guild_gm`;
create table `guild_gm` (
  `id` bigint(20) unsigned not null auto_increment,           #id
  `guildid` bigint(20) unsigned not null default 0,           #公会id
  `charid` bigint(20) unsigned not null default 0,            #charid
  `data` longblob,                                            #指令字符串
  primary key(`id`)
) ENGINE=InnoDB AUTO_INCREMENT=65535 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

#封印
drop table if exists `seal`;
create table `seal` (
  `zoneid` int(10) unsigned not null,               #zoneid
  `teamid` bigint(20) unsigned not null,                #队伍id
  `data` blob,                                       #封印数据
  primary key zoneid_id (`zoneid`, `teamid`),
  index zoneid(`zoneid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#聊天记录
drop table if exists `chat_msg`;
create table `chat_msg` (
  `charid` bigint(20) unsigned not null default 0,                #id
  `portrait` int(10) unsigned not null default 0,                 #portrait
  `time` int(10) unsigned not null default 0,                     #time
  `data`    blob,                                                 #聊天内容
  primary key(`charid`, `time`),
  index id_index (`charid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#mini列表
drop table if exists `mini`;
create table `mini` (
  `zoneid` int(10) unsigned not null,    # zoneid
  `id` int(10) unsigned not null,        # npcid
  `mapid` int(10) unsigned not null,     # 地图id
  `refresh` int(10) unsigned not null,   # 刷新时间
  `dietime` int(10) unsigned not null,   # 死亡时间
  `killer` varchar(32) not null,         # 上次击杀者
  index `zoneid` (`zoneid`),
  primary key `zoneid_id_mapid` (`zoneid`,`id`,`mapid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#活动
drop table if exists `activity`;
create table `activity` (
  `id` bigint(20) unsigned not null auto_increment,
  `name` varchar(128) character set  utf8mb4 COLLATE utf8mb4_bin not null default "", #活动名字
  `iconurl` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin not null default "",
  `begintime` int(10) unsigned not null default 0, #开始时间
  `endtime` int(10) unsigned not null default 0, #结束时间
  `url` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin not null default "", #活动链接
  `countdown` int(10) unsigned not null default 0, #客户端倒计时显示
  `data` blob,
  `md5` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null default "", #json序列md5值
  primary key(`id`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin ;

#子活动
drop table if exists `activity_detail`;
create table `activity_detail` (
  `id` bigint(20) unsigned not null auto_increment,
  `groupid` bigint(20) unsigned not null default 0, #活动表id
  `order` int(10) unsigned not null default 0, #活动序号
  `name` varchar(128) character set  utf8mb4 COLLATE utf8mb4_bin not null default "", #活动标题
  `begintime` int(10) unsigned not null default 0,
  `endtime` int(10) unsigned not null default 0,
  `path` int(10) unsigned not null default 0, #寻路id
  `url` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin not null default "", #活动链接
  `pic_url` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin not null default "", #图片url
  `data` blob,
  `md5` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null default "", #json序列md5值
  primary key(`id`),
  index i_groupid_order (`groupid`,`order`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin ;

drop table if exists `shop`;
create table `shop` (
  `zoneid` int(10) unsigned not null,              
  `refresh` int(10) unsigned not null,             
  `data`    blob,                                               
  primary key(`zoneid`)
) engine=InnoDB default charset=utf8;

#活动模板
drop table if exists `activity_event`;
create table `activity_event` (
  `id` bigint(20) unsigned not null auto_increment,
  `desc` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null default "", #活动描述
  `tf_begintime` int(10) unsigned not null default 0, #测试服开始时间
  `tf_endtime` int(10) unsigned not null default 0, #测试服结束时间
  `release_begintime` int(10) unsigned not null default 0, #正式服开始时间
  `release_endtime` int(10) unsigned not null default 0, #正式服结束时间
  `type` int(10) unsigned not null default 0, #活动类型
  `data` blob, #活动内容配置
  `md5` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null default "", #json序列md5值
  primary key(`id`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

drop table if exists `guild_icon`;
create table `guild_icon` (
  `id` bigint(20) unsigned not null default 0,         # key=guildid*1000+iconindex
  `guildid` bigint(20) unsigned not null default 0,    # 公会id
  `charid` bigint(20) unsigned not null default 0,     # 玩家id
  `iconindex` int(10) unsigned not null default 0,     # 图标索引
  `createtime` int(10) unsigned not null default 0,    # 申请时间
  `state` int(10) unsigned not null default 0,         # 状态
  `isread` int(10) unsigned not null default 0,        # 是否读取
  `format` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null default "",  # 图标格式
  primary key (`id`),
  index index_state (`state`),
  index index_guildid (`guildid`)
) engine=InnoDB default charset=utf8;


#活动参加记录
drop table if exists `globalact_record`;
create table `globalact_record` (
  `aid` bigint(20) unsigned not null auto_increment,#自增id
  `id` int(10) unsigned not null default 0,         #global activity id
  `accid` bigint(20) unsigned not null default 0,   #accid
  `charid` bigint(20) unsigned not null default 0,  #charid
  `time` int(10) unsigned not null default 0,       #参加时间
  primary key(`aid`),
  index i_id_accid_time (`id`,`accid`, `time`),
  index i_id_charid_time (`id`,`charid`, `time`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin ;

#道具兑换码
drop table if exists `item_code`;
create table `item_code` (
  `code` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `type` int(10) unsigned not null default 0,      #类型
  `state` int(10) unsigned not null,      #0,初始 1：使用 2：兑换过
  `charid` bigint(20) unsigned not null default 0, #兑换的玩家cahrid
  `guid` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null,   #使用的道具guid
  `itemid` int(10) unsigned not null,      #道具it
  `usetime` int(10) unsigned not null,     #兑换的时间  
  primary key(`code`, `type`),
  index i_state(`state`),
  index i_guid(`guid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#全服限购商店
drop table if exists `global_shop`;
create table `global_shop` (
  `id` int(10) unsigned not null,         #shop表id
  `count`int(10) unsigned not null,       #出售次数
  primary key(`id`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#订婚
drop table if exists `wedding`;
CREATE TABLE `wedding` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `zoneid` int(10) unsigned NOT NULL DEFAULT '0',
  `date` int(10) unsigned NOT NULL DEFAULT '0',         #20181001
  `starttime` int(10) unsigned NOT NULL DEFAULT '0',      
  `endtime` int(10) unsigned NOT NULL DEFAULT '0',              
  `configid`  int(10) unsigned NOT NULL DEFAULT '0',
  `charid1` bigint(20) unsigned NOT NULL DEFAULT '0',
  `charid2` bigint(20) unsigned NOT NULL DEFAULT '0',
  `manual` longblob,        #订婚手册数据
  `status`    int(10) unsigned NOT NULL DEFAULT '0',    #状态 0 订婚 1 结婚成功 2 离婚
  PRIMARY KEY (`id`),
 KEY `index_charid1` ( `charid1`),
 KEY `index_charid2` ( `charid2`),
 KEY `index_status` ( `status`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

#gm批量任务
drop table if exists `gm_batch_job`;
CREATE TABLE `gm_batch_job` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `type`  int(10) unsigned NOT NULL DEFAULT '0',
  `progress`  int(10) unsigned NOT NULL DEFAULT '0',
  `task` longblob,
  `result` longblob,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;

#邮件模板
drop table if exists `mail_template`;
create table `mail_template` (
  `id` int(10) unsigned not null auto_increment,
  `data` blob,
  primary key (`id`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#global信息
drop table if exists `global`;
create table `global` (
  `name` varchar(32) not null,              #键值
  `data` blob,                              #成员
  primary key (`name`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#宠物装扮
drop table if exists `stat_pet_wear`;
create table `stat_pet_wear`(
  `time` int(10) unsigned not null default 0,
  `white` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `green` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `blue` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `purple` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `orange` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `gold` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `darkgold` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,
  primary key(`time`)
) engine=InnoDB default charset=utf8;

#脚本嫌疑标记
drop table if exists `cheat_tag`;
create table `cheat_tag` (
  `charid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `mininterval` int(10) unsigned NOT NULL DEFAULT '0',
  `frame` int(10) unsigned NOT NULL DEFAULT '0',
  `count` int(10) unsigned NOT NULL DEFAULT '0',
  primary key(`charid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
