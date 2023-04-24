--[[
--服务器分支配置
--具体分支 见branch.lua
--所有配置各个分支都要
--内网 BranchConfig.Debug
--预言之地 BranchConfig.TF
--正式服 BranchConfig.Publish
--]]

BranchConfig = {}

BranchConfig.Debug = {
  tapdb_appid = "mstl7ve57kljoncq",
  --plat_ip = "127.0.0.1",
  --plat_port = 2121,
}
BranchConfig.TF = {
  tapdb_appid = "mstl7ve57kljoncq",
  trade_give_starttime = "2020-02-16 19:00:00",
  --plat_ip = "192.168.200.128",
  --plat_port = 2121,
}
BranchConfig.Publish = {
  tapdb_appid = "s8nltyei9wt4ckxv",
  --plat_ip = "192.168.200.128",
  --plat_port = 2121,
}

BranchConfig.Debug.DataBase = {
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
--  {ip = "172.26.24.122", port = "4000", user = "root", password = ""},
--  {ip = "172.26.24.123", port = "4000", user = "root", password = ""},
--  {ip = "172.26.24.124", port = "4000", user = "root", password = ""},
}
BranchConfig.Debug.TradeDataBase = {
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
}
BranchConfig.Debug.RollbackDataBase = {
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
}
BranchConfig.Debug.Redis = {
  ip = "127.0.0.1", port = "6379", password = ""
}

BranchConfig.TF.DataBase = {
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
}
BranchConfig.TF.TradeDataBase = {
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
}
BranchConfig.TF.RollbackDataBase = {
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
}
BranchConfig.TF.Redis = {
  ip = "127.0.0.1", port = "6379", password = ""
}

BranchConfig.Publish.DataBase = {
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
}
BranchConfig.Publish.TradeDataBase = {
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
}
BranchConfig.Publish.RollbackDataBase = {
  {ip = "127.0.0.1", port = "3306", user = "rogame", password = "gJFAaqVNM3Nybs3J"},
}
BranchConfig.Publish.Redis = {
  ip = "127.0.0.1", port = "6379", password = ""
}
