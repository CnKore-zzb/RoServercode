pythonGlobalProduce = {}
pythonCompare2FileStr = ''

package.path=package.path..';../?.txt'
package.path=package.path..';../Table/?.txt'
require('Table_Reward')
require('Table_Item')
require('Table_ItemType')
require('Table_ProduceLogic')

-- 预处理表格 --
-- Table_Quest --
function formatQuest()
  local table_quest_list = {'Table_Quest', 'Table_Quest_2', 'Table_Quest_3', 'Table_Quest_4', 'Table_Quest_5',
    'Table_Quest_6', 'Table_Quest_7', 'Table_Quest_8', 'Table_Quest_10', 'Table_Quest_11', 'Table_Quest_61'
  }
  for _,v in pairs(table_quest_list) do
    require(v)
  end

  for k,v in pairs(table_quest_list) do
    local keytest = {}
    local tempdata = {}
    for k1, v1  in pairs(_G[v]) do
      table.insert(keytest, k1)
      tempdata[k1] = v1
    end
    table.sort(keytest, function(a,b) return a<b end)

    _G[v] = {}
    local tempgroup = 0
    local step = 0
    for k1, v1 in pairs(keytest) do
      local data = tempdata[v1]
      if tempgroup == 0 then
        tempgroup = data.GroupID
      end
      if data.GroupID == tempgroup then
        step = step + 1
      else
        step = 0
      end
      local id = data.QuestID*10000*1000 + data.GroupID*10000 + step
      _G[v][id] = data
    end
  end
end
-- 预处理表格 --

---------- 生成格式化的 Table_Reward -------------
FormatTableReward = {} -- team 为key
for k,v in pairs(Table_Reward) do
  local teamid = v.team
  if FormatTableReward[teamid] == nil then
    FormatTableReward[teamid] = {}
  end
  table.insert(FormatTableReward[teamid], v)
end

formatQuest()

-- type =1, 4 : 独立以1万为计数 , type = 2, 3 : 按权重计算, type = 5, 同type=2处理
-- type = 1,2,5 对应道具id, 3,4 索引其他teamid
for _, v in pairs(FormatTableReward) do
  local allrate = 10000
  if v[1].type == 1 or v[1].type == 4 then
    allrate = 10000
  elseif v[1].type == 2 or v[1].type == 3 or v[1].type == 5 then
    allrate = 0
    for _, v1 in pairs(v) do
      allrate = allrate + v1.rate
    end
  end

  for _, v1 in pairs(v) do
    v1.allrate = allrate
  end
end
---------- 生成格式化的 Table_Reward -------------

-----------子函数模块-------------------
function table.IsEmpty(tab)
  if tab == nil then return true end
  for _, v in pairs(tab) do
    if v then return false end
  end
  return true
end

function selectok(condition, data)
  local flag = true
  for k,v in pairs(condition) do
    if type(v) == 'table' then
       if selectok(v, data[k]) == false then
         flag = false
         break
       end
    else
      if v ~= data[k] then
        flag = false
        --print("h2 false", k, v, data[k])
        break
      end
    end
  end

  return flag
end

function table.KeyIsLast(tab, kValue)
  if kValue and tab then
    local tabKey = {}
    for k, _ in pairs(tab) do
      table.insert(tabKey, k)
    end
    if not table.IsEmpty(tabKey) then
      local lastKey = tabKey[#tabKey]
      if type(kValue) == type(lastKey) and kValue == lastKey then
        return true
      end
    end
  end
  return false
end

function SerializeWithoutNewline(obj, noquote)
	local lua = ""
	local t = type(obj)
	if t == "number" then
		lua = obj
	elseif t == "boolean" then
		lua = tostring(obj)
	elseif t == "string" then
    if noquote == true then
      lua = obj
    else
		  lua = "\"" .. obj .. "\""
    end
	elseif t == "table" then
		lua = "{"
		for k, v in pairs(obj) do
			local strK = ""
			local kType = type(k)
      if kType == "number" then
        lua = lua .. SerializeWithoutNewline(v) ..  (table.KeyIsLast(obj, k) and '' or ",")
      else
        if kType == "string" then
          strK = k
        elseif kType == "number" then
          strK = '[' .. k .. ']'
        end
        --lua = lua .. strK .. '=' .. SerializeWithoutNewline(v) .. (table.KeyIsLast(obj, k) and '' or ",")
        lua = lua .. strK .. '=' .. SerializeWithoutNewline(v) .. (table.KeyIsLast(obj, k) and '' or ",")
      end
		end
		lua = lua .. '}'
	elseif t == "nil" then
		return nil
	else
		error("Can not serialize a " .. t .. " type.")
	end
	return lua
end

-------------- 通过reward 获取 item 信息 ----------------------------
-- 返回的信息 produce_items = {{itemid=xx, num=xx, rate=xx},{}...}
function calcReward(id, produce_items, baserate)
  if FormatTableReward[id] == nil then
    return
  end
  if baserate == nil then
    baserate = 1 -- 缺省值
  end

  local data = FormatTableReward[id]
 
  -- type =1, 4 : 独立以1万为计数 , type = 2, 3 : 按权重计算, type = 5, 同type=2处理
  -- type = 1,2,5 对应道具id, 3,4 索引其他teamid
  if data[1].type == 1 or data[1].type == 2 or data[1].type == 5 then
    for _,v in pairs(data) do
      if v.rate ~= 0 then
        local onedata  = {}
        onedata['rate'] = v.rate / v.allrate * baserate
        for _, v1 in pairs(v.item) do
          onedata['itemid'] = v1.id
          onedata['num'] = v1.num
          table.insert(produce_items, onedata)
        end
      end
    end
  elseif data[1].type == 3 or data[1].type == 4  then
    for _,v in pairs(data) do
      if v.rate ~= 0 then
        for _, v1 in pairs(v.item) do
          calcReward(v1.id, produce_items, v.rate / v.allrate)
        end
      end
    end
  end
end
-------------- 通过reward 获取 item 信息 ----------------------------

function executeLuaString(str)
  if (_VERSION < 'Lua 5.2') then
    loadstring(str)()
  else
    load(str)()
  end
end

function reloadLuaFile()
  for _,v in pairs(Table_ProduceLogic) do
    tName = v.TableName
    formatName = 'Table_'..tName
    package.loaded[formatName] = nil
  end
  formatQuest()
end

function fromLuaRowToCsvRow(tabValue)
  str = ''
  --['ItemID', 'SourceType', 'ItemName', 'ItemType', 'ItemTypeName', 'SourceTableName', 'Params']
  str = str .. tostring(tabValue['ItemID']) .. ','
  str = str .. tabValue['Type'] .. ','
  str = str .. tabValue['ItemName'] .. ','
  str = str .. tostring(tabValue['ItemType']) .. ','
  str = str .. tabValue['ItemTypeName'] .. ','
  str = str .. tabValue['SourceTableName'] .. ','
  str = str .. '\"' .. SerializeWithoutNewline(tabValue['Params']) .. '\"'
  return str
end

function compTwoParams(params1, params2)
  if #params1 ~= #params2 then
    return false
  end
  local num1 = 0
  local f1 = 0
  for _,v in pairs(params1) do
    for _,v2 in pairs(v) do
      if v2 < 1 then
        f1 = f1 + v2
      else
        num1 = num1 + v2
      end
    end
  end
  local num2 = 0
  local f2 = 0
  for _,v in pairs(params2) do
    for _,v2 in pairs(v) do
      if v2 < 1 then
        f2 = f2 + v2
      else
        num2 = num2 + v2
      end
    end
  end

  if num1 ~= num2 and math.abs(num1-num2) * 1000000 > num1 then
    return false
  end
  if f1 ~= f2 and math.abs(f1-f2) * 1000000 > f1 then
    return false
  end
  return true
end

-------------- 生成道具产出信息 -----------------
function produceItemInfo(starttime, endtime, resName, bCompare, strCompName)
  local results = {}
  for _, v in pairs(Table_ProduceLogic) do
    -- get table data
    tName = v.TableName
    formatName = 'Table_'..tName
    require(formatName)
    local data = _G[formatName]
    local formatProduceStr = v.Produce

    for k1,v1 in pairs(data) do
      -- 检查是否匹配
      local selectflag = true
      if v.EqualCondition ~= nil then
        if selectok(v.EqualCondition, v1) == false then
         selectflag = false
         --print('---false', formatProduceStr)
        end
      end

      OneValue = v1
      CompareStartTime = starttime
      CompareEndTime = endtime

      CompareSelectFlag = false
      if v.CompareCondition ~= nil and v.CompareCondition ~= '' then
        executeLuaString(v.CompareCondition)
      else
        CompareSelectFlag = true
      end

      CompareFileSelectFlag = true
      if v.CompareFile ~= nil and v.CompareFile ~= '' then
        dofile('CompareFile/'..v.CompareFile)
      end

      if selectflag == true and CompareSelectFlag == true and CompareFileSelectFlag == true then
        -- 全局变量 用于执行loadstring(formatProduceStr)()
        AllRewardIDs = {}         --{id1, id2, id3..}
        OneRewardID = nil         --id
        AllItems = {}             --{OneItem, OneItem..}
        OneItem = nil             --{itemid, num}, itemid必须有效, num可为nil默认为1

        executeLuaString(formatProduceStr)
        if OneRewardID ~= nil then
          table.insert(AllRewardIDs, OneRewardID)
        end
        if OneItem ~= nil and OneItem[1] ~= nil then
          table.insert(AllItems, OneItem)
        end

        -- 相关参数
        ProduceParam = {}
        if v.ProduceParam ~= '' and v.ProduceParam ~= nil then
          executeLuaString(v.ProduceParam)
        end

        local OneKey = v.Type..'-'..tostring(k1)

        if table.IsEmpty(AllRewardIDs) == false then
          for _,v2 in pairs(AllRewardIDs) do
            local items = {}
            calcReward(v2, items)
            for _,v3 in pairs(items) do
              local key = tostring(v.Type)..'-'..tostring(v3.itemid)
              local tabResult = {}
              if results[key] ~= nil then
                tabResult = results[key]
              else
                tabResult['Type'] = v.Type
                tabResult['SourceTableName'] = formatName
                tabResult['ItemID'] = v3.itemid
                local itemd = Table_Item[v3.itemid]
                if itemd == nil then
                  tabResult['ItemName'] = '非法'
                  tabResult['ItemType'] = 0
                  tabResult['ItemTypeName'] = '非法'
                else
                  tabResult['ItemName'] = '\"'..itemd.NameZh..'\"'
                  tabResult['ItemType'] = itemd.Type
                  tabResult['ItemTypeName'] = Table_ItemType[itemd.Type] ~= nil and '\"'..Table_ItemType[itemd.Type].Name..'\"' or '非法'
                end
                tabResult['Params'] = {}
              end
              table.insert(tabResult.Params, {['sid']=k1,['rate']=v3.rate})
              results[key] = tabResult
            end
          end
        elseif table.IsEmpty(AllItems) == false then
          for _,v2 in pairs(AllItems) do
            if v2[1] ~= nil then
              local key = v.Type ..'-'..tostring(v2[1])
              local tabResult = {}
              if results[key] ~= nil then
                tabResult = results[key]
              else
                tabResult['Type'] = v.Type
                tabResult['SourceTableName'] = formatName
                tabResult['ItemID'] = v2[1]
                local itemd = Table_Item[tabResult.ItemID]
                if itemd ~= nil then
                  tabResult['ItemName'] = '\"'..itemd.NameZh..'\"'
                  tabResult['ItemType'] = itemd.Type
                  tabResult['ItemTypeName'] = Table_ItemType[itemd.Type] ~= nil and '\"'..Table_ItemType[itemd.Type].Name..'\"' or '非法'
                else
                  tabResult['ItemName'] = '非法'
                  tabResult['ItemType'] = 0
                  tabResult['ItemTypeName'] = '非法'
                end
                tabResult['Params'] = {}
              end
              local num = v2[2] ~= nil and v2[2] or 1 --默认num为1
              table.insert(tabResult.Params, {['sid']=k1,['num']=num})

              results[key] = tabResult
            end
          end
        end
      end
    end
  end

  if bCompare == false then
    -- 非比较模式, 仅生成文件
    local str = 'ItemID,Type,ItemName,ItemType,ItemTypeName,SourceTableName,Params\n'
    str = str .. '道具ID,获取途径,道具名称,道具类型ID,道具类型名,来源表,参数\n'
    local keyTest = {}
    for i in pairs(results) do
         table.insert(keyTest,i)
    end
    table.sort(keyTest, function(a,b) return a<b end)
    for k,v in pairs(keyTest) do
      local data = results[v]
      str = str .. fromLuaRowToCsvRow(data) .. '\n'
    end
    str = string.sub(str, 1, -2) --去掉最后一个'\n'
    local file=io.open(resName, "w+b")
    print(resName)
    file:write(str)
    io.close(file)
  else
    -- 比较模式
    --dofile(strCompName..'.txt')

    local keyTest = {}
    for i in pairs(Table_TempProduceResult) do
         table.insert(keyTest,i)
    end
    table.sort(keyTest, function(a,b) return a<b end)

    local strDel = ''
    for k,v in pairs(keyTest) do
      if results[v] == nil then
        local d = Table_TempProduceResult[v]
        strDel = strDel .. 'Delete,'..fromLuaRowToCsvRow(d) ..'\n'
      end
    end

    keyTest = {}
    for i in pairs(results) do
         table.insert(keyTest,i)
    end
    table.sort(keyTest, function(a,b) return a<b end)
    local strAdd = ''
    local strMod = ''
    local strNoChange = ''
    for k,v in pairs(keyTest) do
      local d = results[v]
      if Table_TempProduceResult[v] == nil then
        strAdd = strAdd .. 'Add,' .. fromLuaRowToCsvRow(d) .. '\n'
      elseif compTwoParams(d.Params, Table_TempProduceResult[v].Params) == false then
        strMod = strMod .. 'Modify,' .. fromLuaRowToCsvRow(d) ..',\"'..SerializeWithoutNewline(Table_TempProduceResult[v].Params) .. '\"\n'
      else
        strNoChange = strNoChange .. 'NoChange,' .. fromLuaRowToCsvRow(d) .. '\n'
      end
    end

    local str = 'Mark,ItemID,Type,ItemName,ItemType,ItemTypeName,SourceTableName,Params, OldParams\n'
    str = str .. '变化类型,道具ID,获取途径,道具名称,道具类型ID,道具类型名,来源表,参数,旧的参数\n'
    str = str .. strDel .. strAdd .. strMod .. strNoChange
    str = string.sub(str, 1, -2) --去掉最后一个'\n'
    local file=io.open(resName, "w+b")
    file:write(str)
    io.close(file)
  end
end

function compTwoFile(filename)
  pythonCompare2FileStr = ''

  local keyTest = {}
  for i in pairs(Table_TempProduceResult_1) do
       table.insert(keyTest,i)
  end
  table.sort(keyTest, function(a,b) return a<b end)

  local change = false
  local strDel = ''
  for k,v in pairs(keyTest) do
    if Table_TempProduceResult_2[v] == nil then
      local d = Table_TempProduceResult_1[v]
      strDel = strDel .. 'Delete,'..fromLuaRowToCsvRow(d) ..'\n'
      change = true
    end
  end

  keyTest = {}
  for i in pairs(Table_TempProduceResult_2) do
       table.insert(keyTest,i)
  end
  table.sort(keyTest, function(a,b) return a<b end)
  local strAdd = ''
  local strMod = ''
  for k,v in pairs(keyTest) do
    local d = Table_TempProduceResult_2[v]
    if Table_TempProduceResult_1[v] == nil then
      strAdd = strAdd .. 'Add,' .. fromLuaRowToCsvRow(d) .. '\n'
      change = true
    elseif compTwoParams(d.Params, Table_TempProduceResult_2[v].Params) == false then
      strMod = strMod .. 'Modify,' .. fromLuaRowToCsvRow(d) ..',\''..SerializeWithoutNewline(Table_TempProduceResult_1[v].Params) .. '\'\n'
      change = true
    end
  end
  if change == true then
    pythonCompare2FileStr = pythonCompare2FileStr .. 'Add:\n' .. strAdd .. '\n'
    pythonCompare2FileStr = pythonCompare2FileStr .. 'Delete:\n' .. strDel .. '\n'
    pythonCompare2FileStr = pythonCompare2FileStr .. 'Modify:\n' .. strMod .. '\n'
  end

  local str = 'Mark,ItemID,Type,ItemName,ItemType,ItemTypeName,SourceTableName,Params, OldParams\n'
  str = str .. '变化类型,道具ID,获取途径,道具名称,道具类型ID,道具类型名,来源表,参数,旧的参数\n'
  str = str .. strDel .. strAdd .. strMod
  str = string.sub(str, 1, -2) --去掉最后一个'\n'
  local file=io.open(filename, "w+b")
  file:write(str)
  io.close(file)
end

