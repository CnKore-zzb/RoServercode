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
