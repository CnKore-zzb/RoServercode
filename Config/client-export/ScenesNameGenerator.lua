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

-- local lfs = require"lfs"

local scenePath = nil
if runtimePlatform == 1 then
    scenePath = '../../client-export/Scene'
elseif runtimePlatform == nil then
    scenePath = './Scene'
end

local tabScenesName = {}
-- for fileName in lfs.dir(scenePath) do
--     if fileName ~= '.' and fileName ~= '..' then
--         local firstPosition, lastPosition = string.find(fileName, 'Scene')
--         if firstPosition == 1 then
--             local scenesName = string.sub(fileName, lastPosition + 1)
--             table.insert(tabScenesName, scenesName)
--         end
--     end
-- end
local popen = io.popen
local pfile = popen('ls -a "' .. scenePath .. '"')
for fileName in pfile:lines() do
    local firstPosition, lastPosition = string.find(fileName, 'Scene')
    if firstPosition == 1 then
        local scenesName = string.sub(fileName, lastPosition + 1)
        table.insert(tabScenesName, scenesName)
    end
end

local str = 'Table_ScenesName='
str = str .. Serialize(tabScenesName)

local fPath = nil
if runtimePlatform == 1 then
    fPath = './Assets/Resources/Script/MConfig/Table_ScenesName.txt'
elseif runtimePlatform == nil then
    fPath = '../client-refactory/Develop/Assets/Resources/Script/MConfig/Table_ScenesName.txt'
end
WriteFile(fPath, str)
print('success.')