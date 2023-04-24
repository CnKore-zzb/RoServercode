--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--奥特曼副本
ServerGame.Altman = {
  trans_buff = {4034},
  default_skill = 910001,
  max_skill_pos= 6,
  raid_id = 601001,
  rewardid = 1951,
  clear_npc_time = 5,

  rewardbox = {id=1209, scale={2.5, 2.5}, dir=180, behavior=1024},  --掉落宝箱定义
  head = {45617, 51144, 45600, 45601, 45602, 47084, 47085, 47086, 51132, 51152, 48030, 47087, 47088, 45603, 45604, 45605,51154,145603,145604,145605},
  extra_head_reward = {{num=1,rewardid=1947},{num=3,rewardid=1948},{num=5,rewardid=1949},{num=7,rewardid=1950},{num=10,rewardid=1951}},
  kill_reward = {{num=30,rewardid=12110},{num=60,rewardid=12111},{num=90,rewardid=12112},{num=120,rewardid=12113}}, --填写邮件ID
  fashion_equip = {51125,51148},
  fashion_buff =6046,

  item_skill = {      --道具类型为73，道具对应的技能ID
    [710146] = 911001,
    [710147] = 912001,
    [710148] = 913001,
    [710149] = 914001,
    [710150] = 915001,
    [710151] = 916001,
  },

  -- 奥特曼副本杀怪等级描述
  kill_rank_desc = {
    [1] = { title = "星云指挥官", killcount = 120 },
    [2] = { title = "异星监测员", killcount = 90 },
    [3] = { title = "恒星观察员", killcount = 60 },
    [4] = { title = "行星巡逻员", killcount = 30 },
  },
}

