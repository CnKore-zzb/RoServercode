-- ServerFunc.lua 服务器计算使用

math.randomseed( tonumber(tostring(os.time()):reverse():sub(1,6)) )

function CalcCameraSummonOdds(user, num, baseodds, isnight)
  if user == nil or baseodds == 0 then
    return false
  end
  local MaxOdds = 100000
  local odds = baseodds * math.pow(2, num + 1)
  if isnight then
    odds = odds / 2
  end
  if user:getMapID() == 9 then
    odds = odds / 2
  end
  odds = math.floor(odds);
  odds = odds < 1 and 1 or odds
  odds = odds > MaxOdds and MaxOdds or odds

  --print("---", odds, num, math.random(1, odds))
  if math.random(1, odds) == 1 then
    return true
  end

  return false
end

function CalcServerMonsterReload(NpcID, NpcType, Mapid, LifeTime, BeServerReloadTime, Star)
  if Star == true then
    return BeServerReloadTime
  end

  -- mvp | mini
  if NpcType == 4 or NpcType == 5 then
    return BeServerReloadTime
  end

  -- 特殊怪物 整点刷新
  if NpcID == 17311 or NpcID == 18132 or NpcID == 19011 then
    local nowtime = os.time()
    local nexthour = math.ceil(nowtime/3600) * 3600
    return nexthour - nowtime
  end

  --[[
      [2] = {
        [2] = {10001,10002},
        [3] = {10004,10005},
      }

      [time] = { [mapid] = {id1, id2}..}
  --]]
  if LifeTime<=10 then
    if ServerGame.ServerGame_MonsterRelive ~= nil and ServerGame.ServerGame_MonsterRelive.ReliveTime2Map ~= nil then
      for k, v in pairs(ServerGame.ServerGame_MonsterRelive.ReliveTime2Map) do
        if v[Mapid] ~= nil then
          for _, d in pairs(v[Mapid]) do
            if d == NpcID then
              return k
            end
          end
        end
      end
    end
  end

  return BeServerReloadTime
end

function CalcSpecEquipAttr(pEquip, equipID, refineLv)
  if pEquip == nil then
    return
  end
--测试精炼  精炼等级>=3,攻击+200;精炼等级>=5,攻击+400
  if equipID == 40327 then
    if refineLv >= 3 and refineLv < 5 then
      pEquip:setSpecAttr("Atk", 200)
    elseif refineLv >= 5 then
      pEquip:setSpecAttr("Atk", 400)
    end
  end
--测试精炼  精炼等级>=3,每提高一级精炼等级,攻击+10;精炼等级>=6,每提高一级精炼等级,攻击+20
	if equipID == 40328 then
		if refineLv >= 3 and refineLv < 6 then
			local atk = (refineLv-3)*10
			pEquip:setSpecAttr("Atk",atk)
		elseif refineLv >= 6 then
			local atk = 3*10 + (refineLv-6)*20
			pEquip:setSpecAttr("Atk",atk)
		end
	end
end
------------------------------吟唱耐性效果
function CalcBreakSkillDamPer(pUser)
  if pUser == nil then
    return 0.1
  end

  local extra = 0
  local count_1 = pUser:getEquipedItemNum(42004)
  local count_2 = pUser:getEquipedItemNum(142004)
  local count_3 = pUser:getEquipedItemNum(20027)
  local count_4 = pUser:getEquipedItemNum(42507)
  local count_5 = pUser:getEquipedItemNum(142507)
  extra = extra + count_1 * 0.2 + count_2 * 0.2 +count_3*0.2 + count_4 * 0.3 + count_5 * 0.3

  --print(0.1+extra)
  return 0.1 + extra
end

-- calc seal-drop-equip refine lv
function CalcSealEquipRefineLv(quality)
  if quality ~= 1 then
    return 0
  end

  local random = math.random(1,100)
  if random >= 1 and random <= 25 then
    return 1
  elseif random >= 26 and random <= 50 then
    return 2
  elseif random >= 51 and random <= 75 then
    return 3
  elseif random >= 76 and random <= 100 then
    return 4
  end

  return 0
end

-- 计算商人集资百分比
function CalcMerchantRaisePercent(pUser)
   if pUser == nil then
      return 0
   end
   return pUser:getSkillLv(267) * 20
end

-- 计算商人贪婪/流氓偷窃触发概率
function CalcMerchantGreedProb(pUser)
   if pUser == nil then
      return 0
   end
   local prob = pUser:getSkillLv(220) * 20
   if prob ~= 0 then
      return prob
   end
   return pUser:getSkillLv(472) * 20
end

-- 计算商人精炼额外概率, refinelv -> refinelv + 1
function CalcRefineExtraRate(user, refinelv)
  if user == nil then
    return 0
  end

  -- 精炼专家 lv * 0.005
  local result = user:getSkillLv(274) * 0.005

  -- 精炼大师 refinelv >= 10, lv * 0.005
  if refinelv >= 10 then
    result = result + user:getSkillLv(275) * 0.005
  end

  return result
end

function calcGuildQuestNextTime(count)
  local nexttime = 24 * 7 / count
  local rand = (math.random(0, 40) - 20) / 100
  rand = nexttime * rand
  return (nexttime + rand) * 3600;
end

-- 宠物冒险效率计算
function calcPetAdventureValue(blvdelta, flvdelta, param, maxpet, petlv)

  local lveff = PetFun.calcPetAdventureLvEff(blvdelta,maxpet,petlv)
  local flyeff = PetFun.calcPetAdventureFlyEff(flvdelta,maxpet)
  local areaeff = PetFun.calcPetAdventureAreaEff(blvdelta,maxpet,flvdelta,param,petlv)
  return lveff + flyeff + areaeff
end

-- 宠物冒险经验计算
function calcPetAdventureBaseExp(adventureexp, roleexp, adventurelv, petblv, maxpet)
  return PetFun.calcPetAdventureBaseExp(adventureexp, roleexp, adventurelv, petblv, maxpet)
end

-- 经验道具使用之后增加base
function calcAdventureGuideBaseExp(BaseLv)
  return CommonFun.calcAdventureGuideBaseExp(BaseLv)
end

-- 经验道具使用之后增加base
function calcAdventureGuideJobExp(BaseLv)
  return CommonFun.calcAdventureGuideJobExp(BaseLv)
end

-- 公会建筑提交材料扣除道具数量
function calcGuildBuildingMaterialItemCount(itemcount, submitcount)
   return CommonFun.calcGuildBuildingMaterialItemCount(itemcount, submitcount)
end

-- 神器打造扣除道具数量
function calcArtifactMaterialItemCount(type, num, count)
   return CommonFun.calcArtifactMaterialItemCount(type, num, count)
end

-- 宠物打工
function calcPetWorkMaxReward(start_time,end_time,max_reward)
  return PetFun.calcPetWorkMaxReward(start_time,end_time,max_reward)
end
function calcPetWorkRewardCount(starttime, endtime, frequency, maxreward, lastcount)
  return PetFun.calcPetWorkRewardCount(starttime, endtime, frequency, maxreward, lastcount)
end
function calcPetWorkFrequency(petid, petbaselv, petfriendlv, spaceid, skillid)
  return PetFun.calcPetWorkFrequency(petid, petbaselv, petfriendlv, spaceid, skillid)
end

function calcDuringTime(startTime, curTime, frequency, rewardcount, daymaxreward, lastcount)
  return PetFun.calcDuringTime(startTime, curTime, frequency, rewardcount, daymaxreward, lastcount)
end

-- 区线名称
CONFIG_ZONE_ID_SUF = {
  "", "a", "c", "d", "e", "f", "g", "h", "k", "m", "n", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "aa", "cc", "dd", "ee", "ff", "gg", "hh", "kk"
}
function ZoneNumToString(num)
  if UnionConfig and UnionConfig.Zone and UnionConfig.Zone.zone_name then
    for i, v in ipairs(UnionConfig.Zone.zone_name) do
       if v.min and v.max and v.name and num >= v.min and num <= v.max then
         return v.name .. (num - v.min + 1)
       end
    end
  end
  if num and num > 0 then
    if num >= 9000 then
      return ""--ZhString.ChangeZoneProxy_PvpLine;
    end
    local length = #CONFIG_ZONE_ID_SUF
    local n1 = math.ceil(num / length)
    local n2 = num % length
    if n2 == 0 then
      n2 = n2 + length
    end
    n2 = CONFIG_ZONE_ID_SUF[n2]

    return n1..n2
  end

  return ""
end

-- 一键出售物品检查
function canQuickSell(id)
  return ItemFun.canQuickSell(id)
end

