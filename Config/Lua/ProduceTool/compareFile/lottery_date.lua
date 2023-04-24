if OneValue.OnlineTime ~= '' then
  local tmpdate1 = os.date("%Y-%m-%d",CompareStartTime)
  local tmpdate2 = os.date("%Y-%m-%d",CompareEndTime)
  if OneValue.OfflineTime == '' then
    CompareFileSelectFlag = (tmpdate1 > OneValue.OnlineTime) or (tmpdate2 > OneValue.OnlineTime)
  else
    CompareFileSelectFlag = (tmpdate1>OneValue.OnlineTime and tmpdate1<OneValue.OfflineTime) or (tmpdate2 > OneValue.OnlineTime and tmpdate2 < OneValue.OfflineTime)
  end
else
  CompareFileSelectFlag = true
end
