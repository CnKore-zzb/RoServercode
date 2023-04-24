if OneValue.UseStartTime ~= '' then
  local tmpdate1 = os.date("%Y-%m-%d",CompareStartTime)
  local tmpdate2 = os.date("%Y-%m-%d",CompareEndTime)
  if OneValue.UseEndTime == '' then
    CompareFileSelectFlag = (tmpdate1 > OneValue.UseStartTime ) or (tmpdate2 > OneValue.UseStartTime)
  else
    CompareFileSelectFlag = (tmpdate1>OneValue.UseStartTime and tmpdate1<OneValue.UseEndTime) or (tmpdate2 > OneValue.UseStartTime and tmpdate2 < OneValue.UseEndTime)
  end
else
  CompareFileSelectFlag = true
end
