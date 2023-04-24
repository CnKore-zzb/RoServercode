local posRecord = string.len(package.path)
package.path = package.path .. ';../client-refactory/Develop/Assets/Resources/Script/MConfig/?.txt'
require("Table_Map")
package.path = string.sub(package.path, 1, posRecord)
package.path = package.path .. ';../client-refactory/Develop/Assets/Resources/Script/MConfig/?.txt'
require("Table_MapIDs")
package.path = string.sub(package.path, 1, posRecord)
package.path = package.path .. ';../client-refactory/Develop/Assets/Resources/Script/ConfigTool/?.txt'
require('ConfGeneratorUtil')
package.path = string.sub(package.path, 1, posRecord)
package.path = package.path .. ';../client-refactory/Develop/Assets/Resources/Script/MConfig/?.txt'
require('MapTeleport')
require('Table_ScenesName')
package.path = string.sub(package.path, 1, posRecord)
package.path = package.path .. ';../client-refactory/Develop/Assets/Resources/Script/Util/?.txt'
require('TableUtil')

local tabMapValidExitPoints = {}

local directoryName = ""
local directoryPrefixName = "Scene"
local tabDirectorySuffixName = Table_ScenesName
for _, v in pairs(tabDirectorySuffixName) do
	local directorySuffixName = v
	directoryName = directoryPrefixName .. directorySuffixName
	require("Scene/" .. directoryName .. "/SceneInfo")

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
				local fromMapInfo = {mapID = mapIDLatter, bornPointID = nextMapInfo.bornPointID, exitPointID = nextMapInfo.exitPointID}
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
		retValue = toMaps
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
	return retValue
end

local tabPaths = {}
for _, v in pairs(_allMapsID) do
	local mapID = v
	local originMap = mapID
	local targetMaps = {}
	for _, v in pairs(_allMapsID) do
		local mapIDLatter = v
		if mapIDLatter ~= originMap then
			table.insert(targetMaps, mapIDLatter)
		end
	end
	local tabTemp1 = {{mapID = originMap, nodePath = {{mapID = originMap}}, cost = 0}}
	local tabTemp2 = {}
	for _, v in pairs(targetMaps) do
		table.insert(tabTemp2, v)
	end
	local b = true
	while (b)
	do
		local tempNodes = {}
		local nodePath = {}
		for _, v in pairs(tabTemp1) do
			local nodeMapID = v.mapID
			local nodePath = v.nodePath
			local nodeCost = v.cost
			local toMaps = GetToMapsInfo(nodeMapID)
			for _, v in pairs(toMaps) do
				local toMap = v
				local toMapID = toMap.mapID
				local isExistTabTemp1 = false
				for _, v in pairs(tabTemp1) do
					local node = v
					if node.mapID == toMapID then
						isExistTabTemp1 = true
					end
				end
				local isExistTabTemp2 = false
				local existIndex = 0
				for i = 1, #tabTemp2, 1 do
					local mapIDUnused = tabTemp2[i]
					if mapIDUnused == toMapID then
						isExistTabTemp2 = true
						existIndex = i
					end
				end
				if not isExistTabTemp1 and isExistTabTemp2 then
					local newNodePath = {}
					for _, v in pairs(nodePath) do
						local node = v
						local nodeBornPointID = node.bornPointID
						local nodeExitPointID = node.exitPointID
						local newNode = {mapID = node.mapID}
						if nodeBornPointID ~= nil then
							newNode.bornPointID = nodeBornPointID
						end
						if nodeExitPointID ~= nil then
							newNode.exitPointID = nodeExitPointID
						end
						table.insert(newNodePath, newNode)
					end
					newNodePath[#newNodePath].exitPointID = toMap.exitPointID
					table.insert(newNodePath, {mapID = toMapID, bornPointID = toMap.bornPointID})
					local innerCost = 0
					if nodeMapID ~= originMap then
						innerCost = GetBPToEPTotalCost(nodeMapID, nodePath[#nodePath].bornPointID, toMap.exitPointID)
					end
					if innerCost ~= nil then
						local newCost = innerCost + 24
						local newNode = {mapID = toMapID, nodePath = newNodePath, cost = newCost}
						tempNodes[existIndex] = newNode
					end
				end
			end
		end
		local costMinNode = nil
		local indexInTabTemp2 = 0
		for k, v in pairs(tempNodes) do
			local tempNode = v
			if costMinNode == nil then
				costMinNode = tempNode
				indexInTabTemp2 = k
			else
				if tempNode.cost < costMinNode.cost then
					costMinNode = tempNode
					indexInTabTemp2 = k
				end
			end
		end
		if costMinNode == nil then
			b = false
		else
			table.insert(tabTemp1, costMinNode)
			table.remove(tabTemp2, indexInTabTemp2)
		end
	end
	tabPaths[originMap] = tabTemp1
end

local tabMapOutterTeleport = {}
for k, v in pairs(tabPaths) do
	local originMapID = k
	local toTargetMapsInfo = v
	tabMapOutterTeleport[originMapID] = {}
	for _, v in pairs(toTargetMapsInfo) do
		local toTargetMapInfo = v
		local targetMapID = toTargetMapInfo.mapID
		if targetMapID ~= originMapID then
			local nodePath = toTargetMapInfo.nodePath
			local originMapInfo = nodePath[1]
			local originMapExitPointID = originMapInfo.exitPointID
			local targetMapInfo = nodePath[#nodePath]
			local targetMapBornPointID = targetMapInfo.bornPointID
			local nextMapInfo = nodePath[2]
			local nextMapExitPointID = nextMapInfo.exitPointID
			local cost = toTargetMapInfo.cost
			local tab1 = {nextEP = nextMapExitPointID, totalCost = cost}
			local tab2 = {}
			tab2[targetMapBornPointID] = tab1
			local tab3 = {}
			tab3[originMapExitPointID] = tab2
			tabMapOutterTeleport[originMapID][targetMapID] = tab3
		end
	end
end

local str = Serialize(tabMapOutterTeleport)
str = '\nMapOutterTeleport =' .. str
local fPath = '../client-refactory/Develop/Assets/Resources/Script/MConfig/MapTeleport.txt'
AppendFile(fPath, str)
print("Success.")

-- not > == > and