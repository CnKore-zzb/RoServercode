--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--随机NPC问答奖励配置，reward_count表示每次发放奖励次数--
ServerGame.QA = {
  reward_count = 20,
  reward_map = {
    {level = 60, rewardid=3038},               --[1,60] 级的 rewardid
    {level = 120, rewardid=3039}              --[61,120] 级的rewardid
  }
}