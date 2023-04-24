local posRecord = string.len(package.path)

package.path = string.sub(package.path, 1, posRecord)
package.path = package.path .. ';./?.txt'
require 'ReferenceFilesName'

package.path = string.sub(package.path, 1, posRecord)
package.path = package.path .. ';../client-refactory/Develop/Assets/Resources/Script/MConfig/?.txt'
require("Table_MapIDs")

package.path = string.sub(package.path, 1, posRecord)

local tabOut = {}
-- structure
-- tabOut = {[1] = {{idMirror = 1, mapID = 1, pos = {1, 1, 1}}}

local tabMapNameAndID = Table_MapIDs
-- structure
-- tabMapNameAndID = {[mapName] = mapID}

local Table_MapInfo = {}
-- structure
-- Table_MapInfo = {[mapID] = {units = {[1] = {1, 2, ...}, [2] = {1, 2, ...}, ...}, exitPoints = {1, 2, ...}}}, ...}

function InsertIntoTabOut(iUnitID, tabUnitInfo)
	if tabUnitInfo and (not table.IsEmpty(tabUnitInfo)) then
		if table.ContainsKey(tabOut, iUnitID) then
			local v = tabOut[iUnitID]
			if v == nil then v = {} end
			table.insert(v, tabUnitInfo)
			tabOut[iUnitID] = v
		else
			tabOut[iUnitID] = {tabUnitInfo}
		end
	end
end

function SearchNPCPoints(tabNPCPoints, strMapName)
	if tabNPCPoints and not table.IsEmpty(tabNPCPoints) then
		for _, v in pairs(tabNPCPoints) do
			if v then
				local tabUnitInfo = {}
				local unitID = v['id']
				if unitID and unitID > 0 then
					local mapName = strMapName
					local mapID = tabMapNameAndID[mapName]
					if mapID and mapID > 0 then
						local unitIDShadow = v['uniqueid']
						local pos = v['pos']
						tabUnitInfo = {['idMirror'] = unitIDShadow, ['mapID'] = mapID, ['pos'] = pos}
						InsertIntoTabOut(unitID, tabUnitInfo)
					end
				end
			end
		end
	end
end

function SearchPVPNPCPoints(tabNPCPoints, strMapName)
	SearchNPCPoints(tabNPCPoints, strMapName .. 'P')
end

function InsertUnitIntoMapInfo(iUnitID, iUnitIDShadow, iMapID)
	if iUnitID > 0 and iMapID > 0 then
		local mapIDIsExist = false
		if table.ContainsKey(Table_MapInfo, iMapID) then
			mapIDIsExist = true
			local mapInfo = Table_MapInfo[iMapID]
			local units = mapInfo.units
			units = units or {}
			if not table.ContainsKey(units, iUnitID) then
				units[iUnitID] = {iUnitIDShadow}
			else
				table.insert(units[iUnitID], iUnitIDShadow)
			end
			mapInfo.units = units
		end
		if not mapIDIsExist then
			local mapInfo = {['units'] = {[iUnitID] = {iUnitIDShadow}}}
			Table_MapInfo[iMapID] = mapInfo
		end
	end
end

function InsertExitPointIntoMapInfo(iExitPointID, iMapID)
	local b = (iMapID == 15)
	if iExitPointID > 0 and iMapID > 0 then
		local mapIDIsExist = false
		if table.ContainsKey(Table_MapInfo, iMapID) then
			mapIDIsExist = true
			local mapInfo = Table_MapInfo[iMapID]
			local exitPoints = mapInfo.exitPoints
			exitPoints = exitPoints or {}
			if not table.ContainsValue(exitPoints, iExitPointID) then
				table.insert(exitPoints, iExitPointID)
				mapInfo.exitPoints = exitPoints
			end
		end
		if not mapIDIsExist then
			local mapInfo = {['exitPoints'] = {iExitPointID}}
			Table_MapInfo[iMapID] = mapInfo
		end
	end
end

function SearchNPCPointsReal(tabNPCPoints, strMapName)
	if tabNPCPoints and not table.IsEmpty(tabNPCPoints) then
		for _, v in pairs(tabNPCPoints) do
			if v then
				local npcID = v.id
				if npcID and npcID > 0 then
					local mapID = tabMapNameAndID[strMapName]
					if mapID and mapID > 0 then
						local idMirror = v['uniqueid']
						InsertUnitIntoMapInfo(npcID, idMirror, mapID)
					end
				end
			end
		end
	end
end

function SearchPVPNPCPointsReal(tabNPCPoints, strMapName)
	SearchNPCPointsReal(tabNPCPoints, strMapName .. 'P')
end

function SearchExitPointsReal(tabExitPoints, strMapName)
	if tabExitPoints and not table.IsEmpty(tabExitPoints) then
		for _, v in pairs(tabExitPoints) do
			if v then
				local exitPointID = v.next_scene_ID
				if exitPointID and exitPointID > 0 then
					local mapID = tabMapNameAndID[strMapName]
					if mapID and mapID > 0 then
						InsertExitPointIntoMapInfo(exitPointID, mapID)
					end
				end
			end
		end
	end
end

function SearchPVPExitPointsReal(tabExitPoints, strMapName)
	SearchExitPointsReal(tabExitPoints, strMapName .. 'P')
end

function Search(strMapName)
	if not table.ContainsKey(tabMapNameAndID, strMapName) then return end

	local tabNPCPoints = Root['NPCPoints']
	SearchNPCPoints(tabNPCPoints, strMapName)
	SearchNPCPointsReal(tabNPCPoints, strMapName)

	local tabExitPoints = Root['ExitPoints']
	SearchExitPointsReal(tabExitPoints, strMapName)

	local tabRaids = Root['Raids']
	if tabRaids then
		tabNPCPoints = tabRaids['NPCPoints']
		SearchNPCPoints(tabNPCPoints, strMapName)
		SearchNPCPointsReal(tabNPCPoints, strMapName)

		tabNPCPoints = tabRaids['RaidNPCPoints']
		SearchNPCPoints(tabNPCPoints, strMapName)
		SearchNPCPointsReal(tabNPCPoints, strMapName)

		tabExitPoints = tabRaids['ExitPoints']
		SearchExitPointsReal(tabExitPoints, strMapName)
	end

	local tabPVP = Root['PVP']
	if tabPVP then
		tabNPCPoints = tabPVP['NPCPoints']
		SearchPVPNPCPoints(tabNPCPoints, strMapName)
		SearchPVPNPCPointsReal(tabNPCPoints, strMapName)

		tabExitPoints = tabPVP['ExitPoints']
		SearchPVPExitPointsReal(tabExitPoints, strMapName)
	end
end

-- Root = {BornPoints={}, ExitPoints={}, NPCPoints={}, ScenicSpots={}, Raids={[1]={BornPoints={}, ExitPoints={}, NPCPoints={}, ScenicSpots={}, RaidNPCPoints={}}}, PVP = {BornPoints={}, ExitPoints={}, NPCPoints={}, ScenicSpots={}}}
-- Raids means fuben
local tabSceneName = Table_ScenesName
for _, v in pairs(tabSceneName) do
	require('./Scene/Scene' .. v .. '/SceneInfo')
	Search(v)
end
for _, v in pairs(Table_Boss) do
	local tabUnitInfo = {}
	local unitID = v['id']
	if unitID and unitID > 0 then
		local sMapID = v['Map']
		for i = 1, #sMapID do
			local mapID = sMapID[i]
			if mapID and mapID > 0 then
				tabUnitInfo['mapID'] = mapID
				tabUnitInfo['pos'] = {}
				InsertIntoTabOut(unitID, tabUnitInfo)
				InsertUnitIntoMapInfo(unitID, nil, mapID)
			end
		end
	end
end

-- TableUtil.Print(tabOut)
local str = Serialize(tabOut)
str = 'Table_MonsterOrigin =\n' .. str
local fPath = '../client-refactory/Develop/Assets/Resources/Script/MConfig/Table_MonsterOrigin.txt'
WriteFile(fPath, str)

-- TableUtil.Print(Table_MapInfo)
str = Serialize(Table_MapInfo)
str = 'Table_MapInfo =\n' .. str
fPath = '../client-refactory/Develop/Assets/Resources/Script/MConfig/Table_MapInfo.txt'
WriteFile(fPath, str)
print("Success.")