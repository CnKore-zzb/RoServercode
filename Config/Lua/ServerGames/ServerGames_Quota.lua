--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--积分额度
ServerGame.Quota = {
  ItemId = 5529,                --猫金打赏卡itemid
  DefautExpireDay = 31,         --默认过期时间，单位天
  LogCountPerPage = 20,         --积分获得记录每页显示条数
  MaxSaveLogCount = 200,        --积分获得最大保存记录条数
  DetailCountPerPage = 20,      --积分详情每页显示条数
  DetailExpireDay = 10,         --积分详情过期X天数后从界面清除天数
  DetailExpireDayDel = 60, 	--过期天数后删除记录天数
}