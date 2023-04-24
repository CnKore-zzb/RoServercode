math.randomseed( tonumber(tostring(os.time()):reverse():sub(1,6)) )

function calcattr(curPoint, joblv, job, attr)
  return CommonFun.calAttrPoint(curPoint, joblv, job, attr)
end

-- skill param begin
SkillEntry = class("SkillEntry")
function SkillEntry:ctor(skillid, indexCount, pvp)
  self.skillIDAndLevel = skillid
  self.hitedCount = math.floor(indexCount/1000)
  self.hitedIndex = indexCount - self.hitedCount * 1000
  self.pvpMap = pvp
end
-- skill param end

-- logger
Logger = class("Logger")
function Logger.error(str)
  cPlusLogError(tostring(str))
end
-- logger

-- entry class begin
Entry = class("Entry")
function Entry:ctor(cPlus_pEntry)
  if cPlus_pEntry == nil then
    return
  end
  self.isServerCall = true
  self.pEntry = cPlus_pEntry
  self.name = cPlus_pEntry:getName()
  self.BaseLv = cPlus_pEntry:getLevel()
  self.race = cPlus_pEntry:getRaceType()
  --self.boss = cPlus_pEntry:isBoss()
  self.randomByMath = true
  self.zoneType = 0
  self.shape = "M"
  local dsize = cPlus_pEntry:getBodySize()
  if dsize == 1 then
    self.shape = "S"
  elseif dsize == 2 then
    self.shape = "M"
  elseif dsize == 3 then
    self.shape = "L"
  end

  self.boss = false
  self.mini = false
  self.changelinepunish = false
  local npc = cPlus_pEntry:getNpcObject()
  if npc ~= nil then
    if npc:getNpcType() == 4 or npc:isFakeMini() then
      self.mini = true
    elseif npc:getNpcType() == 5 then
      self.boss = true
    elseif npc:getNpcType() == 12 then
      self.being = true
    end
    if npc:getChangeLinePunish() == true then
      self.changelinepunish = true
    end
  end

  if npc ~= nil then
    self.zoneType = npc:getNpcZoneType()
  end

  self.arrow_server = 0
  self.have_sharedam_server = false
  self.skillParam = nil
end

function Entry:GetProperty(attrname)
  if self.pEntry == nil then
    return 0
  end
  if self.pEntry:isValidAttrName(attrname) == false then
    local info = string.format("CommonFun 获得属性, 属性名称错误: %s", tostring(attrname))
    cPlusLogError(info)
    return 0
  end
  return self.pEntry:getStrAttr(attrname)
end

function Entry:IsEnemy(targetEntry)
  if targetEntry.pEntry == nil or self.pEntry == nil then
    return 0
  end
  return self.pEntry:isMyEnemy(targetEntry.pEntry)
end

function Entry:GetLernedSkillLevel(skillID)
  if self.pEntry == nil then
    return 0
  else
    return self.pEntry:getSkillLv(skillID)
  end
  return 1
end

function Entry:GetProfressionID()
  if self.pEntry == nil then
    return 0
  end
  return self.pEntry:getProfession()
end

function Entry:GetAttackSkillIDAndLevel()
  if self.pEntry == nil then
    return 0
  end
  return self.pEntry:getNormalSkill()
end

function Entry:DamageAlways1()
  if self.pEntry == nil then
    return false
  end
  return self.pEntry:bDamageAlways1()
end

function Entry:GetRandom()
  if self.randomByMath then
    return (math.random(0,99))
  end
  local value = self.pEntry:getRandomNum()
  return value
end

function Entry:IsImmuneSkill(skillid)
  if self.pEntry == nil then
    return false
  end
  return self.pEntry:isImmuneSkill(skillid)
end

function Entry:SelfBuffChangeSkill()
  return nil
end

function Entry:DefiniteHitAndCritical()
  return false
end

function Entry:IgnoreJinzhanDamage()
  return false
end

function Entry:IsFly()
  -- 客户端接口, 不处理
  return false
end

function Entry:GetArrowID()
  if self.pEntry == nil then
    return 0
  end
  return self.pEntry:getArrowID()
end

function Entry:GetCurSkillAtkAttr()
  if self.pEntry == nil then
    return 0
  end
  local attr = self.pEntry:getTempAtkAttr()
  if attr == 0 then
    attr = 5
  end
  return attr
end

function Entry:GetEquipedWeaponType()
  if self.pEntry == nil then
    return 0
  end
  return self.pEntry:getWeaponType()
end

function Entry:AddBuffDamage(damage)
  if self.pEntry == nil or self.pEntry.addBuffDamage == nil then
    return
  end
  self.pEntry:addBuffDamage(damage)
end

function Entry:GetEquipedItemNum(itemid)
  if self.pEntry == nil or self.pEntry:getUserObject() == nil then
    return 0
  end
  return self.pEntry:getUserObject():getEquipedItemNum(itemid)
end

function Entry:GetJobLv()
  if self.pEntry == nil or self.pEntry:getUserObject() == nil then
    return 0
  end
  return self.pEntry:getUserObject():getJobLv()
end

function Entry:GetPackageItemNum(itemid)
  if self.pEntry == nil or self.pEntry:getUserObject() == nil then
    return 0
  end
  return self.pEntry:getUserObject():getMainPackageItemNum(itemid)
end

function Entry:GetNpcID()
  if self.pEntry == nil or self.pEntry:getNpcObject() == nil then
    return 0
  end
  return self.pEntry:getNpcObject():getNpcID()
end

function Entry:GetGuid()
  if self.pEntry == nil then
    return 0
  end
  return self.pEntry:getGuid()
end

function Entry:GetEquipedID(site)
  if self.pEntry == nil or self.pEntry:getUserObject() == nil then
    return 0
  end

  return self.pEntry:getUserObject():getEquipID(site)
end

function Entry:SetTempAtkAttr(atkattr)
  if self.pEntry == nil or self.pEntry.setTempAtkAttr == nil then
    return
  end

  self.pEntry:setTempAtkAttr(atkattr)
end

function Entry:GetEquipedRefineLv(pos)
  if self.pEntry == nil or self.pEntry:getUserObject() == nil then
    return 0
  end

  return self.pEntry:getUserObject():getEquipRefineLv(pos)
end

function Entry:HasBuffID(buffid)
  if self.pEntry == nil or self.pEntry:getBuff() == nil then
    return 0
  end

  return self.pEntry:getBuff():haveBuff(buffid)
end

function Entry:GetEquipCardNum(pos, cardid)
   if self.pEntry == nil or self.pEntry:getUserObject() == nil then
      return 0
   end

   return self.pEntry:getUserObject():getEquipCardNum(pos, cardid)
end

function Entry:GetBuffListByType(strname)
  local bufflist = {}
  if self.pEntry == nil then
    return bufflist
  end

  local buff = self.pEntry:getBuff()
  if buff == nil then
    return bufflist
  end

  local cplusLuaArray = getOneLuaArray()
  buff:getBuffListByName(strname, cplusLuaArray)

  if cplusLuaArray then
    local size = cplusLuaArray:getDWArraySize()
    for i = 1, size do
      local id = cplusLuaArray:getDWValueByIndex(i)
      table.insert(bufflist, id)
    end
  end

  return bufflist
end

function Entry:GetBuffFromID(buffid)
  if self.pEntry == nil then
    return 0
  end
  local buff = self.pEntry:getBuff()
  if buff == nil then
    return 0
  end

  return buff:getBuffFromID(buffid)
end

function Entry:GetDistance(targetid, dis)
  if self.pEntry == nil then
    return 99999
  end
  return self.pEntry:getDisWithEntry(targetid)
end

function Entry:GetCartNums()
  if self.pEntry == nil then
    return 0,0
  end

  local user = self.pEntry:getUserObject()
  if user == nil then
    return 0, 0
  end
  
  local pack = user:getPackage()
  if pack == nil then
    return 0, 0
  end

  local allslot = pack:getPackSlot(9) -- 9, 手推车类型枚举
  local usedslot = pack:getPackSlotUsed(9)
  return usedslot, allslot
end

function Entry:GetRunePoint(specid)
  if self.pEntry == nil then
    return 0
  end

  -- 生命体取主人星盘点
  if self.being == true then
    local npc = self.pEntry:getNpcObject()
    if npc == nil then
      return 0
    end
    local master = npc:getMasterUser()
    if master == nil then
      return 0
    end
    return master:getRuneSpecNum(specid)
  end

  local userentry = self.pEntry:getUserObject()
  if userentry == nil then
    return 0
  end

  return userentry:getRuneSpecNum(specid)
end

function Entry:DoExtraDamage(damage)
  if self.pEntry == nil then
    return
  end

  self.pEntry:doExtraDamage(damage)
end

-- 获取所在地图信息, type,mapid  type:1 野外, 2 pvp, 3 副本, 4 gvg
function Entry:GetMapInfo()
  if self.pEntry == nil then
    return 0, 0
  end

  local maptype = 1

  if self.pEntry:isInRaid() then
    maptype = 3
    if self.pEntry:isOnPvp() then
      maptype = 2
    elseif self.pEntry:isOnGvg() then
      maptype = 4
    end
  end

  return self.pEntry:getMapID(), maptype
end

function Entry:GetBuffEndTime(buffid)
  if self.pEntry == nil then
    return 0
  end
  local buff = self.pEntry:getBuff()
  if buff == nil then
    return 0
  end
  return buff:getEndTimeByID(buffid)
end

function Entry:GetBuffLayer(buffid)
  -- 如果技能有记录, 优先取技能记录的(可能消耗后, 计算伤害前, buff已删除), -1表示无记录
  if self.skillParam ~= nil then
    local skillbufflayer = self.skillParam:getBuffLayer(buffid)
    if skillbufflayer ~= -1 then
      return skillbufflayer
    end
  end

  if self.pEntry == nil then
    return 0
  end

  local buff = self.pEntry:getBuff()
  if buff == nil then
    return 0
  end

  return buff:getLayerByID(buffid)
end

function Entry:DelSkillBuff(buffid, layer)
  if self.pEntry == nil then
    return
  end

  self.pEntry:delSkillBuff(buffid, layer)
end

function Entry:GetBuffLevel(buffid)
  if self.pEntry == nil then
    return 0
  end
  local buff = self.pEntry:getBuff()
  if buff == nil then
    return 0
  end
  return buff:getLevelByID(buffid)
end

function Entry:GetAppleNum()
  if self.pEntry == nil or self.pEntry:getUserObject() == nil then
    return 0
  end
  return self.pEntry:getUserObject():getAppleNum()
end

function Entry:IsBeingPresent(beingid)
  if self.pEntry == nil or self.pEntry:getUserObject() == nil then
    return false
  end
  return self.pEntry:getUserObject():isBeingPresent(beingid)
end

function Entry:IsEquipForceOff(pos)
  if self.pEntry == nil or self.pEntry:getUserObject() == nil then
    return false
  end
  return self.pEntry:getUserObject():isEquipForceOff(pos)
end

function Entry:GetMasterUser()
  local npc = self.pEntry:getNpcObject()
  if npc ~= nil then
    local master = npc:getMasterUser()
    if master ~= nil then
      local entry = master:getBaseObject()
      if entry ~= nil then
        return Entry.new(entry)
      end
    end
  end
  return nil
end

function Entry:HasSubmitQuest(questid)
  local user = self.pEntry:getUserObject()
  if user == nil then
    return false
  end
  local quest = user:getQuest()
  if quest == nil then
    return false
  end

  return quest:isSubmit(questid)
end

function Entry:InGuildZone()
  if self.pEntry == nil then
    return false
  end

  return self.pEntry:inGuildZone()
end

-- @brief 获取周围敌人的数量 
-- @param distance 距离
function Entry:GetRangeEnemy(distance)
    if self.pEntry == nil then
        return 0
    end
    return self.pEntry:getRangeEnemy(distance)
end

function Entry:InSuperGvg()
  if self.pEntry == nil then
    return false
  end

  return self.pEntry:inSuperGvg()
end


function Entry:SetMissStillBuff()
  if self.pEntry == nil then
    return
  end

  self.pEntry:setMissStillBuff()
end

function Entry:GetBeingGUID()
   if self.pEntry == nil or self.pEntry:getUserObject() == nil then
      return 0
   end
   return self.pEntry:getUserObject():getBeingGUID()
end

-- @brief 判断有没有骑乘某个坐骑
-- @param id 坐骑的id
-- @return 有骑乘返回true 
function Entry:IsRide(id)
  if self.pEntry == nil  or self.pEntry:getUserObject() == nil then 
    return false
  end 
  return self.pEntry:getUserObject():isRide(id) 
end

--@brief 判断是否为指定的PartnerID
--@param id Partner id
function Entry:IsPartner(id)
  if self.pEntry == nil  or self.pEntry:getUserObject() == nil then 
    return false  
  end 
  return self.pEntry:getUserObject():isPartnerID(id) 
end

function Entry:AddBuff(buffid, targetid)
   if self.pEntry == nil then
     return false
   end

   return self.pEntry:addBuff(buffid, targetid)
end

function Entry:getCurElementElfID()
   if self.pEntry == nil or self.pEntry:getUserObject() == nil then
     return false
   end

   return self.pEntry:getUserObject():getCurElementElfID()
end

-- 取合奏同伴
function Entry:GetEnsemblePartner()
  if self.pEntry == nil or self.pEntry:getUserObject() == nil then
    return nil
  end
  local entry = self.pEntry:getUserObject():getEnsemblePartner()
  if entry ~= nil then
    return Entry.new(entry)
  end
  return nil
end

-- entry class end

-- 伤害计算
--function exeDamage(sUser, tUser, skillid, targetsNum, arrow)
function exeDamage(sUser, tUser, pSkillParam)
  if (sUser == nil or tUser == nil or pSkillParam == nil) then
    print("return for suser tuser")
    return 0
  end

  local skillid = pSkillParam:getSkillID()
  local targetsNum = pSkillParam:getTargetsNum()
  if targetsNum <= 1000 then
    return 0
  end

  entry1 = Entry.new(sUser)
  entry1.arrow_server = pSkillParam:getArrowID()
  entry1.skillParam = pSkillParam

  entry2 = Entry.new(tUser)
  entry2.have_sharedam_server = pSkillParam:haveShareDam()

  oSkill = SkillEntry.new(skillid, targetsNum, sUser:isOnPvp())
  oLogger = Logger.new()

  -- client 不参与计算的技能,随机数直接用math.random()随机
  entry1.randomByMath = false
  if Table_Skill[skillid] ~= nil then
    if Table_Skill[skillid].Logic_Param ~= nil and Table_Skill[skillid].Logic_Param.no_select == 1 then
      entry1.randomByMath = true
    else
      -- SkillNpc, 客户端不参与计算, 不使用公用随机数
      local npc = tUser:getNpcObject()
      if npc and npc:getNpcType() == ENPCTYPE_SKILLNPC then
        entry1.randomByMath = true
      end
    end
  end

  if (tUser:isImmuneSkill(skillid) == true) then
    return 0
  end

  local damage, damagetype, shareDam = CommonFun.CalcDamage(entry1, entry2, oSkill, oLogger)
  if damagetype ~= nil then
    tUser:setTempDamageType(damagetype)
  end

  if shareDam ~= nil then
    for i = 1, #shareDam do
      if shareDam[i].damage ~= nil and shareDam[i].type ~= nil then
        pSkillParam:addShareDam(shareDam[i].charid, shareDam[i].damage, shareDam[i].type)
      end
    end
  end

  --print(damagetype, damage)
  return damage
end

-- 公式type -> get value
function calcFormulaValue(sUser, tUser, formulaType)
  if (sUser == nil) then
    return 0
  end

  entry1 = Entry.new(sUser)
  entry2 = nil
  if tUser ~= 0 then
    entry2 = Entry.new(tUser)
  end

  local value = CommonFun.GetFormulaValue(entry1, entry2, formulaType)
  return value
end

-- 吸血计算
function getSuckBlood(sUser, damage, skillid)
  if (sUser == nil) then
    return 0
  end

  entry1 = Entry.new(sUser)
  local value = CommonFun.CalcSuckBlood(entry1, damage, skillid)
  return value
end

function calcBeatBackRate(sUser, tUser, skillid)
  if tUser == nil then
    return 0
  end

  entry1 = Entry.new(tUser)
  return CommonFun.CalcBeatBackRate(entry1, skillid)
end

-- buff 公式计算
function calcBuffValue(sUser, tUser, pParams, dtype)
  if sUser == nil or tUser == nil or pParams == nil then
    return 0
  end
  entry1 = Entry.new(sUser)
  entry2 = Entry.new(tUser)
  local value = CommonFun.calcBuffValue(entry1, entry2, dtype, pParams:getParams(1), pParams:getParams(2), pParams:getParams(3), pParams:getParams(4), pParams:getParams(5), pParams:getParams(6))
  if value ~= nil then
    return value
  end
  return 0
end

function calcNpcAttrValue(npc)
  if npc == nil then
    return
  end

  local attr = {}
  local baseAttr = {}
  local attrstart = 100 --EATTRTYPE_STR
  local attrend = 105 --EATTRTYPE_LUK
  for index = attrstart, attrend do
    attr[index] = npc:getOtherAttr(index)
    baseAttr[index] = npc:getBaseAttr(index)
  end
  attrstart = 200 --EATTRTYPE_ATK
  attrend = 229 --EATTRTYPE_CRIPER
  for index = attrstart, attrend do
    attr[index] = npc:getOtherAttr(index)
  end
  attrstart = 300 --EATTRTYPE_REFINE
  attrend = 301 --EATTRTYPE_MREFINE
  for index = attrstart, attrend do
    attr[index] = npc:getOtherAttr(index)
  end

  attrstart = 305 -- EATTRTYPE_HITPER
  attrend = 312 -- EATTRTYPE_LUKPER
  for index = attrstart, attrend do
    attr[index] = npc:getOtherAttr(index)
    baseAttr[index] = npc:getBaseAttr(index)
  end

  attrstart = 400 --EATTRTYPE_SHOWATK
  --attrend = 413 --EATTRTYPE_SHOWRESTORESPD
  attrend = 415 -- EATTRTYPE_MDAMREDUC
  for index = attrstart, attrend do
    attr[index] = npc:getOtherAttr(index)
  end

  local baseResult = {}
  baseResult = CommonFun.calcNpcAttrValue(baseAttr, attr, profession)

  local result = {}

  if npc:getNpcType() == ENPCTYPE_WEAPONPET then
    local pMaster = npc:getMasterUser()
    local entryMaster = nil
    if pMaster ~= nil then
      local pEntry = pMaster:getBaseObject()
      if pEntry ~= nil then
        entryMaster = Entry.new(pEntry)
      end
    end
    result = CommonFun.calcWeaponPetNpcAttrValue(attr, npc:getProfession(), npc:getLevel(), entryMaster)
  elseif npc:getNpcType() == ENPCTYPE_PETNPC then
    result = CommonFun.calcPetNpcAttrValue(attr, npc:getLevel(), npc:getProfession())
  elseif npc:getNpcType() == ENPCTYPE_BEING then
    local pMaster = npc:getMasterUser()
    local entryMaster = nil
    if pMaster ~= nil then
      local pEntry = pMaster:getBaseObject()
      if pEntry ~= nil then
        entryMaster = Entry.new(pEntry)
      end
    end
    result = CommonFun.calcBeingNpcAttrValue(attr, npc:getNpcID(), npc:getLevel(), entryMaster)
  else
    if npc:getNpcZoneType() == ENPCZONE_ENDLESSTOWER then
      result = CommonFun.calcEndlessTowerNpcAttrValue(attr, npc:getNpcType(), npc:getProfession(), npc:getEndlessLayer())
    elseif npc:getNpcZoneType() == ENPCZONE_LABORATORY then
      result = CommonFun.calcLaboratoryNpcAttrValue(attr, npc:getLevel(), npc:getProfession(), npc:getRoundID())
    elseif npc:getNpcZoneType() == ENPCZONE_SEAL then
      result = CommonFun.calcSealNpcAttrValue(attr, npc:getProfession(), npc:getSealType(), npc:getMapID())
    elseif npc:getNpcZoneType() == ENPCZONE_DOJO then
      result = CommonFun.calcDojoNpcAttrValue(attr, npc:getNpcType(), npc:getProfession(), npc:getDojoLevel())
    elseif npc:getNpcZoneType() == ENPCZONE_DEAD then
      result = CommonFun.calcDeadNpcAttrValue(attr, npc:getNpcType(), npc:getProfession(), npc:getDeadLv())
    elseif npc:getNpcZoneType() == ENPCZONE_WORLD then
      result = CommonFun.calcWorldNpcAttrValue(attr, npc:getNpcType(), npc:getProfession(), npc:getDeadLv())
    end
  end

  -- 若有重复, 优先取副本属性
  for key, value in pairs(result) do
    baseResult[key] = value
  end

  for key, value in pairs(baseResult) do
    npc:setAttr(key, value)
  end
end

function getRaceParam(sUser, tUser)
  if (sUser == nil or tUser == nil) then
    return 0
  end

  local RaceToDamPer = {
    EATTRTYPE_BRUTEDAMPER, EATTRTYPE_DEMIHUMANDAMPER, EATTRTYPE_DEMONDAMPER, EATTRTYPE_PLANTDAMPER,
    EATTRTYPE_DEADLESSDAMPER, EATTRTYPE_FORMLESSDAMPER, EATTRTYPE_FISHDAMPER, EATTRTYPE_ANGLEDAMPER,
    EATTRTYPE_INSECTDAMPER, EATTRTYPE_DRAGONDAMPER }

  local RaceToResPer = {
    EATTRTYPE_BRUTERESPER, EATTRTYPE_DEMIHUMANRESPER, EATTRTYPE_DEMONRESPER, EATTRTYPE_PLANTRESPER,
    EATTRTYPE_DEADLESSRESPER, EATTRTYPE_FORMLESSRESPER, EATTRTYPE_FISHRESPER, EATTRTYPE_ANGLERESPER,
    EATTRTYPE_INSECTRESPER, EATTRTYPE_DRAGONRESPER }

  local race = tUser:getRaceType()
  local addP = 1
  if race > 0 and race < 11 then
    addP = 1 + sUser:getAttr(RaceToDamPer[race])
  end
  race = sUser:getRaceType()
  local delP = 1
  if race > 0 and race < 11 then
    delP = 1 - tUser:getAttr(RaceToResPer[race])
  end

  return addP * delP
end

function getBossParam(sUser, tUser)
  if (sUser == nil or tUser == nil) then
    return 0
  end

  local addP = 1
  if tUser:isBoss() then
    addP = 1 + sUser:getAttr(EATTRTYPE_BOSSDAMPER)
  end
  local delP = 1
  if sUser:isBoss() then
    delP = 1 - tUser:getAttr(EATTRTYPE_BOSSRESPER)
  end
  return addP * delP
end

function getElementParam(sUser, tUser, skill)
  if (sUser == nil or tUser == nil or skill == nil) then
    return 0
  end

  local atkType = skill:getAtkAttr() ~= 0 and skill:getAtkAttr() or sUser:getAttr(EATTRTYPE_ATKATTR)
  --local atkType = sUser:getAttr(EATTRTYPE_ATKATTR)
  local defType = tUser:getAttr(EATTRTYPE_DEFATTR)
  if atkType < 1 or atkType > 10 or defType < 1 or defType > 10 then
    return 1
  end
  local configParam = GameConfig.ElementRestrain[atkType][defType]

  local atkList = {
    EATTRTYPE_WINDDAMPER, EATTRTYPE_EARTHDAMPER, EATTRTYPE_WATERDAMPER, EATTRTYPE_FIREDAMPER,
    EATTRTYPE_NEUTRALDAMPER, EATTRTYPE_HOLYDAMPER, EATTRTYPE_SHADOWDAMPER, EATTRTYPE_GHOSTDAMPER,
    EATTRTYPE_UNDEADDAMPER, EATTRTYPE_POSIONDAMPER
  }
  local defList = {
    EATTRTYPE_BEWINDDAMPER, EATTRTYPE_BEEARTHDAMPER, EATTRTYPE_BEWATERDAMPER, EATTRTYPE_BEFIREDAMPER,
    EATTRTYPE_BENEUTRALDAMPER, EATTRTYPE_BEHOLYDAMPER, EATTRTYPE_BESHADOWDAMPER, EATTRTYPE_BEGHOSTDAMPER,
    EATTRTYPE_BEUNDEADDAMPER, EATTRTYPE_BEPOSIONDAMPER
  }
  local atkParam = 1
  local defParam = 1
  if atkType > 0 and atkType < 11 then
    atkParam = 1 +  sUser:getAttr(atkList[atkType])
    defParam = 1 - tUser:getAttr(defList[atkType])
  end

  return configParam * atkParam * defParam
end

function getBodyParam(sUser, tUser)
  if (sUser == nil or tUser == nil) then
    return 0
  end

  if sUser:isIgnoreBodyDam() then
    return 1
  end

  local bodyDam = {
    EATTRTYPE_SMALLDAMPER, EATTRTYPE_MIDDAMPER, EATTRTYPE_BIGDAMPER
  }
  local bodyRes = {
    EATTRTYPE_SMALLRESPER, EATTRTYPE_MIDRESPER, EATTRTYPE_BIGRESPER
  }
  local sbodysize = sUser:getBodySize()
  local tbodysize = tUser:getBodySize()

  local damParam = 1
  if tbodysize > 0 and tbodysize < 4 then
    damParam = 1 + sUser:getAttr(bodyDam[tbodysize])
  end
  local resParam = 1
  if sbodysize > 0 and sbodysize < 4 then
    resParam = 1 - tUser:getAttr(bodyRes[sbodysize])
  end

  return damParam * resParam
end

--计算怪物真正的掉落经验衰减比率
function calcNpBaseExpRatio(npc, realexp, dailyextra)
  local baseexp = npc:getBaseExp()
  if baseexp == 0 then 
    return 0
  end
  local dailyRatio = CommonFun.getDailyRatio(dailyextra)       
  if dailyRatio == 0 then
    return 0
  end

  local ratio = realexp / dailyRatio / baseexp
  return ratio
end

-- 计算怪物掉落base经验
function calcNpcDropBaseExp(user, npc, damage, killer, dailyextra, membercount)
  if user == nil or npc == nil then
    local info = string.format("[怪物掉落经验], 计算出现异常: %s, %s", tostring(nil), tostring(npc))
    cPlusLogError(info)
    return 0
  end

  local npclv = npc:getLevel()
  local deltalv = npc:getLevel() - user:getLevel()
  local npcbaseexp = npc:getBaseExp()
  local npctype = npc:getNpcType()
  local npczone = npc:getNpcZoneType()
  local npcmaxhp = npc:getAttr(EATTRTYPE_MAXHP)
  local layer = npc:getEndlessLayer()
  local mapid = npc:getMapID()
  local stype = npc:getSummonType()
  local star = npc:isStarMonster()
--  local membercount = user:getTeamMemberCount()

  -- 参数
  local param = 0
  if npctype == ENPCTYPE_MONSTER then
    param = 2
  elseif npctype == ENPCTYPE_MINIBOSS then
    param = 5
  elseif npctype == ENPCTYPE_MVP then
    param = 10
  end

  if npczone ~= ENPCZONE_FIELD then
    npcbaseexp = npcbaseexp * param * npclv
  end

  return CommonFun.calcNpcDropBaseExp(damage, deltalv, npctype, npczone, npcmaxhp, npcbaseexp, 0, membercount, layer, mapid, stype, killer, dailyextra, star)
end

-- 计算怪物掉落job经验
function calcNpcDropJobExp(user, npc, damage, killer, dailyextra, membercount)
  if user == nil or npc == nil then
    return 0
  end

  local npclv = npc:getLevel()
  local deltalv = npc:getLevel() - user:getLevel()
  local npcjobexp = npc:getJobExp()
  local npctype = npc:getNpcType()
  local npczone = npc:getNpcZoneType()
  local npcmaxhp = npc:getAttr(EATTRTYPE_MAXHP)
  local layer = npc:getEndlessLayer()
  local mapid = npc:getMapID()
  local stype = npc:getSummonType()
  local star = npc:isStarMonster()
--  local membercount = user:getTeamMemberCount()

  -- 参数
  local param = 0
  if npctype == ENPCTYPE_MONSTER then
    param = 1
  elseif npctype == ENPCTYPE_MINIBOSS then
    param = 3
  elseif npctype == ENPCTYPE_MVP then
    param = 6
  end

  if npczone ~= ENPCZONE_FIELD then
    npcjobexp = npcjobexp * param * npclv
  end

  return CommonFun.calcNpcDropBaseExp(damage, deltalv, npctype, npczone, npcmaxhp, npcjobexp, 0, membercount, layer, mapid, stype, killer, dailyextra, star)
end

function getNearNum(infNum)
  return math.floor(infNum * 1000 + 0.5) / 1000
end

-- 计算玩家死亡经验掉落
function calcUserDieDropBaseExp(user)
  if user == nil then
    return 0
  end
  local lv = user:getLevel()
  return CommonFun.calcUserDieDropBaseExp(lv);
end

-- 计算玩家离线时间
function calcUserOfflineTime(logintime, offlinetime)
  return CommonFun.calcUserOfflineTime(logintime, offlinetime)
end

-- 计算玩家离线经验
function calcUserOfflineExp(user, logintime, offlinetime)
  return CommonFun.calcUserOfflineExp(user:getLevel(), logintime, offlinetime)
end

-- 计算玩家离线job经验
function calcUserOfflineJobExp(user, logintime, offlinetime)
  return CommonFun.calcUserOfflineJobExp(user:getLevel(), logintime, offlinetime)
end

-- 计算抗击魔潮经验池扣除
function calcDailyExpSub(exp)
  return CommonFun.calcDailyExpSub(exp)
end

-- 计算装备强化所需费用
function calcEquipStrengthCost(lv, q, t)
  return CommonFun.calcEquipStrengthCost(lv, q, t)
end
  
--[[
-- 计算沉迷经衰减率
function calcAddictRatio(user, usedtime)
  if user == nil then
    return 0
  end
  local ratio = 1
  local totaltime = user:getTotalBattleTime()

  if usedtime <= totaltime then
    return ratio
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
  local newexp = calcAddictRatio(user, sec) * exp
  return newexp
end
--]]

-- 计算沉迷怪物掉落衰减
--function calcAddictDropItem(sec, ratio, deltalv)
--  return CommonFun.calcAddictDropItem(sec, ratio, deltalv)
--end

-- 计算沉迷怪物掉落衰减提示时间
function getAddictTipsTime(user)
  local extra = user:getDepositAddictTime()
  return CommonFun.getAddictBase() + extra
end

-- 衰减疲劳与高度疲劳的分界
function getAddictStage()
  return CommonFun.getAddictStage()
end

-- 计算公会祈祷所需贡献
function calcGuildPrayCon(prayid, praylv)
  return GuildFun.calcGuildPrayCon(prayid, praylv)
end

-- 计算公会祈祷所需货币
function calcGuildPrayMon(prayid, praylv)
  return GuildFun.calcGuildPrayMon(prayid, praylv)
end

--- 计算公会祈祷属性
function calcGuildPrayAttr(user, prayid, praylv)
  if user == nil then
    return
  end

  local cfg = Table_Guild_Faith[prayid]
  if cfg == nil then
    return
  end

  local result = GuildFun.calcGuildPrayAttr(prayid, praylv)

  for key, value in pairs(result) do
    --user:setGuildAttr(key, value)
    user:modifyCollect(ECOLLECTTYPE_GUILD, key, value, EATTROPER_ADD)
  end
end
  
-- 计算交易所税率  x%, price 单价
function calcTradeTax(price)
  return CommonFun.calcTradeTax(price)
end

-- 计算交易所赠送税  totalPrice总价，bg背景id
function calcTradeGiveFee(totalPrice, bg)
  return CommonFun.calcTradeGiveFee(totalPrice, bg)
end

-- 商人系制作物品 概率计算 -----------------------------------
function calcProduceRate(user, etype, category, composeid)
  if user == nil then
    return 0
  end

  entry = Entry.new(user)
  return CommonFun.calcProduceRate(entry, etype, category, composeid)
end

-- 暴击概率, 单位:10000
function calcProduceCriRate(user, etype, category)
  if user == nil then
    return 0
  end

  -- 尖锐合金制作概率
  local rate = 0
  if etype == 3 and category == 4 then
    if user:getSkillLv(218) >= 1 then
      rate = 1500
    end

  -- 魔石制作概率
  elseif etype == 3 and category == 3 then
    if user:getSkillLv(269) >= 1 then
      rate = 1500
    end
  end

  return rate
end

-- 商人系制作物品 概率计算 -----------------------------------

-- 商人系交易所数据变化 -----------------------------------
function calcTradeMaxPendingCout(user)
  if user == nil then
    return 0
  end
  entry = Entry.new(user)
  return CommonFun.calcTradeMaxPendingCout(entry)
end

function calcTradeBackMoneyPer(user)
  if user == nil then
    return 0
  end
  entry = Entry.new(user)
  return CommonFun.calcTradeBackMoneyPer(entry)
end

function calcBoothMaxPendingCout(user)
  if user == nil then
    return 0
  end
  entry = Entry.new(user)
  return CommonFun.calcBoothMaxPendingCout(entry)
end
-- 商人系交易所数据变化 -----------------------------------

---------------------------------------  SkillHelperFunc 调用 , 与客户端公用类 -------------------------------
-- 技能前置条件检查
function checkPrecondtion(protype, user, skillid)
  if user == nil then
    return false
  end
  entry = Entry.new(user)
  return SkillHelperFunc.CheckPrecondtionByProType(protype, entry, skillid)
end

-- 技能动态消耗
function doDynamicCost(user, costtype, skillid)
  if user == nil then
    return
  end

  entry = Entry.new(user)
  SkillHelperFunc.DoDynamicCost(costtype, entry, skillid)
end

---------------------------------------  CommonFun 调用 , 与客户端公用类 ----------------------------------
CommonFunHelper = {}
function CommonFunHelper.HasBuffID(userid, buffid)
  local pEntry = getEntryObj(userid)
  if pEntry == nil then
    return false
  end
  return pEntry:getBuff():haveBuff(buffid)
end

function CommonFunHelper.GetUserHP(userid)
  local pEntry = getEntryObj(userid)
  if pEntry == nil then
     return 0
  end
  return pEntry:getAttr(EATTRTYPE_HP)
end

function CommonFunHelper.GetProperty(userid, attrname)
  local pEntry = getEntryObj(userid)
  if pEntry == nil then
     return 0
  end
  if pEntry:isValidAttrName(attrname) == false then
    local info = string.format("CommonFun 获得属性, 属性名称错误: %s", tostring(attrname))
    cPlusLogError(info)
    return 0
  end
  return pEntry:getStrAttr(attrname)
end

function CommonFunHelper.GetLernedSkillLevel(userid, skillID)
  local pEntry = getEntryObj(userid)
  if pEntry == nil then
     return 0
  end
  return pEntry:getSkillLv(skillID)
end

function CommonFunHelper.GetBuffLayer(userid, buffid)
  local pEntry = getEntryObj(userid)
  if pEntry == nil then
    return 0
  end
  local buff = pEntry:getBuff()
  if buff == nil then
    return 0
  end

  return buff:getLayerByID(buffid)
end

-- 计算料理成功率 所有概率都是以千为单位 ,
-- cookerlv:厨师等级，
-- cooklv:料理熟练度， 
-- cookhard：料理烹饪难度，
-- avgmateriallv:食材平均熟练度等级，
-- book_addrate
function calcCookSuccessRate(cookerlv, cooklv, cookhard, avgmateriallv, book_addrate)
  return CommonFun.calcCookSuccessRate(cookerlv, cooklv, cookhard, avgmateriallv, book_addrate)
end

--计算扭蛋需要的扭蛋币价格
--tp:扭蛋类型 2：装备 3:卡片
--count:今天已经扭蛋的次数
function calcLotteryCost(tp, count)
  return CommonFun.calcLotteryCost(tp, count)
end

--计算精炼等级额外回收的扭蛋券个数
--itemid:物品id
--refinelv:精炼等级
--damage:损坏
--type:扭蛋回收类型
function calcRefineRecovery(itemid, refinelv, damage, t)
  return CommonFun.calcRefineRecovery(itemid, refinelv, damage, t)
end


--计算拍卖追加的价格
function calcAuctionPrice(bp, level)
  basePrice = tonumber(bp)
  return CommonFun.calcAuctionPrice(basePrice, level)
end

-- 计算波利乱斗额外积分
function calcExtraScore(maxApple, rank)
  return CommonFun.calcExtraScore(maxApple, rank)
end

-- 计算公会建筑猫砂盆随机掉落的道具数量
function calcCatLitterBoxItemCount(itemid, buildinglv)
   return CommonFun.calcCatLitterBoxItemCount(itemid, buildinglv)
end

-- 秘银强化属性加成
function calcStrengthCost(user, quality, itemtype, lv)
  if user == nil then
    return
  end
  local pack = user:getPackage()
  if pack == nil then
    return
  end

  local cost = ItemFun.calcStrengthCost(quality, itemtype, lv)
  for key, value in pairs(cost) do
    pack:addStrengthCost(key, value)
  end
end

-- 秘银强化属性加成
function calcStrengthAttr(user, quality, equiptype, lv)
  if user == nil then
    return
  end

  local result = ItemFun.calcStrengthAttr(quality, equiptype, lv)
  for key, value in pairs(result) do
    user:modifyCollect(ECOLLECTTYPE_EQUIP, key, value, EATTROPER_ADD)
  end
end

-- 计算亡者boss召唤等级
function CalcRaidDeadBossSummonLevel(raidtype, param)
  return CommonFun.CalcRaidDeadBossSummonLevel(raidtype, param)
end

-- 计算组队排位赛积分
function CalcTeamPwsScore(winteam_ave, loseteam_ave, winmax, losemax, selfscore, iswin)
  return CommonFun.CalcTeamPwsScore(winteam_ave, loseteam_ave, winmax, losemax, selfscore, iswin)
end
