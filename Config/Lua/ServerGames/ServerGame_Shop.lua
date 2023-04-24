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