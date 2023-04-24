local time1 = os.clock();
collectgarbage("collect");
local gabage1 = collectgarbage("count");

local posRecord = string.len(package.path)

package.path = string.sub(package.path, 1, posRecord)

-- string luaFilePath = Application.dataPath + "/../../../client-export/Config_DialogSplit.lua";
print("runtimePlatform:", runtimePlatform)

package.path = package.path .. ';./?.txt' .. ';../Cehua/Lua/Table/?.txt'
package.path = package.path .. ';../Cehua/Lua/SecondaryConfigureGenerator/?.txt'

local nowfile_path = debug.getinfo(1,'S').source:sub(2)
nowfile_path = string.sub(nowfile_path, 1, string.len(nowfile_path)-22);

package.path = package.path .. ';' .. nowfile_path .. '?.txt' 
package.path = package.path .. ';' .. nowfile_path .. '../Cehua/Lua/Table/?.txt'
package.path = package.path .. ';' .. nowfile_path .. '../Cehua/Lua/Table/lua_server/?.txt'
package.path = package.path .. ';' .. nowfile_path .. '../Cehua/Lua/SecondaryConfigureGenerator/?.txt'



require 'ConfGeneratorUtil'
require 'Table_NpcTalk'
require 'Table_Npc'
require 'Table_Dialog'


function string_split(str, delimiter)
	str = tostring(str)
	delimiter = tostring(delimiter)
	if (delimiter=='') then return false end
	local pos,arr = 0, {}
	-- for each divider found
	for st,sp in function() return string.find(str, delimiter, pos, true) end do
		table.insert(arr, string.sub(str, pos, st - 1))
		pos = sp + 1
	end
	table.insert(arr, string.sub(str, pos))
	return arr
end

local removal_map = {};
local empty_table = {};
function CopyDialog(dialogId)
	if(dialogId == nil)then
		return;
	end
	local src = Table_Dialog[ dialogId ];
	if(src == nil or removal_map[dialogId] ~= nil)then
		return;
	end

	removal_map[dialogId] = 1;

	local copy = {};
	for k,v in pairs(src)do
		if(type(v) == "table")then
			if(next(v) and v ~= "_EmptyTable")then
				copy[k] = {};
				for k1,v1 in pairs(v)do
					copy[k][k1] = v1;
				end
			end
		elseif(type(v) == "string")then
			if(v ~= "")then
				copy[k] = v;
			end
		else
			copy[k] = v;
		end
	end
	return copy;
end

local table_insert = table.insert;
function Dialog_Serialize(obj, depth)
	depth = depth or 1;

	local t = type(obj)
	if(t == "number")then
		return obj;
	end
	if(t == "string")then
		obj = string.gsub(obj, '\\', '\\\\');
		obj = string.gsub(obj, '\n', '\\n');
		return "\'" .. tostring(obj) .. "\'";
	end
	if(t == "table")then
		local str_array = {};

		table_insert(str_array, "{");

		local keys = {};
		for k, v in pairs(obj) do
			table.insert(keys, k);
		end
		table.sort(keys, function (a, b)
			return a > b;
		end)
		for i=1,#keys do
			local k,v = keys[i], obj[ keys[i] ];

			local strK

			local kType = type(k)
			if kType == "string" then
				strK = tostring(k)
			elseif kType == "number" then
				strK = '[' .. k .. ']'
			end

			if(depth == 1)then
				strK = "\n\t" .. strK
			end

			table_insert(str_array, " ");
			table_insert(str_array, strK);
			table_insert(str_array, ' = ');
			table_insert(str_array, Dialog_Serialize(v, depth + 1));
			table_insert(str_array, ",");
		end
			
		if(#keys == 0)then
			return "_EmptyTable";
		end

		if(depth == 1)then
			table_insert(str_array, "\n");
		end

		table_insert(str_array, "}");

		return table.concat(str_array);
	end
	if(t == "boolean")then
		return tostring(obj);
	end
	if(depth == 1)then
		return "_EmptyTable";
	end
	return "nil"
end


local Target_FilePath = nowfile_path .. "../client-refactory/Develop/Assets/Resources/Script/MConfig/Dialog/";

local commond_filePath = Target_FilePath;
if(runtimePlatform == "WindowsEditor")then
	commond_filePath = string.gsub(nowfile_path, "/", "\\");
	commond_filePath = commond_filePath .. "..\\client-refactory\\Develop\\Assets\\Resources\\Script\\MConfig\\Dialog";
end

local file = io.open(Target_FilePath);
if file then 
	file:close() 
else
	if(runtimePlatform == "WindowsEditor")then
		os.execute("md " .. commond_filePath) --unix,linux like
	else
		os.execute("mkdir -p " .. commond_filePath) --unix,linux like
	end
end

-- 清空拆分目录
local log_fPath = nowfile_path .. "log.txt";
if(runtimePlatform == "WindowsEditor")then
	os.execute('dir ' .. commond_filePath .. " >" .. log_fPath)
else
	os.execute('ls ' .. commond_filePath .. " >" .. log_fPath)
end
local logfile = io.open(log_fPath);
local filenames = logfile:read("*a");
filenames = string_split(filenames, "\n");
for i=1,#filenames-1 do
	os.remove(Target_FilePath .. filenames[i]);	
end


local combine_keys = {"Text", "Option", "Action", "Voice"};
function Write_DialogFile(t, filename)
	local t_text = filename .. ' =\n' .. Dialog_Serialize(t, 1)

	local final_text = 
	[=[
%s
%s_fields = {"id", "Speaker", "Text", "Option", "Emoji", "Action", "Voice", "SubViewId" }
return %s
	]=];
		final_text = string.format(final_text, 
			t_text,
			filename,
			filename);

	local fPath = Target_FilePath .. filename .. '.txt'
	WriteFile(fPath, final_text)
end

-- Dialog索引表
local dialog_indexmap = {};

-- 1. 优先提取NpcTalk表中的Dialog， 建表Dialog_NpcTalk
local npctalk_filename = "Table_Dialog_NpcTalk";
local npc_talkmap = {};
for k,v in pairs(Table_NpcTalk)do
	local talkids = v.NormalTalk;
	for i=1,#talkids do
		local id = talkids[i];
		local cpy = CopyDialog(id);
		if(cpy ~= nil)then
			npc_talkmap[id] = cpy;

			dialog_indexmap[id] = npctalk_filename;
		end
	end
end
Write_DialogFile(npc_talkmap, npctalk_filename);


-- 2. 提取Npc表中所有的DefaultDialog，建表Dialog_NpcDefault
local npcdefault_filename = "Table_Dialog_NpcDefault";
local defaultDialog_map = {};
for k,v in pairs(Table_Npc)do
	local defaultDialog = v.DefaultDialog;
	if(defaultDialog)then
		local cpy = CopyDialog(defaultDialog);
		if(cpy ~= nil)then
			defaultDialog_map[defaultDialog] = cpy;

			dialog_indexmap[defaultDialog] = npcdefault_filename;
		end
	end
end
Write_DialogFile(defaultDialog_map, npcdefault_filename);

-- 3.根据Quest表格的划分进行dialog的划分
for i=1,#quest_filenames do
	local file_name = quest_filenames[i];
	require (file_name)

	local questIndex = tonumber(string.sub(file_name, 13)) or 1;
	local save_filename = "Table_Dialog_Quest" .. questIndex;

	local t_quest = _G[file_name];
	local quest_DialogMap = {};
	for k, v in pairs(t_quest)do
		local dialogs = v.Params and v.Params.dialog;
		if(type(dialogs) == "table")then
			for j=1,#dialogs do
				local did = dialogs[j];
				if(quest_DialogMap[did] == nil)then
					local cpy = CopyDialog(did);
					if(cpy ~= nil)then
						quest_DialogMap[did] = cpy;

						dialog_indexmap[did] = save_filename;
					end
				end
			end
		elseif(type(dialogs) == "number")then
			local did = dialogs;
			if(quest_DialogMap[did] == nil)then
				local cpy = CopyDialog(did);
				if(cpy ~= nil)then
					quest_DialogMap[did] = cpy;

					dialog_indexmap[did] = save_filename;
				end
			end
		end
	end
	
	Write_DialogFile(quest_DialogMap, save_filename);
end


-- 4.遗漏未划分的Dialog建表 Table_Dialog_Left
local left_filename = "Table_Dialog_Left";
local left_dialogmap = {};
for k, v in pairs(Table_Dialog)do
	local cpy = CopyDialog(k);
	if(cpy ~= nil)then
		left_dialogmap[k] = cpy;

		dialog_indexmap[k] = left_filename;
	end
end
Write_DialogFile(left_dialogmap, left_filename);



local index_filename = "Dialog_Index";
local index_fPath = Target_FilePath .. index_filename .. '.txt'
local str = index_filename .. ' =\n' .. Dialog_Serialize(dialog_indexmap, 1)
WriteFile(index_fPath, str)


package.path = string.sub(package.path, 1, posRecord)




local time2 = os.clock();
local gabage2 = collectgarbage("count");
print("Success!", "耗时：" .. time2 - time1 .. "s", "内存：" .. gabage2 - gabage1 .. "kb");
