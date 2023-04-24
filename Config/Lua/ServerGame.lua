--md5:ServerGames_Artifact.lua:d1c5f3b08483743961371f4ad73e32d1
--md5:ServerGames_GuildChallenge.lua:740fc0b7573cd92a48aa4a7f00166da8
--md5:ServerGames_GuildWelfare.lua:03b27c151c7491ccb5b72ea3707d4da1
--md5:ServerGames_Quota.lua:420466b4e9684da9d9d0b38ae48024af
--md5:ServerGames_Wedding.lua:292e1626efa3033177201b3f816d0fe6
--md5:ServerGame_Achieve.lua:af86a10a8d89efb85b72231cc5321076
--md5:ServerGame_Arena.lua:ac7e1468ac93b060310018a87bcebdd5
--md5:ServerGame_Auction.lua:d36168c4843fddb59452e8d7e611e42a
--md5:ServerGame_Boss.lua:4a5e1a564bc194cea3a7be1ef7448967
--md5:ServerGame_Buff.lua:c7f67526a531f8003155c2b4ed53e10c
--md5:ServerGame_Camera.lua:4d29d90bccf88e0073c7beabdf947f6b
--md5:ServerGame_CapraActivity.lua:021037198e1d950c415d00d99eed5ab3
--md5:ServerGame_Card.lua:0acdf35d440bce80d55a119a6e50d30a
--md5:ServerGame_ChatChannel.lua:cbceb11e61f64d8c5744d215ebdcbcf6
--md5:ServerGame_Courier.lua:6055e6d7373433f6f6e085b049214fd0
--md5:ServerGame_Credit.lua:06e55c486c6c2e358158363f1d2b1d8b
--md5:ServerGame_Deposit.lua:2d51542776d40f5dbce20fa8357314d3
--md5:ServerGame_EffectPath.lua:ca0f4d9fb88cfcdc0cab5a49ebb56dfb
--md5:ServerGame_ExtraReward.lua:4dac9d6790b991c9cc2f7c8f4bcb5477
--md5:ServerGame_Food.lua:0d9760881ec9109847da04f8df13fc9f
--md5:ServerGame_FriendShip.lua:c2235e04c00f13a5b9c80ee87d0821e4
--md5:ServerGame_FunctionSwitch.lua:fd6960f2f7110bc849db0b0c85616862
--md5:ServerGame_GlobalActivity.lua:6df4f0a8fcab67057900c5a0c25ff92f
--md5:ServerGame_Guild.lua:4a25d321eebc0bc3fb9b658094ae632a
--md5:ServerGame_GuildRaid.lua:24cfb90ed5a37eb036f9d7bd3d6cad81
--md5:ServerGame_Item.lua:4e5dd1f1bb01d72d889fdc9f9ff18525
--md5:ServerGame_ItemImage.lua:b0db521532c8296696218a84f78436b7
--md5:ServerGame_LockEffect.lua:823a0fc2f10532f68742c9aa8711294b
--md5:ServerGame_Lottery.lua:7a3d1ac3f1eca9a042752161fbd6288c
--md5:ServerGame_Manual.lua:39896e5ea5b8f28cab1f2b1b6c0d425c
--md5:ServerGame_MercenaryCat.lua:650aac454902460700dfb9a233ab8613
--md5:ServerGame_MoneyCat.lua:e2277372bbeb5c840aee2bad258cb584
--md5:ServerGame_MonsterRelive.lua:dadf8d19acac0c92e4033e594d609fdb
--md5:ServerGame_MontchCardActivity.lua:f68578a0adb2b663597537fbe4c11f1d
--md5:ServerGame_MvpScore.lua:ebb851ef87fc29d28ccba6baf0b71586
--md5:ServerGame_OperateReward.lua:e482e83926fa262cbdae8865bd4ffc64
--md5:ServerGame_Pet.lua:fb789f5c94a306bc22040f4dfd8bbe64
--md5:ServerGame_Photo.lua:f4bbc2eff180511c46ffcc3c2438acd5
--md5:ServerGame_PoliFire.lua:cfd7c19db0d8f3096f79c318d8102823
--md5:ServerGame_PvpCommon.lua:9d13ab849f2160514f12c528d6d590c9
--md5:ServerGame_QA.lua:4a72cbe5154be39afc938bceb5546455
--md5:ServerGame_QuestTableList.lua:cedad064ff4f06f4ed5f9f8772652da3
--md5:ServerGame_RefineChangeRate.lua:b914c33d35e64b0a55c1adb8058893ab
--md5:ServerGame_Shop.lua:10f91f1aa02e0d8df690cc6fea6000f5
--md5:ServerGame_Skill.lua:eb7072e92887c9bfc85de7b945b9d8c0
--md5:ServerGame_StatusProtect.lua:78ac181f121ea0bf432428ff22372449
--md5:ServerGame_System.lua:09b68a64d13e4dd3f84990496546518d
--md5:ServerGame_TimerTableList.lua:ab9805479a07d6081a458e08112a54bd
--md5:ServerGame_ToyDoll.lua:a831a9e7ac5d653ac9b8a0fbec1fb2d6
--md5:ServerGame_TradeBlack.lua:9032ff0d474798276f4b208c76ca1bca
--md5:ServerGame_Treasure.lua:9871c50f3dc3b63893fe8c55fda534f1
--md5:ServerGame_Var.lua:cd91ea4ab3e0759f016e6b294e55b785
--md5:ServerGame_Altman.lua:d26b8053d55496457fa6767b0399ac64
--md5:ServerGame_KFCActivity.lua:c818084f22361353375a9ac39e3e45c8
ServerGame = {} 
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--公会福利
ServerGame.Artifact= {
    retrieve_cd = 7200,                     --神器的回收时间2小时
	building = {
		[5] = {
    		produce_npc_id = 7820,            --打造成功说话npcid

    		produce_msg_id = 3790,           --打造成功说的msgid
		},
		[6] = {                                                                              
    		produce_npc_id = 7823,            --头饰神器的打造成功说话npcid
    		produce_msg_id = 3790,           --打造成功说的msgid
		},
	}
}	
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--公会挑战
ServerGame.GuildChallenge = {
	extra_reward = 3860							--完成某一个挑战任务后可以额外获得的奖励id
}
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--公会福利
ServerGame.GuildWelfare= {
	overdue_time = 1206900,						--奖励的超时时间：14天
}
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--积分额度
ServerGame.Quota = {
  ItemId = 5529,                --猫金打赏卡itemid
  DefautExpireDay = 31,         --默认过期时间，单位天
  LogCountPerPage = 20,         --积分获得记录每页显示条数
  MaxSaveLogCount = 200,        --积分获得最大保存记录条数
  DetailCountPerPage = 20,      --积分详情每页显示条数
  DetailExpireDay = 10,         --积分详情过期X天数后从界面清除天数
  DetailExpireDayDel = 60, 	--过期天数后删除记录天数
}
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

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.Achieve = {
  hand_time_limit = 1, -- 牵手最少计算时长(单位:秒)
  mvp_time_limit = 600,  -- mvp击杀时间限制（单位：秒）
  dead_time_limit = 5, --死亡有效时长（单位：秒）
  item_use_interval_limit = 60, --物品使用间隔（单位：秒）
  collection_exclude = {53537,53538},  --古城珍藏品不包含物品
}


--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--斗技场
ServerGame.Arena = { 
  be_killed_score = 4,
  kill_score = 2,
  help_kill_score = 1,
  combo_kill_score = 1,
  braver_kill_score = 1,
  savior_kill_score = 2,
  help_kill_time = 15,
  combo_kill_time = 20,
  braver_hp_percent = 10,
  savior_hp_percent = 20,
  origin_score = 30,
  metal_npcid=55000,
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--pm交易行
ServerGame.Auction = {
  StartTime = {                     			--拍卖开始时间
    {wday = 5, hour=20,min = 00},           	--每周五20点00分钟
  },    
  VerifySignupDuration = 1800,      			--拍卖报名审核时长。秒
  VerifyTakeDuration = 1800,      				--拍卖成交记录审核时长。秒
  MaxAuctionItemCount = 10,         			--最大拍卖物品个数
  MinAuctionItemCount = 10,         			--最低拍卖物品个数
  DurationPerOrder = 60,        				--每个拍品无人出价时竞拍时间
  NextOrderDuration = 60,           			--倒计时多少秒进入下一个拍品
  TradePriceDiscount = 60,						--拍卖上架时的商品价格，取当前加一所当前换算的逼格猫币的百分比折扣
  AuctionStartDialogTime = 600,					--在拍卖开始时多少时间内推送拍卖开始的playdialog
  PublicityDuration = 28800,					--在拍卖正式开启前8小时时进入拍卖公示期
}

-- BOSS
ServerGame.Boss = {
    refresh_base_times = 60,     	-- Boss刷新时间的随机范围的系数
	deadboss_open_ntf_dialog =108921,
	dead_set_rate =7000,
	refresh_time = {'00:00','2:00','4:00','6:00','8:00','10:00','12:00','14:00','16:00','18:00','20:00','22:00'}
}


ServerGame.Buff = {
  -- 需在pvp,gvg等9屏同步的buff, commonfunc targetUser:HasBuffID使用
  nine_buff = {
    100660,106151,115004,115080,116204,116810,116814,200020,200021,
    200070,31790,32290,41100050,80000240,95491,96050,117660,116810,116813,116204,116470,116490,30001360,30001370,30001380,
    117850,32980,118250,160000,210011,118172
  }
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

-- 幽灵相机
ServerGame.Camera = {
  summon = {
    {id = 10019, search=3, disappeartime = 300, dis = 2, odds = 50},
    {id = 10072, search=3, disappeartime = 300, dis = 2, odds = 50},
    {id = 110008, search=3, disappeartime = 300, dis = 2, odds = 1, day_maxcnt = 1},
    interval = 3,
  },
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--卡普拉活动
ServerGame.CapraActivity = {
 	MapId = 51,  --卡普拉活动的地图id
 	CarryNpc = 1008,
	addquests = {396200001,396210001},
	delquests = {396200001,396210001,396220001,396230001,396240001,396250001},
	delbuffs = {1870,1880,1890,1900,1910,1920},
}
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.Card = {
 decompose_rate={[60]=100,[35]=110,[5]=120},		--分解概率
 }
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--料理配置
ServerGame.ChatChannel = {
  echat_channel_round = 0,
  echat_channel_team = 1,
  echat_channel_guild = 1,
  echat_channel_friend = 0,
  echat_channel_world = 0,
  echat_channel_map = 0,
  echat_channel_sys = 0,
  echat_channel_room = 0,
  echat_channel_barrage = 0,
  echat_channel_chat = 0,
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--姜饼人
ServerGame.Courier = {
  male_dialog = 83018,     --收件人为男性的循环对话
  female_dialog = 83019,   --收件人为女性的循环对话
  dialog_interval = 20,    --对话间隔时间(s)
  Sign_dialog = 83020,     --签收时对话
  Refuse_diaglog = 83021,  --拒绝时对话
  Follow_time = 5*60,      --姜饼人跟随时间 
}

ServerGame.Credit = {
  default_value = 0,			-- os備註有錯別字或者看不懂的請聯繫重熙...這個是初始值
  max_value = 50000,			-- 信用值上限設為50000

  dec_limit_value = 10000,		-- 信用值超過10000
  dec_day_value = 1000,			-- 每天第一次上线扣1000

  monster_value = 1,			-- 擊殺一隻魔物獲得信用
  max_monster_value = 500,		-- 擊殺魔物的每日信用上限
  mini_value = 100,			-- 擊殺一只mini獲得的信用值
  mvp_value = 100,				-- 擊殺一只MVP獲得的信用值

  wantedquest_value = 10,		-- 完成一次看板任務增加的信用值
  wantedquest_times = 10,		-- 每日完獲得的獲得次數上限
  seal_value = 100,				-- 每次完成裂隙增加的信用值
  seal_times = 5,				-- 每日完成裂隙的獲得次數上限

  charge_ratio = 10,			-- 充值一元錢獲得的信用值增加
  buy_ratio = 1000,				-- 交易所購買價格除以此數值=信用值

  add_interval = 3600,			-- 間隔3600秒
  interval_value = 10,			-- 增加信用值10點

  first_bad_value = 10,			-- 初次發送扣除10
  second_bad_value = 10,		-- 比上次懲罰增加10
  bad_interval = 30,			-- 兩次發送敏感字間隔時間30

  repeat_dec_value = 10,		-- 第二次重複說話扣除10
  repeat_interval = 60,			-- 發送間隔時間

  value_limit_personal = 0,		-- 小於0時自言自語模式，服務端不公告
  value_forbid = -300,			-- 當到達-300時，開啟帳號禁言
  time_forbid = 3600,			-- 禁言時間

  check_channel = {1, 2, 3, 4, 5, 6, 7, 8, 8, 10},  --發給我不要忘
  -- 以下是敏感字庫，運營同學請自便
  bad_strs = {"钻","月","微","薇","Q","淘宝","taobao","淘","宝","氵匋",
  "壹","贰","叁","肆","陆","柒","捌","玖","群","君羊","萬","薇信","Αρρух５","V辛宫",
  "内部一百","内部一折","appyx","WWT896","sk888g","WK6136","cgsy65","SHOPP6","818s","xx9999",
  "100=2000万Zeny","专业代练","微宫纵","薇公众","宫纵浩","辛宫纵號","微信zzzpai001","专业DL","APPSYNB","WS4579",
  "冤枉钱","威信","v信","Z闭","加秋","加丘","加抠","加扣","v9866v","Va876s","WK6131",
  "裸聊","玩裸","果照","黄网","黄片","陪裸","V心","工纵","储值","装背","纵浩","亻言","牜勿诚心要","6648","9718",
  "收Z","出z","棺祝","ooo旺","澫Z","澫","syg","On9gamer2","yunyx3","game8765"," ts888k","74519","0006","妆呗",
  "新获","官住","寻求长期合作伙伴","WX886X","淘手游","收号","１２４５９","７１１７７","到咐","612953945","ESMC7263","挂单","代充","有偿带",
"收费带","版本最快","老板咨询","诚信经营","sy568d","V信公众","送极品装备","送卡片套装",
"ndsc85O6","公众V信","V信","SEAGM","先带后付","有偿带","带老板","收费带","老板咨询","www.SEA","wvvw.SEA","wwvv.SEA","GM.COM","M.COM","dai充yue卡",
"bf888s","hipstar","sy568T","13178277823","f888wy","JBDXBL","51312,sy568T","活动价","slbz","mycqz","送宝箱","送礼包","专业友情",
"13178277823","ShouFei","shoufei","SHOUFEI","LXSY868","28280","sy568d","腾Q","自带比例","骗子绕道","代刷","大量出","官方金币","出金",
"此号已挂","估价群","预估","驱魔带妖道","专业友情","带友情","手工不耗疲劳","500友情=30R","加w信","283864217","支持各种交易","支持各种交Y",
"sy568d","XXSY868","LXSY868","sy568d","f888wy","JBDXBL","sy568T","sy568T","bf888s","hipstar","x888sy","fimi22","f888wy","JBDXBL","51312","sy568T","活动价","slbz","mycqz","送宝箱",
"送礼包","bf888s","hipstar","sy568T","13178277823","x888sy","pingguobang520","500友情","带练","带刷","代刷","接友情","30042513","fimi22","先刷后付","xoxocp","专业友情","13178277823",
"ShouFei","shoufei","SHOUFEI","LXSY868","28280","sy568d","腾Q","sy568d","估价","力口","XXSY868","公众","ndsc85O6","自带比例","骗子绕道","代刷","大量出","官方金币","出金","此号已挂","估价群","预估"},
  switch_punish = 1,			--這個是處罰的開關，開關包含自言自語和禁言

  black_value = 1,
  chat_save_req = 1, 			--這個沒有用
  chat_save_char_count = 3, --保存最大人数
  chat_save_max_count = 5000,		--保存的记录最大上限 代码里限制了最大5000？
  chat_calc_max_count = 50, --最大保存列表

  default_lv_value = {
    {min = 1, max = 10, value = 0},		--一次性更新系統時使用 等級預設信用值
    {min = 11, max = 40, value = 1000},
    {min = 41, max = 70, value = 10000},
    {min = 71, max = 100, value = 30000},
  }
}

--充值
ServerGame.Deposit={
    DiscountVersionCard={{fromid=251,toid=250},{fromid=241,toid=241},{fromid=231,toid=231},{fromid=221,toid=221}},   --版本卡打折替换，fromid替换为toid
}
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.EffectPath = {
  leavescene = { effect = "Skill/Teleport", sound = "Common/Teleport" },
  enterscene = { effect = "Common/15EnemyBirth", sound = "Common/EnterScene"},
  TeleportSkill={effect = "Skill/Teleport"},
  BuffImmune={effect="Common/immunity",effectpos=1},   ----异常状态免疫特效
  BuffResist={effect="Skill/Parry",effectpos=2},
}

ServerGame.ExtraReward = {
	{id= 1, etype= "wanted",normal = { count = 2,item = 5260,itemnum = 1}},
	{id= 2, etype= "guild_quest",normal = { count = 2,item = 5260,itemnum = 3}},
	{id= 3, etype= "Seal",normal = { count = 5,item = 5260,itemnum = 2},lv_spec = {lv = 80,count =5,item = 5260,itemnum = 1}},
	{id= 4, etype= "mochao",normal = { count = 1,item = 5260,itemnum = 1}},
	{id= 5, etype= "yanjiusuo",normal = { count = 1,item = 5260,itemnum = 1},lv_spec = {lv = 80,count =1,item = 5260,itemnum = 1}},
	{id= 6, etype= "endless",normal = { count = 10,item = 5260,itemnum = 1}},
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--料理配置
ServerGame.Food = {
  SkillId = 50043001,        --美食家技能id
  CookerKnife = 550000,    --美食家餐刀
  CookerHat = 550001,       --厨师帽子头饰
  DarkFood = 551019,    --黑暗料理
 -- EatSkillId = 80014001,  --吃料理的技能
  CookMeterialExpPerLv = 100; --食材烹饪熟练度每级需要的经验
  MaxCookMeterialLv = 10,     --最大食材熟练度等级
  -- CookFoodExpPerLv = 10,      --料理烹饪熟练度每级需要的经验
  TaserFoodExpPerLv = 10,     --料理美食熟练度每级需要的经验
  SatietyRate = 480,          --饱腹消耗速率  秒/每点
  FoodNpcDuration = 5 * 60,   --料理npc在场景存在的时长 秒
  MaxPutFoodCount = 5,           --最大同时摆放料理个数
  MaxPutMatCount = 3,        --最大同时摆放野餐垫个数
  ChristmasCake = 7028,       --圣诞蛋糕
}
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--友情之证奖励
ServerGame.FriendShip = {
  Reward = {
    Friend = {
      MaxLimitCount = 500,  --每周上限
      Itemid = 147,         --itemid
      SealCount = 5,        --封印奖励个数
      DojoCount = 5,        --道场奖励个数   
      TowerCount_Mini = 2,  --无限塔mini奖励个数 
      TowerCount_Mvp = 5,   --无限塔MVP奖励个数    
      LaboratoryCount = 5,  --研究所奖励个数    
    },
    Guild = {
      MaxLimitCount = 500,  --每周上限
      Itemid = 140,         --itemid
      SealCount = 5,        --封印奖励个数
      DojoCount = 5,        --道场奖励个数   
      TowerCount_Mini = 2,  --无限塔mini奖励个数    
      TowerCount_Mvp = 5,   --无限塔mvp奖励个数      
      LaboratoryCount = 5,  --研究所奖励个数     
    }
  }
}

--功能开关
ServerGame.FunctionSwitch = {
  FreeFreyja = 0,    -------免费传送
  FreePackage = 0,   -------免费打开仓库
  FreeFreyja_Team = 0, -------免费组队传送  
  }

-- 全服活动配置
-- id = 活动ID
-- name用于备注，可以是中文
-- rewardID可以填rewardID或者邮件ID
-- time表示reward的倍数
-- 新添加活动需告知后端
ServerGame.GlobalActivity = {
  {id = 14, name = "Augury", rewardID = 10006, times = 1},
  {id = 15, name = "GuildQuest", rewardID = 0, times = 2},
  {id = 16, name = "GuildDonate", rewardID = 0, times = 2},
  {id = 17, name = "GuildFuben", rewardID = 0, times = 2},
  {id = 19, name = "NoviceWelfare", rewardID = 10006, times = 1},
  {id = 20, name = "RefineOff", rewardID = 0, times = 50},  --普通精炼zeny5折
  {id = 21, name = "safeRefineOff", rewardID = 0, times = 80},  --安全精炼材料+zeny8折
  {id = 22, name = "safeRefineOff", rewardID = 0, times = 0},  --安全精炼装备消耗8折
}
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

-- 公会
ServerGame.Guild = {
  default_npc_count = 5,
  npc_clear_time = 172800,--npc刷新时间48小时
  quest_protect_time = 86400,--任务保护时间，24小时
  maintenance_protect_lv = 4,--公会降级等级保护
  newicon_ntf_lv = {1, 9},--新icon提示公会等级
  -- photo_max_show = 32,--公会墙：最大照片显示数量
  photo_max_permember = 10,--公会墙：成员最大展示数量
  -- photo_max_horizontal = 25,--公会墙：最大横向照片数量
  -- photo_max_vertical = 7,--公会墙：最大竖向照片数量
  photo_refresh_time = 180,--公会墙：照片刷新间隔
  photo_frame = {101,102,103,104,105,106,107,110,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,207},
  photo_member_active_day = 7, --公会墙：成员要求活跃天数 
  photo_max_frame_photo = 30,--公会墙：最大相框照片数量 
  city_giveup_cd = 7200,
  auth_name = {[1] = "邀请入会", [2] = "批准入会", [3] = "申请列表删除", [4] = "编辑公告", [5] = "编辑招募",
            [6] = "变更图标", [7] = "公会升级", [8] = "公会建筑升级", [9] = "公会科技升级", [10] = "发布任务",
            [11] = "踢出会员", [12] = "踢出副会长", [13] = "变更职位", [14] = "编辑职位名字", [15] = "会长交接",
            [16] = "解散公会", [17] = "公会搬家", [18] = "开启异界", [19] = "照片墙"}
}


-- 公会副本
ServerGame.GuildRaid = {
  boss_num = 5,
  map_depth = {min=7, max=9},
  map_num = {min = 12, max = 23},
  unsteady_time = 3600,
  reward_ratio = 0.6,
}

--道具获取上限对应提示
ServerGame.GetLimitMsg = {
     [1] = 64,
     [7] = 63,
 }

--物品
ServerGame.Item = {
  kapula_map_item = 5040,       -- 卡普拉传送代扣券
  kapula_store_item = 5041,     -- 卡普拉仓库代扣券
  item_pickup_mode = 1,         -- 拾取模式
  equip_upgrade_refinelv_dec = 2,  -- 升级后精炼回退2个等级 
  extra_hint_item = {5501},     -- 额外提示物品
  temp_pack_overtime_itemcount = 10,  --临时背包进入倒计时个数
  no_color_machine_mount = {},   --------变机甲时，穿戴这些机甲无视职业服装变色
  machine_mount = {25400,25401},       --------所有机甲
}

ServerGame.ItemCheckDel = {
  {id = 25102, type = "Level", param = 15},{id = 25107, type = "Level", param = 15}
}

--分解时的金属价值计算
ServerGame.Decompose = {
  price = {
    [52821] = 50000,
  }
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--二人世界
ServerGame.ItemImage = {
  range = 6,
  npcid = 1524,    --爱心特效
  lovenpcid = 0,  --装饰NPC
}
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.LockEffect = {
  ------判定循环时间
  interval = 5,
  ------判定循环范围
  range = 2.5,
  ------闪避上下限数量
  fleeminnum = 2,
  fleemaxnum = 7,
  ------最多闪避降低百分比
  fleemaxper = 0.5,
  ------防御上下限数量
  defminnum = 2,
  defmaxnum = 7,
  ------最多防御降低百分比
  defmaxper = 0.05,



  ------自动反击时间
  autohittime = 5,

  chainAtkLmt_time = 5,
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.Lottery = {
  DayBuyLotteryGiveCnt = 50,    --扭蛋每天购买的赠送盒数量
  Price = 30,     --扭蛋一次的价格
  Discount = 30,  --当月的折扣，30表示折扣30%

  --ItemType对应当前批次
  Batch = {

[1] = 7,
[2] = 7,
[3] = 2,
[4] = 7,
[5] = 2,
[6] = 2,
[7] = 7,
[8] = 2,
[9] = 2,
[10] = 2,
[11] = 2,
[12] = 2,
[13] = 2,
[14] = 2,
[15] = 2,
[16] = 2,
[17] = 2,

  },
  
  --B格猫金币和B格猫福利券权重
  --往期类别 = 100 + 当期类别
  CoinWeight = { -- B格猫金币大类权重
    [103] = 10, --绝版头饰
    [104] = 10, --绝版武器
    [105] = 20, --绝版特典
    [1] = 200, --最新皮肤
    [101] = 20, --过往皮肤
    [2] = 1000, --最新头饰
    [102] = 100, --过往头饰
    [110] = 500, --发型
    [118] = 500, --时装染色
    [109] = 1000, --美瞳
    [111] = 800, --料理
    [106] = 600, --血迹树枝
    [7] = 1000, --最新时装碎片
    [117] = 900, --Zeny+残页
    [112] = 650, --货币
    [113] = 640, --便捷
    [114] = 350, --宠物
    [115] = 500, --经验
    [116] = 600, --祈祷
    [108] = 500, --卡片
    [107] = 0, --过往时装碎片
    [4] = 100, --最新武器



  },  
  TicketWeight = {  -- B格猫福利券大类权重  
    [103] = 10, --绝版头饰
    [104] = 10, --绝版武器
    [105] = 20, --绝版特典
    [1] = 100, --最新皮肤
    [101] = 10, --过往皮肤
    [2] = 500, --最新头饰
    [102] = 100, --过往头饰
    [110] = 740, --发型
    [118] = 740, --时装染色
    [109] = 1240, --美瞳
    [111] = 1040, --料理
    [106] = 300, --血迹树枝
    [7] = 500, --最新时装碎片
    [117] = 450, --Zeny+残页
    [112] = 890, --货币
    [113] = 880, --便捷
    [114] = 590, --宠物
    [115] = 740, --经验
    [116] = 840, --祈祷
    [108] = 250, --卡片
    [107] = 0, --过往时装碎片
    [4] = 50, --最新武器



  },  
  DayMaxCount = {     --每日金币扭蛋的最大次数
    [1] = 50, --头饰
    [2] = 50, --装备
    [3] = 50, --卡片
    [5] = 50, --活动
  },
  BasePriceRateMin = 250,-----保底抽取最小倍率2.5
  BasePriceRateMax = 350,-----保底抽取最大倍率3.5
  SaveRateMax = 50000,-----概率值小于等于该值是保存

  Linkage = {
   {type=1,year=2018,month=6,end_time="2018-11-30 5:00:00"},
  }--联动扭蛋关闭时间
}

 
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.Manual = {
  head_item_return_mail = 1010,
  card_item_return_mail = 1011,
  head_unlock_return_mail = 1012,
  card_unlock_return_mail = 1013,
  level_return_mail = 1008,
  quality_return_mail = 1014,
  skill_return_mail = 1009,
  unsolvedphoto_overtime = 1728000, -- 临时相册过期时间(单位:秒)

  level_return = {[10] = 2, [15] = 5, [20] = 10, [25] = 15, [30] = 20, [35] = 30},
  quality_return = {
    [1] = {[3] = {6021,1}, [6] = {6021,2}, [10] = {6021,3}, [20] = {6021,6}, [30] = {6021,10}, [50] = {6021,15}},
    [2] = {[3] = {6022,1}, [6] = {6022,2}, [10] = {6022,3}, [20] = {6022,4}, [30] = {6022,6}, [50] = {6022,8}},
    [3] = {[2] = {6022,1}, [5] = {6022,2}, [10] = {6022,3}, [20] = {6023,2}, [30] = {6023,3}, [40] = {6023,4}, [50] = {6024,2}},
    [4] = {[2] = {6025,1}, [5] = {6025,2}, [10] = {6024,2}, [20] = {6024,3}, [30] = {6024,4}},
  },
  quality_name = {
  [1] = "白色",
  [2] = "绿色",
  [3] = "蓝色",
  [4] = "紫色",
  }
}


-- 雇佣猫
ServerGame.MercenaryCat = {
  help_Teamer_dis = 12,
  leave_owner_time = 2,
  leave_teamer_time = 2,
}

ServerGame.MoneyCat = {
  max_money = 6666666,
  rand_money = {
    {step=1, min_money=4888, max_money=8021},
    {step=2, min_money=7888, max_money=8112},
  }
}

--怪物刷新
ServerGame.ServerGame_MonsterRelive = {
	ReliveTime2Map =
	{
		[2] = {
				[2]={
				10001, 10002, 10003, 10014,
				},
				[3]={
				10004, 10005, 10006, 10007,
				},
				[4]={
				10005, 10006, 10007, 10008, 10009,
				},
				[5]={
				10010, 10011, 10012, 10013,
				},
				[8]={
				10017, 10018, 10020, 10021,
				},
				[11]={
				10017, 10023, 10024, 10025,
				},
				[12]={
				10017, 10025, 10026, 10027, 10028, 10111,
				},
				[15]={
				10040, 10041, 10042, 10043, 10044, 10045, 10051,
				},
				[16]={
				10046, 10047, 10048, 10049, 10101, 10139,
				},
				[19]={
				10054, 10055, 10056,
				},
				[21]={
				10062, 10063, 10064,
				},
				[22]={
				10058, 10061, 10065, 10066, 10067, 17106,
				},
				[26]={
				10077, 10078, 10079,
				},
				[27]={
				10082, 10083, 10084,
				},
				[28]={
				10085, 10086, 10087, 10088,
				},
				[32]={
				10055, 10056, 10057, 10058,
				},
				[34]={
				10102, 10103, 10104, 10105, 10106,
				},
				[35]={
				10040, 10041, 10042, 10046, 10047, 10048,
				},
				[37]={
				10080, 10081, 10120, 10121,
				},
				[38]={
				10040, 10041, 10042, 10043, 10044, 10045,
				},
				[46]={
				10132, 10133, 10134, 10135,
				},
				[49]={
				10140, 10141, 10142, 10143, 10145,
				},
				[50]={
				10141, 10144, 10145, 10146, 19011,
				},
				[58]={
				10105, 10104, 10103, 10106, 10102,
				},
				[53]={
				10063, 10064, 10062, 
				},
			},

		[3] = {
				[6]={
				10001, 10002, 10003, 10010, 10011, 10012, 10013, 10014, 10015, 10016, 10038, 17003,
				},
				[20]={
				10017, 10059, 10060, 10061, 17106,
				},
				[23]={
				10068, 10069, 10070,
				},
				[33]={
				10029, 10030, 10031, 10032, 10033, 10034, 10035, 17200, 17201, 17202, 17203, 17204,
				},
			}
	}
}
--月卡活动
ServerGame.MontchCardActivity = {
  ActivityID = 18,
  MontchCardReward = {
  [1]={
      CostCard=1,   --激活月卡数
      RewardItemId = 710044, --奖励物品
      Count= 1,  --奖励物品个数--
  },
  [2]={
      CostCard=3,   --激活月卡数
      RewardItemId = 710044, --奖励物品
      Count= 2,  --奖励物品个数--
  },
  [3]={
      CostCard=5,   --激活月卡数
      RewardItemId = 710044, --奖励物品
      Count= 3,  --奖励物品个数--
  },
  }
}

ServerGame.MvpScore = {
  noticeTime = 3,
  validTime = 300,
  damageScore = {9, 6, 4, 4, 3, 3, 3},
  beLockScore = {5, 3, 1, 1},
  healScore = {3, 2, 2, 1, 1, 1, 1},
  rebirthScore = {1, 1},
  deadHitScore = {10},
  deadHitTime = 1000,
  firstHitScore = 3,
  damageDecScore = {{per = 36, dec = 1}, {per = 20, dec = 2},{per = 10, dec = 8}}, -- 伤害低于总伤害百分比 扣分
  
  showNames = {
    top1damage = {name="最高伤害", show_order=1},
    damage = {name="伤害输出", show_order=2},
    belock = {name="吸引火力",show_order=3},
    heal = {name="有效治疗",show_order=4},
    rebirth = {name="复活玩家",show_order=5},
    deadhit ={name="致命一击",show_order=6},
    firsthit = {name="最先参战",show_order=7},
    breakskill = {name="打断技能",show_order=8},
  }
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--预言之地充值返回截止时间
ServerGame.OperateReward = {
  expiretime = "2017-04-01 5:00:00"
}

-- 宠物
ServerGame.Pet = {
  archer_partner = {1065,5049,5090,6657};
  merchant_partner = {4425,1705,1706};
  merchant_barrow={[25300]=4425,[25301]=1705,[25302]=1706,},
  equip_to_useitem = {{45300,410001},{45301,410009},{45302,410016},{45303,410019},{45304,410023},{45305,410024},{45306,410029},{45307,410023},{45308,410033},{45309,410040},{45310,410041}},

  userpet_tick = 60,
  userpet_happly_gift = 1,					-- 开心状态增加礼物值
  userpet_happly_friend = 1,				-- 开心状态增加好感度
  userpet_excite_gift = 1,					-- 兴奋状态增加礼物值
  userpet_excite_friend = 1,				-- 兴奋状态增加好感度
  userpet_happiness_gift = 1,				-- 幸福满满增加礼物值
  userpet_happiness_friend = 1,				-- 幸福满满增加好感度
  userpet_touch_time = 96 * 60,				-- 抚摸一次持续时间(单位:秒)
  userpet_touch_friend = 10,				-- 抚摸一次增加好感度
  userpet_touch_perday = 15,				-- 一天可抚摸次数
  userpet_touch_tick = 5 * 60,				-- 抚摸间隔CD
  userpet_feed_time = 180 * 60,				-- 喂食一次持续时间(单位:秒)
  userpet_feed_friend = 20,					-- 喂食一次增加好感度
  userpet_feed_perday = 8,					-- 一天可喂食次数
  userpet_feed_tick = 15 * 60,				-- 喂食间隔CD
  userpet_gift_time = 144 * 60,			-- 赠送一次持续时间(单位:秒)
  userpet_gift_perday = 10,					-- 一天可赠送次数
  userpet_gift_friend = 1000,				-- 赠送一次增加好感度
  userpet_gift_maxvalue = 360,				-- 獎勵值上限

  transformEggBuff = 10003000, 				-- 捕捉时, 变身为蛋的变身buff
  userpet_touch_effectid = 2,
  userpet_feed_effectid = 2,

  maxCatchTime = 300, 						-- 捕获总时长
  maxCatchPetDistance = 15, 				-- 超出该距离, 捕获失败
  offlineKeepCatchTime = 30, 				-- 下线保护时间(掉线)
  noOperationNoticeTime = 20, 				-- 捕获时,超过该时间没有操作提示
  catchSkill = 90003001, 					-- 捕获技能
  catchSkillPlayTime =1,
  eatFoodSkill = 109000001,					-- 宠物吃料理技能
  birthRange = 1, -- 宠物出生距离玩家范围
  catchLineID = 10, -- 捕捉宠物连线id
  overUserLevel = 0, -- 允许宠物超过玩家的等级
  reliveTime = 180, -- 宠物复活时间
  hatchFadeIn = 800, -- 宠物孵化淡入时间（毫秒)
  hatchFadeOut = 800, -- 宠物回收淡出时间 (毫秒)
  adventure_default_area={1}, --宠物冒险进行时默认解锁的区域
  userpet_gift_monthcard= 2, --月卡用户丸子翻倍
  
  ---重置技能
  reset_skill={
  [12369]={[1]=100,[2]=100,[3]=100,[4]=80,[5]=70,[6]=70,[7]=60,[8]=60,[9]=50,[10]=50},
  [12375]={[5]=100,[6]=100,[7]=100,[8]=90,[9]=80,[10]=70},
  [12377]={[7]=100,[8]=90,[9]=80,[10]=70},
  }
}

--个人相册
ServerGame.Photo = {
   DefaultSize = 20, -- 默认大小
   SkillIncreaseSize = {
      [50041001] = 15, -- [摄影大师技能id] = 增加大小
      [50041002] = 15,
   },
}

-- 商店相关配置
ServerGame.PoliFire = {
  drop_range = 1.5,  -- 掉落范围
  item_disp_time = 300,  -- 道具消失时间
  default_skill = 91001001,  -- 默认技能
--  item_skill = {
--    {710125,91002001}, -- 加速
--	{710126,91003001}, -- 护盾
--	{710127,91004001}, -- 突进
--	{710128,91005001}, -- 闪现
--	{710129,91006001}, -- 击退
--	{710130,91007001}, -- 香蕉皮
--	{710131,91009001}, -- 隐身
--	{710132,91010001}, -- 恶作剧
--	{710133,91012001}, -- 贪吃鬼
--	{710134,91013001}, -- 变大	
--  },  -- 道具id与技能id关联关系
  max_skill_pos = 6,  -- 最大技能数量
  gold_apple_id = 157,  -- 金苹果道具id
  ghost_poli_buff = 200090,  -- 变身幽灵波利buffid
  recover_num = 2,  -- 幽灵波利复活所需金苹果
  default_score = 4,        --开局拥有的金苹果数量
--   trans_buffid = {200100,200101,200102,200103,200104,200105,200106,200107,200108,200109},    --变身十个波利buffid
  god_buffid = 200052,      --无敌buffid
  god_duration = 11, --无敌持续时间
  apple_limit_count = 600, --积分上限
  score_itemid = 158,  --积分对应的道具id
  level_reward = {{59,40066},{79,40067},{99,40068},{110,40084}},  --等级对应的奖励id
  show_buff = 200200,  --金苹果数量显示BUFF
  mask_buff = {1170,100500},  --波利乱斗中屏蔽的buffid
  pre_close_msgtime = 20  --倒计时
}
ServerGame.PvpCommon = {
 KillCoin = 10,
 HelpKillCoin = 3,
 DesertWinCoin = 100,
 GlamWinCoin = 200,
 DayMaxCoin = 300,
 WeekMaxCoin = 1000,
 HpRate = 4,
 DesertWinScore = 20,
 ExtraReward = {coinnum=100, itemid=5260, num=1},
 HealRate = 4,
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--随机NPC问答奖励配置，reward_count表示每次发放奖励次数--
ServerGame.QA = {
  reward_count = 20,
  reward_map = {
    {level = 60, rewardid=3038},               --[1,60] 级的 rewardid
    {level = 120, rewardid=3039}              --[61,120] 级的rewardid
  }
}
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--任务相关表格--
ServerGame.QuestTableList = {
  {key = "Table_Quest", value = Table_Quest},
  {key = "Table_Quest_2", value = Table_Quest_2},
  {key = "Table_Quest_3", value = Table_Quest_3},
  {key = "Table_Quest_4", value = Table_Quest_4},
  {key = "Table_Quest_5", value = Table_Quest_5},
  {key = "Table_Quest_6", value = Table_Quest_6},
  {key = "Table_Quest_7", value = Table_Quest_7},
  {key = "Table_Quest_8", value = Table_Quest_8},
  {key = "Table_Quest_9", value = Table_Quest_9},
  {key = "Table_Quest_10", value = Table_Quest_10},
  {key = "Table_Quest_11", value = Table_Quest_11},
  {key = "Table_Quest_61", value = Table_Quest_61},
  
}
ServerGame.Quest = {
	dailyrand_per_day = {[48]={3,6},[49]={3,6},[50]={3,6},[63]={3,7},[64]={3,7},[65]={3,7}},
}

ServerGame.RefineChangeRate = {
  begin_change_lv = 5,
  last_success_dec = 3, --百分比
  last_fail_add = 2.7,
  repair_per_add = 0.001,
}

-- 商店相关配置
ServerGame.Shop = {
  randbylv = {
   [3003] = { -- 按玩家等级随机
      [1] = {-- 商店ID
         count = 7,-- 数量
         duplicate = 1,-- 是否重复(是填1,否填0)
      }
	},
[3004] = { -- 按账号下最大角色等级随机
     [1] = {
        count = 3,
       duplicate = 0 --(是填1,否填0),
     },
  }
--   },
--  weekrand = {
--  [1] = {weight=10,shopid={250060}},
--  [2] = {weight=10,shopid={250070}},
--	[3] = {weight=10,shopid={250080}},
--	[4] = {weight=10,shopid={250090}},
--	[5] = {weight=10,shopid={250100}},
--	[6] = {weight=10,shopid={250110}},
--	[7] = {weight=10,shopid={250120}},
--	[8] = {weight=10,shopid={250130}},
--	[9] = {weight=10,shopid={250140}},
--	[10] = {weight=10,shopid={250150}},
--	[11] = {weight=10,shopid={250160}}
	
  }	
}

ServerGame.ExchangeShop = {
   --追赶商店 延迟出现的物品 最少延迟时间 单位分钟
   MinDelayStartTime = 30,
   --最大延迟时间 单位分钟
   MaxDelayStartTime = 60,
   --世界Base等级经验Buff
   BaseWorldLevelExpBuffID = 158,
   --世界Job等级经验Buff
   JobWorldLevelExpBuffID = 159,
   --不开放追赶机制的职业ID
   noopen_pros = {82,83,84,102,103,104,112,113,114},
}
ServerGame.Skill = {
  manual_action_skill = {50031001, 60},

  skillarg = {
  Solo = {buff = {119300},},--独奏技能类型
  Ensemble ={buff = {119301,119302,119303},}, ---合奏技能类型
  },
  ---------------------需要同步给前端的合奏技能
  ensemble_skill = {
  [42]={1419,1420,1421,1422,1429,1430,1431,1432,1441,1442,1443,1444},-------------诗人
  [43]={1369,1370,1371,1372,1379,1380,1381,1382,1392,1393,1394,1395},-------------舞娘
  },

}

--异常状态的免疫周期和累计最大时长
ServerGame.StatusProtect = {
{status = 1, period = 60, maxtime=60},
{status = 2, period = 60, maxtime=60},
{status = 3, period = 60, maxtime=60},
{status = 4, period = 10, maxtime=5},
{status = 5, period = 10, maxtime=6},
{status = 6, period = 10, maxtime=6},
{status = 7, period = 10, maxtime=6},
{status = 8, period = 10, maxtime=5},
{status = 9, period = 10, maxtime=6},
{status = 10, period = 10, maxtime=6},
{status = 11, period = 10, maxtime=6},
{status = 12, period = 10, maxtime=6},
}

ServerGame.System = {
  valid_pos_check = 1,
  valid_pos_radius = 5,
  max_extra_add_battletime = 36000,  --可累积的最大战斗时长
  max_mercenary_cat_num = 4,  --可以携带的猫数量上限
  debt_mail_id = 10003,     --进入负债发送邮件Id
  sell_limit_warning = 1000000,
  npc_fade_out_time = 2,
  kill_monster_period = 60,  ---战斗效率改动
  shop_random_cnt = 50,    --稀有商店每日随机多少次
  open_tower_layer = 70,    --时间胶囊爬塔的开放层数
  gift_msg_1 = "[礼包码] 查找玩家accid 失败,数据库找不到",
  gift_msg_2 = "[礼包码] 查找玩家accid 失败。",
  gift_msg_3 = "[礼包码] 数据库异常",
  gift_msg_4 = "[礼包码] 对应码不存在",
  gift_msg_5 = "[礼包码] 已经过期",
  gift_msg_6 = "[礼包码] 超出服使用范围",
  gift_msg_7 = "[礼包码] 已经被使用",
  gift_msg_8 = "[礼包码] 礼包码已达到最大使用次数,已使用次数：",
  gift_msg_9 = "批次最大使用次数：",
  gift_msg_10 = "[礼包码] 您已经领取过该批次的兑换码",
  gift_msg_11 = "[礼包码] 错误的物品配置",
  gift_msg_12 = "[礼包码] 恭喜您，礼包码兑换成功,请注意查收邮件",
  gift_msg_13 = "[礼包码] 礼包码兑换失败",
  max_seat_dis = 20,  --优雅坐下最大距离
  monster_kill_statnum = 40,  --玩家一分钟杀怪，达到此数量是，记录玩家杀怪数量
  
  map_group_num =   --同屏显示的地图编号与最大人数
  {
  [2]=60,         
  },
}

-- TIMER
 ServerGame.TimerTableList = {
   {key = "Table_Timer", value = Table_Timer},
   {key = "Table_Timer_2", value = Table_Timer_2},
   {key = "Table_Timer_3", value = Table_Timer_3},
 }

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.ToyDoll = {
  Body = {{1, 8}, {11, 16}, {19, 24}, {27, 32}, {35, 40}},
  Head = {{45001,45016}, {45018,45031}, {45033,45034}, {45036,45038}, {45040,45058}},
  Hair = {{1, 20}, {998,999}},
  HairColor = {{1, 8}},
  Eye = {{1, 2}},
  time = 150,

  birth_emoji = {odds=100, value={2,27}},  -----出生时表情
  birth_dialog = {odds=100, value={1312203}}, -----出生时对话
  disp_emoji = {odds=100, value={1,18,24}},   -----消亡时表情
  disp_dialog = {odds=100, value={1312204}},  -----消亡时对话
  normal_emoji = {odds=80, value={4,25,26}},  -----平时表情
  normal_dialog = {odds=60, value={1312207,1312208,1312209}}, -----平时对话
  attack_emoji = {odds=80, value={20,23,30}},  -----主人攻击时表情
  attack_dialog = {odds=60, value={1312205,1312206}}, -----主人攻击时对话
  emoji_interval = 6,    ------------------------表情间隔时间
  dialog_interval = 10,   ------------------------对话间隔时间
}

ServerGame.TradeBlack = {
  [0] = {1806,87654},  
  [1] = {1806,464636,461450,464674,464669,464639,464670,464640,464630,464667,463838,464679,10280,464641,464765,464767,22402,10077,464799,157584,10078,464836,464869,464868,464873,464872,14909,461701,446345,10149,446345,10149,461450},       --0:内网，1：预言之地，2：正式服
  [2] = {1806,55007},
} 
--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.Treasure = {
  gold_tree_max = 4,
  gold_tree_refreshtime = 1200,
  magic_tree_max = 3,
  magic_tree_refreshtime = 3600,
  high_tree_max = 2,
  high_tree_refreshtime = 5400,

  shake_action_npc = 1001,
  shake_action_monster = 2001,
  dis_time = 600,
  known_buff = 6013,
}

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.Var = {
  offset = 0,    		--所有的向未来偏移x秒
  var2offset = {            
	[3] = 0,           --与角色相关向未来偏移x秒
	},     
  accvar2offset = {
	[93] = 0,           --与账号相关向未来偏移x秒
	},  
  }

--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--奥特曼副本
ServerGame.Altman = {
  trans_buff = {4034},
  default_skill = 910001,
  max_skill_pos= 6,
  raid_id = 601001,
  rewardid = 1951,
  clear_npc_time = 5,

  rewardbox = {id=1209, scale={2.5, 2.5}, dir=180, behavior=1024},  --掉落宝箱定义
  head = {45617, 51144, 45600, 45601, 45602, 47084, 47085, 47086, 51132, 51152, 48030, 47087, 47088, 45603, 45604, 45605,51154,145603,145604,145605},
  extra_head_reward = {{num=1,rewardid=1947},{num=3,rewardid=1948},{num=5,rewardid=1949},{num=7,rewardid=1950},{num=10,rewardid=1951}},
  kill_reward = {{num=30,rewardid=12110},{num=60,rewardid=12111},{num=90,rewardid=12112},{num=120,rewardid=12113}}, --填写邮件ID
  fashion_equip = {51125,51148},
  fashion_buff =6046,

  item_skill = {      --道具类型为73，道具对应的技能ID
    [710146] = 911001,
    [710147] = 912001,
    [710148] = 913001,
    [710149] = 914001,
    [710150] = 915001,
    [710151] = 916001,
  },

  -- 奥特曼副本杀怪等级描述
  kill_rank_desc = {
    [1] = { title = "星云指挥官", killcount = 120 },
    [2] = { title = "异星监测员", killcount = 90 },
    [3] = { title = "恒星观察员", killcount = 60 },
    [4] = { title = "行星巡逻员", killcount = 30 },
  },
}


--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.KFCActivity = {
    start_time = "2018-03-30 05:00:00",
    end_time = "2018-04-13 05:00:00",
    reward = 1879,
}
