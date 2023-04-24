ServerGame.MvpScore = {
  noticeTime = 3,
  validTime = 300,
  damageScore = {9, 6, 4, 4, 3, 3, 3},
  beLockScore = {5, 3, 1, 1},
  healScore = {3, 2, 2, 1, 1, 1, 1},
  rebirthScore = {1, 1},
  deadHitScore = {10},
  deadHitTime = 1000,
  firstHitScore = 3,
  damageDecScore = {{per = 36, dec = 1}, {per = 20, dec = 2},{per = 10, dec = 8}}, -- 伤害低于总伤害百分比 扣分
  
  showNames = {
    top1damage = {name="最高伤害", show_order=1},
    damage = {name="伤害输出", show_order=2},
    belock = {name="吸引火力",show_order=3},
    heal = {name="有效治疗",show_order=4},
    rebirth = {name="复活玩家",show_order=5},
    deadhit ={name="致命一击",show_order=6},
    firsthit = {name="最先参战",show_order=7},
    breakskill = {name="打断技能",show_order=8},
  }
}
