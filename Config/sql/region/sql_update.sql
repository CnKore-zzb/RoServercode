# 使用说明: 版本号用 "###" 作为标记头, "###" 后为实际更新的版本号, 需前后一致
# 备注使用 "#" 或者"-- "

###update_20160817_1
  truncate table updatesql;
###update_20160817_1

###update_20160908_1
  update globaldata set data = NULL where name = 'tower';
  update globaldata set data = NULL where name = 'quest';
###update_20160908_1

###update_20160928_1
  drop table if exists `seal`;
  create table `seal` (
    `zoneid` int(10) unsigned not null,
    `teamid` bigint(20) unsigned not null,
    `data` blob,
    unique index zoneid_id (`zoneid`, `teamid`),
    index zoneid(`zoneid`)
  ) engine=InnoDB default charset=utf8;
###update_20160928_1

###update_20160928_2
  alter table team drop column seal;
###update_20160928_2

###update_20160930_1
  alter table team add column pickupmode int(10) unsigned not null default 0 after autoaccept;
###update_20160930_1

###update_20161008_1
  alter table guild add column nextzone int(10) unsigned not null default 0 after createtime;
  alter table guild add column zonetime int(10) unsigned not null default 0 after createtime;
###update_20161008_1

###update_20161009_1
  alter table social add column map int(10) unsigned not null default 0 after title;
  alter table social add column zoneid int(10) unsigned not null default 0 after map;
  alter table social add column blink int(10) unsigned not null default 0 after zoneid;
###update_20161009_1

########################################  1.0.0  #############################################

###update_20161012_1
  alter table `charbase` drop index charbase_accid;
  alter table `charbase` add index charbase_accid (`accid`);
###update_20161012_1

###update_20161012_2
  alter table `charbase` drop index charbase_name;
  alter table `charbase` add unique index charbase_name (`name`);
###update_20161012_2

###update_20161013_1
drop table if exists `char_share`;
drop table if exists `char_store`;
create table `char_store` (
  `accid` bigint(20) unsigned not null default 0,     #accid
  `itemid` varchar(128) character set utf8 not null,  #itemid
  `item` blob,                                        #物品信息
  primary key (`accid`, `itemid`),
  index id_index (`accid`)
) engine=InnoDB default charset=utf8;
###update_20161013_1

###update_20161014_1
  alter table charbase add column hair int(10) unsigned not null default 0 after body;
  alter table charbase add column haircolor int(10) unsigned not null default 0 after hair;
###update_20161014_1

###update_20161014_2
drop table if exists `accbase`;
create table `accbase` (
  `accid` bigint(20) unsigned not null default 0,  #accid
  `charid` bigint(20) unsigned not null default 0,  #charid 使用该数据的角色 使用时设置 退出时重置
  primary key (`accid`)
) engine=InnoDB default charset=utf8;
###update_20161014_2

########################################  2.0.0  #############################################

###update_20161015_1
  alter table charbase add column lefthand int(10) unsigned not null default 0 after haircolor;
  alter table charbase add column righthand int(10) unsigned not null default 0 after lefthand;
  alter table charbase add column head int(10) unsigned not null default 0 after righthand;
  alter table charbase add column back int(10) unsigned not null default 0 after head;
  alter table charbase add column face int(10) unsigned not null default 0 after back;
  alter table charbase add column tail int(10) unsigned not null default 0 after face;
  alter table charbase add column mount int(10) unsigned not null default 0 after tail;
###update_20161015_1

###update_20161017_1
  alter table offmsg drop column voicetime;
  alter table offmsg drop column voiceid;
  alter table offmsg add column chat blob after gmcmd;
###update_20161017_1

###update_20161018_1
  alter table charbase add column sequence int(10) unsigned not null default 0 after charid;
###update_20161018_1

###update_20161018_2
  alter table accbase add column fourthchar int(10) unsigned not null default 0;
  alter table accbase add column fifthchar int(10) unsigned not null default 0;
###update_20161018_2

########################################  2.0.1  #############################################

###update_20161019_1
  alter table accbase add column lastselect bigint(20) unsigned not null default 0;
###update_20161019_1

###update_20161020_1
  alter table accbase add column `deletecharid` bigint(20) unsigned not null default 0;
  alter table accbase add column `deletetime` int(10) unsigned not null default 0;
  alter table accbase add column `lastdeletetime` int(10) unsigned not null default 0;
  alter table accbase add column `deletecount` int(10) unsigned not null default 0;
  alter table accbase add column `deletecountdays` int(10) unsigned not null default 0;
###update_20161020_1

########################################  2.0.2  2016-10-22  #############################################

###update_20161022_1
  alter table charbase add column title int(10) unsigned not null default 0 after mount;
###update_20161022_1

###update_20161022_2
  drop table if exists `trade_price_adjust_log`;
  create table `trade_price_adjust_log` (
    `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,            #id 唯一id
    `itemid` int(10) unsigned not null,                  #item type id
    `refinelv` int(10) unsigned not null,                  #refine lv
    `last_time` int(10) unsigned not null default 0,    #最近的计算服务器价格的时间
    `t` int(10) unsigned not null default 0,
    `d` int(10) unsigned not null default 0,
    `k` int(10) unsigned not null default 0,
    `qk` float unsigned not null default 0,
    `p0` int(10) unsigned not null default 0,
    `oldprice` int(10) unsigned not null default 0,
    `newprice` int(10) unsigned not null default 0,
    primary key (`id`)
  ) engine=InnoDB default charset=utf8;
###update_20161022_2

###update_20161025_1
  drop table if exists `trade_item_info`;
  create table `trade_item_info` (
    `itemid` int(10) unsigned not null,                  #item type id
    `refinelv` int(10) unsigned not null default 0,                  #refine lv
    `last_server_price` int(10) unsigned not null default 0,   #最近的服务器价格
    `last_calc_price_time` int(10) unsigned not null default 0,#最近的计算服务器价格的时间
    `last_sys_pending_time` int(10) unsigned not null default 0,  #最近的系统补仓的时间
    unique index itemid_refinelv (`itemid`,`refinelv`)
  ) engine=InnoDB default charset=utf8;
###update_20161025_1

########################################  2.0.4  2016-10-24  #############################################

###update_20161026_3
alter table trade_pending_list drop index price_index;
alter table trade_pending_list drop index itemid_index;
alter table trade_pending_list drop index refine_lv_index;
alter table trade_pending_list drop index pendingtime_index;
alter table trade_pending_list add index itemid_pendingtime_lv_index(`itemid`,`pendingtime`,`refine_lv`);
alter table trade_pending_list add index index_a(`itemid`,`sellerid`,`pendingtime`);

drop table if exists `stat_normal`;
create table `stat_normal` (
  `zoneid` int(10) unsigned not null,
  `type` int(10) unsigned not null,
  `skey`  bigint(20) unsigned not null,
  `subkey`  bigint(20) unsigned not null default 0,
  `subkey2`  bigint(20) unsigned not null default 0,
  `level`  int(10) unsigned not null default 0,
  `time` int(10) unsigned not null,
  `count`  int(10) unsigned not null default 0,
  `value1`  int(10) unsigned not null default 0,
  `value2`  int(10) unsigned not null default 0,
  `value3`  int(10) unsigned not null default 0,
  `isfloat`  bool not null default false,
  primary key (`zoneid`,`type`,`skey`,`time`,`subkey`, `subkey2`,`level`)
 ) engine=InnoDB default charset=utf8;
###update_20161026_3

###update_20161102_1
  alter table `social` add index destid_index (`destid`);
###update_20161102_1
########################################  2.0.8  2016-10-27  #############################################

###update_20161104_1
  alter table charbase add column eye int(10) unsigned not null default 0 after title;
  update charbase set eye = 1 where gender = 1;
  update charbase set eye = 2 where gender = 2;
###update_20161104_1

###update_20161108_1
  update guildoffline  set donate = "";
###update_20161108_1

###update_20161109_1
  alter table charbase add column friendship int(10) unsigned not null default 0 after garden;
###update_20161109_1

###update_20161111_1
alter table trade_pending_list add index index_sellerid(`sellerid`);
###update_20161111_1

###update_20161115_1
  alter table charbase add column partnerid int(10) unsigned not null default 0 after eye;
###update_20161115_1

###update_20161118_1
  alter table mail add column receiveaccid int(10) unsigned not null default 0 after receiveid;
  alter table sysmail add column receiveaccid int(10) unsigned not null default 0 after receiveid;
  alter table mail add index receiveaccid_sysid_index (`receiveaccid`, `sysid`);
###update_20161118_1

###update_20161121_1
  update charbase set eye = 1 where gender = 1;
  update charbase set eye = 2 where gender = 2;
###update_20161121_1

###update_20161124_1
  alter table guild add column event blob after pack;
###update_20161124_1

###update_20161124_2
  update music set npcid = 1259 where npcid != 1259
###update_20161124_2

###update_20161201_1
  drop table if exists `charge`;
create table `charge` (
  `id`  bigint(20) unsigned not null auto_increment,
  `orderid` varchar(128) character set utf8 not null,
  primary key(`id`),
  unique index orderid (`orderid`) 
) engine=InnoDB default charset=utf8;
###update_20161201_1

###update_20161203_1
  delete from trade_item_info where refinelv > 0;
  alter table trade_item_info drop index itemid_refinelv;
  alter table trade_item_info add unique index itemid (`itemid`);
  alter table trade_item_info add column `t` int(10) unsigned not null default 0;
  alter table trade_item_info drop column `refinelv`;
  alter table trade_item_info drop column `last_sys_pending_time`;
###update_20161203_1

###update_20161206_1
  alter table trade_pending_list add column `name` varchar(512) default "";
###update_20161206_1

########################################  2.0.34  2016-12-07  #############################################

###update_20161208_1
  alter table sysmail add index sysid_index (`sysid`);
###update_20161208_1

###update_20161209_1
  alter table charbase add column portrait int(10) unsigned not null default 0 after partnerid;
###update_20161209_1

###update_20161215_1
  alter table guild add column bossraid blob after event;
###update_20161215_1

###update_20161221_1
  alter table mail add column `groupid` varchar(256) character set utf8 default "";
###update_20161221_1

###update_20161222_1
  alter table charbase add column `usedbattletime` int (10) unsigned not null default 0 after battletime;
  alter table charbase add column `rebattletime` int(10) unsigned not null default 0 after battletime;
###update_20161222_1

###update_20161223_1
  update charbase set portrait = 0;
###update_20161223_1


###update_20161226_1
  drop table if exists `trade_publicity`;
  create table `trade_publicity` (
    `id` int(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
    `uniqueid` varchar(255) not null,
    `itemid`  int(10) unsigned not null default 0,
    `endtime`  int(10) unsigned not null default 0,
    `price` int(20) unsigned not null default 0,
    `buy_people` int(10) unsigned not null default 0,
    unique index i_uniqueid (`uniqueid`)
  ) engine=InnoDB default charset=utf8;

  alter table trade_pending_list drop column `refine_data`;
  alter table trade_pending_list drop column `itemtype`;
  alter table trade_pending_list drop column `qualitytype`;
  alter table trade_pending_list drop column `namzh`;
  
  alter table trade_saled_list drop column `refine_data`;
  alter table trade_buyed_list drop column `refine_data`;
  
  alter table trade_pending_list add column `publicity_id` int(10) unsigned not null default 0;
  alter table trade_pending_list add column `endtime` int(10) unsigned not null default 0;
  
  alter table trade_buyed_list add column `logtype` int(10) unsigned not null default 2;
  alter table trade_saled_list add column `logtype` int(10) unsigned not null default 1;   

###update_20161226_1

###update_20161226_2
  drop table if exists `trade_publicity_buy`;
  create table `trade_publicity_buy` (
    `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
    `publicity_id` int(10) unsigned not null default 0,
    `buyerid` bigint(20) unsigned not null,
    `count`  int(10) unsigned not null default 0
  ) engine=InnoDB default charset=utf8;
###update_20161226_2

###update_20161228_1
  alter table trade_buyed_list add column `buyername` varchar(512) default "匿名";
###update_20161228_1

###update_20170105_1
  alter table charbase add column destzoneid int(10) unsigned not null default 0 after zoneid;
  alter table charbase add column originalzoneid int(10) unsigned not null default 0 after destzoneid;
###update_20170105_1

###update_20170106_1
  alter table mail add index sysid_index (`sysid`);
###update_20170106_1

######################################## release 1.0.0 merge #############################################

###update_20170117_1
  alter table social drop column accid;
  alter table social drop column lv;
  alter table social drop column portrait;
  alter table social drop column frame;
  alter table social drop column hair;
  alter table social drop column haircolor;
  alter table social drop column body;
  alter table social drop column offlinetime;
  alter table social drop column createtime;
  alter table social drop column profession;
  alter table social drop column gender;
  alter table social drop column manuallv;
  alter table social drop column manualexp;
  alter table social drop column title;
  alter table social drop column map;
  alter table social drop column zoneid;
  alter table social drop column blink;
  alter table social drop column name;
  alter table social drop column guildname;
  alter table social drop column guildportrait;
###update_20170117_1

###update_20170120_1
alter table trade_saled_list add index i_itemid_time_refine_lv_count(`itemid`, `tradetime`,`refine_lv`,`count`);
###update_20170120_1

###update_20170122_1
  alter table accbase add column credit blob after deletecountdays;
###update_20170122_1


###update_20170207_1
alter table trade_saled_list add column `tax` int(10) unsigned not null default 0;
alter table trade_saled_list add column `status` int(10) unsigned default 1;
alter table trade_saled_list add column `damage` int(10) unsigned default 0;
alter table trade_saled_list add column `buyerinfo` blob;
 
alter table trade_buyed_list add column `failcount` int(10) unsigned not null default 0;
alter table trade_buyed_list add column `status` int(10) unsigned default 0;            
alter table trade_buyed_list add column `endtime` int(10) unsigned default 0;     
alter table trade_buyed_list add column `damage` int(10) unsigned default 0;
alter table trade_buyed_list add column `totalcount` int(10) unsigned not null default 0;
alter table trade_buyed_list add column `sellerinfo` blob;           
###update_20170207_1

###update_20170210_1
  delete from globaldata where name = "quest";
###update_20170210_1

###update_20170210_2
delete from trade_saled_list;
delete from trade_buyed_list;
###update_20170210_2

###update_20161217_1
  alter table charbase add column mouth int(10) unsigned not null default 0 after partnerid;
###update_20161217_1

###update_20170217_2
drop table if exists `social_global`;
create table `social_global` (
  `id` bigint(20) unsigned not null default 0,                #id
  `destid` bigint(20) unsigned not null default 0,            #destid
  primary key(`id`, `destid`),
  index id_index (`id`),
  index destid_index (`destid`)
) engine=InnoDB default charset=utf8;
###update_20170217_2

###update_20170218_1
drop table if exists `quest_patch`;
create table `quest_patch` (
  `charid` bigint(20) unsigned not null default 0,                #id
  primary key(`charid`),
  index id_index (`charid`)
) engine=InnoDB default charset=utf8;
###update_20170218_1

###update_20170220_1
drop table if exists `chat_msg`;
create table `chat_msg` (
  `charid` bigint(20) unsigned not null default 0,                #id
  `portrait` int(10) unsigned not null default 0,                 #portrait
  `time` int(10) unsigned not null default 0,                     #time
  `data`    blob,                                                 #聊天内容
  primary key(`charid`, `time`),
  index id_index (`charid`)
) engine=InnoDB default charset=utf8;
###update_20170220_1

###update_20170220_2
  alter table team add column wantedquest blob DEFAULT NULL after apply;
###update_20170220_2

###update_20170223_1
alter table trade_buyed_list add index status_tradetime_index (status,tradetime);
alter table trade_saled_list add index status_tradetime_index (status,tradetime);
###update_20170223_1

###update_20170223_2
alter table sysmail add column starttime int(10) unsigned not null default 0 after time;
alter table sysmail add column endtime int(10) unsigned not null default 0 after starttime;
###update_20170223_2

###update_20170224_2
  alter table charbase add column `totalbattletime` int(10) unsigned not null default 0 after usedbattletime;
###update_20170224_2

###update_20170224_3
  alter table charbase drop column `totalbattletime`;
###update_20170224_3

###update_20170314_1
  alter table trade_buyed_list add column `receiveid` bigint(20) unsigned not null default 0 after `sellerinfo`;
  alter table trade_buyed_list add column `receivername` varchar(512) default "" after `receiveid`;
  alter table trade_buyed_list add column `receiverzoneid` int(10) unsigned not null default 0 after `receivername`;
  alter table trade_buyed_list add column `expiretime` int(10) unsigned default 0 after `receiverzoneid`;
  alter table trade_buyed_list add column `anonymous` bool not null default false after `expiretime`;
  alter table trade_buyed_list add column `background` int(10) unsigned default 0 after `anonymous`;
  alter table trade_buyed_list add column `content` varchar(512) default "" after `background`;
  alter table trade_buyed_list add index idx_receiveid (`receiveid`);  
  alter table trade_buyed_list add index idx_receiveid_expiretime (`receiveid`, `expiretime`);
###update_20170314_1

###update_20170401_1
  alter table stat_normal modify column value1 bigint(20) UNSIGNED NOT NULL DEFAULT 0;
###update_20170401_1

###update_20170410_1
  alter table accbase add column zoneid int(10) unsigned not null default 0 after charid;
###update_20170410_1

###update_20170414_1
  alter table charbase modify column silver bigint(20) unsigned not null default "0";
###update_20170414_1

###update_20170502_1
  alter table guild drop column bossraid;
###update_20170502_1

###update_20170506_1
  alter table trade_publicity add column `laststoptime` int(10) unsigned not null default 0;
###update_20170506_1

###update_20170506_2
  alter table trade_buyed_list add column `publicity_id` int(10) unsigned not null default 0;
  alter table trade_buyed_list add index index_publicity_id_endtime(`publicity_id`, `endtime`);
###update_20170506_2

###update_20170510_1
  alter table trade_publicity add column `starttime` int(10) unsigned not null default 0 after `itemid`;
###update_20170510_1



###update_20170518_1
  alter table accbase add column `pwdresettime` int(10) unsigned not null default "0" after `deletecountdays`;
  alter table accbase add column `password` blob default null after `pwdresettime`;
###update_20170518_1

###update_20170619_1
  create table `mini` (
    `zoneid` int(10) unsigned not null,    # zoneid
    `id` int(10) unsigned not null,        # npcid
    `mapid` int(10) unsigned not null,     # 地图id
    `refresh` int(10) unsigned not null,   # 刷新时间
    `dietime` int(10) unsigned not null,   # 死亡时间
    `killer` varchar(32) not null,         # 上次击杀者
    index `zoneid` (`zoneid`),
    unique index `zoneid_id_mapid` (`zoneid`,`id`,`mapid`)
  ) engine=InnoDB default charset=utf8;
###update_20170619_1
###update_20170622_1
create table `charge_card` (
  `charid` bigint(20) unsigned not null default 0,              #id
  `ym` int(10) unsigned not null default 0,                     #year*100+month
  `count` int(10) unsigned not null default 0,                  #次数
  primary key(`charid`, `ym`)
) engine=InnoDB default charset=utf8;
###update_20170622_1

###update_20170714_1
  alter table guild add column photo longblob after pack;
###update_20170714_1

###update_20170804_1
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
  ) engine=InnoDB default charset=utf8;
###update_20170804_1

###update_20170821_1
  alter table accbase add column quest longblob after credit;
###update_20170821_1

###update_20170811_1
  create table `activity` (
    `id` bigint(20) unsigned not null auto_increment,
    `name` varchar(128) character set utf8 not null default "", #活动名字
    `iconurl` varchar(512) character set utf8 not null default "",
    `begintime` int(10) unsigned not null default 0, #开始时间
    `endtime` int(10) unsigned not null default 0, #结束时间
    `url` varchar(512) character set utf8 not null default "", #活动链接
    `countdown` int(10) unsigned not null default 0, #客户端倒计时显示
    `md5` varchar(32) character set utf8 not null default "", #json序列md5值
    primary key(`id`)
  ) engine=InnoDB default charset=utf8;
  create table `activity_detail` (
    `id` bigint(20) unsigned not null auto_increment,
    `groupid` bigint(20) unsigned not null default 0, #活动表id
    `order` int(10) unsigned not null default 0, #活动序号
    `name` varchar(128) character set utf8 not null default "", #活动标题
    `begintime` int(10) unsigned not null default 0,
    `endtime` int(10) unsigned not null default 0,
    `path` int(10) unsigned not null default 0, #寻路id
    `url` varchar(512) character set utf8 not null default "", #活动链接
    `pic_url` varchar(512) character set utf8 not null default "", #图片url
    `md5` varchar(32) character set utf8 not null default "", #json序列md5值
    primary key(`id`),
    index i_groupid_order (`groupid`,`order`)
  ) engine=InnoDB default charset=utf8;
###update_20170811_1

###update_20170809_1
  alter table charge_card add column `fdcnt` int(10) unsigned not null default "0" after `count`;
###update_20170809_1

###update_20170905_1
drop table if exists `charge_accid`;
create table `charge_accid` (
  `accid` bigint(20) unsigned not null default 0,              #id
  `ym` int(10) unsigned not null default 0,                     #year*100+month
  `fdcnt` int(10) unsigned not null default 0,                  #福袋次数
  primary key(`accid`, `ym`)
) engine=InnoDB default charset=utf8;
###update_20170905_1

###update_20170911_1
  alter table charge_card drop column `fdcnt`;
###update_20170911_1
###update_20170920_1
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
  `choose` bool not null default false,
  `orderid` int(10) unsigned not null default 0,
  `signup_id` bigint(20) unsigned not null default 0,
  `offerprice_id` bigint(20)  unsigned not null default 0,
  `status` int(10) unsigned not null default 0,         #竞拍状态  
  `trade_price` bigint(20)  unsigned not null default 0,
  `time` int(10) unsigned not null default 0,
  unique index i_batchid_itemid(`batchid`, `itemid`)
) engine=InnoDB default charset=utf8;

drop table if exists `auction_item_signup`;
create table `auction_item_signup` (
  `id` bigint(20) unsigned not null PRIMARY KEY AUTO_INCREMENT,
  `batchid` bigint(20) unsigned not null,
  `itemid` int(10) unsigned not null,
  `charid` bigint(20) unsigned not null,
  `name` varchar(512) default "",
  `zoneid` int(10) unsigned not null,
  `type` int(10) unsigned not null,
  `take_status` int(10) unsigned not null default 0,
  `buyer_name` varchar(512) default "",
  `buyer_zoneid` int(10) unsigned not null default 0,
  `verify_time` int(10) unsigned not null default 0,
  `earn` bigint(20) unsigned not null default 0,            #税后赚取的钱
  `tax` bigint(20) unsigned not null default 0,             #扣税
  `time` int(10) unsigned not null default 0,
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
  `name` varchar(512) default "",
  `zoneid` int(10) unsigned not null,
  `price` bigint(20) unsigned not null,
  `type` int(10) unsigned not null,
  `take_status` int(10) unsigned not null default 0,
  `verify_time` int(10) unsigned not null default 0,
  `seller_name`  varchar(512) default "",
  `seller_zoneid` int(10) unsigned not null default 0,
  `time` int(10) unsigned not null default 0,
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
  `name` varchar(512) default "",
  `zoneid` int(10) unsigned not null,
  `max_price` int(10) unsigned not null default 0,
  `time` int(10) unsigned not null,
  index i_batchid_itemid(`batchid`, `itemid`)
) engine=InnoDB default charset=utf8;

###update_20170920_1

###update_20170927_1
  alter table charbase add column guildid bigint(20) unsigned not null default 0 after charid;
###update_20170927_1

###update_20171009_1
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
) engine=InnoDB default charset=utf8;
###update_20171009_1

###update_20171026_1
ALTER TABLE team CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE team MODIFY COLUMN name VARCHAR(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;

ALTER TABLE activity CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity MODIFY COLUMN name VARCHAR(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;

ALTER TABLE activity_detail CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity_detail MODIFY COLUMN name VARCHAR(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;

###update_20171026_1


###update_20171031_1
ALTER TABLE machine CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE machine MODIFY COLUMN `host` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE machine MODIFY COLUMN `ip` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE platform CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE platform MODIFY COLUMN `platname` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE platform MODIFY COLUMN `version` varchar(48) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE platform MODIFY COLUMN `min_version` varchar(48) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE platform MODIFY COLUMN `port` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE platform MODIFY COLUMN `auth_port` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE platform MODIFY COLUMN `msg` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE platform MODIFY COLUMN `picurl` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;

ALTER TABLE proxy CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE proxy MODIFY COLUMN `ip` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE proxy MODIFY COLUMN `version` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE region CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region MODIFY COLUMN `regionname` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region MODIFY COLUMN `nickname` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region MODIFY COLUMN `opentime` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region MODIFY COLUMN `maintainstart` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region MODIFY COLUMN `maintainend` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region MODIFY COLUMN `content` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region MODIFY COLUMN `tip` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region MODIFY COLUMN `picture` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE region_svrlist CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region_svrlist MODIFY COLUMN `servertype` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE region_svrlist MODIFY COLUMN `ip` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE zone CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE zone MODIFY COLUMN `zonename` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE zone MODIFY COLUMN `ip` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE cdkey CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE cdkey MODIFY COLUMN `code` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE cdkey MODIFY COLUMN `uid` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE cdkey MODIFY COLUMN `phoneplat` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE gm_log CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gm_log MODIFY COLUMN `servername` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gm_log MODIFY COLUMN `operator` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gm_log MODIFY COLUMN `targetname` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gm_log MODIFY COLUMN `action` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE service CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE service MODIFY COLUMN `type` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE service MODIFY COLUMN `cmd` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE gift_batch_list CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_batch_list MODIFY COLUMN `name` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_batch_list MODIFY COLUMN `regionids` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_batch_list MODIFY COLUMN `zoneids` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_batch_list MODIFY COLUMN `channelids` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_batch_list MODIFY COLUMN `pre_code` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_batch_list MODIFY COLUMN `items` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_batch_list MODIFY COLUMN `mail_title` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_batch_list MODIFY COLUMN `mail_msg` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE gift_code CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_code MODIFY COLUMN `code` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE gift_code_use CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_code_use MODIFY COLUMN `code` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gift_code_use MODIFY COLUMN `username` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE plat_account CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE plat_account MODIFY COLUMN `uid` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE plat_account MODIFY COLUMN `plat` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE proxy_version CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE proxy_version MODIFY COLUMN `server_version` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE client_code_version CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_code_version MODIFY COLUMN `phone_plat` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_code_version MODIFY COLUMN `branch` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_code_version MODIFY COLUMN `code_version` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_code_version MODIFY COLUMN `max_server_version` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE function_switch CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE function_switch MODIFY COLUMN `name` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE function_switch MODIFY COLUMN `description` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE plat_redirect CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE plat_redirect MODIFY COLUMN `plat_id` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE plat_redirect MODIFY COLUMN `client_code_version` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE plat_redirect MODIFY COLUMN `dest_plat_id` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE client_version CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_version MODIFY COLUMN `server` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_version MODIFY COLUMN `phone_plat` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_version MODIFY COLUMN `branch` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_version MODIFY COLUMN `version` varchar(2048) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_version MODIFY COLUMN `cur` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE client_version MODIFY COLUMN `appurl` varchar(2048) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE alter_msg CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE alter_msg MODIFY COLUMN `title` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE alter_msg MODIFY COLUMN `message` varchar(2048) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE resource_package CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE resource_package MODIFY COLUMN `id` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE resource_package MODIFY COLUMN `etag` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE char_store CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE char_store MODIFY COLUMN `itemid` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE auction_item_signup CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE auction_item_signup MODIFY COLUMN `name` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE auction_item_signup MODIFY COLUMN `buyer_name` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE auction_offerprice CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE auction_offerprice MODIFY COLUMN `name` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE auction_offerprice MODIFY COLUMN `seller_name` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE auction_event CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE auction_event MODIFY COLUMN `name` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE charge CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE charge MODIFY COLUMN `orderid` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE updatesql CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE updatesql MODIFY COLUMN `fileList` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE guildmusic CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE guildmusic MODIFY COLUMN `name` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE mail CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE mail MODIFY COLUMN `title` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE mail MODIFY COLUMN `sender` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE mail MODIFY COLUMN `msg` varchar(2048) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE mail MODIFY COLUMN `groupid` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE sysmail CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE sysmail MODIFY COLUMN `title` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE sysmail MODIFY COLUMN `sender` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE sysmail MODIFY COLUMN `msg` varchar(2048) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE boss CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE boss MODIFY COLUMN `killer` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE offmsg CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE offmsg MODIFY COLUMN `sendername` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE offmsg MODIFY COLUMN `msg` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE offmsg MODIFY COLUMN `gmcmd` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE globaldata CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE globaldata MODIFY COLUMN `name` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE guild CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE guild MODIFY COLUMN `name` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE guild MODIFY COLUMN `board` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE guild MODIFY COLUMN `recruit` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE guild MODIFY COLUMN `portrait` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE mini CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE mini MODIFY COLUMN `killer` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;


ALTER TABLE activity CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity MODIFY COLUMN `name` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity MODIFY COLUMN `iconurl` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity MODIFY COLUMN `url` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity MODIFY COLUMN `md5` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE activity_detail CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity_detail MODIFY COLUMN `name` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity_detail MODIFY COLUMN `url` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity_detail MODIFY COLUMN `pic_url` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE activity_detail MODIFY COLUMN `md5` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;



ALTER TABLE gateinfo CONVERT TO CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;
ALTER TABLE gateinfo MODIFY COLUMN `ipstr` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin;

###update_20171031_1

###update_20171102_1
  alter table auction_item add column `zeny_price` bigint(20) unsigned not null after base_price;
###update_20171102_1

###update_20171130_1
  alter table charge_accid add column  `modern` int(10) unsigned not null default 0;                
###update_20171130_1

  

###update_20171120_1
ALTER TABLE accbase add column  `nologintime` int(10) unsigned not null default 0 after `pwdresettime`;
###update_20171120_1

###update_20171122_1
create table `shop` (
  `zoneid` int(10) unsigned not null,              
  `refresh` int(10) unsigned not null,             
  `data`    blob,                                               
  primary key(`zoneid`)
) engine=InnoDB default charset=utf8;
###update_20171122_1

###update_20171011_1
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
###update_20171011_1

###update_20171207_1
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
###update_20171207_1

###update_20171208_1
drop table if exists `ach_cat_patch`;
drop table if exists `quest_patch_2`;
CREATE TABLE `quest_patch_2` (
  `accid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `quest_39300` int(10) unsigned NOT NULL DEFAULT '0',
  `quest_39301` int(10) unsigned NOT NULL DEFAULT '0',
  `quest_39309` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`accid`),
  KEY `id_index` (`accid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
###update_20171208_1

###update_20171220_1
drop table if exists `guild_icon`;
create table `guild_icon` (
  `id` bigint(20) unsigned not null default 0,         # key = guildid*1000+iconindex
  `guildid` bigint(20) unsigned not null default 0,    # 公会id
  `charid` bigint(20) unsigned not null default 0,     # 玩家id
  `iconindex` int(10) unsigned not null default 0,     # 图标索引
  `createtime` int(10) unsigned not null default 0,    # 申请时间
  `state` int(10) unsigned not null default 0,         # 状态
  `isread` int(10) unsigned not null default 0,        # 是否读取
  primary key (`id`),
  index index_state (`state`),
  index index_guildid (`guildid`)
) engine=InnoDB default charset=utf8;

###update_20171220_1

###update_20171123_1
ALTER TABLE `guildoffline` ADD COLUMN `var` blob DEFAULT "" AFTER `donate`;
ALTER TABLE `guildoffline` ADD COLUMN `building` blob DEFAULT "" AFTER `var`;
ALTER TABLE `guildoffline` ADD COLUMN `exittime` int(10) unsigned not null default 0 AFTER `building`;
###update_20171123_1
###update_20171219_1
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
###update_20171219_1

###update_20171225_1
  update questconfig set txt=replace(txt, 'kill_monster', 'kill_groupId');
  update questconfig set txt=replace(txt, 'gather_monster', 'gather_groupId');
###update_20171225_1

###update_20171228_1
#新增公会城池
alter table guild_city add column city_1004 bigint(20) unsigned not null default 0 after city_1003;
alter table guild_city add column city_2004 bigint(20) unsigned not null default 0 after city_2003;
alter table guild_city add column city_3004 bigint(20) unsigned not null default 0 after city_3003;
alter table guild_city add column city_4004 bigint(20) unsigned not null default 0 after city_4003;
###update_20171228_1

###update_20171211_1
#道具兑换码
drop table if exists `item_code`;
create table `item_code` (
  `code` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `state` int(10) unsigned not null,      #0,初始 1：使用 2：兑换过
  `charid` bigint(20) unsigned not null default 0, #兑换的玩家cahrid
  `guid` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null,   #使用的道具guid
  `itemid` int(10) unsigned not null,      #道具it
  `usetime` int(10) unsigned not null,     #兑换的时间  
  primary key(`code`),
  index i_state(`state`),
  index i_guid(`guid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
###update_20171211_1

###update_20171226_1
#全服限购商店
drop table if exists `global_shop`;
create table `global_shop` (
  `id` int(10) unsigned not null,         #shop表id
  `count`int(10) unsigned not null,       #出售次数
  primary key(`id`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
###update_20171226_1
###update_20171229_1
#充值-ep特典
drop table if exists `charge_epcard`;
create table `charge_epcard` (
  `accid` bigint(20) unsigned not null default 0,              
  `dataid` int(10) unsigned not null default 0,                 #充值id 折扣的算原价id
  `epcard` int(10) unsigned not null default 0,                 #次数
  primary key(`accid`, `dataid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
###update_20171229_1

###update_20180104_1
#召回列表
drop table if exists `social_recall`;
create table `social_recall` (
  `id` bigint(20) unsigned not null default 0,                #id
  `destid` bigint(20) unsigned not null default 0,            #destid
  primary key(`id`, `destid`),
  index id_index (`id`),
  index destid_index (`destid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
###update_20180104_1

###update_20180111_1
#公会离线操作
drop table if exists `guild_gm`;
create table `guild_gm` (
  `id` bigint(20) unsigned not null auto_increment,           #id
  `guildid` bigint(20) unsigned not null default 0,           #公会id
  `charid` bigint(20) unsigned not null default 0,            #charid
  `data` longblob,                                            #指令字符串
  primary key(`id`)
) ENGINE=InnoDB AUTO_INCREMENT=65535 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
###update_20180111_1

###update_20180115_1
drop table if exists `auction_can_signup`;  
create table `auction_can_signup` (
  `itemid` int(10) unsigned not null,
  `auction` int(10) unsigned not null,
  primary key i_itemid(`itemid`)
) engine=InnoDB default charset=utf8;
###update_20180115_1

###update_20180122_1
drop table if exists `guild_city`;
create table `guild_city`(
  `zoneid` int(10) unsigned not null default 0,
  `cityid` int(10) unsigned not null default 0,
  `guildid` bigint(20) unsigned not null default 0,
  `times` int(10) unsigned not null default 0,
  `oldguildid` bigint(20) unsigned not null default 0,
  primary key (`zoneid`,`cityid`),
  key (`zoneid`)
) engine=InnoDB default charset=utf8;
###update_20180122_1
###update_20180129_1
  alter table guild_icon add column `format` varchar(32) character SET utf8mb4 COLLATE utf8mb4_bin not null default "";
###update_20180129_1

###update_20180208_1
drop table if exists `achieve_patch`;
###update_20180208_1

###update_20180131_1
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
   `status`    int(10) unsigned NOT NULL DEFAULT '0',    #状态
  PRIMARY KEY (`id`),
 KEY `index_charid1` ( `charid1`),
 KEY `index_charid2` ( `charid2`),
 KEY `index_status` ( `status`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
###update_20180131_1

###update_20180306_1
drop table if exists `charge_virgin`;
create table `charge_virgin` (
  `accid` bigint(20) unsigned not null default 0,
  `tag` varchar(2048) character set utf8mb4 COLLATE utf8mb4_bin not null,    #标识
  primary key(`accid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
###update_20180306_1
###update_20180228_1
  alter table auction_event add column `player_id` bigint(20) unsigned not null default 0;
###update_20180228_1

###update_20180312_1
alter table `item_code` add column `type` int(10) unsigned not null default 0 after `code`;
update `item_code` set `type`=4000;
alter table `item_code` drop primary key;
alter table `item_code` add primary key(`code`, `type`);
###update_20180312_1

###update_20180404_1
drop table if exists `ach_cat_patch`;
drop table if exists `quest_patch_2`;
###update_20180404_1

###update_20180413_1
drop table if exists `stat_kill_monster`;
create table `stat_kill_monster`(
  `time` int(10) unsigned not null default 0,
  `monsterid` int(10) unsigned not null default 0,
  `rank` int(10) unsigned not null default 0,
  `num` int(10) unsigned not null default 0,
  `userid` bigint(20) unsigned not null default 0,
  `zone` int(10) unsigned not null default 0,
  `profession` int(10) unsigned not null default 0,
  primary key(`time`,`monsterid`,`rank`),
  key(`time`),
  key(`monsterid`)
) engine=InnoDB default charset=utf8;
###update_20180413_1

###update_20180419_1
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
###update_20180419_1

###update_20180416_1
drop table if exists `gm_batch_job`;
CREATE TABLE `gm_batch_job` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `type`  int(10) unsigned NOT NULL DEFAULT '0',
  `progress`  int(10) unsigned NOT NULL DEFAULT '0',
  `task` longblob,
  `result` longblob,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_bin;
###update_20180416_1

###update_20180505_1
  alter table charbase add column clothcolor int(10) unsigned not null default 0 after portrait;
###update_20180505_1

###update_20180505_2
  alter table accbase add column maincharid bigint(20) unsigned not null default 0 after `nologintime`;
###update_20180505_2
###update_20180512_1
#多职业备份
drop table if exists `char_profession`;
create table `char_profession` (
  `charid` bigint(20) unsigned not null default 0,  # 玩家guid
  `branch` int(10) unsigned not null default 0,     # 分支
  `data`   blob,                                    # 分支数据
  primary key(`charid`, `branch`),
  index char_index (`charid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
###update_20180512_1
###update_20180517_1
  alter table charbase add column maxpro int(10) unsigned not null default 0 after `nologintime`;
###update_20180517_1

###update_20180511_1
drop table if exists `guild_city_history`;
create table `guild_city_history`(
  `time` int(10) unsigned not null default 0,
  `zoneid` int(10) unsigned not null default 0,
  `cityid` int(10) unsigned not null default 0,
  `guildid` bigint(20) unsigned not null default 0,
  `guildname` varchar(32) character set utf8mb4 not null,
  `leadername` varchar(64) character set utf8mb4 not null,
  `times` int(10) unsigned not null default 0,
  `perfect` bool not null default false,
  primary key (`time`,`zoneid`,`cityid`),
  key (`time`),
  key (`zoneid`)
) engine=InnoDB default charset=utf8;
###update_20180511_1

###update_20180615_1
drop table if exists `day_get_zeny_top`;
create table `day_get_zeny_top`(
  `time` int(10) unsigned not null default 0,
  `charid` bigint(20) unsigned not null default 0,  # 玩家guid
  `name` varchar(64) character set utf8mb4 not null, #角色名
  `baselv` int(10) unsigned not null default 0,
  `joblv` int(10) unsigned not null default 0,
  `profession` int(10) unsigned not null default 0,
  `normalzeny` bigint(20) unsigned not null default 0,
  `chargezeny` bigint(20) unsigned not null default 0,
  `totalzeny` bigint(20) unsigned not null default 0,
  primary key (`time`)
) engine=InnoDB default charset=utf8;
###update_20180615_1

###update_20180601_1
  alter table auction_item_signup add column fm_point int(10) unsigned default 0 after `time`;
  alter table auction_item_signup add column fm_buff int(10) unsigned default 0 after `time`;
  alter table auction_item_signup add column itemdata blob after `time`;
  alter table auction_item_signup add column zeny_price bigint(20) unsigned default 0 after `time`;
  alter table auction_item_signup add column base_price bigint(20) unsigned default 0 after `time`;

  alter table auction_item drop index i_batchid_itemid;
  alter table auction_item add column auction int(10) unsigned default 0 after `time`;
  alter table auction_item add unique key `i_batchid_itemid_signupid` (`batchid`, `itemid`, `signup_id`);

  alter table auction_offerprice add column signup_id bigint(20) unsigned default 0 after `time`;
  alter table auction_offerprice add column itemdata blob;

  alter table auction_event add column `signup_id` bigint(20) unsigned not null default 0;
###update_20180601_1

###update_20180704_1
alter table `activity` add column `data` blob after `countdown`;
alter table `activity_detail` add column `data` blob after `pic_url`;

drop table if exists `mail_template`;
create table `mail_template` (
  `id` int(10) unsigned not null auto_increment,
  `data` blob,
  primary key (`id`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

use ro_global;
alter table `zone` add column `language` int(10) unsigned not null default 10 after `port`;

use ro_global;
drop table if exists `region_notice`;
create table `region_notice` (
  `regionid` int(10) unsigned not null,
  `language` int(10) unsigned not null,
  `content` varchar(1024) character set utf8mb4 COLLATE utf8mb4_bin not null,        #服务器维护内容文字
  `tip` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin not null,             #服务器维护提示文字
  primary key (`regionid`, `language`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
###update_20180704_1

###update_20180711_1
  alter table boss add column killerid bigint(20) unsigned not null default 0 after killer;
  alter table mini add column killerid bigint(20) unsigned not null default 0 after killer;
  update boss,charbase set boss.killerid=charbase.charid where boss.killer=charbase.name;
  update mini,charbase set mini.killerid=charbase.charid where mini.killer=charbase.name;
###update_20180711_1

###update_20180802_1
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
###update_20180802_1

###update_20180619_1
drop table if exists `world_level_stat`;
create table `world_level_stat`(
  `time` int(10) unsigned not null default 0,
  `totalbaselevel` int(10) unsigned not null default 0,
  `totalbaselevelcount` int(10) unsigned not null default 0,
  `totaljoblevel` int(10) unsigned not null default 0,
  `totaljoblevelcount` int(10) unsigned not null default 0,
  primary key (`time`)
) engine=InnoDB default charset=utf8;
###update_20180619_1

###update_20180803_1
drop table if exists `global`;
create table `global` (
  `name` varchar(32) not null,              #键值
  `data` blob,                              #成员
  primary key (`name`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

alter table `boss` add column `settime` int(10) unsigned not null default 0 after `killerid`;
alter table `boss` add column `lv` int(10) unsigned not null default 0 after `settime`;
alter table `boss` add column `deadkiller` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null after `killerid`;
alter table `boss` add column `deadkillerid` bigint(20) unsigned NOT NULL DEFAULT 0 after `deadkiller`;
###update_20180803_1

###update_20180819_1
drop table if exists `team_pws`;
create table `team_pws`(
  `charid` bigint(20) unsigned not null,
  `score` int(10) unsigned not null default 0,
  `rank` int(10) unsigned not null default 0,
  `time` bigint(20) unsigned not null default 0,
  primary key(`charid`),
  key(`rank`)
) engine=InnoDB default charset=utf8mb4;
###update_20180819_1

###update_20180906_1
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
###update_20180906_1

###update_20180913_1
drop table if exists `cheat_tag`;
create table `cheat_tag` (
  `charid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `mininterval` int(10) unsigned NOT NULL DEFAULT '0',
  `frame` int(10) unsigned NOT NULL DEFAULT '0',
  `count` int(10) unsigned NOT NULL DEFAULT '0',
  primary key(`charid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
###update_20180913_1

###update_20180921_1
alter table `boss` add column `summontime` int(10) unsigned not null default 0 after `deadkiller`;
###update_20180921_1

