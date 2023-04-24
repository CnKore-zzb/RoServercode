--【改动任何内容都请单独改动每个分支下的配置文件条目，禁止偷懒直接将整个文件从一个分支复制到另外一个分支！】
--【配置表下按类别拆分，新内容如有必要则按格式重建一个配置表单独对应！】

--pm交易行
ServerGame.Auction = {
  StartTime = {                     			--拍卖开始时间
    {wday = 5, hour=20,min = 00},           	--每周五20点00分钟
  },    
  VerifySignupDuration = 1800,      			--拍卖报名审核时长。秒
  VerifyTakeDuration = 1800,      				--拍卖成交记录审核时长。秒
  MaxAuctionItemCount = 10,         			--最大拍卖物品个数
  MinAuctionItemCount = 10,         			--最低拍卖物品个数
  DurationPerOrder = 60,        				--每个拍品无人出价时竞拍时间
  NextOrderDuration = 60,           			--倒计时多少秒进入下一个拍品
  TradePriceDiscount = 60,						--拍卖上架时的商品价格，取当前加一所当前换算的逼格猫币的百分比折扣
  AuctionStartDialogTime = 600,					--在拍卖开始时多少时间内推送拍卖开始的playdialog
  PublicityDuration = 28800,					--在拍卖正式开启前8小时时进入拍卖公示期
}
