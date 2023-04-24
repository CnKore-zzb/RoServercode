--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

ServerGame.EffectPath = {
  leavescene = { effect = "Skill/Teleport", sound = "Common/Teleport" },
  enterscene = { effect = "Common/15EnemyBirth", sound = "Common/EnterScene"},
  TeleportSkill={effect = "Skill/Teleport"},
  BuffImmune={effect="Common/immunity",effectpos=1},   ----异常状态免疫特效
  BuffResist={effect="Skill/Parry",effectpos=2},
}
