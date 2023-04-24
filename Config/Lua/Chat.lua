------------------- 获取聊天重复扣分 ----------------
function CheckInvalidChat(channel, text, user)
  if user == nil or user:getChatParam() == nil then
    return 0
  end

  local params = user:getChatParam()

  if string.len(text) < 10 then
    return 0
  end

  local subText = string.sub(text, 5, 10)
  local invalid = params:find(subText);
  local minus = -10

  if invalid then
    if os.time() - params:getInvalidTime() < 60 then
      params:addInvalidCount()
    else
      params:setInvalidCount(1)
    end
    params:setInvalidTime(os.time())
  else
    params:setInvalidCount(0)
    params:putMsg(text)
    -- param:resetSize(xx) 设置记录长度, 默认为5
  end

  minus = minus * params:getInvalidCount()

  --print(minus)
  return minus
end

