#update—20171214-01
ALTER TABLE `trade_record_sold` ADD `log_id` VARCHAR(32) NOT NULL COMMENT '唯一id防止重复插入' AFTER `item_data`;
UPDATE `trade_record_sold` SET `log_id` = `id`;
ALTER TABLE `trade_record_sold` ADD UNIQUE (`log_id`);

ALTER TABLE `trade_record_bought` ADD `log_id` VARCHAR(32) NOT NULL COMMENT '唯一id防止重复插入' AFTER `item_data`;
UPDATE `trade_record_bought` SET `log_id` = `id`;
ALTER TABLE `trade_record_bought` ADD UNIQUE (`log_id`);

ALTER TABLE `zzz_archived_trade_record_sold_201712` ADD `log_id` VARCHAR(32) NOT NULL COMMENT '唯一id防止重复插入' AFTER `item_data`;
ALTER TABLE `zzz_archived_trade_record_sold_201711` ADD `log_id` VARCHAR(32) NOT NULL COMMENT '唯一id防止重复插入' AFTER `item_data`;
ALTER TABLE `zzz_archived_trade_record_bought_201711` ADD `log_id` VARCHAR(32) NOT NULL COMMENT '唯一id防止重复插入' AFTER `item_data`;
ALTER TABLE `zzz_archived_trade_record_bought_201712` ADD `log_id` VARCHAR(32) NOT NULL COMMENT '唯一id防止重复插入' AFTER `item_data`;
#update—20171214-01


#update-20171226-01
ALTER TABLE `trade_record_sold` ADD INDEX(` goods_id `);
#update-20171226-01

#update-20180226-01
CREATE TABLE `trade_finance` ( `id` INT UNSIGNED NOT NULL AUTO_INCREMENT , `item_id` INT UNSIGNED NOT NULL COMMENT '物品id' , `price` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT '当前价格' , `last_price` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT '上次最后价格' , `deal_count` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT '成交量' , `time` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT '统计时间' , `created_time` INT UNSIGNED NOT NULL DEFAULT '0' COMMENT '创建记录时间' , PRIMARY KEY (`id`)) ENGINE = InnoDB;
#update-20180226-01

#update-20180412-01
CREATE TABLE `trade_finance` (
  `id` int(10) UNSIGNED NOT NULL,
  `item_id` int(10) UNSIGNED NOT NULL COMMENT '物品id',
  `price` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '当前价格',
  `last_price` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '上次最后价格',
  `deal_count` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '成交量',
  `last_deal_count` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '上次成交量',
  `price_ratio` float NOT NULL DEFAULT '0' COMMENT '涨跌幅',
  `deal_count_ratio` float NOT NULL DEFAULT '0' COMMENT '成交量涨跌幅',
  `time` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '统计时间',
  `created_time` int(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '创建记录时间'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `trade_finance`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `itemid_time` (`item_id`,`time`),
  ADD KEY `time` (`time`);

ALTER TABLE `trade_finance`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
#update-20180412-01


#update-20180810-01
ALTER TABLE `trade_prohibition` ADD `trade_type` SMALLINT NOT NULL DEFAULT '1' COMMENT '交易所系统类型：0：所有，1：交易所，2：摆摊' AFTER `id`;
ALTER TABLE `trade_prohibition` DROP INDEX `unique_id`, ADD UNIQUE `unique_id` (`trade_type`, `unique_id`(128)) USING BTREE;
#update-20180810-01


#update-20180813-01
ALTER TABLE `trade_item_list` ADD `is_hot` TINYINT UNSIGNED NOT NULL DEFAULT '1' COMMENT '是否热数据' AFTER `delay_time`;
#update-20180813-01
