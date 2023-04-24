-- 物品使用检测
-- return x ： x=0,可以使用; x=-1, 不可以使用,不需要提示; x=其他, 不可以使用, 提示msg id=x
function itemUserCheck(sUser, itemid, tUser)
  -- 苍蝇翅膀
  if sUser == nil then
    return 0
  end

  if Table_Item[itemid] == nil or Table_Item[itemid].Type == nil then
    return -1
  end

  -- 组队蝴蝶翅膀需要技能50036001
  if itemid == 50002 then
    if sUser:getSkillLv(50036) < 1 then
      return 355
    end
  end

  -- 天地树叶子只能对死亡玩家使用
  if itemid == 5023 then
    if sUser:getSkillStatus() == 2 then
      return -1
    end
    if tUser == nil or tUser:isAlive() or tUser:isMyEnemy(sUser) then
      return -1
    end
    if tUser:isReliveByOther() then
      return 2509
    end
  end

  -- 枯树枝,血迹树枝
  if itemid == 12020 or itemid == 12021 then
    -- 不能再副本中使用
    local maptype = sUser:getRaidType()
    if maptype ~= 10 then
      return 863
    end
  end

  -- 苍蝇翅膀
  if itemid == 5024 or itemid == 50003 then
    -- 不能再副本中使用
    local maptype = sUser:getRaidType()
    if maptype == 2 or maptype == 3 or maptype == 4 or maptype == 5 or maptype == 28 then
      return 112
    end
    -- 部分地图无法使用
    local mapid = sUser:getMapID()
    local forbidmap = { 10001, 3001, 3003, 1004, 1005, 1006, 1002, 3002, 50001, 50002, 50003, 50004, 50005, 50006, 50007, 50008, 50009, 50010, 50011, 50012, 50013, 50014, 50015,
    50016, 50017, 50018, 50019, 50020, 50021, 50022, 50023, 50024 }
    for i = 1, #forbidmap, 1 do
      if mapid == forbidmap[i] then
        return 112
      end
    end
    -- 技能吟唱无法使用
    if sUser:getSkillStatus() == 2 then
      return 3003
    end
    -- 组队苍蝇翅膀需要技能50037001
    if itemid == 50003 and sUser:getSkillLv(50037) < 1 then
      return 358
    end
    -- 尼夫海姆原野需要激活全部传送阵才可使用
    if mapid == 71 or mapid == 72 then
      local self = sUser:canUseWingOfFly()
      if not self then
        if mapid == 71 then
          return 25802
        else
          return 25799
        end
      end
      -- 若在队伍中使用巨大苍蝇翅膀,需要判断同地图队友是否激活了传送阵
      local team = sUser:canTeamUseWingOfFly()
      if itemid == 50003 and not team then
        return 25798
      end
    end
    return 0
  end

  -- check arrow can use
  if Table_Item[itemid].Type == 43 then  
    local weapon = sUser:getWeaponType()
    if weapon ~= 210 then
      return 3051
    end
    --[[
    local skill_lv = sUser:getSkillLv(127)

    if itemid == 12501 and skill_lv < 2 then
      return 3050
    end
    if itemid == 12502 and skill_lv < 5 then
      return 3050
    end
    return 0
    --]]
  end

  -- in spec status
  local status = CommonFun.getBits(sUser:getAttr(EATTRTYPE_STATEEFFECT))
  local eType = Table_Item[itemid].Type
  local getStatus = {}

  if GameConfig.ItemsNoUseWhenRoleStates ~= nil then
    for key, val in pairs(GameConfig.ItemsNoUseWhenRoleStates) do
      for key2, val2 in pairs(val) do
        if val2 == eType then
          table.insert(getStatus, key)
        end
      end
    end
  end

  for key, val in pairs(getStatus) do
    if status[val] ~= nil and status[val] == 1 then
      return -1
    end
  end

  -- 掠夺许可证, buff 层数达到上限时不可使用
  if itemid == 5032 then
    if sUser:isBuffLayerEnough(EBUFFTYPE_ROBREWARD) then
      return 2510
    end
  end

  -- 手推车2 需要改装手推车技能263005
  if GameConfig.EquipedLimitBySkill ~= nil then
    local skilldata = GameConfig.EquipedLimitBySkill[itemid]
    if skilldata ~= nil then
      if skilldata[1] ~= nil and skilldata[2] ~= nil then
        local skilllv = sUser:getSkillLv(skilldata[1])
        if skilllv < skilldata[2] then
          return 5020
        end
      end
    end
  end
  --if itemid == 25301 or itemid == 25302 then
    --if sUser:getSkillLv(263) < 5 then
      --return 5020
    --end
  --end

  -- 锁链雷锭和锁链德洛米同时只能使用一个
  if itemid == 5039 then        -- 锁链雷锭
    if sUser:isBuffLayerEnough(EBUFFTYPE_ROBREWARD) then
      return 521
    end
  end

  if itemid == 5042 then        -- 锁链德洛米
    if sUser:isBuffLayerEnough(EBUFFTYPE_MULTITIME) then
      return 520
    end
  end

  -- 浓缩Job药水,三转及以上无法使用
  if itemid == 12387 and sUser:getEvo() >= 4 then
    return 25820
  end
  
  return 0
end

-- 药水恢复公式
function CalcItemHealValue(baseValue, sUser, formula)
  if sUser == nil or formula == nil then
    return 0
  end

  local hpRestoreSpd = sUser:getAttr(EATTRTYPE_ITEMRESTORESPD)
  local spRestoreSpd = sUser:getAttr(EATTRTYPE_ITEMSPRESTORESPD)
  local vit = sUser:getAttr(EATTRTYPE_VIT)
  local int = sUser:getAttr(EATTRTYPE_INT)

  if formula == 1 then
    return baseValue * (1 + hpRestoreSpd) * (1 + vit * 0.01)
  elseif formula == 2 then
    return baseValue * (1 + hpRestoreSpd) * (1 + vit * 0.015)
  elseif formula == 3 then
    return baseValue * (1 + hpRestoreSpd) * (1 + vit * 0.02)
  elseif formula == 4 then
    return baseValue * (1 + hpRestoreSpd) * (1 + vit * 0.025)
  elseif formula == 5 then
    return baseValue * (1 + spRestoreSpd) * (1 + int * 0.015)  
  end

  return baseValue
end

-- 物品使用后
function doAfterUsingItem(user, itemid)
  if user == nil then
    return
  end

  -- 部分道具使用后打断跟随
  local userdata = user:getUserSceneData()
  if userdata ~= nil then
    local item = {50001, 5024}
    for key, val in pairs(item) do
      if val == itemid then
        userdata:setFollowerID(0)
        break
      end
    end
  end
end

