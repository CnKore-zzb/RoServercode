local posRecord = string.len(package.path)

if runtimePlatform == 1 then
	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';../../client-export/?.txt'
	require 'ReferenceFilesName'
elseif runtimePlatform == nil then
	package.path = string.sub(package.path, 1, posRecord)
	package.path = package.path .. ';./?.txt'
	require 'ReferenceFilesName'
end

package.path = string.sub(package.path, 1, posRecord)

local tabMapNameAndID = {}
-- format
-- tabMapNameAndID = {[mapName] = mapID}
if Table_Map then
	for _, v in pairs(Table_Map) do
		if v then
			local mapName = v.NameEn
			if mapName and string.len(mapName) > 0 then
				local mapID = v.id
				if mapID and mapID > 0 then
					local mapMode = v.Mode
					if mapMode then
						if mapMode == 2 then
							tabMapNameAndID[mapName .. 'P'] = mapID
						elseif mapMode == 3 then
							tabMapNameAndID[mapName .. 'R'] = mapID
						elseif mapMode == 1 then
							tabMapNameAndID[mapName] = mapID
						end
					end
				end
			end
		end
	end
end

local str = Serialize(tabMapNameAndID)
str = 'Table_MapIDs=' .. str
local fPath = nil
if runtimePlatform == 1 then
	fPath = './Assets/Resources/Script/MConfig/Table_MapIDs.txt'
elseif runtimePlatform == nil then
	fPath = '../client-refactory/Develop/Assets/Resources/Script/MConfig/Table_MapIDs.txt'
end
WriteFile(fPath, str)
print("Success.")