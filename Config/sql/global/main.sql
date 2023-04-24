# 机器列表
drop table if exists `machine`;
create table `machine` (
  `id` int(10) unsigned not null default 0,
  `host` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `ip` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  primary key (`id`),
  unique index machine_host (`host`),
  unique index machine_ip (`ip`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

# 平台列表
drop table if exists `platform`;
create table `platform` (
  `platid` int(10) unsigned NOT NULL,
  `platname` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `version` varchar(48) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `min_version` varchar(48) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `port` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `auth_port` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `starttime` int(10) unsigned NOT NULL default 0,
  `endtime` int(10) unsigned NOT NULL default 0,
  `msg` varchar(1024) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `tips` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `picurl` varchar(256) character set utf8mb4 COLLATE utf8mb4_bin default "",
  primary key (`platid`),
  unique key `platname` (`platname`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

# 代理网关列表
drop table if exists `proxy`;
create table `proxy` (
  `id` int(10) unsigned NOT NULL,
  `ip` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `port` int(10) unsigned not null,
  `ext_port` int(10) unsigned not null,
  `version` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  primary key (`id`),
  unique index ip_port (`ip`, `port`),
  index index_version (`version`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
alter table proxy add `version` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null;

# 大区列表
drop table if exists `region`;
create table `region` (
  `regionid` int(10) unsigned NOT NULL,
  `platid` int(10) unsigned NOT NULL,
  `regionname` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `nickname` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `status` int(10) unsigned NOT NULL,
  `hide` int(10) unsigned NOT NULL,
  `opentime` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `maintainstart` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `maintainend` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `content` varchar(1024) character set utf8mb4 COLLATE utf8mb4_bin not null,        #服务器维护内容文字
  `tip` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin not null,             #服务器维护提示文字
  `picture` varchar(256) character set utf8mb4 COLLATE utf8mb4_bin not null,         #服务器维护图片url
  primary key (`regionid`),
  unique index platid_regionname (`platid`, `regionname`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

# 大区-服列表
drop table if exists `region_svrlist`;
create table `region_svrlist` (
  `regionid` int(10) unsigned NOT NULL,
  `servertype` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `ip` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `port` int(10) unsigned not null,
  unique index ip_port (`ip`, `port`),
  primary key regionid_t(`regionid`, `servertype`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

# 区列表
drop table if exists `zone`;
create table `zone` (
  `zoneid` bigint(20) unsigned NOT NULL,
  `platid` int(10) unsigned NOT NULL,
  `regionid` int(10) unsigned NOT NULL,
  `zonename` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `ip` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `port` int(10) unsigned not null,
  `language` int(10) unsigned not null default 10,
  `active` int(10) unsigned not null,
  `online` int(10) unsigned not null,
  `status` int(10) unsigned not null,
  `opentime` int(10) unsigned not null,
  `mergeid` bigint(20) unsigned NOT null,
  `openindex` int(10) unsigned not null,
  `category` int(10) unsigned not null default 0,
  primary key (`zoneid`),
  unique index ip_port (`ip`, `port`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

 alter table zone add index opentime_index(`opentime`);

# 白名单
drop table if exists `whitelist`;
create table `whitelist` (
  `accid` bigint(20) unsigned NOT NULL,
  `flag` int(10) unsigned NOT NULL,
  primary key (`accid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

# 激活码
drop table if exists `cdkey`;
create table `cdkey` (
  `code` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `used` int(10) unsigned not null,
  `uid` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `idx` int(10) unsigned not null,
  `phoneplat` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  primary key (`code`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

# 激活列表
drop table if exists `activatedlist`;
create table `activatedlist` (
  `accid` bigint(20) unsigned NOT NULL,
  primary key (`accid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#GM后台日志
drop table if exists `gm_log`;
create table `gm_log` (
  `id`  bigint(20) unsigned not null auto_increment,
  `serverid` bigint(20) unsigned NOT NULL,
  `servername` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `time` int(10) unsigned not null,                           #操作时间
  `operator`   varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null,      #操作者
  `targetid`  bigint(20) unsigned not null default 0,         #对象id
  `targetname`  varchar(128) character set utf8mb4 COLLATE utf8mb4_bin default "",   #对面名字
  `action` varchar(1024) character set utf8mb4 COLLATE utf8mb4_bin not null,         #操作详情
   primary key(`id`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

# 服务配置
drop table if exists `service`;
create table `service` (
  `type`  varchar(32) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `machineid` int(10) unsigned not null,
  `port` int(10) unsigned not null,
  `inner_port` int(10) unsigned not null,
  `valid` int(10) unsigned not null,
  `cmd`  varchar(32) character set utf8mb4 COLLATE utf8mb4_bin default "",
  primary key index_machineid_port (`machineid`, `inner_port`),
  index index_port(`port`),
  index index_type(`type`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#礼包码批次
drop table if exists `gift_batch_list`;
create table `gift_batch_list` (
  `id`  int(20) unsigned not null auto_increment,
  `groupid`  bigint(20) unsigned not null default 0,
  `name` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `type` int(10) unsigned not null,
  `regionids` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `zoneids` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `channelids` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin default "",
  `pre_code` varchar(32) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `num` int(10) not null,
  `items` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `expiretime` int(10) unsigned not null,
  `mail_title` varchar(64) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `mail_msg` varchar(64) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `enable` int(10) not null default 1,               #是否禁用，1不，0禁用
  `maxcount` int(10) unsigned not null default 1,
  `createtime` int(10) unsigned not null default 0,
  primary key(`id`),
  unique index id_groupid (`id`, `groupid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#礼包码
drop table if exists `gift_code`;
create table `gift_code` (
  `code` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `batchid` int(10) unsigned not null,
  `createtime` int(10) unsigned not null,
  primary key(`code`),
  index i_batchid(`batchid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

#礼包码使用记录
drop table if exists `gift_code_use`;
create table `gift_code_use` (
  `accid` bigint(20) unsigned not null,
  `charid` bigint(20) unsigned not null,
  `batchid`  int(20) unsigned not null,
  `code` varchar(128) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `usetime` int(10) unsigned not null,
  `username` varchar(64) character set utf8mb4 COLLATE utf8mb4_bin not null,
  `groupid`  bigint(20) unsigned not null default 0,
  primary key accid_code (`accid`, `code`),
  index accid_groupid (`accid`, `groupid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

alter table gift_code_use add index index_code(`code`);

drop table if exists `plat_account`;
create table `plat_account` (
  `accid` bigint(20) unsigned not null auto_increment,   #accid 平台唯一
  `uid` varchar(128) not null,
  `plat` varchar(32) not null,
  primary key (`accid`),
  unique index (`plat`, `uid`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin auto_increment=10000;

drop table if exists `proxy_version`;
create table `proxy_version` (
  `server_version` varchar(32) not null,
  `port` int(10) unsigned not null,
  primary key (`server_version`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

drop table if exists `client_code_version`;
create table `client_code_version` (
  `phone_plat` varchar(32) not null,
  `branch` varchar(32) not null,
  `code_version` varchar(32) not null,
  `max_server_version` varchar(32) not null,
  `enabled` int(10) not null,
  primary key (`phone_plat`, `branch`, `code_version`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

drop table if exists `function_switch`;
create table `function_switch` (
  `name` varchar(32) not null,
  `status` int(10) unsigned not null,
  `description` varchar(128) not null,
  primary key (`name`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

drop table if exists `plat_redirect`;
create table `plat_redirect` (
  `plat_id` varchar(32) not null,
  `client_code_version` varchar(32) not null,
  `dest_plat_id` varchar(32) not null,
  primary key (`plat_id`, `client_code_version`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

drop table if exists `client_version`;
create table `client_version` (
  `server` varchar(64) not null,
  `phone_plat` varchar(32) not null,
  `branch` varchar(32) not null,
  `version` varchar(2048) not null,
  `cur` varchar(32) not null,
  `appurl` varchar(2048) not null,
  primary key (`server`, `phone_plat`, `branch`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

drop table if exists `alter_msg`;
create table `alter_msg` (
  `id`  int(20) unsigned not null auto_increment,    # id 主键 自增
  `title` varchar(64) not null default "",
  `message` varchar(2048) not null default "",
  `notify` int(10) unsigned not null default 0,
  `event` int(10) unsigned not null default 0,
  primary key (`id`),
  index notify_index(`notify`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;

drop table if exists `resource_package`;
create table `resource_package` (
  `id`  varchar(512) not null default "",
  `etag` varchar(1024) not null default "",
  `size` int(20) unsigned not null default 0,
  primary key (`id`)
) engine=InnoDB default charset=latin1;

#大区公告
drop table if exists `region_notice`;
create table `region_notice` (
  `regionid` int(10) unsigned not null,
  `language` int(10) unsigned not null,
  `content` varchar(1024) character set utf8mb4 COLLATE utf8mb4_bin not null,        #服务器维护内容文字
  `tip` varchar(512) character set utf8mb4 COLLATE utf8mb4_bin not null,             #服务器维护提示文字
  primary key (`regionid`, `language`)
) engine=InnoDB default charset=utf8mb4 COLLATE utf8mb4_bin;
