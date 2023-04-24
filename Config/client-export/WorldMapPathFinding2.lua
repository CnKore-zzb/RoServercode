local posRecord = string.len(package.path)
if runtimePlatform == 1 then
	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';../../client-export/?.txt'
	require 'ReferenceFilesName'

	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';./Assets/Resources/Script/MConfig/?.txt'
	require("Table_MapIDs")

	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';./Assets/Resources/Script/MConfig/?.txt'
	require('MapTeleport')
elseif runtimePlatform == nil then
	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';./?.txt'
	require 'ReferenceFilesName'

	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';../client-refactory/Develop/Assets/Resources/Script/MConfig/?.txt'
	require("Table_MapIDs")

	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';../client-refactory/Develop/Assets/Resources/Script/MConfig/?.txt'
	require('MapTeleport')
end
package.path = string.sub(package.path, 1, posRecord)

local tabMapValidExitPoints = {}
local directoryName = ""
local directoryPrefixName = "Scene"
local tabDirectorySuffixName = Table_ScenesName
for _, v in pairs(tabDirectorySuffixName) do
	local directorySuffixName = v
	directoryName = directoryPrefixName .. directorySuffixName
	if runtimePlatform == 1 then
		require("../../client-export/Scene/" .. directoryName .. "/SceneInfo.lua")
	elseif runtimePlatform == nil then
		require("Scene/" .. directoryName .. "/SceneInfo")
	end

	local mapENName = directorySuffixName
	local mapID = Table_MapIDs[mapENName]
	if mapID ~= nil then
		local mapConf = Table_Map[mapID]
		if mapConf ~= nil then
			if not (mapConf.LeapsMapNavigation == 1) then
				local validExitPoints = {}
				local tabExitPoints = Root.ExitPoints
				if tabExitPoints ~= nil then
					for _, v in pairs(tabExitPoints) do
						local exitPoint = v
						if (not (exitPoint.next_scene_ID == 0)) and exitPoint.privategear == 0 then
							table.insert(validExitPoints, exitPoint)
						end
					end
				end
				tabMapValidExitPoints[mapID] = validExitPoints
			end
		end
	end

	local mapPVPENName = mapENName .. 'P'
	local mapID = Table_MapIDs[mapPVPENName]
	if mapID ~= nil then
		local mapConf = Table_Map[mapID]
		if mapConf ~= nil then
			if not (mapConf.LeapsMapNavigation == 1) then
				local validExitPoints = {}
				local pvp = Root.PVP
				if pvp ~= nil then
					local tabExitPoints = pvp.ExitPoints
					if tabExitPoints ~= nil then
						for _, v in pairs(tabExitPoints) do
							local exitPoint = v
							if (not (exitPoint.next_scene_ID == 0)) and exitPoint.privategear == 0 then
								table.insert(validExitPoints, exitPoint)
							end
						end
					end
				end
				tabMapValidExitPoints[mapID] = validExitPoints
			end
		end
	end

	local raids = Root.Raids
	if raids ~= nil then
		for k, v in pairs(raids) do
			local mapID = k
			local mapConf = Table_Map[mapID]
			if mapConf ~= nil then
				if not (mapConf.LeapsMapNavigation == 1) then
					local validExitPoints = {}
					local raid = v
					local tabExitPoints = raid.ExitPoints
					if tabExitPoints ~= nil then
						for _, v in pairs(tabExitPoints) do
							local exitPoint = v
							if (not (exitPoint.next_scene_ID == 0)) and exitPoint.privategear == 0 then
								table.insert(validExitPoints, exitPoint)
							end
						end
					end
					tabMapValidExitPoints[mapID] = validExitPoints
				end
			end
		end
	end
end

local tabMapValidNPCExitPoints = {}
for k, v in pairs(Table_Map) do
	local mapID = k
	local outterTeleportIsValid = not (v.LeapsMapNavigation == 1)
	if outterTeleportIsValid then
		local fromMap = v.NpcMapID
		local fromNPC = v.EntranceNpc
		if fromMap and fromMap > 0 and fromNPC and fromNPC > 0 then
			local npcExitPoint = {npcID = fromNPC, mapID = mapID}
			if table.ContainsKey(tabMapValidNPCExitPoints, fromMap) then
				local cachedNPCPoints = tabMapValidNPCExitPoints[fromMap]
				table.insert(cachedNPCPoints, npcExitPoint);
			else
				tabMapValidNPCExitPoints[fromMap] = {npcExitPoint}
			end
		end
		local toMap = v.ExitMap
		local npc = v.ExitNpc
		if toMap and toMap > 0 and npc and npc > 0 then
			local npcExitPoint = {npcID = npc, mapID = toMap}
			if table.ContainsKey(tabMapValidNPCExitPoints, mapID) then
				local cachedNPCPoints = tabMapValidNPCExitPoints[mapID]
				table.insert(cachedNPCPoints, npcExitPoint);
			else
				tabMapValidNPCExitPoints[mapID] = {npcExitPoint}
			end
		end
	end
end

local tabLinkNet = {}
for k, v in pairs(tabMapValidExitPoints) do
	local mapID = k
	local toMaps = {}
	local exitPoints = v
	for _, v in pairs(exitPoints) do
		local exitPoint = v
		local nextMapID = exitPoint.next_scene_ID
		local exitPointID = exitPoint.ID
		local bornPointID = exitPoint.next_scene_born_point_ID
		local nextMapInfo = {mapID = nextMapID, exitPointID = exitPointID, bornPointID = bornPointID}
		table.insert(toMaps, nextMapInfo)
	end
	if not table.IsEmpty(toMaps) then
		local linkMaps = {}
		linkMaps.toMaps = toMaps
		tabLinkNet[mapID] = linkMaps
	end
end
for k, v in pairs(tabMapValidNPCExitPoints) do
	local mapID = k
	local toMaps = {}
	local npcExitPoints = v
	if table.ContainsKey(tabLinkNet, mapID) then
		local cachedToMaps = tabLinkNet[mapID].toMaps
		for _, v in pairs(npcExitPoints) do
			local npcExitPoint = v
			table.insert(cachedToMaps, npcExitPoint)
		end
	else
		local linkMaps = {toMaps = npcExitPoints}
		tabLinkNet[mapID] = linkMaps
	end
end
for k, v in pairs(tabLinkNet) do
	local fromMaps = {}
	local mapID = k
	for k, v in pairs(tabLinkNet) do
		local mapIDLatter = k
		if mapIDLatter ~= mapID then
			local linkMaps = v
			local toMaps = linkMaps.toMaps
			local nextMapsInfo = {}
			for _, v in pairs(toMaps) do
				local nextMapInfo = v
				if nextMapInfo.mapID == mapID then
					table.insert(nextMapsInfo, nextMapInfo)
					break
				end
			end
			for _, v in pairs(nextMapsInfo) do
				local nextMapInfo = v
				local fromMapInfo = nil
				if nextMapInfo.npcID and nextMapInfo.npcID > 0 then
					fromMapInfo = {mapID = mapIDLatter, npcID = nextMapInfo.npcID}
				else
					fromMapInfo = {mapID = mapIDLatter, bornPointID = nextMapInfo.bornPointID, exitPointID = nextMapInfo.exitPointID}
				end
				table.insert(fromMaps, fromMapInfo)
			end
		end
	end
	if not table.IsEmpty(fromMaps) then
		tabLinkNet[mapID].fromMaps = fromMaps
	end
end

local _allMapsID = {}
for k, _ in pairs(tabLinkNet) do
	local mapID = k
	if not table.ContainsValue(_allMapsID, mapID) then
		table.insert(_allMapsID, mapID)
	end
end

function GetToMapsInfo(origin_map_id)
	local retValue = nil
	local linkMaps = tabLinkNet[origin_map_id]
	if linkMaps ~= nil then
		local toMaps = linkMaps.toMaps
		retValue = {}
		for _, v in pairs(toMaps) do
			local toMap = v
			table.insert(retValue, toMap)
		end
	end
	return retValue
end

function GetToMapInfo(origin_map_id, target_map_id)
	local retValue = nil
	local toMaps = GetToMapsInfo(origin_map_id)
	if toMaps ~= nil then
		for _, v in pairs(toMaps) do
			local toMap = v
			local toMapID = v.mapID
			if toMapID == target_map_id then
				retValue = toMap
				break
			end
		end
	end
	return retValue
end

function GetBPToEPTotalCost(map_id, bp_id, ep_id)
	local retValue = nil
	local mapInnerInfo = MapInnerTeleport[map_id]
	if mapInnerInfo ~= nil then
		local mapBPToEPInfo = mapInnerInfo.outter
		local epsInfo = mapBPToEPInfo[bp_id]
		if epsInfo ~= nil then
			local epInfo = epsInfo[ep_id]
			if epInfo ~= nil then
				retValue = epInfo.totalCost
			end
		end
	end
	if nil == retValue then
		print(string.format("no bp->ep cost: %d, %d->%d", map_id, bp_id, ep_id))
		return 0
	end
	return retValue
end

function GetMapIndex(map_id)
	for i = 1, #_allMapsID do
		local mapID = _allMapsID[i]
		if mapID == map_id then
			return i
		end
	end
	return 0
end

local tabAllPaths = {}
function AddIntoPaths(origin_map, target_map, path)
	if table.ContainsKey(tabAllPaths, origin_map) then
		local toTargetMaps = tabAllPaths[origin_map]
		if table.ContainsKey(toTargetMaps, target_map) then
			local toTargetMapPaths = toTargetMaps[target_map]
			table.insert(toTargetMapPaths, path)
		else
			toTargetMaps[target_map] = {path}
		end
	else
		tabAllPaths[origin_map] = {}
		tabAllPaths[origin_map][target_map] = {path}
	end
end

function FindPath(origin_map, target_map)
	local originMap = origin_map
	local targetMap = target_map
	local tabMapHaveVisitedMaps = {}
	for _, v in pairs(_allMapsID) do
		local mapID = v
		tabMapHaveVisitedMaps[mapID] = {}
	end
	local stack = {}
	function GetVisitedMaps(map_id)
		return tabMapHaveVisitedMaps[map_id]
	end
	function GetUnvisitedMap(map_id)
		local visitedMaps = GetVisitedMaps(map_id)
		local toMaps = GetToMapsInfo(map_id)
		if toMaps ~= nil then
			for _, v in pairs(toMaps) do
				local toMap = v
				local toMapID = toMap.mapID
				if not table.ContainsValue(visitedMaps, toMapID) and not table.ContainsValue(stack, toMapID) then
					table.insert(visitedMaps, toMapID)
					return toMapID
				end
			end
		end
		return 0
	end
	table.insert(stack, originMap)
	while not table.IsEmpty(stack) do
		local toMapID = GetUnvisitedMap(stack[#stack])
		if toMapID == 0 then
			tabMapHaveVisitedMaps[stack[#stack]] = {}
			table.remove(stack, #stack)
		else
			table.insert(stack, toMapID)
		end
		if not table.IsEmpty(stack) and stack[#stack] == targetMap then
			local pathFromStack = {}
			for _, v in pairs(stack) do
				local mapID = v
				table.insert(pathFromStack, mapID)
			end
			AddIntoPaths(originMap, targetMap, pathFromStack)
			table.remove(stack, #stack)
		end
	end
end

for i = 1, #_allMapsID do
	local originMap = _allMapsID[i]
	for _, v in pairs(_allMapsID) do
		local targetMap = v
		if targetMap ~= originMap then
			FindPath(originMap, targetMap)
		end
	end
end

local tabAllDetailPaths = {}
local _path = {}
function NextMapIDOnPath(map_id)
	for i = 1, #_path do
		local mapID = _path[i]
		if mapID == map_id and i < #_path then
			return _path[i + 1]
		end
	end
	return 0
end
local detailPath = {}
local cost = 0
function GetDetailMapOnDetailPath(map_id)
	for _, v in pairs(detailPath) do
		local detailMap = v
		if detailMap.mapID == map_id then
			return detailMap
		end
	end
	return nil
end
function BuildDetailPathWithMapID(map_id)
	local detailMap = GetDetailMapOnDetailPath(map_id)
	if detailMap == nil then
		table.insert(detailPath, {mapID = map_id})
	end
end
function BuildDetailPathWithBP(map_id, born_point_id)
	local detailMap = GetDetailMapOnDetailPath(map_id)
	if detailMap == nil then
		table.insert(detailPath, {mapID = map_id, bornPointID = born_point_id})
	else
		detailMap.bornPointID = born_point_id
	end
end
function BuildDetailPathWithEP(map_id, exit_point_id)
	local detailMap = GetDetailMapOnDetailPath(map_id)
	if detailMap == nil then
		table.insert(detailPath, {mapID = map_id, exitPointID = exit_point_id})
	else
		detailMap.exitPointID = exit_point_id
	end
end
function BuildDetailPathWithNPC(map_id, npc_id, next_map_id)
	local detailMap = GetDetailMapOnDetailPath(map_id)
	if detailMap == nil then
		table.insert(detailPath, {mapID = map_id, npcID = npc_id, nextMapID = next_map_id})
	else
		detailMap.npcID = npc_id
		detailMap.nextMapID = next_map_id
	end
end
function GetBornPointID(map_id)
	for _, v in pairs(detailPath) do
		local detailMap = v
		if detailMap.mapID == map_id then
			return detailMap.bornPointID
		end
	end
	return nil
end
function HandleMapIDOnPath(map_id)
	--print(string.format("FUN >>> HandleMapIDOnPath, <param>map_id = %s</>", tostring(map_id)))
	local nextMapID = NextMapIDOnPath(map_id)
	if nextMapID == 0 then return end
	local toMap = GetToMapInfo(map_id, nextMapID)
	local exitPointID = toMap.exitPointID
	local bornPointID = toMap.bornPointID
	local npcID = toMap.npcID
	local isNPCTeleport = npcID ~= nil
	if isNPCTeleport then
		BuildDetailPathWithNPC(map_id, npcID, nextMapID)
		BuildDetailPathWithMapID(nextMapID)
	else
		BuildDetailPathWithEP(map_id, exitPointID)
		BuildDetailPathWithBP(nextMapID, bornPointID)
		local innerCost = 0
		local bornPointID = GetBornPointID(map_id)
		if bornPointID == nil then
			innerCost = 0
		else
			innerCost = GetBPToEPTotalCost(map_id, bornPointID, exitPointID)
		end
		cost = cost + innerCost
	end
	cost = cost + 24
	HandleMapIDOnPath(nextMapID)
end

-- debug
-- local testPath = tabAllPaths[1][4][1]
-- _path = testPath
-- TableUtil.Print(testPath)
-- HandleMapIDOnPath(testPath[1])
-- TableUtil.Print(detailPath)
-- print(GetBPToEPTotalCost(30001, 1, 1))

for k, v in pairs(tabAllPaths) do
	local originMap  = k
	local toTargetMaps = v
	for k, v in pairs(toTargetMaps) do
		local targetMap = k
		local paths = v
		for _, v in pairs(paths) do
			local path = v
			_path = path
			--TableUtil.Print(_path)
			HandleMapIDOnPath(_path[1])
			--TableUtil.Print(detailPath)
			--print(cost)
			if not table.ContainsKey(tabAllDetailPaths, originMap) then
				tabAllDetailPaths[originMap] = {}
				tabAllDetailPaths[originMap][targetMap] = {{path = detailPath, cost = cost}}
			else
				local toTargetMapsDetail = tabAllDetailPaths[originMap]
				if not table.ContainsKey(toTargetMapsDetail, targetMap) then
					toTargetMapsDetail[targetMap] = {{path = detailPath, cost = cost}}
				else
					toTargetMapPathsDetail = toTargetMapsDetail[targetMap]
					table.insert(toTargetMapPathsDetail, {path = detailPath, cost = cost})
				end
			end
			detailPath = {}
			cost = 0
		end
	end
end

local tabMapOutterTeleport = {}
function BuildOriginMapEPToTargetMapBP_Normal(path)
	for i = 1, #path do
		if i > 1 then
			local secondMapIndex = i + 1
			if secondMapIndex <= #path then
				local startMapDetail = path[i]
				local originMapID = startMapDetail.mapID
				local exitPointID = startMapDetail.exitPointID
				local secondMapDetail = path[secondMapIndex]
				local exitPointIDSecond = secondMapDetail.exitPointID
				local targetMapDetail = path[#path]
				local targetMapID = targetMapDetail.mapID
				local targetMapBornPointID = targetMapDetail.bornPointID

				if not table.ContainsKey(tabMapOutterTeleport, originMapID) then
					tabMapOutterTeleport[originMapID] = {}
				end
				local tab1 = tabMapOutterTeleport[originMapID]
				if not table.ContainsKey(tab1, targetMapID) then
					tab1[targetMapID] = {}
				end
				local tab2 = tab1[targetMapID]
				if not table.ContainsKey(tab2, exitPointID) then
					tab2[exitPointID] = {}
				end
				local tab3 = tab2[exitPointID]
				if not table.ContainsKey(tab3, targetMapBornPointID) then
					tab3[targetMapBornPointID] = {totalCost = 999999, nextEP = exitPointIDSecond}
				end
			end
		end
	end
end

local tabDetailPathMinCost = {}
for k, v in pairs(tabAllDetailPaths) do
	local originMapID = k
	local toTargetMapsDetail = v
	for k, v in pairs(toTargetMapsDetail) do
		local targetMapID = k
		local pathsDetail = v

		-- caculate MIN cost ver.2
		local pathsDetail_Normal = nil
		local pathsDetail_NPC = nil
		for _, v in pairs(pathsDetail) do
			local pathDetail = v
			local path = pathDetail.path
			local isNeedNpcTeleport = false
			for i = 1, #path do
				local mapDetail = path[i]
				local tempNPCID = mapDetail.npcID
				if tempNPCID ~= nil then
					isNeedNpcTeleport = true
					break
				end
			end
			if not isNeedNpcTeleport then
				if pathsDetail_Normal == nil then
					pathsDetail_Normal = {}
				end
				table.insert(pathsDetail_Normal, pathDetail)
			else
				if pathsDetail_NPC == nil then
					pathsDetail_NPC = {}
				end
				table.insert(pathsDetail_NPC, pathDetail)
			end
		end
		local pathDetailMinCost_Normal = nil
		if pathsDetail_Normal ~= nil then
			-- used for find min cost
			local indicator = 0
			local minCost = 0
			local minCostExitPoint = 0
			for _, v in pairs(pathsDetail_Normal) do
				local pathDetail = v
				local cost = pathDetail.cost
				local bPathIsMinCost = false
				if indicator == 0 then
					minCost = cost
					bPathIsMinCost = true
				else
					if cost < minCost then
						minCost = cost
						bPathIsMinCost = true
					end
				end
				indicator = indicator + 1
				if bPathIsMinCost then
					pathDetailMinCost_Normal = pathDetail
				end
			end
		end
		local pathDetailMinCost_NPC = nil
		if pathsDetail_NPC ~= nil then
			-- used for find min cost
			local indicator = 0
			local minCost = 0
			local minCostExitPoint = 0
			for _, v in pairs(pathsDetail_NPC) do
				local pathDetail = v
				local cost = pathDetail.cost
				local bPathIsMinCost = false
				if indicator == 0 then
					minCost = cost
					bPathIsMinCost = true
				else
					if cost < minCost then
						minCost = cost
						bPathIsMinCost = true
					end
				end
				indicator = indicator + 1
				if bPathIsMinCost then
					pathDetailMinCost_NPC = pathDetail
				end
			end
		end
		local crossMapCount_PathDetailMinCost_Normal = 999999
		local crossMapCount_PathDetailMinCost_NPC = 999999
		if pathDetailMinCost_Normal ~= nil then
			crossMapCount_PathDetailMinCost_Normal = #(pathDetailMinCost_Normal.path)
		end
		if pathDetailMinCost_NPC ~= nil then
			crossMapCount_PathDetailMinCost_NPC = #(pathDetailMinCost_NPC.path)
		end

		if not table.ContainsKey(tabDetailPathMinCost, originMapID) then
			tabDetailPathMinCost[originMapID] = {}
		end
		local tabA = tabDetailPathMinCost[originMapID]
		if not table.ContainsKey(tabMapOutterTeleport, originMapID) then
			tabMapOutterTeleport[originMapID] = {}
		end
		local tab1 = tabMapOutterTeleport[originMapID]
		if not table.ContainsKey(tab1, targetMapID) then
			tab1[targetMapID] = {}
		end
		local tab2 = tab1[targetMapID]
		if crossMapCount_PathDetailMinCost_Normal <= crossMapCount_PathDetailMinCost_NPC then
			tabA[targetMapID] = pathDetailMinCost_Normal

			local path = pathDetailMinCost_Normal.path
			local startMapDetail = path[1]
			local exitPointID = startMapDetail.exitPointID
			local secondMapDetail = path[2]
			local exitPointIDSecond = secondMapDetail.exitPointID
			local targetMapDetail = path[#path]
			local targetMapBornPointID = targetMapDetail.bornPointID

			if not table.ContainsKey(tab2, exitPointID) then
				tab2[exitPointID] = {}
			end
			local tab3 = tab2[exitPointID]
			tab3[targetMapBornPointID] = {totalCost = pathDetailMinCost_Normal.cost, nextEP = exitPointIDSecond}

			BuildOriginMapEPToTargetMapBP_Normal(path)
		else
			tabA[targetMapID] = pathDetailMinCost_NPC

			local path = pathDetailMinCost_NPC.path
			for i = 1, #path do
				local mapDetail = path[i]
				local tempNPCID = mapDetail.npcID
				if tempNPCID ~= nil then
					npcID = tempNPCID
					npcFromMapID = mapDetail.mapID
					npcToMapID = mapDetail.nextMapID
					tab2.transitMap = npcFromMapID
					tab2.transitNPC = npcID
					tab2.transitNPCToMap = npcToMapID
					break
				end
			end
		end

		-- caculate MIN cost ver.1
		-- -- used for find min cost
		-- local indicator = 0
		-- local minCost = 0
		-- local minCostExitPoint = 0
		-- for _, v in pairs(pathsDetail) do
		-- 	local pathDetail = v
		-- 	local isNeedNpcTeleport = false
		-- 	local npcID = 0
		-- 	local npcFromMapID = 0
		-- 	local npcToMapID = 0
		-- 	local path = pathDetail.path
		-- 	for i = 1, #path do
		-- 		local mapDetail = path[i]
		-- 		local tempNPCID = mapDetail.npcID
		-- 		if tempNPCID ~= nil then
		-- 			isNeedNpcTeleport = true
		-- 			npcID = tempNPCID
		-- 			npcFromMapID = mapDetail.mapID
		-- 			npcToMapID = mapDetail.nextMapID
		-- 			break
		-- 		end
		-- 	end
		-- 	local bPathIsMinCost = false
		-- 	local cost = pathDetail.cost
		-- 	if indicator == 0 then
		-- 		minCost = cost
		-- 		bPathIsMinCost = true
		-- 	else
		-- 		if cost < minCost then
		-- 			bPathIsMinCost = true
		-- 		end
		-- 	end
		-- 	indicator = indicator + 1
		-- 	if bPathIsMinCost then
		-- 		local startMapDetail = path[1]
		-- 		-- local npcIDFromStartMap = startMapDetail.npcID
		-- 		local exitPointID = startMapDetail.exitPointID
		-- 		-- local isNPCTeleport = npcIDFromStartMap ~= nil
		-- 		local secondMapDetail = path[2]
		-- 		-- local npcIDSecond = secondMapDetail.npcID
		-- 		-- local bornPointIDSecond = secondMapDetail.bornPointID
		-- 		local exitPointIDSecond = secondMapDetail.exitPointID

		-- 		if not table.ContainsKey(tabMapOutterTeleport, originMapID) then
		-- 			tabMapOutterTeleport[originMapID] = {}
		-- 		end
		-- 		local tab1 = tabMapOutterTeleport[originMapID]
		-- 		if not table.ContainsKey(tab1, targetMapID) then
		-- 			tab1[targetMapID] = {}
		-- 		end
		-- 		local tab2 = tab1[targetMapID]
		-- 		if isNeedNpcTeleport then
		-- 			tab2.transitMap = npcFromMapID
		-- 			tab2.transitNPC = npcID
		-- 			tab2.transitNPCToMap = npcToMapID
		-- 		else
		-- 			if minCostExitPoint > 0 then
		-- 				tab2[minCostExitPoint] = nil
		-- 			end
		-- 			minCostExitPoint = exitPointID
		-- 			tab2[exitPointID] = {}
		-- 			local tab3 = tab2[exitPointID]
		-- 			local targetMapDetail = path[#path]
		-- 			local targetMapBornPointID = targetMapDetail.bornPointID
		-- 			tab3[targetMapBornPointID] = {totalCost = cost, nextEP = exitPointIDSecond}
		-- 		end
		-- 	end
		-- end
	end
end

local str = 'Table_MapsNavPath={'
local isFirstItem = true
for k, v in pairs(tabDetailPathMinCost) do
	local mapID = k
	local toTargetMapsDetail = v
	if isFirstItem then
		str = str .. '\n'
		isFirstItem = false
	else
		str = str .. ',\n'
	end
	str = str .. '\t[' .. mapID .. ']=' .. SerializeWithoutNewline(toTargetMapsDetail)
end
str = str .. '\n}'
local fPath = nil
if runtimePlatform == 1 then
	fPath = '../../client-export/Table_MapsNavPath.txt'
elseif runtimePlatform == nil then
	fPath = './Table_MapsNavPath.txt'
end
WriteFile(fPath, str)

-- require ("LuaXML")
-- local xmlObj_AllMapsNavData = xml.new('AllMapsNavData')
-- local xmlObj_data = xmlObj_AllMapsNavData:append('data')
-- for k, v in pairs(tabDetailPathMinCost) do
-- 	local originMapID = k
-- 	local toTargetMapsPath = v
-- 	for k, v in pairs(toTargetMapsPath) do
-- 		local targetMapID = k
-- 		local pathDetal = v
-- 		local strPath = Serialize(pathDetal)
-- 		local tempXMLObj = xmlObj_data:append('MapsNavDataForEditor')
-- 		tempXMLObj:append('originMapID')[1] = originMapID
-- 		tempXMLObj:append('originMapName')[1] = Table_Map[originMapID].NameEn
-- 		tempXMLObj:append('targetMapID')[1] = targetMapID
-- 		tempXMLObj:append('targetMapName')[1] = Table_Map[targetMapID].NameEn
-- 		tempXMLObj:append('path')[1] = strPath
-- 	end
-- end
-- if runtimePlatform == 1 then
-- 	fPath = '../../client-export/MapsNavData.xml'
-- elseif runtimePlatform == nil then
-- 	fPath = './MapsNavData.xml'
-- end
-- xmlObj_AllMapsNavData:save(fPath)

str = '\nMapOutterTeleport={'
isFirstItem = true
for k, v in pairs(tabMapOutterTeleport) do
	local mapID = k
	local outterTeleport = v
	if isFirstItem then
		str = str .. '\n'
		isFirstItem = false
	else
		str = str .. ',\n'
	end
	str = str .. '\t[' .. mapID .. ']=' .. SerializeWithoutNewline(outterTeleport)
end
str = str .. '\n}'
-- str = str .. Serialize(tabMapOutterTeleport)
if runtimePlatform == 1 then
	fPath = './Assets/Resources/Script/MConfig/MapTeleport.txt'
elseif runtimePlatform == nil then
	fPath = '../client-refactory/Develop/Assets/Resources/Script/MConfig/MapTeleport.txt'
end
AppendFile(fPath, str)
print("Success.")