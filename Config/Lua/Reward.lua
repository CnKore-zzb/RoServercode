-- 获取怪物概率加成
function getNpcExtraRatio(npczone)
  local curtime = os.time()
  local ratio = 1

  -- 无限塔怪物
  if npczone == ENPCZONE_ENDLESSTOWER then
    local starttab = {year=2015, month=7, day=1, hour=0, min=0}
    local endtab = {year=2015, month=9, day=1, hour=0, min=0}

    local starttime = os.time(starttab)
    local endtime = os.time(endtab)
    if curtime > starttime and curtime < endtime then
      ratio = 4
    end
  end

  return ratio
end

-- 获取地图概率加成
function getMapExtraRatio(npc)
  local curtime = os.time()
  local ratio = 1
  local mapid = npc:getMapID()
  
  -- 幽灵暴走活动
  if mapid == 8 or mapid == 9 then
    local starttab = {year=2016, month=10, day=30, hour=0, min=0}
    local endtab = {year=2016, month=11, day=1, hour=23, min=59}

    local starttime = os.time(starttab)
    local endtime = os.time(endtab)
    if curtime > starttime and curtime < endtime then
      if npc:getNpcID()== 10019 or npc:getNpcID()== 10020 or npc:getNpcID()== 10022 or npc:getNpcID()== 20004 or npc:getNpcID()==30004 then
        ratio = 5
      end
    end
  end

  return ratio
end

-- 无限塔概率计算
function calcTowerRewardRatio(npctype, zonetype)

  -- 获取npc加成
  local ratio = getNpcExtraRatio(zonetype)

  -- miniboss和mvp 掉落系数
  if npctype == ENPCTYPE_MINIBOSS then
    return ratio * 0.4
  end
  
  if npctype == ENPCTYPE_MVP then
    return ratio * 0.2
  end
  
  return ratio
end

-- 野外怪概率计算
-- ismvp:是否是mvp专属掉率
function calcMapRewardRatio(npc, user, isquest, ismvp)   --任务表中的reward不受影响
  if isquest == true then
    return 1
  end
  
  if npc == nil or user == nil then
    return 1
  end
  
  if npc:getNpcZoneType() == ENPCZONE_TASK then   --任务怪的掉落reward不受影响
    return 1
  end
  -- 草类怪掉落不受战斗时长和等级差影响
  if npc:getNpcID() >= 40001 and npc:getNpcID() <= 49999 then
    return 1
  end
  -- --尖叫曼陀罗掉落不受战斗时长和等级差影响
  -- if npc:getNpcID() >= 17000 and npc:getNpcID() <= 17002 then
  --   return 1
  -- end
  --B格猫掉落不受战斗时长和等级差影响
  if npc:getNpcID() >= 10107 and npc:getNpcID() <= 10116 then
    return 1
  end
  
  local ratio = getMapExtraRatio(npc)

  if npc:getNpcType() == ENPCTYPE_MINIBOSS or npc:getNpcType() == ENPCTYPE_MVP then
    return ratio
  end

  if ismvp then 
    return ratio
  end

  local newratio = ratio * user:getTeamAddictRatio()

  local deltalv = npc:getLevel() - user:getLevel()
  local star = npc:isStarMonster()

  if user.getKillCount ~= nil and npc.getPeriodCnt ~= nil then
    local killcnt = user:getKillCount(npc:getNpcID())
    local normalcnt = npc:getPeriodCnt()
    if killcnt ~= 0 and normalcnt ~= 0 and killcnt > normalcnt then
      newratio = normalcnt / killcnt * newratio
    end

    if killcnt ~= 0 and normalcnt == 0 and npc:getNpcZoneType() == ENPCZONE_FIELD then
      local begintime = 1527782400 -- 6月1日开始新的衰减
      local curtime = os.time()
      local newnormalcnt = 44 -- 3月28日停止时的个数
      if curtime > begintime then
        newnormalcnt = 44 - (curtime - begintime) / 86400 * 11 / 28 -- 4周从44衰减到33, 每天衰减11/28
        if newnormalcnt < 33 then
          newnormalcnt = 33
        end
      end

      if killcnt > newnormalcnt then
        newratio = newnormalcnt / killcnt * newratio
      end
    end
  end

  return newratio * CommonFun.getItemReduceValue(deltalv, star)
end

-- 封印怪概率计算
function calcSealRewardRatio(membercount)
  return membercount / 6
end

-- 计算沉迷经衰减率
function calcAddictRatio(user, usedtime, exclude_skill)
  if user == nil then
    return 0
  end
  local ratio = 1
  local totaltime = user:getTotalBattleTime()

  if usedtime <= totaltime then
    local Advance = 1
    local TeamPros = 0
      
    if exclude_skill ~= true and user:isSkillEnable(50045001) then  
        local TeamNum = user:getMapTeamPros()
        if TeamNum >=5 then
          TeamPros = 0.20
        elseif TeamNum ==4 then
          TeamPros = 0.15
        elseif TeamNum ==3 then
          TeamPros = 0.10
        end
    end
  
     Advance = user:getNormalDrop(Advance + TeamPros)
     return ratio * Advance
  end

  local stage = CommonFun.getAddictStage()
  if stage == 0 then
    return ratio
  end

  ratio = 1 - (usedtime - totaltime) / stage * 0.95
  if ratio < 0.05 then
    return 0.05
  end

  return ratio
end

-- 计算沉迷经验衰减
function calcAddictDropExp(user,sec, exp)
  -- 经验不受合作捕猎影响
  local newexp = calcAddictRatio(user, sec, true) * exp
  return newexp
end

-- 计算溶解数量
function calcDecomposeResult(param)
  if param >= 500 and param <= 750 then
    return 2
  elseif param >= 800 and param <= 1200 then
    return 3
  elseif param >= 1250 and param <= 1450 then
    return 4
  elseif param >= 1500 then
    return 5
  end

  return 1
end
function calcDecomposeFloatParam(showtype)
  if showtype == 1 then
    return 0.5
  end
  if showtype == 2 then
    return 1.5
  end

  math.randomseed(os.time())
  local r = math.random(0, 1660)
  if r <= 20 then
    return 0.5
  elseif r <= 60 then
    return 0.55
  elseif r <= 120 then
    return 0.6
  elseif r <= 200 then
    return 0.65
  elseif r <= 290 then
    return 0.7
  elseif r <= 380 then
    return 0.75
  elseif r <= 480 then
    return 0.8
  elseif r <= 580 then
    return 0.85
  elseif r <= 680 then
    return 0.9
  elseif r <= 780 then
    return 0.95
  elseif r <= 880 then
    return 1
  elseif r <= 980 then
    return 1.05
  elseif r <= 1080 then
    return 1.1
  elseif r <= 1180 then
    return 1.15
  elseif r <= 1280 then
    return 1.2
  elseif r <= 1370 then
    return 1.25
  elseif r <= 1460 then
    return 1.3
  elseif r <= 1540 then
    return 1.35
  elseif r <= 1600 then
    return 1.4
  elseif r <= 1640 then
    return 1.45
  elseif r <= 1660 then
    return 1.5
  end
  return 0
end

function getRefineParam(refinelv)
  local refineparam = {
    [1] = 1,
    [2] = 1,
    [3] = 1,
    [4] = 1,
    [5] = 1.2,
    [6] = 1.4,
    [7] = 1.6,
    [8] = 2.2,
    [9] = 3.4,
    [10] = 5,
    [11] = 7.4,
    [12] = 12.4,
    [13] = 19.8,
    [14] = 35.2,
    [15] = 57.4
  }
  if refineparam[refinelv] == nil then
    return 1
  end

  return refineparam[refinelv]
end
function calcDecomposeCount(user, decomposenum, decomposeorinum, mtotalprice, metalrate, metalprice, refinelv, floatparam)
  if user == nil then
    return 0
  end

  local refineparam = getRefineParam(refinelv)
  if metalprice == 0 or floatparam == 0 then
    --print("metalprice="..metalprice.." floatparam="..floatparam)
    return 0
  end

  --print("decomposenum="..decomposenum) 装备金属价值
  --print("decomposeorinum="..decomposeorinum) 原始数量
  --print("mtotalprice="..mtotalprice) 材料总价
  --print("metalrate="..metalrate)金属概率
  --print("refinelv="..refinelv) 精炼等级
  --print("metalprice="..metalprice) 金属低价
  --print("floatparam="..floatparam) 浮动系数
  --print("refineparam="..refineparam) 精炼系数
  --print("------------------------")
  local basecnt = decomposenum * metalrate / metalprice * floatparam * refineparam
  local userentry = Entry.new(user) -- ServerLua : Entry
  local metalparam = CommonFun.calcOrideconResearch(userentry)

  return basecnt * (1 + metalparam)
end

