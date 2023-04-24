--道具获取上限对应提示
ServerGame.GetLimitMsg = {
     [1] = 64,
     [7] = 63,
 }

--物品
ServerGame.Item = {
  kapula_map_item = 5040,       -- 卡普拉传送代扣券
  kapula_store_item = 5041,     -- 卡普拉仓库代扣券
  item_pickup_mode = 1,         -- 拾取模式
  equip_upgrade_refinelv_dec = 2,  -- 升级后精炼回退2个等级 
  extra_hint_item = {5501},     -- 额外提示物品
  temp_pack_overtime_itemcount = 10,  --临时背包进入倒计时个数
  no_color_machine_mount = {},   --------变机甲时，穿戴这些机甲无视职业服装变色
  machine_mount = {25400,25401},       --------所有机甲
}

ServerGame.ItemCheckDel = {
  {id = 25102, type = "Level", param = 15},{id = 25107, type = "Level", param = 15}
}

--分解时的金属价值计算
ServerGame.Decompose = {
  price = {
    [52821] = 50000,
  }
}
