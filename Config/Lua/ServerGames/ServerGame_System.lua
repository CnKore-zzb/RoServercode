ServerGame.System = {
  valid_pos_check = 1,
  valid_pos_radius = 5,
  max_extra_add_battletime = 36000,  --可累积的最大战斗时长
  max_mercenary_cat_num = 4,  --可以携带的猫数量上限
  debt_mail_id = 10003,     --进入负债发送邮件Id
  sell_limit_warning = 1000000,
  npc_fade_out_time = 2,
  kill_monster_period = 60,  ---战斗效率改动
  shop_random_cnt = 50,    --稀有商店每日随机多少次
  open_tower_layer = 70,    --时间胶囊爬塔的开放层数
  gift_msg_1 = "[礼包码] 查找玩家accid 失败,数据库找不到",
  gift_msg_2 = "[礼包码] 查找玩家accid 失败。",
  gift_msg_3 = "[礼包码] 数据库异常",
  gift_msg_4 = "[礼包码] 对应码不存在",
  gift_msg_5 = "[礼包码] 已经过期",
  gift_msg_6 = "[礼包码] 超出服使用范围",
  gift_msg_7 = "[礼包码] 已经被使用",
  gift_msg_8 = "[礼包码] 礼包码已达到最大使用次数,已使用次数：",
  gift_msg_9 = "批次最大使用次数：",
  gift_msg_10 = "[礼包码] 您已经领取过该批次的兑换码",
  gift_msg_11 = "[礼包码] 错误的物品配置",
  gift_msg_12 = "[礼包码] 恭喜您，礼包码兑换成功,请注意查收邮件",
  gift_msg_13 = "[礼包码] 礼包码兑换失败",
  max_seat_dis = 20,  --优雅坐下最大距离
  monster_kill_statnum = 40,  --玩家一分钟杀怪，达到此数量是，记录玩家杀怪数量
  
  map_group_num =   --同屏显示的地图编号与最大人数
  {
  [2]=60,         
  },
}
