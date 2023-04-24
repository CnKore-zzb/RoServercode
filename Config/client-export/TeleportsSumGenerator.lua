local posRecord = string.len(package.path)

if runtimePlatform == 1 then
	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';../../client-export/?.txt'
	require 'ReferenceFilesName'

	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';./Assets/Resources/Script/MConfig/?.txt'
	require("Table_MapIDs")
elseif runtimePlatform == nil then
	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';./?.txt'
	require 'ReferenceFilesName'

	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';../client-refactory/Develop/Assets/Resources/Script/MConfig/?.txt'
	require("Table_MapIDs")
end

package.path = string.sub(package.path, 1, posRecord)

local tabInnerTeleports = {}

function PutIntoTabInner(tab_all_inner, tab_inner)
	if tab_inner ~= nil then
		for k, v in pairs(tab_inner) do
			tab_all_inner[k] = v
		end
	end
end

function PutIntoTabOutter(tab_all_outter, tab_outter)
	if tab_outter ~= nil then
		for k, v in pairs(tab_outter) do
			tab_all_outter[k] = v
		end
	end
end

function LoopAndCollect(str_map_en_name)
	local mapID = 0
	local tabPVE = Root.PVE
	if tabPVE ~= nil then
		local mapENName = str_map_en_name
		if table.ContainsKey(Table_MapIDs, mapENName) then
			mapID = Table_MapIDs[mapENName]
			local tabPVEInner = tabPVE.inner
			local tabPVEOutter = tabPVE.outter
			tabInnerTeleports[mapID] = {inner = tabPVEInner, outter = tabPVEOutter}
		end
	end
	local tabPVP = Root.PVP
	if tabPVP ~= nil then
		local mapENName = str_map_en_name .. 'P'
		if table.ContainsKey(Table_MapIDs, mapENName) then
			mapID = Table_MapIDs[mapENName]
			local tabPVPInner = tabPVP.inner
			local tabPVPOutter = tabPVP.outter
			tabInnerTeleports[mapID] = {inner = tabPVPInner, outter = tabPVPOutter}
		end
	end
	local tabRaids = Root.Raids
	if tabRaids ~= nil then
		for _, v in pairs(tabRaids) do
			local tabRaid = v
			mapID = tabRaid.ID
			local tabRaidsInner = tabRaid.inner
			local tabRaidsOutter = tabRaid.outter
			tabInnerTeleports[mapID] = {inner = tabRaidsInner, outter = tabRaidsOutter}
		end
	end
end

local tabSceneName = Table_ScenesName
for _, v in pairs(tabSceneName) do
	local path = nil
	if runtimePlatform == 1 then
		path = '../../client-export/Scene/Scene' .. v .. '/TeleportInfo.lua'
	elseif runtimePlatform == nil then
		path = './Scene/Scene' .. v .. '/TeleportInfo'
	end
	local bFileExists = nil
	if runtimePlatform == 1 then
		bFileExists = FileExists(path)
	elseif runtimePlatform == nil then
		bFileExists = FileExists(path .. '.lua')
	end
	if bFileExists then
		require(path)
		local strMapENName = v
		LoopAndCollect(strMapENName)
	end
end

local str = 'MapInnerTeleport={'
local isFirstItem = true
for k, v in pairs(tabInnerTeleports) do
	local mapID = k
	local innerTeleport = v
	if isFirstItem then
		str = str .. '\n'
		isFirstItem = false
	else
		str = str .. ',\n'
	end
	str = str .. '\t[' .. mapID .. ']=' .. SerializeWithoutNewline(innerTeleport)
end
str = str .. '\n}'
-- str = str .. Serialize(tabInnerTeleports)
local fPath = nil
if runtimePlatform == 1 then
	fPath = './Assets/Resources/Script/MConfig/MapTeleport.txt'
elseif runtimePlatform == nil then
	fPath = '../client-refactory/Develop/Assets/Resources/Script/MConfig/MapTeleport.txt'
end
WriteFile(fPath, str)
print("Success.")