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