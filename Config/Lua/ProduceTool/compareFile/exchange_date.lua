if OneValue.TradeTime ~= '' then
  local tmpdate1 = os.date("%Y-%m-%d %H:%M:%S",CompareStartTime)
  local tmpdate2 = os.date("%Y-%m-%d %H:%M:%S",CompareEndTime)
  if OneValue.UnTradeTime == '' then
    CompareFileSelectFlag = (tmpdate1 > OneValue.TradeTime ) or (tmpdate2 > OneValue.TradeTime )
  else
    CompareFileSelectFlag = (tmpdate1>OneValue.TradeTime  and tmpdate1<OneValue.UnTradeTime) or (tmpdate2 > OneValue.TradeTime  and tmpdate2 < OneValue.UnTradeTime)
  end
else
  CompareFileSelectFlag = true
end
