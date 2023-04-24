--------------------玩家属性计算公式
--------------------物理职业攻击成长
local BaseLvAtkRate1New={[1]=0,
    [11]=0,[12]=0,[13]=0,[14]=0,
    [72]=0,[73]=0,[74]=0,
    [21]=0,[22]=0,[23]=0,[24]=0,
    [82]=0,[83]=0,[84]=0,
    [31]=2,[32]=3,[33]=4,[34]=4,
    [92]=3,[93]=4,[94]=4,
    [41]=0,[42]=0,[43]=0,[44]=0,
    [102]=0,[103]=0,[104]=0,
    [112]=0,[113]=0,[114]=0,
    [51]=0,[52]=0,[53]=0,[54]=0,
    [122]=3,[123]=4,[124]=4,
    [61]=0,[62]=0,[63]=0,[64]=0,
    [132]=0,[133]=0,[134]=0
    }
--------------------魔法职业攻击成长
local BaseLvAtkRate2New={[1]=0,
    [11]=0,[12]=0,[13]=0,[14]=0,
    [72]=0,[73]=0,[74]=0,
    [21]=0,[22]=0,[23]=0,[24]=0,
    [82]=0,[83]=0,[84]=0,
    [31]=0,[32]=0,[33]=0,[34]=0,
    [92]=0,[93]=0,[94]=0,
    [41]=0,[42]=0,[43]=0,[44]=0,
    [102]=0,[103]=0,[104]=0,
    [112]=0,[113]=0,[114]=0,
    [51]=0,[52]=0,[53]=0,[54]=0,
    [122]=0,[123]=0,[124]=0,
    [61]=0,[62]=0,[63]=0,[64]=0,
    [132]=0,[133]=0,[134]=0
    }
--------------------职业物防成长
local BaseLvDefRateNew={[1]=0,
    [11]=0,[12]=0,[13]=0,[14]=0,
    [72]=1,[73]=1,[74]=1,
    [21]=0,[22]=0,[23]=0,[24]=0,
    [82]=0,[83]=0,[84]=0,
    [31]=0,[32]=0,[33]=0,[34]=0,
    [92]=0,[93]=0,[94]=0,
    [41]=0,[42]=0,[43]=0,[44]=0,
    [102]=0,[103]=0,[104]=0,
    [112]=0,[113]=0,[114]=0,
    [51]=0,[52]=0,[53]=0,[54]=0,
    [122]=0,[123]=0,[124]=0,
    [61]=0,[62]=0,[63]=0,[64]=0,
    [132]=0,[133]=0,[134]=0
    }
--------------------职业魔防成长
local BaseLvMDefRateNew={[1]=0,
    [11]=0,[12]=0,[13]=0,[14]=0,
    [72]=0,[73]=0,[74]=0,
    [21]=0,[22]=0,[23]=0,[24]=0,
    [82]=0,[83]=0,[84]=0,
    [31]=0,[32]=0,[33]=0,[34]=0,
    [92]=0,[93]=0,[94]=0,
    [41]=0,[42]=0,[43]=0,[44]=0,
    [102]=0,[103]=0,[104]=0,
    [112]=0,[113]=0,[114]=0,
    [51]=0,[52]=0,[53]=0,[54]=0,
    [122]=0,[123]=0,[124]=0,
    [61]=0,[62]=0,[63]=0,[64]=0,
    [132]=0,[133]=0,[134]=0
    }
local BaseLvRateNew={[1]=5,
    [11]=20,[12]=20,[13]=20,[14]=20,
    [72]=20,[73]=20,[74]=20,
    [21]=10,[22]=10,[23]=10,[24]=12,
    [82]=10,[83]=10,[84]=12,
    [31]=12,[32]=12,[33]=12,[34]=14,
    [92]=12,[93]=12,[94]=14,
    [41]=12,[42]=12,[43]=12,[44]=16,
    [102]=14,[103]=14,[104]=18,
    [112]=14,[113]=14,[114]=18,
    [51]=5,[52]=5,[53]=5,[54]=14,
    [122]=5,[123]=5,[124]=16,
    [61]=12,[62]=12,[63]=12,[64]=14,
    [132]=12,[133]=12,[134]=16
    }
local HpRateNew={[1]=0,
    [11]=1.5,[12]= 1.75,[13]= 2,[14]= 3,
    [72]=4,[73]=5,[74]=5,
    [21]=0.9,[22]=1.1,[23]=1.2,[24]=2.1,
    [82]=1.2,[83]=1.4,[84]=2.3,
    [31]=1.1,[32]= 1.3,[33]= 1.5,[34]= 2.3,
    [92]=1.3,[93]=1.5,[94]=2.4,
    [41]=1,[42]=1.2,[43]=1.3,[44]=2.1,
    [102]=1.5,[103]=1.8,[104]=2.6,
    [112]=1.5,[113]=1.8,[114]=2.6,
    [51]=0.9,[52]=1.1,[53]=1.2,[54]=2.2,
    [122]=1.2,[123]=1.4,[124]=2.1,
    [61]=1,[62]= 1.4,[63]= 1.6,[64]= 2.5,
    [132]=1.4,[133]=1.6,[134]=2.2,
    }
local BaseHpNew={[1]=0,
    [11] =400,[12] =400,[13] =400,[14]=1000,
    [72] =1000,[73] =1500,[74] =1500,
    [21] =  0,[22] =  0,[23] =  0,[24]=1000,
    [82] =  0,[83] =  0,[84] =  1000,
    [31] =  0,[32] =  0,[33] =  0,[34]=1000,
    [92] =  0,[93] =  0,[94] =  1000,
    [41] =  0,[42] =  0,[43] =  0,[44]=1000,
    [102]=  0,[103]=  0,[104]=  1000,
    [112]=  0,[113]=  0,[114]=  1000,
    [51] =  0,[52] =  0,[53] =  0,[54]=1500,
    [122]=  0,[123]=  0,[124]=  1000,
    [61] =  0,[62] =  0,[63] =  0,[64]=1000,
    [132]=  0,[133]=  0,[134]=  1000,
    }
------------------------------职业空手攻速
local BaseJobAtkSpdNew={[1]=156,
    [11]=153,[12]=156,[13]=156,[14]=156,
    [72]=156,[73]=156,[74]=156,
    [21]=146,[22]=146,[23]=151,[24]=151,
    [82]=156,[83]=156,[84]=156,
    [31]=156,[32]=156,[33]=156,[34]=156,
    [92]=156,[93]=156,[94]=156,
    [41]=156,[42]=156,[43]=156,[44]=156,
    [102]=156,[103]=156,[104]=156,
    [112]=156,[113]=156,[114]=156,
    [51]=156,[52]=156,[53]=151,[54]=151,
    [122]=156,[123]=156,[124]=156,
    [61]=156,[62]=156,[63]=156,[64]=156,
    [132]=156,[133]=156,[134]=156,
    }
-- 长矛各职业对应攻速
local SpearAtkSpdNew={[1]=0,
    -- 这是剑士和骑士系列
    [11]=-17, [12]=-15, [13]= -8, [14]= -8,
    [72]=-13, [73]=-10, [74]=-10,
    [21]=  0, [22]=  0, [23]=  0, [24]=  0,
    [82]=  0, [83]=  0, [84]=  0,
    [31]=  0, [32]=  0, [33]=  0, [34]=  0,
    [92]=  0, [93]=  0, [94]=  0,
    [41]=  0, [42]=  0, [43]=  0, [44]=  0,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51]=  0, [52]=  0, [53]=  0, [54]=  0,
    [122]= 0, [123]= 0, [124]= 0,
    [61]=  0, [62]=  0, [63]=  0, [64]=  0,
    [132]= 0, [133]= 0, [134]= 0,
    }
-- 长剑各职业对应攻速
local SwordAtkSpdNew={[1]=-17,
    [11]= -7, [12]= -5, [13]=-12, [14]= -12,
    [72]= -3, [73]= -5, [74]= -5,
    [21]=  0, [22]=  0, [23]=  0, [24]=  0,
    [82]=  0, [83]=  0, [84]=  0,
    [31]=-10, [32]=-10, [33]=-25, [34]=-25,
    [92]=-10, [93]=-25, [94]=-25,
    [41]=  0, [42]=  0, [43]=  0, [44]=  0,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51]=  0, [52]=  0, [53]=  0, [54]=  0,
    [122]= 0, [123]= 0, [124]= 0,
    [61]=-12, [62]=-10, [63]=-25, [64]=-25,
    [132]=-10, [133]=-25, [134]=-25,
    }
-- 锤子各职业对应攻速
local MaceAtkSpdNew={[1]=-10,
    [11]=-10, [12]= -5, [13]= -5, [14]= -5,
    [72]= -5, [73]= -4, [74]= -4,
    [21]=  0, [22]=  0, [23]=  0, [24]=  0,
    [82]=  0, [83]=  0, [84]=  0,
    [31]=  0, [32]=  0, [33]=  0, [34]=  0,
    [92]=  0, [93]=  0, [94]=  0,
    [41]=  0, [42]=  0, [43]=  0, [44]=  0,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51]= -5, [52]= -3, [53]=  0, [54]=  0,
    [122]=-3, [123]=-5, [124]=-5,
    [61]=-10, [62]= -8, [63]= -8, [64]= -8,
    [132]=-8, [133]= -8, [134]= -8,
    }
-- 拳刃各职业对应攻速
local KatarAtkSpdNew={[1]=0,
    [11]=  0, [12]=  0, [13]=  0, [14]=  0,
    [72]=  0, [73]=  0, [74]=  0,
    [21]=  0, [22]=  0, [23]=  0, [24]=  0,
    [82]=  0, [83]=  0, [84]=  0,
    [31]=  0, [32]= -2, [33]= -2, [34]= -2,
    [92]=  0, [93]=  0, [94]=  0,
    [41]=  0, [42]=  0, [43]=  0, [44]=  0,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51]=  0, [52]=  0, [53]=  0, [54]=  0,
    [122]= 0, [123]= 0, [124]= 0,
    [61]=  0, [62]=  0, [63]=  0, [64]=  0,
    [132]= 0, [133]= 0, [134]= 0,
    }
-- 弓各职业对应攻速
local BowAtkSpdNew={[1]=0,
    [11]=  0, [12]=  0, [13]=  0, [14]=  0,
    [72]=  0, [73]=  0, [74]=  0,
    [21]=  0, [22]=  0, [23]=  0, [24]=  0,
    [82]=  0, [83]=  0, [84]=  0,
    [31]=  0, [32]=  0, [33]=  0, [34]=  0,
    [92]=-10, [93]=-10, [94]=-10,
    [41]=-10, [42]= -8, [43]= -9, [44]= -9,
    [102]= -9, [103]= -9, [104]= -9,
    [112]= -9, [113]= -9, [114]= -9,
    [51]=  0, [52]=  0, [53]=  0, [54]=  0,
    [122]= 0, [123]= 0, [124]= 0,
    [61]=  0, [62]=  0, [63]=  0, [64]=  0,
    [132]= 0, [133]= 0, [134]= 0,
    }
-- 鞭子各职业对应攻速
local WhipAtkSpdNew={[1]=0,
    [11]=  0, [12]=  0, [13]=  0, [14]=  0,
    [72]=  0, [73]=  0, [74]=  0,
    [21]=  0, [22]=  0, [23]=  0, [24]=  0,
    [82]=  0, [83]=  0, [84]=  0,
    [31]=  0, [32]=  0, [33]=  0, [34]=  0,
    [92]=-10, [93]=-10, [94]=-10,
    [41]=-10, [42]= -8, [43]= -9, [44]= -9,
    [102]= -9, [103]= -9, [104]= -9,
    [112]= -9, [113]= -9, [114]= -9,
    [51]=  0, [52]=  0, [53]=  0, [54]=  0,
    [122]= 0, [123]= 0, [124]= 0,
    [61]=  0, [62]=  0, [63]=  0, [64]=  0,
    [132]= 0, [133]= 0, [134]= 0,
    }
-- 琴各职业对应攻速
local HarpAtkSpdNew={[1]=0,
    [11]=  0, [12]=  0, [13]=  0, [14]=  0,
    [72]=  0, [73]=  0, [74]=  0,
    [21]=  0, [22]=  0, [23]=  0, [24]=  0,
    [82]=  0, [83]=  0, [84]=  0,
    [31]=  0, [32]=  0, [33]=  0, [34]=  0,
    [92]=-10, [93]=-10, [94]=-10,
    [41]=-10, [42]= -8, [43]= -9, [44]= -9,
    [102]= -9, [103]= -9, [104]= -9,
    [112]= -9, [113]= -9, [114]= -9,
    [51]=  0, [52]=  0, [53]=  0, [54]=  0,
    [122]= 0, [123]= 0, [124]= 0,
    [61]=  0, [62]=  0, [63]=  0, [64]=  0,
    [132]= 0, [133]= 0, [134]= 0,
    }
-- 法杖各职业对应攻速
local StaffAtkSpdNew={[1]=-25,
    [11]=  0, [12]=  0, [13]=  0, [14]=  0,
    [72]=  0, [73]=  0, [74]=  0,
    [21]= -5, [22]= -3, [23]= -5, [24]= -5,
    [82]= -10, [83]= -9, [84]= -8,
    [31]=  0, [32]=  0, [33]=  0, [34]=  0,
    [92]=  0, [93]=  0, [94]=  0,
    [41]=  0, [42]=  0, [43]=  0, [44]=  0,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51]=-20, [52]=-20, [53]=-15, [54]=-15,
    [122]=-20, [123]=-10, [124]=-10,
    [61]=-10, [62]= -8, [63]= -8, [64]= -8,
    [132]= 0, [133]= 0, [134]= 0,
    }
-- 书各职业对应攻速
local BookAtkSpdNew={[1]=-25,
    [11]=  0, [12]=  0, [13]=  0, [14]=  0,
    [72]=  0, [73]=  0, [74]=  0,
    [21]= -5, [22]= -3, [23]= -5, [24]= -5,
    [82]= -5, [83]= -3, [84]= -3,
    [31]=  0, [32]=  0, [33]=  0, [34]=  0,
    [92]=  0, [93]=  0, [94]=  0,
    [41]=  0, [42]=  0, [43]=  0, [44]=  0,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51]=-20, [52]=-20, [53]=-15, [54]=-15,
    [122]=-20, [123]=-10, [124]=-10,
    [61]=-10, [62]= -8, [63]= -8, [64]= -8,
    [132]= 0, [133]= 0, [134]= 0,
    }
-- 匕首各职业对应攻速
local KnifeAtkSpdNew={[1]=-15,
    [11]= -7, [12]= -9, [13]=-10, [14]=-10,
    [72]= -8, [73]= -7, [74]= -7,
    [21]=  0, [22]= -4, [23]= -7, [24]= -7,
    [82]=  0, [83]=  0, [84]=  0,
    [31]= -8, [32]= -2, [33]= -2, [34]= -2,
    [92]= -2, [93]= -2, [94]= -2,
    [41]=-15, [42]=-13, [43]=-10, [44]=-10,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51]=  0, [52]=  0, [53]=  0, [54]=  0,
    [122]= 0, [123]= 0, [124]= 0,
    [61]=-12, [62]=-10, [63]=-20, [64]=-20,
    [132]=-10, [133]=-20, [134]=-20,
    }
-- 斧头各职业对应攻速
local AxeAtkSpdNew={[1]=-15,
    [11]= -20, [12]= -15, [13]=-12, [14]=-12,
    [72]= -20, [73]= -15, [74]=-15,
    [21]=  0, [22]=  0, [23]=  0, [24]=  0,
    [82]=  0, [83]=  0, [84]=  0,
    [31]= 0, [32]= 0, [33]= 0, [34]= 0,
    [92]=  0, [93]=  0, [94]=  0,
    [41]=  0, [42]=  0, [43]=  0, [44]= 0,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51]=  0, [52]=  0, [53]=  0, [54]=  0,
    [122]= 0, [123]= 0, [124]= 0,
    [61]=-15, [62]=-13, [63]=-10, [64]=-10,
    [132]=-13, [133]=-10, [134]=-10,
    } 
-- 拳套各职业对应攻速
local FistAtkSpdNew={[1] = 0,
    [11] = 0, [12] = 0, [13] = 0, [14]= 0,
    [72] = 0, [73] = 0, [74] = 0,
    [21] = 0, [22] = 0, [23] = 0, [24]= 0,
    [82] = 0, [83] = 0, [84] = 0,
    [31] = 0, [32] = 0, [33] = 0, [34]= 0,
    [92] = 0, [93] = 0, [94] = 0,
    [41] = 0, [42] = 0, [43] = 0, [44]= 0,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51] = 0, [52] =-20, [53] =-5, [54]=-5,
    [122]= 0, [123]= -1, [124]=-1,
    [61] = 0, [62] = 0, [63] = 0, [64]= 0,
    [132]= 0, [133]= 0, [134]= 0,
    }

-- 副手各职业对应攻速(暂时修改掉副手的ASPD惩罚)
local ShieldAtkSpdNew={[1]= 0,
    [11]=  0, [12]=  0, [13]=  0, [14]=  0,
    [72]=  0, [73]=  0, [74]=  0,
    [21]=  0, [22]=  0, [23]=  0, [24]=  0,
    [82]=  0, [83]=  0, [84]=  0,
    [31]=  0, [32]=  0, [33]=  0, [34]=  0,
    [92]=  0, [93]=  0, [94]=  0,
    [41]=  0, [42]=  0, [43]=  0, [44]=  0,
    [102]= 0, [103]= 0, [104]= 0,
    [112]= 0, [113]= 0, [114]= 0,
    [51]=  0, [52]=  0, [53]=  0, [54]=  0,
    [122]= 0, [123]= 0, [124]= 0,
    [61]=  0, [62]=  0, [63]=  0, [64]=  0,
    [132]= 0, [133]= 0, [134]= 0,
    }

-- 等差数列求和
function CalcSumNew(num)
  local result=0
  for i=num,0,-1 do
    result=result+i
  end
  return result
end

-- 值修正
function modifyValue(value, adjust, action)
  if action == "great" then
    if value >= adjust then
      value = adjust
    end
  elseif action == "less" then
    if value <= adjust then
      value = adjust
    end
  elseif action == "floor" then
    if value ~= 0 then
      value = math.floor(value * 1000 + 0.5) / 1000
    end
  end

  return value
end

-- 武器基本攻速(170--长矛;180---剑;190---法杖;200---拳刃;210---弓;220---锤子;230---斧头;240---书;250---匕首;260---乐器;270---鞭子;280---试管;290---拳套)
function WeaponAtkSpdNew(type,profession)
  if type==170 then
     return SpearAtkSpdNew[profession]
  elseif type==180 then
     return SwordAtkSpdNew[profession]
  elseif type==190 then
     return StaffAtkSpdNew[profession]
  elseif type==200 then
     return KatarAtkSpdNew[profession]
  elseif type==210 then
     return BowAtkSpdNew[profession]
  elseif type==220 then
     return MaceAtkSpdNew[profession]
  elseif type== 230 then
     return AxeAtkSpdNew[profession]
  elseif type== 240 then
     return BookAtkSpdNew[profession] 
  elseif type==250 then
     return KnifeAtkSpdNew[profession]
  elseif type==260 then
     return HarpAtkSpdNew[profession]  
  elseif type==270 then
     return WhipAtkSpdNew[profession]  
  elseif type==290 then
     return FistAtkSpdNew[profession]
  elseif type==510 or type==511 or type==512 or type==513 or type==514 or type==515 then
     return ShieldAtkSpdNew[profession]
  end
  return 0
end

-- 刷新玩家属性
local types = { EATTRTYPE_STR, EATTRTYPE_INT, EATTRTYPE_AGI, EATTRTYPE_DEX, EATTRTYPE_VIT, EATTRTYPE_LUK,
                EATTRTYPE_ATK, EATTRTYPE_DEF, EATTRTYPE_MATK, EATTRTYPE_MDEF, EATTRTYPE_MAXHP, EATTRTYPE_MAXSP,
                EATTRTYPE_HIT, EATTRTYPE_FLEE, EATTRTYPE_CRI, EATTRTYPE_CRIRES, EATTRTYPE_CRIDAMPER, EATTRTYPE_CRIDEFPER,
                EATTRTYPE_MOVESPD, EATTRTYPE_MOVESPDPER, EATTRTYPE_CASTSPD, EATTRTYPE_ATKPER, EATTRTYPE_DEFPER, EATTRTYPE_MATKPER, EATTRTYPE_MDEFPER,
                EATTRTYPE_MAXHPPER, EATTRTYPE_MAXSPPER, EATTRTYPE_REFINE, EATTRTYPE_MREFINE, EATTRTYPE_RESTORESPD, EATTRTYPE_SPRESTORESPD,
                EATTRTYPE_EQUIPASPD, EATTRTYPE_SKILLASPD, EATTRTYPE_ATKSPD, EATTRTYPE_CRIPER, EATTRTYPE_DAMREDUC, EATTRTYPE_MDAMREDUC,EATTRTYPE_RESTORESPDPER,EATTRTYPE_SPRESTORESPDPER,EATTRTYPE_HITPER,EATTRTYPE_FLEEPER}
--local types = { [EATTRTYPE_STR] = EATTRTYPE_LUK, [EATTRTYPE_ATK] = EATTRTYPE_CRIPER, [EATTRTYPE_REFINE] = EATTRTYPE_LUKPER, [EATTRTYPE_SHOWATK] = EATTRTYPE_MDAMREDUC}
                --[EATTRTYPE_SHOWATK] = EATTRTYPE_DEADMDAMREDUC,
                --[EATTRTYPE_CTCHANGE] = EATTRTYPE_CDCHANGEPER, [EATTRTYPE_SPCOST] = EATTRTYPE_HPCOSTPER, [EATTRTYPE_DELAYCDCHANGE] = EATTRTYPE_DCHANGEPER, [EATTRTYPE_NOSKILL] = EATTRTYPE_FUNCLIMIT,
                --[EATTRTYPE_ATKATTR] = EATTRTYPE_POISONINGATK, [EATTRTYPE_BRUTEDAMPER] = EATTRTYPE_DRAGONRESPER, [EATTRTYPE_SMALLDAMPER] = EATTRTYPE_NPCRESPER, [EATTRTYPE_SILENCEATK] = EATTRTYPE_CURSEDEF,
                --[EATTRTYPE_TRANSFORMID] = EATTRTYPE_SLEEPDEF, [EATTRTYPE_BEHEALENCPER] = EATTRTYPE_HEALENCPER, [EATTRTYPE_DEADSOON] = EATTRTYPE_JOBEXPPER
function calcUserAttr(user, index)
  local profession = user:getProfession()
  if profession == nil or profession == 0 then
    print("calcUserAttr profession error")
    return
  end
  local level = user:getLevel()
  if level == nil or level == 0 then
    print("calcUserAttr level error")
    return
  end
  local map = 0
  if user:isOnPvp() then
    map = 1
  elseif user:isOnGvg() then
    map = 2
  end

  local attrs = {}
  for i = 1, #types do
    if user:testAttr(types[i]) == true then
      attrs[#attrs + 1] = types[i]
    end
  end

  for i = EATTRTYPE_STR, EATTRTYPE_LUK do
    if user:getAttr(i) < 0 then
      user:setAttr(i, 0)
    end
  end
  attrs[#attrs + 1] = EATTRTYPE_ATKSPD

  for i = 1, #attrs do
    calcAttr(user, level, profession, map, attrs[i])
  end
  --print("第 "..index.." 次计算---------------------------")
end

-- 计算属性
function calcAttr(user, lv, pro, map, attr)
  if lv == nil or lv == 0 then
    print("calcAttr level error")
    return
  end
  if pro == nil or pro == 0 then
    print("calcAttr profession error")
    return
  end

  local extra = 0
  local final = 0

  if attr == EATTRTYPE_STR then
    calcAttr(user, lv, pro, map, EATTRTYPE_ATK)
  elseif attr == EATTRTYPE_INT then
    calcAttr(user, lv, pro, map, EATTRTYPE_MATK)
    calcAttr(user, lv, pro, map, EATTRTYPE_MDEF)
    calcAttr(user, lv, pro, map, EATTRTYPE_MAXSP)
  elseif attr == EATTRTYPE_AGI then
    calcAttr(user, lv, pro, map, EATTRTYPE_FLEE)
    calcAttr(user, lv, pro, map, EATTRTYPE_ATKSPD)
  elseif attr == EATTRTYPE_DEX then
    calcAttr(user, lv, pro, map, EATTRTYPE_ATK)
    calcAttr(user, lv, pro, map, EATTRTYPE_HIT)
    calcAttr(user, lv, pro, map, EATTRTYPE_CASTSPD)
  elseif attr == EATTRTYPE_VIT then
    calcAttr(user, lv, pro, map, EATTRTYPE_DEF)
    calcAttr(user, lv, pro, map, EATTRTYPE_MAXHP)
    calcAttr(user, lv, pro, map, EATTRTYPE_RESTORESPD)
  elseif attr == EATTRTYPE_LUK then
    calcAttr(user, lv, pro, map, EATTRTYPE_ATK)
    calcAttr(user, lv, pro, map, EATTRTYPE_CRI)
    calcAttr(user, lv, pro, map, EATTRTYPE_CRIRES)
  elseif attr == EATTRTYPE_ATK then
    local dex = user:getAttr(EATTRTYPE_DEX)
    local str = user:getAttr(EATTRTYPE_STR)
    local luk = user:getAttr(EATTRTYPE_LUK)

 ----------玩家物理攻击计算公式
    if CommonFun.checkRemoteAtk(pro, user:getWeaponType()) then
      extra = dex * 2 + math.floor(dex * dex / 100) + math.floor(str / 5) + math.floor(luk / 5) + BaseLvAtkRate1New[pro] * lv
    else
      extra = str * 2 + math.floor(str * str / 100) + math.floor(dex / 5) + math.floor(luk / 5) + BaseLvAtkRate1New[pro] * lv
    end
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = extra + user:getOtherAttr(EATTRTYPE_ATK)
    final = modifyValue(final, 0, "less")
    user:setAttr(attr, final)

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWATK)
 ----------玩家物理防御计算公式
  elseif attr == EATTRTYPE_DEF then
    extra = user:getAttr(EATTRTYPE_VIT)
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = extra + user:getOtherAttr(EATTRTYPE_DEF)
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWDEF)
 ----------玩家魔法攻击计算公式   
  elseif attr == EATTRTYPE_MATK then
    local int = user:getAttr(EATTRTYPE_INT)
    extra = int * 2 + math.floor(int * int / 100)
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = extra + user:getOtherAttr(EATTRTYPE_MATK)
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMATK)
 ----------玩家魔法防御计算公式
  elseif attr == EATTRTYPE_MDEF then
    extra = user:getAttr(EATTRTYPE_INT)
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = extra + user:getOtherAttr(EATTRTYPE_MDEF)
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMDEF)
 ----------玩家生命上限计算公式
  elseif attr == EATTRTYPE_MAXHP then
    local vit = user:getAttr(EATTRTYPE_VIT)
    extra = (100 + lv * BaseLvRateNew[pro] + CalcSumNew(lv) * HpRateNew[pro] + BaseHpNew[pro]) * (1 + vit / 100)
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    if map == 1 or map == 2 then----map=1 表示pvp,map=2 表示gvg
      final = 4 * (extra + user:getOtherAttr(EATTRTYPE_MAXHP)) * (1 + user:getAttr(EATTRTYPE_MAXHPPER))
    else
      final = (extra + user:getOtherAttr(EATTRTYPE_MAXHP)) * (1 + user:getAttr(EATTRTYPE_MAXHPPER))
    end
    user:setAttr(attr, final)

    calcAttr(user, lv, pro, map, EATTRTYPE_RESTORESPD)
    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMAXHP)
 ----------玩家魔法上限计算公式
  elseif attr == EATTRTYPE_MAXSP then
    local int = user:getAttr(EATTRTYPE_INT)
    extra = (20 + (lv * GameConfig.NewRole.recover[pro].sp)) * (1 + int / 100)
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = (extra + user:getOtherAttr(EATTRTYPE_MAXSP)) * (1 + user:getAttr(EATTRTYPE_MAXSPPER))
    user:setAttr(attr, final)

    calcAttr(user, lv, pro, map, EATTRTYPE_SPRESTORESPD)
    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMAXSP)
  elseif attr == EATTRTYPE_HITPER then
    calcAttr(user, lv, pro, map, EATTRTYPE_HIT)
 ----------玩家命中计算公式
  elseif attr == EATTRTYPE_HIT then
    extra = lv + user:getAttr(EATTRTYPE_DEX)
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = (extra + user:getOtherAttr(EATTRTYPE_HIT)) * (1 + user:getOtherAttr(EATTRTYPE_HITPER))
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWHIT)
 ----------玩家闪避计算公式
  elseif attr == EATTRTYPE_FLEEPER then
    calcAttr(user, lv, pro, map, EATTRTYPE_FLEE)
  elseif attr == EATTRTYPE_FLEE then
    extra = lv + user:getAttr(EATTRTYPE_AGI)
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = (extra + user:getOtherAttr(EATTRTYPE_FLEE)) * (1 + user:getOtherAttr(EATTRTYPE_FLEEPER))
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWFLEE)
 ----------玩家暴击计算公式
  elseif attr == EATTRTYPE_CRI then
    extra = 1 + math.floor(user:getAttr(EATTRTYPE_LUK) / 3)
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = (extra + user:getOtherAttr(EATTRTYPE_CRI)) * (1 + user:getOtherAttr(EATTRTYPE_CRIPER))
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWCRI)
 ----------玩家防暴计算公式
  elseif attr == EATTRTYPE_CRIRES then
    extra = 1 + math.floor(user:getAttr(EATTRTYPE_LUK) / 5)
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = extra + user:getOtherAttr(EATTRTYPE_CRIRES)
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWCRIRES)
 ----------玩家暴伤计算公式
  elseif attr == EATTRTYPE_CRIDAMPER then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_CRIDAMPER)
    user:setAttr(attr, modifyValue(final, 0, "less"))
 ----------玩家暴伤防护计算公式
  elseif attr == EATTRTYPE_CRIDEFPER then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_CRIDEFPER)
    user:setAttr(attr, modifyValue(final, 0, "less"))
 ----------玩家移动速度计算公式
  elseif attr == EATTRTYPE_MOVESPD then
    local extra = 1
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = (extra + user:getOtherAttr(EATTRTYPE_MOVESPD)) * (1 + user:getOtherAttr(EATTRTYPE_MOVESPDPER))
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMOVESPD)
 ----------玩家移动速度百分比计算公式
  elseif attr == EATTRTYPE_MOVESPDPER then
    calcAttr(user, lv, pro, map, EATTRTYPE_MOVESPD)
 ----------玩家吟唱速度计算公式
  elseif attr == EATTRTYPE_CASTSPD then
    local extra = user:getAttr(EATTRTYPE_DEX) / 30
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_CASTSPD)
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWCASTSPD)
 ----------玩家物理攻击百分比计算公式
  elseif attr == EATTRTYPE_ATKPER then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_ATKPER)
    user:setAttr(attr, modifyValue(final, -1, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWATK)
 ----------玩家物理防御百分比计算公式
  elseif attr == EATTRTYPE_DEFPER then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_DEFPER)
    user:setAttr(attr, modifyValue(final, -1, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWDEF)
 ----------玩家魔法攻击百分比计算公式
  elseif attr == EATTRTYPE_MATKPER then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_MATKPER)
    user:setAttr(attr, modifyValue(final, -1, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMATK)
 ----------玩家魔法防御百分比计算公式
  elseif attr == EATTRTYPE_MDEFPER then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_MDEFPER)
    user:setAttr(attr, modifyValue(final, -1, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMDEF)
 ----------玩家生命上限百分比计算公式
  elseif attr == EATTRTYPE_MAXHPPER then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_MAXHPPER)
    user:setAttr(attr, modifyValue(final, -1, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_MAXHP)
 ----------玩家魔法上限百分比计算公式
  elseif attr == EATTRTYPE_MAXSPPER then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_MAXSPPER)
    user:setAttr(attr, modifyValue(final, -1, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_MAXSP)
 ----------玩家物理精炼计算公式
  elseif attr == EATTRTYPE_REFINE then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_REFINE)
    user:setAttr(attr, final)
 ----------玩家魔法精炼计算公式
  elseif attr == EATTRTYPE_MREFINE then
    local extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local final = extra + user:getOtherAttr(EATTRTYPE_MREFINE)
    user:setAttr(attr, final)
 ----------玩家素质生命恢复计算公式
  elseif attr == EATTRTYPE_RESTORESPD then
    extra = math.floor(user:getAttr(EATTRTYPE_VIT) / 5)
    user:setPointAttr(attr, extra)

    final = (extra + user:getOtherAttr(EATTRTYPE_RESTORESPD) + math.floor(user:getAttr(EATTRTYPE_MAXHP) / 200)) * (1 + user:getOtherAttr(EATTRTYPE_RESTORESPDPER))
    user:setAttr(attr, final)

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWRESTORESPD)
  -------HP恢复百分比发生变化需要计算HP
  elseif attr == EATTRTYPE_RESTORESPDPER then
    calcAttr(user, lv, pro, map, EATTRTYPE_RESTORESPD)
 ----------玩家素质魔法恢复计算公式
  elseif attr == EATTRTYPE_SPRESTORESPD then
    extra = 1 + math.floor(user:getAttr(EATTRTYPE_INT) / 6)
    user:setPointAttr(attr, extra)

    final = (extra + user:getOtherAttr(EATTRTYPE_SPRESTORESPD) + math.floor(user:getAttr(EATTRTYPE_MAXSP) / 100)) * (1 + user:getOtherAttr(EATTRTYPE_SPRESTORESPDPER))
    user:setAttr(attr, final)
  -------SP恢复百分比发生变化需要计算SP
  elseif attr == EATTRTYPE_SPRESTORESPDPER then
    calcAttr(user, lv, pro, map, EATTRTYPE_SPRESTORESPD)

  elseif attr == EATTRTYPE_EQUIPASPD then
    extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = extra + user:getOtherAttr(EATTRTYPE_EQUIPASPD)
    user:setAttr(attr, final)

    calcAttr(user, lv, pro, map, EATTRTYPE_ATKSPD)
  elseif attr == EATTRTYPE_SKILLASPD then
    extra = 0
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    final = extra + user:getOtherAttr(EATTRTYPE_SKILLASPD)
    user:setAttr(attr, final)

    calcAttr(user, lv, pro, map, EATTRTYPE_ATKSPD)
  ----------玩家素质攻击速度计算公式
  ----------最终ASPD=ASPD值+装备ASPD值
  ----------ASPD值=取整((200-(200-(职业基础ASPD+盾牌ASPD-ASPD校验值+开方((敏捷总和*9.999)+(灵巧总和*0.19212))*ASPD惩罚值))*(1-ASPD药水部分-技能ASPD修正)),3)
  ----------装备ASPD值=取整((195-ASPD值)*装备ASPD修正),2)
  ----------ASPD校验值=IF(敏捷总和<205,向上取整((开方(205)-开方(敏捷总和))/7.15,3),0)
  ----------ASPD惩罚值=IF(职业基础ASPD>145,1-(职业基础ASPD-144)/50,0.96)
  ----------每秒攻击次数=50/(200-取整(最终ASPD,0))
  elseif attr == EATTRTYPE_ATKSPD then
    local agi = user:getAttr(EATTRTYPE_AGI)
    local ASPD_CHECKVALUE = 0
    if agi < 205 then
      ASPD_CHECKVALUE = math.floor((math.sqrt(205) - math.sqrt(agi)) * 1000 / 7.15) / 1000
    else
      ASPD_CHECKVALUE = 0
    end
    local BaseJobASPD = BaseJobAtkSpdNew[pro] + WeaponAtkSpdNew(user:getWeaponType(), pro)
    local ASPD_PANISHVALUE = 0
    if BaseJobASPD > 145 then
      ASPD_PANISHVALUE = 1 - (BaseJobASPD - 144) / 50
    else
      ASPD_PANISHVALUE = 0.96
    end

    local extra = math.floor((200 - (200 - (BaseJobASPD - ASPD_CHECKVALUE + math.sqrt(agi * 9.999) * ASPD_PANISHVALUE)) * (1 - user:getAttr(EATTRTYPE_SKILLASPD))) * 1000) / 1000
    user:setPointAttr(attr, modifyValue(extra, 0, ""))

    local EquipASPD = math.floor((195 - extra) * user:getAttr(EATTRTYPE_EQUIPASPD) * 100) / 100
    local BaseASPD = extra + EquipASPD

    if BaseASPD >= 2275 / 12 then
      BaseASPD  = 2275 / 12
    end
    if BaseASPD <= 50 then
      BaseASPD = 50
    end
    ----------玩家最终攻击速度计算公式
    final = 50 / (200 - BaseASPD)
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWATKSPD)
  elseif attr == EATTRTYPE_CRIPER then
    final = user:getOtherAttr(EATTRTYPE_CRIPER)
    user:setAttr(attr, modifyValue(final, 0, "less"))

    calcAttr(user, lv, pro, map, EATTRTYPE_CRI)
  elseif attr == EATTRTYPE_DAMREDUC then
    final = user:getOtherAttr(EATTRTYPE_DAMREDUC)
    user:setAttr(attr, modifyValue(final, 1, "great"))
  elseif attr == EATTRTYPE_MDAMREDUC then
    final = user:getOtherAttr(EATTRTYPE_MDAMREDUC)
    user:setAttr(attr, modifyValue(final, 1, "great"))
----------玩家面板属性计算公式
  elseif attr == EATTRTYPE_SHOWATK then
    final = user:getAttr(EATTRTYPE_ATK) * (1 + user:getAttr(EATTRTYPE_ATKPER))
    user:setAttr(attr, modifyValue(final, 0, ""))
  elseif attr == EATTRTYPE_SHOWMATK then
    final = user:getAttr(EATTRTYPE_MATK) * (1 + user:getAttr(EATTRTYPE_MATKPER))
    user:setAttr(attr, modifyValue(final, 0, ""))
  elseif attr == EATTRTYPE_SHOWMAXHP then
    final = user:getAttr(EATTRTYPE_MAXHP)
    user:setAttr(attr, modifyValue(final, 0, ""))
  elseif attr == EATTRTYPE_SHOWMAXSP then
    final = user:getAttr(EATTRTYPE_MAXSP)
    user:setAttr(attr, modifyValue(final, 0, ""))
  elseif attr == EATTRTYPE_SHOWDEF then
    final = user:getPointAttr(EATTRTYPE_DEF) + user:getOtherAttr(EATTRTYPE_DEF) * (1 + user:getAttr(EATTRTYPE_DEFPER))
    user:setAttr(attr, modifyValue(final, 0, ""))
  elseif attr == EATTRTYPE_SHOWMDEF then
    final = user:getPointAttr(EATTRTYPE_MDEF) + user:getPointAttr(EATTRTYPE_MDEF) * (1 + user:getAttr(EATTRTYPE_MDEF))
    user:setAttr(attr, modifyValue(final, 0, ""))
  elseif attr == EATTRTYPE_SHOWHIT then
    final = user:getAttr(EATTRTYPE_HIT)
    user:setAttr(attr, modifyValue(final, 0, ""))
  elseif attr == EATTRTYPE_SHOWFLEE then
    final = modifyValue(user:getAttr(EATTRTYPE_FLEE), 0, "")
    user:setAttr(attr, final)
  elseif attr == EATTRTYPE_SHOWCRI then
    final = modifyValue(user:getAttr(EATTRTYPE_CRI), 0, "")
    user:setAttr(attr, final)
  elseif attr == EATTRTYPE_SHOWCRIRES then
    final = modifyValue(user:getAttr(EATTRTYPE_CRIRES), 0, "less")
    user:setAttr(attr, final)
  elseif attr == EATTRTYPE_SHOWATKSPD then
    finala = modifyValue(user:getAttr(EATTRTYPE_ATKSPD), 0, "")
    user:setAttr(attr, final)
  elseif attr == EATTRTYPE_SHOWMOVESPD then
    final = modifyValue(user:getAttr(EATTRTYPE_MOVESPD), 0, "")
    user:setAttr(attr, final)
  elseif attr == EATTRTYPE_SHOWCASTSPD then
    final = modifyValue(user:getAttr(EATTRTYPE_CASTSPD), 0, "")
    user:setAttr(attr, final)
  elseif attr == EATTRTYPE_SHOWRESTORESPD then
    final = modifyValue(user:getAttr(EATTRTYPE_RESTORESPD), 0, "")
    user:setAttr(attr, final)
  end

  --print("calc attr = "..attr.." extra = "..extra.." final = "..final)
end

-- 计算转职提升属性
function calcUserShowAttrValuePro(user, lv, oldpro, newpro)
  if user == nil then
    return
  end

  local weapon = user:getWeaponType()

  local oldvalue = 50 + lv * BaseLvRateNew[oldpro] + CalcSumNew(lv) * HpRateNew[oldpro] * (1 + user:getAttr(EATTRTYPE_VIT) / 100) + user:getAttr(EATTRTYPE_VIT) * 20
  local newvalue = 50 + lv * BaseLvRateNew[newpro] + CalcSumNew(lv) * HpRateNew[newpro] * (1 + user:getAttr(EATTRTYPE_VIT) / 100) + user:getAttr(EATTRTYPE_VIT) * 20
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_MAXHP, newvalue - oldvalue)
  end

  local dex = user:getBaseAttr(EATTRTYPE_DEX)
  local str = user:getBaseAttr(EATTRTYPE_STR)
  local luk = user:getBaseAttr(EATTRTYPE_LUK)
  local int = user:getBaseAttr(EATTRTYPE_INT)
  if CommonFun.checkRemoteAtk(oldpro, weapon) then
    oldvalue = dex * 3 + dex * dex / 100 + str / 5 + luk / 5 + lv * 2
  else
    oldvalue = str * 3 + str * str / 100 + dex / 5 + luk / 5 + lv*2
  end

  if CommonFun.checkRemoteAtk(newpro, weapon) then
    newvalue = dex * 3 + dex * dex / 100 + str / 5 + luk / 5 + lv * 2
  else
    newvalue = str * 3 + str * str / 100 + dex / 5 + luk / 5 + lv*2
  end
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_ATK, newvalue - oldvalue)
  end

  oldvalue = 20 + (lv * GameConfig.NewRole.recover[oldpro].sp) * (1 + int / 100) + int * 2
  newvalue = 20 + (lv * GameConfig.NewRole.recover[newpro].sp) * (1 + int / 100) + int * 2
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_MAXSP, newvalue - oldvalue)
  end
end

-- 计算升级提升属性
function calcUserShowAttrValueLv(user, profession, oldlv, newlv)
  if user == nil then
    return
  end

  local weapon = user:getWeaponType()

  ------------生命值的等级提升效果展示
  local oldvalue = 50 + oldlv * BaseLvRateNew[profession] + CalcSumNew(oldlv) * HpRateNew[profession] + BaseHpNew[profession]
  local newvalue = 50 + newlv * BaseLvRateNew[profession] + CalcSumNew(newlv) * HpRateNew[profession] + BaseHpNew[profession]
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_MAXHP, newvalue - oldvalue)
  end

  ------------物理攻击的等级提升效果展示
  local dex = user:getBaseAttr(EATTRTYPE_DEX)
  local str = user:getBaseAttr(EATTRTYPE_STR)
  local luk = user:getBaseAttr(EATTRTYPE_LUK)
  local int = user:getBaseAttr(EATTRTYPE_INT)
  local vit = user:getBaseAttr(EATTRTYPE_VIT)
  local agi = user:getBaseAttr(EATTRTYPE_AGI)
  if CommonFun.checkRemoteAtk(profession, weapon) then
    oldvalue = dex * 3 + dex * dex / 100 + str / 5 + luk / 5 + oldlv * BaseLvAtkRate1New[profession]
  else
    oldvalue = str * 3 + str * str / 100 + dex / 5 + luk / 5 + oldlv * BaseLvAtkRate1New[profession]
  end
  if CommonFun.checkRemoteAtk(newpro, weapon) then
    newvalue = dex * 3 + dex * dex / 100 + str / 5 + luk / 5 + newlv * BaseLvAtkRate1New[profession]
  else
    newvalue = str * 3 + str * str / 100 + dex / 5 + luk / 5 + newlv * BaseLvAtkRate1New[profession]
  end
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_ATK, newvalue - oldvalue)
  end

  ------------魔法攻击的等级提升效果展示
  oldvalue = int * 3 + int * int / 100 + oldlv * BaseLvAtkRate2New[profession]
  newvalue = int * 3 + int * int / 100 + newlv * BaseLvAtkRate2New[profession]
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_MATK, newvalue - oldvalue)
  end

  ------------物理防御的等级提升效果展示
  oldvalue = vit * 2 + vit * vit / 100 + oldlv * BaseLvDefRateNew[profession]
  newvalue = vit * 2 + vit * vit / 100 + newlv * BaseLvDefRateNew[profession]
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_DEF, newvalue - oldvalue)
  end

  ------------魔法防御的等级提升效果展示
  oldvalue = int * 2 + int * int / 100 + oldlv * BaseLvMDefRateNew[profession]
  newvalue = int * 2 + int * int / 100 + newlv * BaseLvMDefRateNew[profession]
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_MDEF, newvalue - oldvalue)
  end

  ------------魔法恢复的等级提升效果展示
  oldvalue = 20 + (oldlv * GameConfig.NewRole.recover[profession].sp) + int * 2
  newvalue = 20 + (newlv * GameConfig.NewRole.recover[profession].sp) + int * 2
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_MAXSP, newvalue - oldvalue)
  end

  ------------命中的等级提升效果展示  
  oldvalue = oldlv + dex
  newvalue = newlv + dex
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_HIT, newvalue - oldvalue)
  end

  ------------闪避的等级提升效果展示 
  oldvalue = oldlv + agi
  newvalue = newlv + agi
  if newvalue > oldvalue + 1 then
    user:setShowAttr(EATTRTYPE_FLEE, newvalue - oldvalue)
  end
end

-- 计算战斗力
function calcBattlePoint(user, lv, profession, skillbt)
  --local total, extra = CommonFun.calcUserAttrValue(attr, lv, profession, weapon)
  local maxhp = user:getAttr(EATTRTYPE_SHOWMAXHP)
  local atk = user:getAttr(EATTRTYPE_SHOWATK)
  local matk = user:getAttr(EATTRTYPE_SHOWMATK)
  local def = user:getAttr(EATTRTYPE_SHOWDEF)
  local mdef = user:getAttr(EATTRTYPE_SHOWMDEF)
  local hit = user:getAttr(EATTRTYPE_SHOWHIT)
  local flee = user:getAttr(EATTRTYPE_SHOWFLEE)
  local cri = user:getAttr(EATTRTYPE_SHOWCRI)
  local crires = user:getAttr(EATTRTYPE_SHOWCRIRES)
  local atkspd = user:getAttr(EATTRTYPE_SHOWATKSPD)
  local movespd = user:getAttr(EATTRTYPE_SHOWMOVESPD)
  local castspd = user:getAttr(EATTRTYPE_SHOWCASTSPD)
  local restorespd = user:getAttr(EATTRTYPE_SHOWRESTORESPD)

  local weapon = user:getWeaponType()
  return maxhp * 5 + (atk + matk) / 2 + (def + mdef) + hit + flee + cri * 2 + crires * 2 + atkspd * 10 + movespd * 10 + castspd * 10 + restorespd * 10 + skillbt * 5
end
