-- ---------------------------------补丁---------------------------------

-- 功能 : 处理附魔任务更新导致重置材料补偿
-- 制定 : 李志鹏
-- 日期 : 2017-01-04
function Patch_2017_01_04(user)
  local version = 201701042153

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local sender = "系统"
  local title = "来自猫猫岛的来信"
  local msg = "由于猫猫岛总部发来新的中级和高级附魔考验，之前未完成的冒险者将会接取到新的附魔任务，旧的附魔任务中所消耗的材料会随邮件发还给冒险者，已经完成考验的冒险者不会受到影响哦~。给冒险者大人们带来的不便，敬请谅解~~喵~~"

  -- 进阶-中级附魔
  local step = user:getEnchantQuestMStep()
  if step >= 15 then
    sendOptMail(user:getTempID(), sender, title, msg, 52104, 10)
  end
  if step >= 21 then
    sendOptMail(user:getTempID(), sender, title, msg, 52203, 10)
  end
  if step >= 27 then
    sendOptMail(user:getTempID(), sender, title, msg, 52204, 5)
  end
  if step >= 31 then
    sendOptMail(user:getTempID(), sender, title, msg, 135, 5)
  end
  print("step="..step)

  -- 进阶-高级附魔
  step = user:getEnchantQuestSStep()
  if step >= 6 then
    sendOptMail(user:getTempID(), sender, title, msg, 52305, 3)
  end
  if step >= 12 then
    sendOptMail(user:getTempID(), sender, title, msg, 52307, 3)
  end
  if step >= 18 then
    sendOptMail(user:getTempID(), sender, title, msg, 52306, 3)
  end
  if step >= 27 then
    sendOptMail(user:getTempID(), sender, title, msg, 135, 10)
  end

  print("step="..step)

  user:addPatchLoad(version)
end

-- 功能 : 处理任务完成状态
-- 制定 : 李志鹏
-- 日期 : 2017-01-05
function Patch_2017_01_05(user)
  local version = 201701051920

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  if quest == nil then
    return
  end

  -- 检查任务311200002,完成大任务31120
  if quest:isSubmit(311200002) == true then
    quest:finishBigQuest(31120)
  end
  -- 检查任务311210002,完成大任务31121
  if quest:isSubmit(311210002) == true then
    quest:finishBigQuest(31121)
  end
  -- 检查任务311220002,完成大任务31122
  if quest:isSubmit(311220002) == true then
    quest:finishBigQuest(31122)
  end

  user:addPatchLoad(version)
end

-- 功能 : 刷新'队友保护'冒险技能,修正已创角的玩家刷新出技能
-- 指定 : 张申林 & 虞佳
-- 日期 : 2017-01-11
function Patch_2017_01_11(user)
  local version = 2017011117
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local fighter = user:getCurFighter()
  if fighter == nil then
    return
  end

  local skill = fighter:getSkill()
  if skill == nil then
    return
  end

  skill:refreshEnableSkill()
  user:addPatchLoad(version)
end

-- 功能 : 增加月卡激活地图传送,处理已完成传送任务,未能激活新配置地图
-- 指定 : momo
-- 日期 : 2017-01-13
function Patch_2017_01_13(user)
  local version = 2017011316
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  local freyja = user:getFreyja()
  if quest == nil or freyja == nil then
    return
  end

  local questmap = {
                    [340040001] = {1, 2, 3, 4},
                    [340050001] = {5},
                    [340060001] = {6},
                    [340080001] = {7, 8, 31},
                    [340090001] = {9},
                    [340100001] = {10, 11, 12},
                    [340330001] = {33},
                    [340140001] = {14},
                    [340160001] = {15, 16},
                    [340350001] = {35},
                    [340170001] = {21, 17},
                    [340190001] = {19},
                    [340320001] = {32},
                    [340240001] = {24},
                    [340200001] = {20, 22},
                    [340230001] = {23},
                    [340250001] = {25},
                    [340340001] = {34}
                  }

  for k, v in pairs(questmap) do
    if quest:isSubmit(k) == true then
      for i = 1, #v do
        if freyja:isVisible(v[i]) == false then
          freyja:addFreyja(v[i])
        end
      end
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 处理冒险升级经验修正导致经验溢出玩家无法立即升级
-- 指定 : momo
-- 日期 : 2017-01-13
function Patch_2017_01_13_2(user)
  local version = 2017011317
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  manual:addPoint(1)

  user:addPatchLoad(version)
end

-- 功能 : 弓手系职业重置
-- 指定 : 张申林 & 虞佳
-- 日期 : 2017-01-13
function Patch_2017_01_13_3(user)
  local version = 2017011323
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local fighter = user:getCurFighter()
  if fighter == nil then
    return
  end

  local skill = fighter:getSkill()
  if skill == nil then
    return
  end

  local profes = user:getProfession()
  if profes >= 41 and profes <= 44 then
    skill:resetSkill()
  end

  user:addPatchLoad(version)
end


-- 功能 : 账号根据等级添加默认信用度
-- 指定 : 钟重熙
-- 日期 : 2017-01-17
function Patch_2017_01_17(user)
  local version = 201701172039
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local userdata = user:getUserSceneData()
  if userdata == nil then
    return
  end

  local userlv = user:getLevel()
  local value = 0
  if ServerGame.Credit ~= nil and ServerGame.Credit.default_lv_value ~= nil then
    for key, val in pairs(ServerGame.Credit.default_lv_value) do
      if val.min ~= nil and val.max ~= nil and
        val.min <= userlv and val.max >= userlv then
        value = val.value
        break
      end
    end
  end

  userdata:addCredit(value)
  user:addPatchLoad(version)
end

-- 功能 : 加点异常账号重置技能点
-- 指定 : 老邢
-- 日期 : 2017-02-10
function Patch_2017_02_10(user)
  local version = 201702101152
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local fighter = user:getCurFighter()
  if fighter == nil then
    return
  end

  local skill = fighter:getSkill()
  if skill == nil then
    return
  end

  if skill:checkSkillPointIllegal() == true then
    skill:resetSkill()
  end

  user:addPatchLoad(version)
end

-- 功能 : 处理任务更新导致重置材料补偿
-- 制定 : 李志鹏
-- 日期 : 2017-02-17
function Patch_2017_02_17(user)
  local version = 201702171

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  if quest == nil then
    return
  end

  local sender = "系统"
  local title = "来自洛马斯的来信"
  local msg = "由于某种原因我的考核失败了，但是我还再试一次，冒险者，你愿意帮助我吗？在旧有的考验中所消耗的材料会随邮件返还给冒险者。"

  -- 任务 : 350100001
  local step = quest:getPatchStep()
  if step >= 8 then
    sendOptMail(user:getTempID(), sender, title, msg, 52103, 30)
  end

  user:addPatchLoad(version)
end

-- 功能 : 处理冒险任务未正常缴纳材料,接取新任务
-- 制定 : 李志鹏
-- 日期 : 2017-02-18
function Patch_2017_02_18(user)
  local version = 201702181

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local patchmgr = getPatchManager()
  local quest = user:getQuest()
  if patchmgr == nil or quest == nil then
    return
  end

  if patchmgr:isPatchChar(user:getTempID()) == true then
    if quest:acceptQuest(600010001, true) == false then
      return
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 友情之证从货币移动到背包
-- 制定 : momo
-- 日期 : 2017-02-23
function Patch_2017_02_23(user)
  local version = 201702231

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local userdata = user:getUserSceneData()
  if userdata == nil then
    return
  end

  local friendship = userdata:getFriendShip()
  if friendship ~= 0 then
    local package = user:getPackage()
    local itemmgr = getItemManager()
    if package == nil or itemmgr == nil then
      return
    end
    local mainpack = package:getPackage(EPACKTYPE_MAIN)
    if mainpack == nil then
      return
    end

    local item = itemmgr:createItemLua(147, friendship, userdata:getOnlineMapID())
    if item == nil then
      return
    end
    mainpack:addItemObj(item, true, ESOURCE_GM)
    userdata:setFriendShip(0)
  end

  user:addPatchLoad(version)
end

-- 功能 : 配置表更改,处理已经开启1116的玩家,重新开启一次
-- 制定 : momo
-- 日期 : 2017-02-23
function Patch_2017_02_23_1(user)
  local version = 201702232

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local menu = user:getMenu()
  local event = user:getEvent()
  if menu == nil or event == nil then
    return
  end

  if menu:isOpen(1116) == true then
    menu:processMenuEvent(1116)
    event:onMenuOpen(1116)
  end

  user:addPatchLoad(version)
end

-- 功能 : 因版本代码没合,处理已经开启月卡未获得冒险经验的玩家
-- 制定 : 孔鸣
-- 日期 : 2017-02-23
function Patch_2017_02_23_2(user)
  local version = 201702233

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local monthcfg_1 = Table_Item[800101]
  local monthcfg_2 = Table_Item[800102]
  if monthcfg_1 == nil or monthcfg_2 == nil then
    return
  end

  local status = manual:getCollectionStatus(monthcfg_1.id)
  if status > 2 then
    manual:addPoint(monthcfg_1.AdventureValue, EMANUALTYPE_COLLECTION, monthcfg_1.id)
  end
  status = manual:getCollectionStatus(monthcfg_2.id)
  if status > 2 then
    manual:addPoint(monthcfg_2.AdventureValue, EMANUALTYPE_COLLECTION, monthcfg_2.id)
  end

  user:addPatchLoad(version)
end

-- 功能 : 配置错误,处理冒险手册中因为配置错误的选项
-- 制定 : 伟冲
-- 日期 : 2017-03-07
function Patch_2017_03_07(user)
  local version = 201703071

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local item = {17107, 17108, 17200, 17201, 17202, 17203, 17204, 17205, 17206, 17207, 17208, 17209, 17210, 17211, 17212, 17213, 17214}
  for i = 1, #item do
    manual:removeItem(EMANUALTYPE_MONSTER, item[i])
  end

  user:addPatchLoad(version)
end

-- 功能 : 刷新装备强化价格,修正强化价格错误
-- 制定 : 孙惠伟
-- 日期 : 2017-04-29
function Patch_2017_04_29(user)
  local version = 201704029

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local pack = user:getPackage()
  if pack == nil then
    return
  end

  pack:patch_equip_strengthlv()

  user:addPatchLoad(version)
end

-- 功能 : 成就,处理任务老数据
-- 制定 : 孙惠伟
-- 日期 : 2017-06-07
function Patch_2017_06_07(user)
  local version = 201706071

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local achieve = user:getAchieve()
  if achieve == nil then
    return
  end

  local quest = {
                  270126,270134,270141,
                  270150,99620007,99630008,
                  270230,270220,270200,270210,
                  270241, 270242, 270243, 270244
                }

  for i = 1, #quest do
    --print(quest[i])
    achieve:onQuestSubmit(quest[i])
  end

  user:addPatchLoad(version)
end

-- 功能 : 成就,处理任务老数据
-- 制定 : 孙惠伟
-- 日期 : 2017-06-20
function Patch_2017_06_20(user)
  local version = 201706201

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local achieve = user:getAchieve()
  if achieve == nil then
    return
  end

  achieve:onAchieveFinishLua()
  user:addPatchLoad(version)
end

-- 功能 : 修正配置表错误,修正包裹道具12111改为710030
-- 制定 : 孙惠伟
-- 日期 : 2017-07-17
function Patch_2017_07_17(user)
  local version = 201707171

  if user == nil or user:getTempID() == 0 then
    return
  end
  if user:hasPatchLoad(version) == true then
    return
  end

  local pack = user:getPackage()
  if pack == nil then
    return
  end

  pack:itemModify(12111, 710030)
  user:addPatchLoad(version)
end

-- 功能 : 骑士的起源奖励修改,给已完成任务玩家补偿奖励
-- 制定 : momo
-- 日期 : 2017-08-14
function Patch_2017_08_14(user)
  local version = 201708141

  if user == nil or user:getTempID() == 0 then
    return
  end
  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  if quest == nil then
    return
  end

  local questid = {700020001, 700020002, 700020003, 700020004, 700020005}
  for i = 1, #questid do
    if quest:isSubmit(questid[i]) == true then
      user:addMoney(EMONEYTYPE_GARDEN, 200, ESOURCE_QUEST)
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 处理任务完成未解锁地图
-- 指定 : 孙惠伟
-- 日期 : 2017-09-04
function Patch_2017_09_04(user)
  local version = 201709041
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  local freyja = user:getFreyja()
  if quest == nil or freyja == nil then
    return
  end

  local questmap = {
                    [340420001] = {42},
                    [340430001] = {43},
                  }

  for k, v in pairs(questmap) do
    if quest:isSubmit(k) == true then
      for i = 1, #v do
        if freyja:isVisible(v[i]) == false then
          freyja:addFreyja(v[i])
        end
      end
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 配置表更改,处理已经开启1116的玩家,重新开启一次
-- 制定 : momo
-- 日期 : 2017-09-08
function Patch_2017_09_08(user)
  local version = 20170908

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local menu = user:getMenu()
  local event = user:getEvent()
  if menu == nil or event == nil then
    return
  end

  if menu:isOpen(1901) == true then
    menu:processMenuEvent(1901)
    event:onMenuOpen(1901)
  end

  user:addPatchLoad(version)
end

-- 功能 : 钟楼补丁
-- 指定 : momo
-- 日期 : 2017-09-08
function Patch_2017_09_08_1(user)
  local version = 201709081
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  local freyja = user:getFreyja()
  if quest == nil or freyja == nil then
    return
  end

  local questmap = {[340440001] = {44,45,46},}

  for k, v in pairs(questmap) do
    if quest:isSubmit(k) == true then
      for i = 1, #v do
        if freyja:isVisible(v[i]) == false then
          freyja:addFreyja(v[i])
        end
      end
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : menu功能未实现率先更新配置
-- 制定 : momo
-- 日期 : 2017-09-09
function Patch_2017_09_09(user)
  local version = 201709091

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local menu = user:getMenu()
  local event = user:getEvent()
  if menu == nil or event == nil then
    return
  end

  local menuIDs = {89, 90, 91, 94}

  for i = 1, #menuIDs do
    if menu:isOpen(menuIDs[i]) == true then
      menu:processMenuEvent(menuIDs[i])
      event:onMenuOpen(menuIDs[i])
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 修正武僧和十字军已经做完转职任务
-- 制定 : 孙惠伟
-- 日期 : 2017-09-11
function Patch_2017_09_11(user)
  local version = 201709111

  if user == nil then
    return
  end
  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  if quest == nil then
    return;
  end

  quest:patch_2017_09_11()

  user:addPatchLoad(version)
end

-- 功能 : 修正任务因为其他几只多给的道具
-- 制定 : 刘俊
-- 日期 : 2017-10-09
function Patch_2017_10_09(user)
  local version = 201710091

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  local pack = user:getPackage()
  if quest == nil and pack == nil then
    return
  end

  local quest_item = {[600620001] = 53082, [600630001] = 53083, [600640001] = 53084, [600650001] = 53085, [600660001] = 53086, [600670001] = 53087,
                      [600740001] = 53088, [600750001] = 53089, [600760001] = 53090, [600770001] = 53091, [600780001] = 53092}

  for k, v in pairs(quest_item) do
    if quest:isSubmit(k) == true then
      pack:itemRemove(v, EPACKTYPE_MAIN, ESOURCE_QUEST)
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 刷新冒险称号任务
-- 制定 : 孙惠伟
-- 日期 : 2017-12-07
function Patch_2017_12_07(user)
  local version = 201712071

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local title = user:getTitle()
  local quest = user:getQuest()
  if title == nil or quest == nil then
    return
  end

  local titlequest = {
    [1002] = {390010001},
    [1003] = {390020001},
    [1004] = {390030001, 390030002, 390030003, 390030004, 390030005, 390030006, 390030007},
    [1005] = {390040001, 390040002, 390040003, 390040004, 390040005},
    [1006] = {390060001, 390060002, 390060003, 390060004, 390060005, 390060006, 390060007, 390060008, 390060009, 390060010, 390060011}
  }

  for k, v in pairs(titlequest) do
    if title:hasTitle(k) == true then
      for i = 1, #v do
        if quest:isAccept(v[i]) == true then
          --print("title = "..k.."accept = "..v[i])
          quest:processQuest(EQUESTMETHOD_DEL_ACCEPT, v[i])
        end
        if quest:isSubmit(v[i]) == false then
          --print("title = "..k.."submit = "..v[i])
          quest:processQuest(EQUESTMETHOD_ADD_SUBMIT, v[i])
        end
      end
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 刷新冒险称号任务
-- 制定 : 刘俊
-- 日期 : 2018-01-03
function Patch_2018_01_03(user)
  local version = 201801031

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local questnpc = user:getQuestNpc()
  if questnpc == nil then
    return
  end

  local questids = {6003, 39001, 39002, 39003, 39004, 39006, 9925, 9926, 9951, 60093,
                    60099, 60100, 60101, 60102, 60103, 60104, 60105, 60106, 60107, 60108,
                    60109, 60110, 60111, 60112, 60113, 60114, 60115, 60116, 30009, 39022,
                    39024, 39404}
  for i = 1, #questids do
    --print("questids = "..questids[i])
    questnpc:setQuestStatus(questids[i], true)
  end

  user:addPatchLoad(version)
end

-- 功能 : 配置表更改,处理已经开启641的玩家,重新开启一次
-- 制定 : momo
-- 日期 : 2018-01-08
function Patch_2018_01_08(user)
  local version = 201801081

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local menu = user:getMenu()
  local event = user:getEvent()
  if menu == nil or event == nil then
    return
  end

  if menu:isOpen(641) == true then
    menu:processMenuEvent(641)
    event:onMenuOpen(641)
  end

  user:addPatchLoad(version)
end

-- 功能 : 配置表更改,处理已经开启641的玩家,重新开启一次
-- 制定 : 孔鸣
-- 日期 : 2018-01-08
function Patch_2018_01_08_2(user)
  local version = 201801082

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  if quest == nil then
    return
  end
  if quest:getQuestStep(390060003) == 4 then
    quest:setQuestStep(390060003, 0)
  end

  user:addPatchLoad(version)
end

-- 功能 : 删除无用的任务道具
-- 制定 : 孙惠伟
-- 日期 : 2018-01-13
function Patch_2018_01_13(user)
  local version = 201801131

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local pack = user:getPackage()
  if pack == nil then
    return
  end

  local items = {53008, 54012, 54013}
  for i = 1, #items do
    pack:itemRemove(items[i], EPACKTYPE_QUEST, ESOURCE_QUEST)
  end

  user:addPatchLoad(version)
end

-- 功能 : 附魔任务处理
-- 制定 : 孙惠伟
-- 日期 : 2018-01-22
function Patch_2018_01_22(user)
  local version = 201801221

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  if quest == nil then
    return
  end

  if quest:isSubmit(690010002) == true and quest:isSubmit(690010003) == true and quest:isSubmit(690010004) == true and quest:isSubmit(690010005) == true then
    quest:acceptQuest(690010006, true)
  end
  if quest:isSubmit(690020002) == true and quest:isSubmit(690020003) == true and quest:isSubmit(690020004) == true and quest:isSubmit(690020005) == true then
    quest:acceptQuest(690020006, true)
  end

  user:addPatchLoad(version)
end

-- 功能 : 流氓职业添加现学现用冒险技能
function Patch_2018_01_25(user)
  local version = 2018012517
  if user == nil then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local fighter = user:getCurFighter()
  if fighter == nil then
    return
  end

  local skill = fighter:getSkill()
  if skill == nil then
    return
  end
  local profes = user:getProfession()
  if profes == 92 or profes == 93 then
    skill:addSkill(466001, 999, 1, false)
  end

  user:addPatchLoad(version)
end

-- 功能 : 任务处理
-- 日期 : 2018-02-07
function Patch_2018_02_07(user)
  local version = 201802071

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  if quest == nil then
    return
  end

  if quest:isSubmit(311650002) == true and quest:isSubmit(311650003) == true then
    quest:acceptQuest(311650004, true)
  end
  if quest:isSubmit(311660002) == true and quest:isSubmit(311660003) == true then
    quest:acceptQuest(311660004, true)
  end
  if quest:isSubmit(311670002) == true and quest:isSubmit(311670003) == true then
    quest:acceptQuest(311670004, true)
  end
  if quest:isSubmit(311680002) == true and quest:isSubmit(311680003) == true then
    quest:acceptQuest(311680004, true)
  end

  user:addPatchLoad(version)
end

-- 功能 : 由于玩家可以无变身进入奥特曼副本,处理这天积分无效,全部清0,以后拉名单补
-- 日期 : 2018-06-15
function Patch_2018_06_15(user)
  local version = 201806151

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local var = user:getVar()
  if var == nil then
    return
  end

  var:setVarValue(EVARTYPE_ALTMAN_KILL, 0)

  user:addPatchLoad(version)
end

-- 功能 : 临时处理任务成就不刷新
-- 日期 : 2018-08-07
function Patch_2018_08_07(user)
  local version = 201807072

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local achieve = user:getAchieve()
  if achieve == nil then
    return
  end

  local quest = {
                  391010001,
                  396050002,
                  17020001,
                  390070005,
                  270126,270134,270141,
                  270150,99620007,99630008,
                  270230,270220,270200,270210,
                  270241,
                  270242,
                  270243,
                  270244,
                  200020007,200050007,
                  200030005,600850001,
                  600530003,600540003,600550003,
                  200040005,200100008,99680005,
                  600680001,600790001,
                  200060001,
                  200060002,
                  200060003,
                  200060004,
                  200180003,200290001,200370002,
                  200460005,
                  200550004,
                  601540003,601560003,601700003,
                  601680001,601780001,201090001,
                  601810003,200980008,601800011,
                  200940004,201000003,
                  200920001,200930001,
                  200600001,
                  201290001,
                  201140007,
                  201230005,
                  201420004,
                  201650004,
                  201730001,201580001,201640001,
                  601820003,601840002,601870003,601880003,601930003,601940002,
                  601950003,601970008,601960004,
                  201930003,
                  396060019,
                  601580006,
                  202270001,202320001,
                  202350006,201420003,201410004,
                  201380001,201390001,
                  201400004,
                  201940009,201950002,202340011,
                }

  for i = 1, #quest do
    achieve:onQuestSubmit(quest[i])
  end

  user:addPatchLoad(version)
end

-- 功能 : 完成700030001任务的玩家, 包裹添加宠物手册
-- 日期 : 20180911
function Patch_2018_09_11(user)
  local version = 201809111

  if user == nil then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  if quest == nil then
    return
  end

  if quest:isSubmit(700030001) == true then
    local package = user:getPackage()
    if package ~= nil then
      package:luaAddItem(5630,1,201809111)
    end
  end

  user:addPatchLoad(version)
end

-- ---------------------------------事件---------------------------------

function onLogin(user)
  -- 功能 : 处理附魔任务更新导致重置材料补偿(由于补丁idbug,之后运营邮件补偿)
  --Patch_2017_01_04(user)
  -- 功能 : 处理任务完成状态
  Patch_2017_01_05(user)
  -- 功能 : 刷新'队友保护'冒险技能,修正已创角的玩家刷新出技能
  Patch_2017_01_11(user)
  -- 功能 : 增加月卡激活地图传送,处理已完成传送任务,未能激活新配置地图
  Patch_2017_01_13(user)
  -- 功能 : 处理冒险升级经验修正导致经验溢出玩家无法立即升级
  Patch_2017_01_13_2(user)
  -- 功能 : 弓手系职业重置
  Patch_2017_01_13_3(user)
  -- 功能 : 账号根据等级添加默认信用度
  Patch_2017_01_17(user)
  -- 功能 : 加点异常账号重置技能点
  Patch_2017_02_10(user)
  -- 功能 : 处理任务更新导致重置材料补偿
  Patch_2017_02_17(user)
  -- 功能 : 处理冒险任务未正常缴纳材料,接取新任务
  Patch_2017_02_18(user)
  -- 功能 : 友情之证从货币移动到背包
  Patch_2017_02_23(user)
  -- 功能 : 配置表更改,处理已经开启1116的玩家,重新开启一次
  Patch_2017_02_23_1(user)
  -- 功能 : 因版本代码没合,处理已经开启月卡未获得冒险经验的玩家
  Patch_2017_02_23_2(user)
  -- 功能 : 配置错误,处理冒险手册中因为配置错误的选项
  Patch_2017_03_07(user)
  -- 功能 : 配置表更改,处理已经开启115的玩家,重新开启一次
  Patch_2017_04_07(user)
  -- 功能 : 学习对应的冒险技能点数退回
  Patch_2017_04_10(user)
  -- 功能 : 修正对外已开放的所有头饰和卡片在背包中已有但并未入册的情况
  Patch_2017_04_20(user)
  -- 功能 : 2个老的珍藏品道具需要进冒险手册
  --Patch_2017_04_21(user)    该补丁不再使用
  -- 功能 : 刷新装备强化价格,修正强化价格错误
  Patch_2017_04_29(user)
  -- 功能 : 包袱进冒险手册
  Patch_2017_05_15(user)
  -- 功能 : 调整外网玩家身上已有的高级附魔属性比例
  Patch_2017_05_19(user)
  -- 功能 : 鹗枭首领，冒险手册BUG
  Patch_2017_05_23(user)
  -- 功能 : 清除附魔id
  Patch_2017_06_06(user)
  -- 功能 : 成就,处理任务老数据
  Patch_2017_06_07(user)
  -- 功能 : menu id已经开启过857的人 再给他开启一次857
  Patch_2017_06_19_01(user)
  -- 功能 : 成就,处理任务老数据
  Patch_2017_06_20(user)
  -- 功能 : 月卡redis,处理
  Patch_2017_06_22(user)
-- 功能 : 运营活动月卡错误领取日志
  Patch_2017_07_11(user)
  -- 功能 : 修正配置表错误,修正包裹道具12111改为710030
  Patch_2017_07_17(user)
  -- 功能 : 修正配置表错误,兑换或交易所购买过这些星座头饰的角色，该头饰为解锁
  Patch_2017_07_28(user)
  -- 功能 : 冒险手册里删掉魔物图鉴-跳跳虫
  Patch_2017_07_28_01(user)
  -- 功能 : 道具45206进入冒险手册
  Patch_2017_07_31(user)
  -- 功能 : 之前完成成就没有给称号的玩家补发称号奖励
  Patch_2017_08_08(user)
  -- 功能 : menu表进入冒险手册的卡片补丁
  Patch_2017_08_09(user)
  -- 功能 : 骑士的起源奖励修改,给已完成任务玩家补偿奖励
  Patch_2017_08_14(user)
  -- 功能 : 23102进入冒险手册
  Patch_2017_08_16(user)
  -- 功能 : 删除之前客户端添加的红点
  Patch_2017_08_17(user)
  -- 功能 : 处理任务完成未解锁地图
  Patch_2017_09_04(user)
  -- 功能 : 萌新福利
  Patch_2017_09_07(user)
  -- 功能 : 配置表更改,处理已经开启1116的玩家,重新开启一次
  Patch_2017_09_08(user)
  -- 功能 : 钟楼补丁
  Patch_2017_09_08_1(user)
  -- 功能 : menu功能未实现率先更新配置
  Patch_2017_09_09(user)
  -- 功能 : 修正武僧和十字军已经做完转职任务
  Patch_2017_09_11(user)
  -- 功能 : 修正任务因为其他几只多给的道具
  -- 2017-09-26屏蔽,任务id配错,导致误删,稍后开启
  --Patch_2017_09_12(user)
  -- 功能 : 道具550001(厨师帽)进入冒险手册
  Patch_2017_09_13(user)
  -- 功能 : menu表进入冒险手册的卡片补丁
  Patch_2017_09_18(user)
  -- 功能 : 修正任务因为其他几只多给的道具
  Patch_2017_10_09(user)
  -- 功能 : 刷新冒险称号任务
  Patch_2017_12_07(user)
  -- 功能 : 刷新冒险称号任务
  Patch_2018_01_03(user)
  -- 功能 : 配置表更改,处理已经开启641的玩家,重新开启一次
  Patch_2018_01_08(user)
  -- 功能 : 配置表更改,处理已经开启641的玩家,重新开启一次
  Patch_2018_01_08_2(user)
  -- 功能 : 删除无用的任务道具
  Patch_2018_01_13(user)
  -- 功能 : 附魔任务处理
  Patch_2018_01_22(user)
  -- 功能 : 流氓职业添加现学现用冒险技能
  Patch_2018_01_25(user)
  -- 功能 : 料理任务
  Patch_2018_01_29(user)
  -- 功能 : 任务处理
  Patch_2018_02_07(user)
  -- 功能 : 23111进入冒险手册
  Patch_2018_03_07(user)
  -- 功能 : 冒险手册51081和51082状态设置为灰显状态
  --Patch_2018_03_15(user)
  -- 功能 : 获得成就的添加通关层数
  Patch_2018_03_20(user)
  -- 功能 : 技能解锁menu,刷新
  Patch_2018_04_02(user)
  -- 功能 : EOperateType 类型活动,领奖数据从Redis移动到Var值保存
  Patch_2018_04_09(user)
  -- 功能 : 部分玩家未消耗掉30点巅峰等级技能点, 便激活了4转技能, 须重置该类玩家的技能
  Patch_2018_05_24(user)
  --功能 : 任务奖励的生命体技能 在加载存档时会丢失，补给玩家
  Patch_2018_06_13(user)
  -- 功能 : 由于玩家可以无变身进入奥特曼副本,处理这天积分无效,全部清0,以后拉名单补
  Patch_2018_06_15(user)
  --功能 : 生命体技能加点bug, 有重复技能。检测重置有重复技能的生命体
  Patch_2018_06_21(user)
  -- 功能 : 临时处理任务成就不刷新
  Patch_2018_08_07(user)
  -- 功能 : 女仆巅峰引导
  Patch_2018_08_10(user)
  -- 功能 : 老的商品删除后, id被新商品复用, 导致买过老商品的玩家的商品购买次数>0, 影响购买新商品
  Patch_2018_08_14_acc(user)
  -- 功能 : 完成700030001任务的玩家, 包裹添加宠物手册
  Patch_2018_09_11(user)
end

function onDailyRefresh(user)
  -- 功能 : 千人测试获取额外邮件奖励
  --sendTestReward(user)
end

----  进入场景 -------
function onEnterScene(user)
end

-- 功能 : 处理已完成传送任务,未能激活新配置地图 menuid 115
-- 指定 : momo
-- 日期 : 2017-04-07
function Patch_2017_04_07(user)
  local version = 20170407
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  local freyja = user:getFreyja()
  if quest == nil or freyja == nil then
    return
  end

  local questmap = {
                    [340160001] = {15, 16, 38}
                  }

  for k, v in pairs(questmap) do
    if quest:isSubmit(k) == true then
      for i = 1, #v do
        if freyja:isVisible(v[i]) == false then
          freyja:addFreyja(v[i])
        end
      end
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 学习指定冒险技能，退换点数
-- 制定 : momo
-- 日期 : 2017-04-10
function Patch_2017_04_10(user)
  local version = 2017040810
  if user == nil then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  local fighter = user:getCurFighter()
  if fighter == nil then
    return
  end

  local skill = fighter:getSkill()
  if skill == nil then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local skillmap = {
                    [50019001] = {1,1001,4005},
                    [50021001] = {1,1002,4006},
                    [50020001] = {2,1003,4007},
                    [50022001] = {2,1004,4008}
                  }

  for k, v in pairs(skillmap) do
    if skill:isSkillEnable(k) == true then
      manual:addSkillPoint(v[1])
      sendMail(user:getTempID(), v[2])
      sendMsg(user:getTempID(), v[3])
    end
  end

  user:addPatchLoad(version)

end

-- 功能 : 修正对外已开放的所有头饰和卡片在背包中已有但并未入册的情况
-- 制定 : momo
-- 日期 : 2017-04-20
function Patch_2017_04_20(user)
  local version = 2017042002
  if user == nil then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  local package = user:getPackage()
  if package == nil then
    return
  end

  local equippack = package:getEquipPackage()
  if equippack == nil then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local itemmap = {23100,20001,20002,23200,20003,22001,22002,23001,20004,20005,20006,23204,20007,20008,20009,23002,20010,20011,20012,20013,22003,20014,20015,20016,
  23003,20017,20018,20021,20020,20019,20022,22004,23004,20023,20024,20025,23205,22005,20026,20027,20028,23005,20029,20030,20031,22006,23006,20032,20033,20034,20035,
  22007,23007,20036,20040,20041,20042,23009,23203,20037,20038,20039,23008,23201,23202,20043,20094,22008,20044,20045,20046,20047,20048,20049,20050,22009,23010,20051,
  20052,22010,23011,20053,20054,20055,23206,20056,20057,20058,22018,23012,20059,20060,20061,22019,23013,20062,20063,20064,22011,23014,20065,20066,20067,22012,23015,
  20068,20069,20070,23016,20072,20071,23207,20073,22013,23017,20095,20096,20097,23024,20074,20075,20100,20101,22020,23027,20076,20077,20078,23018,22014,20079,20080,
  20081,20082,22015,20083,20084,20086,20085,22016,23020,20087,20088,20089,20090,22017,23021,20098,23023,23101,
  45073,47019,45076,145195,45188,48529,45092,145092,48531,45077,145193,45180,45078,145079,48558,45027,45082,45181,145080,145116,48559,47008,145097,145081,145107,
  48505,145029,48534,145172,145122,145120,145144,47020,45026,45083,48533,145170,45126,48545,45064,45032,45047,45128,48539,48548,145084,145125,145108,45191,145138,
  45112,45192,145102,45110,45118,45204,145154,145129,48006,145186,145185,45141,45001,48501,45002,45003,45004,145005,145006,48502,45007,48540,45008,45009,45010,45011,
  145179,45088,45013,48537,145014,48503,48504,45015,145016,145018,145019,48506,48507,48508,45020,48509,48510,48511,48512,48554,48555,48556,45021,48513,48517,45022,
  48515,45023,45024,45025,45030,145031,48519,48516,145028,45033,48518,145182,45012,45034,45035,45183,45045,48521,145136,145046,145132,45036,45037,45038,145039,48514,
  45040,145041,48520,45042,145043,145044,45094,145184,48007,145171,45048,48523,45049,48524,145050,45051,45052,45053,145054,45119,45055,145098,145095,45056,45177,
  48550,45117,145178,48004,145057,48560,45058,45061,45062,47016,45063,48525,48551,45065,45066,48526,45059,145060,145149,48552,145173,145203,47018,48527,45067,45068,
  145069,48535,45134,45070,45071,47015,48528,45072,48553,145090,48547,48008,45143,47021,45075,48530,48557,145146,47002,145194,145196,45199,45198,45200,145201,145086,
  45197,145202,47022,45205}
  for i=1,#itemmap do
    local blExist = package:checkItemCountByID(itemmap[i],1,EPACKTYPE_MAIN) or package:checkItemCountByID(itemmap[i],1,EPACKTYPE_PERSONAL_STORE)
                    or package:checkItemCountByID(itemmap[i],1,EPACKTYPE_TEMP_MAIN) or equippack:getEquipedItemNum(itemmap[i]) > 0
    if blExist == true then
      manual:onItemAdd(itemmap[i],true,true)
    end
  end

  user:addPatchLoad(version)

end

-- 功能 : 包袱进冒险手册
-- 制定 : 陈伟冲
-- 日期 : 2017-05-15
function Patch_2017_05_15(user)
  local version = 2017051501
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local itemmap = {4294967820, 4294997387, 4294987297, 4295027315}

  for i=1,#itemmap do
    if user:getCharIDString() == tostring(itemmap[i]) then
      manual:onItemAdd(45187,true,true,false,ESOURCE_COMPOSE)
      sendMail(user:getTempID(), 1005)
      user:addPatchLoad(version)
      return
    end
  end

  user:addPatchLoad(version)

end

-- 功能 : 调整外网玩家身上已有的高级附魔属性比例
-- 制定 : 张申林
-- 日期 : 2017-05-19
function Patch_2017_05_19(user)
   local version = 2017051901
   if user == nil or user:getTempID() == 0 then
      return
   end

   if user:hasPatchLoad(version) then
      return
   end

   local package = user:getPackage()
   if package == nil then
      return
   end

   package:fixEnchantAttr()

   user:addPatchLoad(version)
end

-- 功能 : 鹗枭首领，冒险手册BUG
-- 制定 : 陈伟冲
-- 日期 : 2017-05-23
function Patch_2017_05_23(user)
  local version = 2017052301
  if user == nil or user:getTempID() == 0 then 
    return
  end

  if user:hasPatchLoad(version) then 
    return
  end

  local manual = user:getManual()
  if manual == nil then 
    return
  end

  manual:removeItem(EMANUALTYPE_MONSTER,18039)

  user:addPatchLoad(version)

end

-- 功能 : 调整外网玩家遗留的附魔buffid
-- 制定 : 张申林
-- 日期 : 2017-06-06
function Patch_2017_06_06(user)
   local version = 2017060601
   if user == nil or user:getTempID() == 0 then
      return
   end

   if user:hasPatchLoad(version) then
      return
   end

   local package = user:getPackage()
   if package == nil then
      return
   end
  local guidmap = {
                    {4301040631, "1-100010783-18-1488957265"},
                    {4295107104, "5-100010302-11381752-1488813220"},
                    {4295658163, "1-100011202-18-1493879740"},
                    {4304489490, "1-100010185-18-1492297043"},
                    {4295098834, "1-100010034-18-1493742848"},
                    {4297396135, "1-100010124-16-1488797093"},
                    {4295163026, "1-100010064-16-1492893781"},
                    {4294982424, "1-100011082-23-1488448553"},
                    {4294982887, "1-100011292-16-1493357652"},
                    {4299074728, "2-100010243-16-1491485231"},
                    {4297293476, "1-100011862-24-1491041331"},
                    {4297293476, "3-100011682-24-1493183417"},
                    {4300203848, "1-100011294-16-1491035752"},
                    {4295083082, "1-100010093-18-1493280137"},
                    {4294974529, "1-100010813-16-1486663724"},
                    {4303202352, "1-100010692-18-1491442501"},
                    {4295165912, "1-100010273-16-1490914540"},
                    {4303978572, "1-100011592-11254272-1491058736"},
                    {4302545914, "3-100010002-16-1492699617"},
                    {4295640247, "1-100010182-27-1493171930"},
                    {4300287977, "1-100011923-13-1491585730"},
                    {4295018255, "1-100010934-13-1488819539"},
                    {4294992066, "1-100010304-18-1487307870"},
                    {4295959658, "1-100011833-11149733-1492008768"},
                    {4295959658, "1-100011563-18-1491563766"},
                    {4295519298, "1-100010242-16-1494396085"},
                    {4294979156, "1-100011082-16-1489344502"},
                    {4302343116, "20-100010092-11199142-1496636924"},
                    {4294999966, "2-100011292-18-1488126212"},
                    {4295123182, "1-100011504-16-1494436003"},
                    {4297478869, "1-100011832-18-1491399404"},
                    {4298612806, "1-100011323-18-1491015591"},
                    {4294981879, "1-100010814-16-1487502154"},
                    {4295328302, "1-100010602-18-1494996750"},
                    {4298661519, "2-100010065-16-1490934857"},
                    {4295191517, "1-100010274-16-1488986051"},
                    {4294985618, "1-100011593-18-1492926877"},
                    {4298938932, "1-100010573-23-1490964958"},
                    {4295193126, "1-100010544-16-1488342308"},
                    {4297525968, "1-100011624-16-1493044981"},
                    {4295498025, "1-100010844-16-1486813861"},
                    {4302099201, "1-100010724-16-1488305382"},
                    {4302099201, "1-100010692-16-1488304642"},
                    {4298977737, "2-100010212-16-1494625629"},
                    {4299173470, "1-100010752-18-1495845956"},
                    {4299094817, "1-100011623-18-1495637862"},
                    {4295128235, "1-100011142-16-1486460547"},
                    {4295182665, "2-100011144-16-1492941670"},
                    {4295004052, "1-100010574-11156044-1491758997"},
                    {4299787902, "1-100010094-16-1488471154"},
                    {4297668408, "1-100010245-16-1496464869"},
                    {4296914517, "1-100011623-18-1491522494"},
                    {4295574014, "1-100011173-16-1487775092"},
                    {4295680738, "1-100010932-18-1493025118"},
                    {4295010953, "1-100011083-18-1494664417"},
                    {4298151747, "1-100010362-16-1490787443"},
                    {4294986040, "2-100010393-11204893-1487346687"},
                    {4294986040, "2-100010002-11181542-1487246268"},
                    {4295056287, "1-100011684-11139524-1487504972"},
                    {4297997639, "1-100010783-11175243-1491828364"},
                    {4297997639, "1-100010754-18-1489924295"},
                    {4295586751, "1-100010844-13-1491490543"},
                    {4304478624, "1-100011353-33-1494757365"},
                    {4295548509, "1-100011142-16-1490664821"},
                    {4295112400, "1-100010362-18-1496641522"},
                    {4298776679, "1-100011623-16-1487932107"},
                    {4300936683, "2-100011623-23-1492157594"},
                    {4297505168, "3-100011653-35-1496656294"},
                    {4297969905, "1-100011774-16-1493737470"},
                    {4295062631, "1-100010002-13282772-1492843839"},
                    {4295062631, "1-100011144-18-1492698000"},
                    {4300536249, "4-100011294-9-1491470110"},
                    {4297455199, "1-100011352-16-1496641179"},
                    {4295700370, "1-100011473-18-1487694169"},
                    {4298192519, "1-100010994-23-1491907747"},
                    {4298636551, "1-100011682-16-1488332482"},
                    {4302934933, "2-100010663-16-1491766926"},
                    {4294985120, "1-100010092-16-1489852008"},
                    {4305644015, "1-100011773-13-1495298074"},
                    {4295045254, "2-100011624-11147224-1487258990"},
                    {4295500324, "1-100010782-16-1486945871"},
                    {4298674158, "1-100011203-16-1486606365"},
                    {4294985638, "1-100011862-18-1489821849"},
                    {4295113765, "1-100011562-18-1495531055"},
                    {4296813699, "1-100011384-16-1487986407"},
                    {4301323826, "1-100011023-16-1494912777"},
                    {4302466329, "1-100011412-18-1495464753"},
                    {4306843231, "1-100011444-18-1486652295"},
                    {4304132255, "2-100011892-16-1491306923"},
                    {4300149119, "2-100010243-16-1490629542"},
                    {4297112033, "1-100010242-16-1489666401"},
                    {4298568789, "1-100011052-16-1490197011"},
                    {4299028869, "1-100010183-16-1490939568"},
                    {4296724657, "1-100010005-18-1490340362"},                              
                  }

    for k, v in pairs(guidmap) do
      if user:getCharIDString() == tostring(v[1]) then
        package:clearEnchantBuffid(v[2])       
      end
    end
  user:addPatchLoad(version)
end


-- 功能 : menu id已经开启过857的人 再给他开启一次857
-- 制定 : 孔明
-- 日期 : 2017-06-19-01
function Patch_2017_06_19_01(user)
  local version = 2017061902

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local menu = user:getMenu()
  local event = user:getEvent()
  if menu == nil or event == nil then
    return
  end

  if menu:isOpen(857) == true then
    menu:processMenuEvent(857)
    event:onMenuOpen(857)
  end

  user:addPatchLoad(version)
end

-- 功能 : 月卡redis数据转到mysql
-- 日期 : 2017-06-06-22
function Patch_2017_06_22(user)
   local version = 2017062201
   if user == nil or user:getTempID() == 0 then
      return
   end

   if user:hasPatchLoad(version) then
      return
   end
   user:redisPatch() 
   user:addPatchLoad(version)
end

-- 功能 : 运营活动错误领取日志
-- 日期 : 2017-07-11
function Patch_2017_07_11(user)
   local version = 20170711
   if user == nil or user:getTempID() == 0 then
      return
   end

   if user:hasPatchLoad(version) then
      return
   end
   user:showMonthCardErrorLog()
   user:addPatchLoad(version)
end

-- 功能 : 兑换或交易所购买过这些星座头饰的角色，该头饰为解锁
-- 指定 ：momo
-- 日期 : 2017-07-28
function Patch_2017_07_28(user)
  local version = 20170728
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end
  
  local package = user:getPackage()
  if package == nil then
    return
  end

  local equippack = package:getEquipPackage()
  if equippack == nil then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local itemmap = {145220,145221,145222,145223,145224,145225,145226,145227,145228,145229,145230,145231,
                   45158,45159,45160,45161,45162,45163,45164,45165,45166,45167,45168,45169}
  for i=1,#itemmap do
    local blExist = package:checkItemCountByID(itemmap[i],1,EPACKTYPE_MAIN) 
                    or package:checkItemCountByID(itemmap[i],1,EPACKTYPE_PERSONAL_STORE)
                    or package:checkItemCountByID(itemmap[i],1,EPACKTYPE_STORE) 
                    or package:checkItemCountByID(itemmap[i],1,EPACKTYPE_TEMP_MAIN) 
                    or package:checkItemCountByID(itemmap[i],1,EPACKTYPE_BARROW)                   
                    or equippack:getEquipedItemNum(itemmap[i]) > 0
    if blExist == true then
      manual:onItemAdd(itemmap[i],true,true)
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 冒险手册里删掉魔物图鉴-跳跳虫
-- 日期 : 2017-07-28
function Patch_2017_07_28_01(user)
  local version = 2017072801
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  manual:delManualItem(EMANUALTYPE_MONSTER,53371)
  user:addPatchLoad(version)
end

-- 功能 : 45206进入冒险手册
-- 日期 : 2017-07-31
function Patch_2017_07_31(user)
  local version = 2017073102
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  local package = user:getPackage()
  if package == nil then
    return
  end
  local mainpack = package:getPackage(EPACKTYPE_MAIN)
  if mainpack == nil then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local blExist = package:checkItemCountByID(45206,1,EPACKTYPE_MAIN) or package:checkItemCountByID(45206,1,EPACKTYPE_BARROW)
    or package:checkItemCountByID(45206,1,EPACKTYPE_TEMP_MAIN)

  if blExist == true then
    manual:onItemAdd(45206,true,true)
  end
  user:addPatchLoad(version)
end

-- 功能 : 之前完成成就没有给称号的玩家补发称号奖励
-- 日期 : 2017-08-08
function Patch_2017_08_08(user)
  local version = 2017080801

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local achieve = user:getAchieve()
  if achieve == nil then
    return
  end

  local achieveIDs = {1201004, 1202004, 1202024, 1302003, 1302005, 1401013, 1401015, 1401023, 1401025, 1501004, 1601003, 1601005, 1601015, 1601025, 1601053,
                        1601055, 1602023, 1602025, 1603035}
  for i=1,#achieveIDs do
    achieve:patch_1(achieveIDs[i]);
  end

  user:addPatchLoad(version)
end

-- 功能 : menu表进入冒险手册的卡片补丁
-- 制定 : 彭冉
-- 日期 : 2017-08-09-01
function Patch_2017_08_09(user)
  local version = 2017080901

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local menu = user:getMenu()
  if menu == nil then
    return
  end

  local menuIDs = {1401, 1403, 1405, 1409}
  for i=1,#menuIDs do
    if menu:isOpen(menuIDs[i]) == true then
      menu:processMenuEvent(menuIDs[i])
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : 23102进入冒险手册
-- 日期 : 2017-08-16
function Patch_2017_08_16(user)
  local version = 2017081601
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  local package = user:getPackage()
  if package == nil then
    return
  end
  local mainpack = package:getPackage(EPACKTYPE_MAIN)
  if mainpack == nil then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local blExist = package:checkItemCountByID(23102,1,EPACKTYPE_MAIN) or package:checkItemCountByID(23102,1,EPACKTYPE_BARROW)
    or package:checkItemCountByID(23102,1,EPACKTYPE_PERSONAL_STORE)

  if blExist == true then
    manual:onItemAdd(23102,true,true)
  end
  user:addPatchLoad(version)
end


-- 功能 : 删除之前客户端添加的红点
-- 日期 : 2017-08-17
function Patch_2017_08_17(user)
  local version = 2017081701

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local tips = user:getTip()
  if tips == nil then
    return
  end

  tips:patch_1()

  user:addPatchLoad(version)
end
-- 功能 : 萌新福利-【修行礼包】助力成长
-- 日期 : 2017-09-07
function Patch_2017_09_07(user)
  local version = 2017090702
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  user:getPracticeReward()

  user:addPatchLoad(version)
end

-- 功能 : 道具550001(厨师帽)进入冒险手册
-- 日期 : 2017-09-13
function Patch_2017_09_13(user)
  local version = 2017091302
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  local package = user:getPackage()
  if package == nil then
    return
  end
  local mainpack = package:getPackage(EPACKTYPE_MAIN)
  if mainpack == nil then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local blExist = package:checkItemCountByID(550001,1,EPACKTYPE_MAIN) or package:checkItemCountByID(550001,1,EPACKTYPE_BARROW)
    or package:checkItemCountByID(550001,1,EPACKTYPE_TEMP_MAIN)

  if blExist == true then
    manual:onItemAdd(550001,true,true)
  end
  user:addPatchLoad(version)
end


-- 功能 : 配置表更改,处理已经开启29的玩家,重新开启一次
-- 制定 : 彭冉
-- 日期 : 2017-09-18
function Patch_2017_09_18(user)
  local version = 20170918
  local menuid = 29
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local menu = user:getMenu()
  local event = user:getEvent()
  if menu == nil or event == nil then
    return
  end

  if menu:isOpen(menuid) == true then
    menu:processMenuEvent(menuid)
    event:onMenuOpen(menuid)
  end

  user:addPatchLoad(version)
end

-- 功能 : 厨师等级升级前置任务丢失
-- 制定 : 孔鸣
-- 日期 : 2018-01-29
function Patch_2018_01_29(user)
  local version = 20180129
  if user:hasPatchLoad(version) == true then
    return
  end
  if user == nil or user:getTempID() == 0 then
    return
  end

  Patch_CookerLv(user)

  Patch_TasterLv(user)
  user:addPatchLoad(version) 
end

function Patch_CookerLv(user) 
  local quest = user:getQuest()
  --cooker lv
  local preQuest = {
      [2] = 600960001,   
      [3] = 600990001,
      [4] = 601000001,
      [5] = 601010001,
      [6] = 601020001,
      [7] = 601030001,
      [8] = 601040001,
      [9] = 601050001,
      [10] = 601060001
  }
  local lv = user:getCookerLv()
  if lv <= 0 then 
    -- print("料理补丁", lv, "厨师等级很低")
    return 
  end
  lv = lv + 1
  local questId = preQuest[lv]
  if questId == nil then 
    -- print("料理补丁", lv, questId, "没找到任务")
    return 
  end

  if quest:isSubmit(questId) == true then
    -- print("料理补丁", lv, questId, "任务已经提交")
    return
  end
  --
  if quest:isAccept(questId) == true then
    -- print("料理补丁", lv, questId, "任务正在接")
    return 
  end
  -- print("料理补丁", lv, questId, "补丁成功")
  quest:finishQuest(questId)
end

function Patch_TasterLv(user) 
  local quest = user:getQuest()
  local preQuest = {
      [3] = 601080001,
      [4] = 601090001,
      [5] = 601100001,
      [6] = 601110001,
      [7] = 601120001,
      [8] = 601130001,
      [9] = 601140001,
      [10] = 601150001
  }
  local lv = user:getTasterLv()
  if lv <= 0 then 
    return 
  end
  lv = lv + 1
  local questId = preQuest[lv]
  if questId == nil then 
    return 
  end

  if quest:isSubmit(questId) == true then
    return
  end
  --
  if quest:isAccept(questId) == true then
    return 
  end

  quest:finishQuest(questId)  
end

-- 功能 : 23111进入冒险手册
-- 日期 : 2018-03-07
function Patch_2018_03_07(user)
  local version = 2018030703
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  local package = user:getPackage()
  if package == nil then
    return
  end
  local mainpack = package:getPackage(EPACKTYPE_MAIN)
  if mainpack == nil then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  local blExist = package:checkItemCountByID(23111,1,EPACKTYPE_MAIN) or package:checkItemCountByID(23111,1,EPACKTYPE_BARROW)
    or package:checkItemCountByID(23111,1,EPACKTYPE_PERSONAL_STORE)

  if blExist == true then
    manual:addManualItem(2,23111,2,true)
  end
  user:addPatchLoad(version)
end

-- 功能 : 冒险手册51081和51082状态设置为灰显状态
-- 日期 : 2018-03-15
function Patch_2018_03_15(user)
  local version = 2018031501
  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) then
    return
  end

  local package = user:getPackage()
  if package == nil then
    return
  end
  local mainpack = package:getPackage(EPACKTYPE_MAIN)
  if mainpack == nil then
    return
  end

  local manual = user:getManual()
  if manual == nil then
    return
  end

  if manual:getFashionStatus(51083) > 1 then
    manual:addManualItem(1,51083,1,false)
  end

  user:addPatchLoad(version)
end

-- 功能 : 获得成就的玩家添加无限塔通关层数
-- 指定 : momo
-- 日期 : 2018-03-20
function Patch_2018_03_20(user)
  local version = 2018032001

  if user == nil or user:getTempID() == 0 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local achieve = user:getAchieve()
  if achieve == nil then
    return
  end

  local tower = user:getTower()
  if tower == nil then
    return
  end

  if achieve:isFinishAchieve(1602021) == true then
    for i = 1, 20 do
      tower:addEverPassLayer(i);
    end
  end

  if achieve:isFinishAchieve(1602022) == true then
    for i = 21, 40 do
      tower:addEverPassLayer(i);
    end
  end

  if achieve:isFinishAchieve(1602023) == true then
    for i = 41, 60 do
      tower:addEverPassLayer(i);
    end
  end

  user:addPatchLoad(version)
end

-- 功能 : menu 修改, 出现bug, 连续升级技能不能解锁menu
-- 日期 : 2018-04-02
function Patch_2018_04_02(user)
  local version = 201804021

  if user == nil then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local menu = user:getMenu()
  if menu == nil then
    return
  end

  menu:refreshNewMenu(5)
  user:addPatchLoad(version)
end

-- 功能 : EOperateType 类型活动,领奖数据从Redis移动到Var值保存
-- 日期 : 2018-04-09
function Patch_2018_04_09(user)
  local version = 2018040901

  if user == nil then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  user:patch_OperateReward()

  user:addPatchLoad(version)
end

function Patch_2018_05_24(user)
  local version = 2018052401

  if user == nil then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local fighter = user:getCurFighter()
  if fighter == nil then
    return
  end

  local skill = fighter:getSkill()
  if skill == nil then
    return
  end

  skill:fixEvo4Skill()

  user:addPatchLoad(version)
end

--功能 : 任务奖励的生命体技能 在加载存档时会丢失，补给玩家
function Patch_2018_06_13(user)
  local version = 2018061301

  if user == nil then
    return
  end
  local profes = user:getProfession()
  if profes < 132 or profes > 134 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local quest = user:getQuest()
  if quest == nil then
    return
  end

  local being = user:getUserBeing()
  if being == nil then
    return
  end

  -- 检查任务396480001, 丽芙的升华
  if quest:isSubmit(396480001) == true then
    if being:checkHasSkill(600010, 1132001) == false and being:checkHasSkill(600010, 1132002) == false and being:checkHasSkill(600010, 1132003) == false then
      being:addSkill(600010, 1132001)
    end
  end
  -- 检查任务396490001, 艾米斯的升华
  if quest:isSubmit(396490001) == true then
    if being:checkHasSkill(600020, 1133001) == false and being:checkHasSkill(600020, 1133002) == false and being:checkHasSkill(600020, 1133003) == false then
      being:addSkill(600020, 1133001)
    end
  end
  -- 检查任务396500001, 巴尼米的升华
  if quest:isSubmit(396500001) == true then
    if being:checkHasSkill(600030, 1134001) == false and being:checkHasSkill(600030, 1134002) == false and being:checkHasSkill(600030, 1134003) == false then
      being:addSkill(600030, 1134001)
    end
  end

  user:addPatchLoad(version)
end

--功能 : 生命体技能加点bug, 有重复技能。检测重置有重复技能的生命体
function Patch_2018_06_21(user)
  local version = 2018062101
  if user == nil then
    return
  end

  local profes = user:getProfession()
  if profes < 132 or profes > 134 then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local being = user:getUserBeing()
  if being == nil then
    return
  end

  -- 检查生命体是否有重复技能
  local beingids = {600010, 600020, 600030}
  for _,v in ipairs(beingids) do
    if being:checkHasSameSkill(v) == true then
      being:resetAllSkill(v)
    end
  end

  user:addPatchLoad(version)
end

--功能 : 女仆巅峰引导,为漏配置的任务打补丁
function Patch_2018_08_10(user)
  local version = 2018081001
  if user == nil then
    return
  end

  if user:hasPatchLoad(version) == true then
    return
  end

  local servant = user:getServant()
  if servant == nil then
    return
  end

  servant:checkForeverRecommend()

  user:addPatchLoad(version)
end

-- 功能 : 老的商品删除后, id被新商品复用, 导致买过老商品的玩家的商品购买次数>0, 影响购买新商品
local acclist2018081401 = nil
function Patch_2018_08_14_acc(user)
  local version = 2018081401
  if user == nil then
    return
  end

  -- 该补丁每个账号只执行一次
  if user:hasAccPatchLoad(version) == true then
    return
  end

  if acclist2018081401 == nil then
    local f, ret = pcall(dofile, "Patch/acclist2018081401.lua")
    if f and ret then
      acclist2018081401 = ret
    else
      return
    end
  end

  local shop = user:getSceneShop()
  if shop == nil then
    return
  end

  if shop:getShopItemCount(230100) > 0 and acclist2018081401[tonumber(user:getAccIDString())] == nil then
    shop:setShopItemCount(230100, 0, false)
  end

  user:addAccPatchLoad(version)
end
