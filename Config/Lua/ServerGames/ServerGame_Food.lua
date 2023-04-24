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