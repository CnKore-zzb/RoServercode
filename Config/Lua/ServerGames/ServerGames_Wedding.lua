--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.Wedding = {
	max_reserve_day = 14,     --能提前预定的最大天数
	max_ticket_reserve_day = 21,  --使用券预定的最大天数
	DivorceBuffid = 146,	--离婚惩罚buff id
	wedding_raid_id = 10008,	--婚礼副本id
	wedding_manual_itemid =6055,  --结婚手册道具id

	wedding_quest_show=396110001,--与主教对话任务ID
	pre_question = {1,2,3,4,5,6,7},
	first_stage_question={8,9,10,11},-- 第一阶段问题ID,对应Question表ID
	second_stage_question={12,13},--第二阶段问题ID, 对应Question表
	wedding_msg_time = {5,2},--举办前15分钟，5分钟婚礼全线公告
	wedding_msg_id=9616, -- 婚礼公告id
	invitation_mail_id = 2001, -- 邀请函邮件id
	invitation_item_id = 6056, -- 邀请函道具id
	del_manual_mail_id = 2002, -- 删除手册邮件id
	del_invitation_mail_id = 2003, -- 删除邀请函邮件id
	wedding_dress_letter_id = 6075, --婚纱情书idf

	wedding_ring_id = 6057, -- 默认婚戒id
	wedding_certificate_id = 5802,  --结婚证id

	divorce_carrierid = 22,    --离婚过山车载具id
	divorce_roller_coaster_mapid = 250001,   --离婚过山车map

	default_ring_id = 6057,  --默认自动购买的戒指id
	top_package_id = 6078,  ----表示最高级套餐id
	wedding_carrier_id = 2, ----婚礼载具id
	wedding_carrier_line = 1, ----婚礼载具的line
	
	roller_coaster_mapid = 10009,   --结婚过山车

    marry_skill = {2000001,2003001},  --结婚技能
	wedding_team_buff = 139,  --守护你BUFF

	ring_shop_type = 6015,       -----戒指商店类型,
	ring_shop_id = 1,        -----戒指商店id,

	ring_shop_type = 6015,       -----戒指商店类型,
	ring_shop_id = 1,        -----戒指商店id,
		
	wedding_gender_check = 1, --婚姻性别检查
}
