if OneValue.AddDate ~= '' then
  local tmpdate1 = os.date("%Y/%m/%d",CompareStartTime)
  local tmpdate2 = os.date("%Y/%m/%d",CompareEndTime)
  if OneValue.RemoveDate == '' then
    CompareFileSelectFlag = (tmpdate1 > OneValue.AddDate) or (tmpdate2 > OneValue.AddDate)
  else
    CompareFileSelectFlag = (tmpdate1>OneValue.AddDate and tmpdate1<OneValue.RemoveDate) or (tmpdate2 > OneValue.AddDate and tmpdate2 < OneValue.RemoveDate)
  end
else
  CompareFileSelectFlag = true
end
