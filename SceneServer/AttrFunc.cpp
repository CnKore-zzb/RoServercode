#include "AttrFunc.h"
#include "SceneUser.h"

AttrFunc::AttrFunc()
{
  // --------------------玩家属性计算公式
  // --------------------物理职业攻击成长
  BaseLvAtkRate1New[1]=0,
    BaseLvAtkRate1New[11]=0,BaseLvAtkRate1New[12]=0,BaseLvAtkRate1New[13]=0,BaseLvAtkRate1New[14]=0,
    BaseLvAtkRate1New[72]=0,BaseLvAtkRate1New[73]=0,BaseLvAtkRate1New[74]=0,
    BaseLvAtkRate1New[21]=0,BaseLvAtkRate1New[22]=0,BaseLvAtkRate1New[23]=0,BaseLvAtkRate1New[24]=0,
    BaseLvAtkRate1New[82]=0,BaseLvAtkRate1New[83]=0,BaseLvAtkRate1New[84]=0,
    BaseLvAtkRate1New[31]=2,BaseLvAtkRate1New[32]=3,BaseLvAtkRate1New[33]=4,BaseLvAtkRate1New[34]=4,
    BaseLvAtkRate1New[92]=3,BaseLvAtkRate1New[93]=4,BaseLvAtkRate1New[94]=4,
    BaseLvAtkRate1New[41]=0,BaseLvAtkRate1New[42]=0,BaseLvAtkRate1New[43]=0,BaseLvAtkRate1New[44]=0,
    BaseLvAtkRate1New[102]=0,BaseLvAtkRate1New[103]=0,BaseLvAtkRate1New[104]=0,
    BaseLvAtkRate1New[112]=0,BaseLvAtkRate1New[113]=0,BaseLvAtkRate1New[114]=0,
    BaseLvAtkRate1New[51]=0,BaseLvAtkRate1New[52]=0,BaseLvAtkRate1New[53]=0,BaseLvAtkRate1New[54]=0,
    BaseLvAtkRate1New[122]=3,BaseLvAtkRate1New[123]=4,BaseLvAtkRate1New[124]=4,
    BaseLvAtkRate1New[61]=0,BaseLvAtkRate1New[62]=0,BaseLvAtkRate1New[63]=0,BaseLvAtkRate1New[64]=0,
    BaseLvAtkRate1New[132]=0,BaseLvAtkRate1New[133]=0,BaseLvAtkRate1New[134]=0;
  // --------------------魔法职业攻击成长
  BaseLvAtkRate2New[1]=0,
    BaseLvAtkRate2New[11]=0,BaseLvAtkRate2New[12]=0,BaseLvAtkRate2New[13]=0,BaseLvAtkRate2New[14]=0,
    BaseLvAtkRate2New[72]=0,BaseLvAtkRate2New[73]=0,BaseLvAtkRate2New[74]=0,
    BaseLvAtkRate2New[21]=0,BaseLvAtkRate2New[22]=0,BaseLvAtkRate2New[23]=0,BaseLvAtkRate2New[24]=0,
    BaseLvAtkRate2New[82]=0,BaseLvAtkRate2New[83]=0,BaseLvAtkRate2New[84]=0,
    BaseLvAtkRate2New[31]=0,BaseLvAtkRate2New[32]=0,BaseLvAtkRate2New[33]=0,BaseLvAtkRate2New[34]=0,
    BaseLvAtkRate2New[92]=0,BaseLvAtkRate2New[93]=0,BaseLvAtkRate2New[94]=0,
    BaseLvAtkRate2New[41]=0,BaseLvAtkRate2New[42]=0,BaseLvAtkRate2New[43]=0,BaseLvAtkRate2New[44]=0,
    BaseLvAtkRate2New[102]=0,BaseLvAtkRate2New[103]=0,BaseLvAtkRate2New[104]=0,
    BaseLvAtkRate2New[112]=0,BaseLvAtkRate2New[113]=0,BaseLvAtkRate2New[114]=0,
    BaseLvAtkRate2New[51]=0,BaseLvAtkRate2New[52]=0,BaseLvAtkRate2New[53]=0,BaseLvAtkRate2New[54]=0,
    BaseLvAtkRate2New[122]=0,BaseLvAtkRate2New[123]=0,BaseLvAtkRate2New[124]=0,
    BaseLvAtkRate2New[61]=0,BaseLvAtkRate2New[62]=0,BaseLvAtkRate2New[63]=0,BaseLvAtkRate2New[64]=0,
    BaseLvAtkRate2New[132]=0,BaseLvAtkRate2New[133]=0,BaseLvAtkRate2New[134]=0;
  // --------------------职业物防成长
  BaseLvDefRateNew[1]=0,
    BaseLvDefRateNew[11]=0,BaseLvDefRateNew[12]=0,BaseLvDefRateNew[13]=0,BaseLvDefRateNew[14]=0,
    BaseLvDefRateNew[72]=1,BaseLvDefRateNew[73]=1,BaseLvDefRateNew[74]=1,
    BaseLvDefRateNew[21]=0,BaseLvDefRateNew[22]=0,BaseLvDefRateNew[23]=0,BaseLvDefRateNew[24]=0,
    BaseLvDefRateNew[82]=0,BaseLvDefRateNew[83]=0,BaseLvDefRateNew[84]=0,
    BaseLvDefRateNew[31]=0,BaseLvDefRateNew[32]=0,BaseLvDefRateNew[33]=0,BaseLvDefRateNew[34]=0,
    BaseLvDefRateNew[92]=0,BaseLvDefRateNew[93]=0,BaseLvDefRateNew[94]=0,
    BaseLvDefRateNew[41]=0,BaseLvDefRateNew[42]=0,BaseLvDefRateNew[43]=0,BaseLvDefRateNew[44]=0,
    BaseLvDefRateNew[102]=0,BaseLvDefRateNew[103]=0,BaseLvDefRateNew[104]=0,
    BaseLvDefRateNew[112]=0,BaseLvDefRateNew[113]=0,BaseLvDefRateNew[114]=0,
    BaseLvDefRateNew[51]=0,BaseLvDefRateNew[52]=0,BaseLvDefRateNew[53]=0,BaseLvDefRateNew[54]=0,
    BaseLvDefRateNew[122]=0,BaseLvDefRateNew[123]=0,BaseLvDefRateNew[124]=0,
    BaseLvDefRateNew[61]=0,BaseLvDefRateNew[62]=0,BaseLvDefRateNew[63]=0,BaseLvDefRateNew[64]=0,
    BaseLvDefRateNew[132]=0,BaseLvDefRateNew[133]=0,BaseLvDefRateNew[134]=0;
  // --------------------职业魔防成长
  BaseLvMDefRateNew[1]=0,
    BaseLvMDefRateNew[11]=0,BaseLvMDefRateNew[12]=0,BaseLvMDefRateNew[13]=0,BaseLvMDefRateNew[14]=0,
    BaseLvMDefRateNew[72]=0,BaseLvMDefRateNew[73]=0,BaseLvMDefRateNew[74]=0,
    BaseLvMDefRateNew[21]=0,BaseLvMDefRateNew[22]=0,BaseLvMDefRateNew[23]=0,BaseLvMDefRateNew[24]=0,
    BaseLvMDefRateNew[82]=0,BaseLvMDefRateNew[83]=0,BaseLvMDefRateNew[84]=0,
    BaseLvMDefRateNew[31]=0,BaseLvMDefRateNew[32]=0,BaseLvMDefRateNew[33]=0,BaseLvMDefRateNew[34]=0,
    BaseLvMDefRateNew[92]=0,BaseLvMDefRateNew[93]=0,BaseLvMDefRateNew[94]=0,
    BaseLvMDefRateNew[41]=0,BaseLvMDefRateNew[42]=0,BaseLvMDefRateNew[43]=0,BaseLvMDefRateNew[44]=0,
    BaseLvMDefRateNew[102]=0,BaseLvMDefRateNew[103]=0,BaseLvMDefRateNew[104]=0,
    BaseLvMDefRateNew[112]=0,BaseLvMDefRateNew[113]=0,BaseLvMDefRateNew[114]=0,
    BaseLvMDefRateNew[51]=0,BaseLvMDefRateNew[52]=0,BaseLvMDefRateNew[53]=0,BaseLvMDefRateNew[54]=0,
    BaseLvMDefRateNew[122]=0,BaseLvMDefRateNew[123]=0,BaseLvMDefRateNew[124]=0,
    BaseLvMDefRateNew[61]=0,BaseLvMDefRateNew[62]=0,BaseLvMDefRateNew[63]=0,BaseLvMDefRateNew[64]=0,
    BaseLvMDefRateNew[132]=0,BaseLvMDefRateNew[133]=0,BaseLvMDefRateNew[134]=0;

  BaseLvRateNew[1]=5,
    BaseLvRateNew[11]=20,BaseLvRateNew[12]=20,BaseLvRateNew[13]=20,BaseLvRateNew[14]=20,
    BaseLvRateNew[72]=20,BaseLvRateNew[73]=20,BaseLvRateNew[74]=20,
    BaseLvRateNew[21]=10,BaseLvRateNew[22]=10,BaseLvRateNew[23]=10,BaseLvRateNew[24]=12,
    BaseLvRateNew[82]=10,BaseLvRateNew[83]=10,BaseLvRateNew[84]=12,
    BaseLvRateNew[31]=12,BaseLvRateNew[32]=12,BaseLvRateNew[33]=12,BaseLvRateNew[34]=14,
    BaseLvRateNew[92]=12,BaseLvRateNew[93]=12,BaseLvRateNew[94]=14,
    BaseLvRateNew[41]=12,BaseLvRateNew[42]=12,BaseLvRateNew[43]=12,BaseLvRateNew[44]=16,
    BaseLvRateNew[102]=12,BaseLvRateNew[103]=12,BaseLvRateNew[104]=16,
    BaseLvRateNew[112]=12,BaseLvRateNew[113]=12,BaseLvRateNew[114]=16,
    BaseLvRateNew[51]=5,BaseLvRateNew[52]=5,BaseLvRateNew[53]=5,BaseLvRateNew[54]=14,
    BaseLvRateNew[122]=5,BaseLvRateNew[123]=5,BaseLvRateNew[124]=16,
    BaseLvRateNew[61]=12,BaseLvRateNew[62]=12,BaseLvRateNew[63]=12,BaseLvRateNew[64]=14,
    BaseLvRateNew[132]=12,BaseLvRateNew[133]=12,BaseLvRateNew[134]=16;

  HpRateNew[1]=0,
    HpRateNew[11]=1.5,HpRateNew[12]= 1.75,HpRateNew[13]= 2,HpRateNew[14]= 3,
    HpRateNew[72]=4,HpRateNew[73]=5,HpRateNew[74]=5,
    HpRateNew[21]=0.9,HpRateNew[22]=1.1,HpRateNew[23]=1.2,HpRateNew[24]=2.1,
    HpRateNew[82]=1.1,HpRateNew[83]=1.2,HpRateNew[84]=2.1,
    HpRateNew[31]=1.1,HpRateNew[32]= 1.3,HpRateNew[33]= 1.5,HpRateNew[34]= 2.3,
    HpRateNew[92]=1.3,HpRateNew[93]=1.5,HpRateNew[94]=2.4,
    HpRateNew[41]=1,HpRateNew[42]=1.2,HpRateNew[43]=1.3,HpRateNew[44]=2.1,
    HpRateNew[102]=1.2,HpRateNew[103]=1.3,HpRateNew[104]=2.1,
    HpRateNew[112]=1.2,HpRateNew[113]=1.3,HpRateNew[114]=2.1,
    HpRateNew[51]=0.9,HpRateNew[52]=1.1,HpRateNew[53]=1.2,HpRateNew[54]=2.2,
    HpRateNew[122]=1.2,HpRateNew[123]=1.4,HpRateNew[124]=2.1,
    HpRateNew[61]=1,HpRateNew[62]= 1.4,HpRateNew[63]= 1.6,HpRateNew[64]= 2.5,
    HpRateNew[132]=1.4,HpRateNew[133]=1.6,HpRateNew[134]=2.2;

    BaseHpNew[1]=0,
    BaseHpNew[11] =400,BaseHpNew[12] =400,BaseHpNew[13] =400,BaseHpNew[14]=1000,
    BaseHpNew[72] =1000,BaseHpNew[73] =1500,BaseHpNew[74] =1500,
    BaseHpNew[21] =  0,BaseHpNew[22] =  0,BaseHpNew[23] =  0,BaseHpNew[24]=1000,
    BaseHpNew[82] =  0,BaseHpNew[83] =  0,BaseHpNew[84] =  1000,
    BaseHpNew[31] =  0,BaseHpNew[32] =  0,BaseHpNew[33] =  0,BaseHpNew[34]=1000,
    BaseHpNew[92] =  0,BaseHpNew[93] =  0,BaseHpNew[94] =  1000,
    BaseHpNew[41] =  0,BaseHpNew[42] =  0,BaseHpNew[43] =  0,BaseHpNew[44]=1000,
    BaseHpNew[102]=  0,BaseHpNew[103]=  0,BaseHpNew[104]=  1000,
    BaseHpNew[112]=  0,BaseHpNew[113]=  0,BaseHpNew[114]=  1000,
    BaseHpNew[51] =  0,BaseHpNew[52] =  0,BaseHpNew[53] =  0,BaseHpNew[54]=1500,
    BaseHpNew[122]=300,BaseHpNew[123]=300,BaseHpNew[124]=1000,
    BaseHpNew[61] =  0,BaseHpNew[62] =  0,BaseHpNew[63] =  0,BaseHpNew[64]=1000,
    BaseHpNew[132]=  0,BaseHpNew[133]=  0,BaseHpNew[134]=  1000;

  // ------------------------------职业空手攻速
  BaseJobAtkSpdNew[1]=156,
    BaseJobAtkSpdNew[11]=153,BaseJobAtkSpdNew[12]=156,BaseJobAtkSpdNew[13]=156,BaseJobAtkSpdNew[14]=156,
    BaseJobAtkSpdNew[72]=156,BaseJobAtkSpdNew[73]=156,BaseJobAtkSpdNew[74]=156,
    BaseJobAtkSpdNew[21]=146,BaseJobAtkSpdNew[22]=146,BaseJobAtkSpdNew[23]=151,BaseJobAtkSpdNew[24]=151,
    BaseJobAtkSpdNew[82]=156,BaseJobAtkSpdNew[83]=156,BaseJobAtkSpdNew[84]=156,
    BaseJobAtkSpdNew[31]=156,BaseJobAtkSpdNew[32]=156,BaseJobAtkSpdNew[33]=156,BaseJobAtkSpdNew[34]=156,
    BaseJobAtkSpdNew[92]=156,BaseJobAtkSpdNew[93]=156,BaseJobAtkSpdNew[94]=156,
    BaseJobAtkSpdNew[41]=156,BaseJobAtkSpdNew[42]=156,BaseJobAtkSpdNew[43]=156,BaseJobAtkSpdNew[44]=156,
    BaseJobAtkSpdNew[102]=156,BaseJobAtkSpdNew[103]=156,BaseJobAtkSpdNew[104]=156,
    BaseJobAtkSpdNew[112]=156,BaseJobAtkSpdNew[113]=156,BaseJobAtkSpdNew[114]=156,
    BaseJobAtkSpdNew[51]=156,BaseJobAtkSpdNew[52]=156,BaseJobAtkSpdNew[53]=151,BaseJobAtkSpdNew[54]=151,
    BaseJobAtkSpdNew[122]=156,BaseJobAtkSpdNew[123]=156,BaseJobAtkSpdNew[124]=156,
    BaseJobAtkSpdNew[61]=156,BaseJobAtkSpdNew[62]=156,BaseJobAtkSpdNew[63]=156,BaseJobAtkSpdNew[64]=156,
    BaseJobAtkSpdNew[132]=156,BaseJobAtkSpdNew[133]=156,BaseJobAtkSpdNew[134]=156;
  // -- 长矛各职业对应攻速
  // -- 这是剑士和骑士系列
  SpearAtkSpdNew[1]=0,
    SpearAtkSpdNew[11]=-17, SpearAtkSpdNew[12]=-15, SpearAtkSpdNew[13]= -8, SpearAtkSpdNew[14]= -8,
    SpearAtkSpdNew[72]=-13, SpearAtkSpdNew[73]=-10, SpearAtkSpdNew[74]=-10,
    SpearAtkSpdNew[21]=  0, SpearAtkSpdNew[22]=  0, SpearAtkSpdNew[23]=  0, SpearAtkSpdNew[24]=  0,
    SpearAtkSpdNew[82]=  0, SpearAtkSpdNew[83]=  0, SpearAtkSpdNew[84]=  0,
    SpearAtkSpdNew[31]=  0, SpearAtkSpdNew[32]=  0, SpearAtkSpdNew[33]=  0, SpearAtkSpdNew[34]=  0,
    SpearAtkSpdNew[92]=  0, SpearAtkSpdNew[93]=  0, SpearAtkSpdNew[94]=  0,
    SpearAtkSpdNew[41]=  0, SpearAtkSpdNew[42]=  0, SpearAtkSpdNew[43]=  0, SpearAtkSpdNew[44]=  0,
    SpearAtkSpdNew[102]= 0, SpearAtkSpdNew[103]= 0, SpearAtkSpdNew[104]= 0,
    SpearAtkSpdNew[112]= 0, SpearAtkSpdNew[113]= 0, SpearAtkSpdNew[114]= 0,
    SpearAtkSpdNew[51]=  0, SpearAtkSpdNew[52]=  0, SpearAtkSpdNew[53]=  0, SpearAtkSpdNew[54]=  0,
    SpearAtkSpdNew[122]= 0, SpearAtkSpdNew[123]= 0, SpearAtkSpdNew[124]= 0,
    SpearAtkSpdNew[61]=  0, SpearAtkSpdNew[62]=  0, SpearAtkSpdNew[63]=  0, SpearAtkSpdNew[64]=  0,
    SpearAtkSpdNew[132]= 0, SpearAtkSpdNew[133]= 0, SpearAtkSpdNew[134]= 0;
  // -- 长剑各职业对应攻速
  SwordAtkSpdNew[1]=-17,
    SwordAtkSpdNew[11]= -7, SwordAtkSpdNew[12]= -5, SwordAtkSpdNew[13]=-12, SwordAtkSpdNew[14]= -12,
    SwordAtkSpdNew[72]= -3, SwordAtkSpdNew[73]= -5, SwordAtkSpdNew[74]= -5,
    SwordAtkSpdNew[21]=  0, SwordAtkSpdNew[22]=  0, SwordAtkSpdNew[23]=  0, SwordAtkSpdNew[24]=  0,
    SwordAtkSpdNew[82]=  0, SwordAtkSpdNew[83]=  0, SwordAtkSpdNew[84]=  0,
    SwordAtkSpdNew[31]=-10, SwordAtkSpdNew[32]=-10, SwordAtkSpdNew[33]=-25, SwordAtkSpdNew[34]=-25,
    SwordAtkSpdNew[92]=-10, SwordAtkSpdNew[93]=-25, SwordAtkSpdNew[94]=-25,
    SwordAtkSpdNew[41]=  0, SwordAtkSpdNew[42]=  0, SwordAtkSpdNew[43]=  0, SwordAtkSpdNew[44]=  0,
    SwordAtkSpdNew[102]= 0, SwordAtkSpdNew[103]= 0, SwordAtkSpdNew[104]= 0,
    SwordAtkSpdNew[112]= 0, SwordAtkSpdNew[113]= 0, SwordAtkSpdNew[114]= 0,
    SwordAtkSpdNew[51]=  0, SwordAtkSpdNew[52]=  0, SwordAtkSpdNew[53]=  0, SwordAtkSpdNew[54]=  0,
    SwordAtkSpdNew[122]= 0, SwordAtkSpdNew[123]= 0, SwordAtkSpdNew[124]= 0,
    SwordAtkSpdNew[61]=-12, SwordAtkSpdNew[62]=-10, SwordAtkSpdNew[63]=-25, SwordAtkSpdNew[64]=-25,
    SwordAtkSpdNew[132]=-10, SwordAtkSpdNew[133]=-25, SwordAtkSpdNew[134]=-25;
  // -- 锤子各职业对应攻速
  MaceAtkSpdNew[1]=-10,
    MaceAtkSpdNew[11]=-10, MaceAtkSpdNew[12]= -5, MaceAtkSpdNew[13]= -5, MaceAtkSpdNew[14]= -5,
    MaceAtkSpdNew[72]= -5, MaceAtkSpdNew[73]= -4, MaceAtkSpdNew[74]= -4,
    MaceAtkSpdNew[21]=  0, MaceAtkSpdNew[22]=  0, MaceAtkSpdNew[23]=  0, MaceAtkSpdNew[24]=  0,
    MaceAtkSpdNew[82]=  0, MaceAtkSpdNew[83]=  0, MaceAtkSpdNew[84]=  0,
    MaceAtkSpdNew[31]=  0, MaceAtkSpdNew[32]=  0, MaceAtkSpdNew[33]=  0, MaceAtkSpdNew[34]=  0,
    MaceAtkSpdNew[92]=  0, MaceAtkSpdNew[93]=  0, MaceAtkSpdNew[94]=  0,
    MaceAtkSpdNew[41]=  0, MaceAtkSpdNew[42]=  0, MaceAtkSpdNew[43]=  0, MaceAtkSpdNew[44]=  0,
    MaceAtkSpdNew[102]= 0, MaceAtkSpdNew[103]= 0, MaceAtkSpdNew[104]= 0,
    MaceAtkSpdNew[112]= 0, MaceAtkSpdNew[113]= 0, MaceAtkSpdNew[114]= 0,
    MaceAtkSpdNew[51]= -5, MaceAtkSpdNew[52]= -3, MaceAtkSpdNew[53]=  0, MaceAtkSpdNew[54]=  0,
    MaceAtkSpdNew[122]=-3, MaceAtkSpdNew[123]=-5, MaceAtkSpdNew[124]=-5,
    MaceAtkSpdNew[61]=-10, MaceAtkSpdNew[62]= -8, MaceAtkSpdNew[63]= -8, MaceAtkSpdNew[64]= -8,
    MaceAtkSpdNew[132]=-8, MaceAtkSpdNew[133]= -8, MaceAtkSpdNew[134]= -8;
  // -- 拳刃各职业对应攻速
  KatarAtkSpdNew[1]=0,
    KatarAtkSpdNew[11]=  0, KatarAtkSpdNew[12]=  0, KatarAtkSpdNew[13]=  0, KatarAtkSpdNew[14]=  0,
    KatarAtkSpdNew[72]=  0, KatarAtkSpdNew[73]=  0, KatarAtkSpdNew[74]=  0,
    KatarAtkSpdNew[21]=  0, KatarAtkSpdNew[22]=  0, KatarAtkSpdNew[23]=  0, KatarAtkSpdNew[24]=  0,
    KatarAtkSpdNew[82]=  0, KatarAtkSpdNew[83]=  0, KatarAtkSpdNew[84]=  0,
    KatarAtkSpdNew[31]=  0, KatarAtkSpdNew[32]= -2, KatarAtkSpdNew[33]= -2, KatarAtkSpdNew[34]= -2,
    KatarAtkSpdNew[92]=  0, KatarAtkSpdNew[93]=  0, KatarAtkSpdNew[94]=  0,
    KatarAtkSpdNew[41]=  0, KatarAtkSpdNew[42]=  0, KatarAtkSpdNew[43]=  0, KatarAtkSpdNew[44]=  0,
    KatarAtkSpdNew[102]= 0, KatarAtkSpdNew[103]= 0, KatarAtkSpdNew[104]= 0,
    KatarAtkSpdNew[112]= 0, KatarAtkSpdNew[113]= 0, KatarAtkSpdNew[114]= 0,
    KatarAtkSpdNew[51]=  0, KatarAtkSpdNew[52]=  0, KatarAtkSpdNew[53]=  0, KatarAtkSpdNew[54]=  0,
    KatarAtkSpdNew[122]= 0, KatarAtkSpdNew[123]= 0, KatarAtkSpdNew[124]= 0,
    KatarAtkSpdNew[61]=  0, KatarAtkSpdNew[62]=  0, KatarAtkSpdNew[63]=  0, KatarAtkSpdNew[64]=  0,
    KatarAtkSpdNew[132]= 0, KatarAtkSpdNew[133]= 0, KatarAtkSpdNew[134]= 0;
  // -- 弓各职业对应攻速
  BowAtkSpdNew[1]=0,
    BowAtkSpdNew[11]=  0, BowAtkSpdNew[12]=  0, BowAtkSpdNew[13]=  0, BowAtkSpdNew[14]=  0,
    BowAtkSpdNew[72]=  0, BowAtkSpdNew[73]=  0, BowAtkSpdNew[74]=  0,
    BowAtkSpdNew[21]=  0, BowAtkSpdNew[22]=  0, BowAtkSpdNew[23]=  0, BowAtkSpdNew[24]=  0,
    BowAtkSpdNew[82]=  0, BowAtkSpdNew[83]=  0, BowAtkSpdNew[84]=  0,
    BowAtkSpdNew[31]=  0, BowAtkSpdNew[32]=  0, BowAtkSpdNew[33]=  0, BowAtkSpdNew[34]=  0,
    BowAtkSpdNew[92]=-10, BowAtkSpdNew[93]=-10, BowAtkSpdNew[94]=-10,
    BowAtkSpdNew[41]=-10, BowAtkSpdNew[42]= -8, BowAtkSpdNew[43]= -9, BowAtkSpdNew[44]= -9,
    BowAtkSpdNew[102]= 0, BowAtkSpdNew[103]= 0, BowAtkSpdNew[104]= 0,
    BowAtkSpdNew[112]= 0, BowAtkSpdNew[113]= 0, BowAtkSpdNew[114]= 0,
    BowAtkSpdNew[51]=  0, BowAtkSpdNew[52]=  0, BowAtkSpdNew[53]=  0, BowAtkSpdNew[54]=  0,
    BowAtkSpdNew[122]= 0, BowAtkSpdNew[123]= 0, BowAtkSpdNew[124]= 0,
    BowAtkSpdNew[61]=  0, BowAtkSpdNew[62]=  0, BowAtkSpdNew[63]=  0, BowAtkSpdNew[64]=  0,
    BowAtkSpdNew[132]= 0, BowAtkSpdNew[133]= 0, BowAtkSpdNew[134]= 0;
  // -- 法杖各职业对应攻速
  StaffAtkSpdNew[1]=-25,
    StaffAtkSpdNew[11]=  0, StaffAtkSpdNew[12]=  0, StaffAtkSpdNew[13]=  0, StaffAtkSpdNew[14]=  0,
    StaffAtkSpdNew[72]=  0, StaffAtkSpdNew[73]=  0, StaffAtkSpdNew[74]=  0,
    StaffAtkSpdNew[21]= -5, StaffAtkSpdNew[22]= -3, StaffAtkSpdNew[23]= -5, StaffAtkSpdNew[24]= -5,
    StaffAtkSpdNew[82]=  0, StaffAtkSpdNew[83]=  0, StaffAtkSpdNew[84]=  0,
    StaffAtkSpdNew[31]=  0, StaffAtkSpdNew[32]=  0, StaffAtkSpdNew[33]=  0, StaffAtkSpdNew[34]=  0,
    StaffAtkSpdNew[92]=  0, StaffAtkSpdNew[93]=  0, StaffAtkSpdNew[94]=  0,
    StaffAtkSpdNew[41]=  0, StaffAtkSpdNew[42]=  0, StaffAtkSpdNew[43]=  0, StaffAtkSpdNew[44]=  0,
    StaffAtkSpdNew[102]= 0, StaffAtkSpdNew[103]= 0, StaffAtkSpdNew[104]= 0,
    StaffAtkSpdNew[112]= 0, StaffAtkSpdNew[113]= 0, StaffAtkSpdNew[114]= 0,
    StaffAtkSpdNew[51]=-20, StaffAtkSpdNew[52]=-20, StaffAtkSpdNew[53]=-15, StaffAtkSpdNew[54]=-15,
    StaffAtkSpdNew[122]=-20, StaffAtkSpdNew[123]=-10, StaffAtkSpdNew[124]=-10,
    StaffAtkSpdNew[61]=-10, StaffAtkSpdNew[62]= -8, StaffAtkSpdNew[63]= -8, StaffAtkSpdNew[64]= -8,
    StaffAtkSpdNew[132]= 0, StaffAtkSpdNew[133]= 0, StaffAtkSpdNew[134]= 0;
  // -- 匕首各职业对应攻速
  KnifeAtkSpdNew[1]=-15,
    KnifeAtkSpdNew[11]= -7, KnifeAtkSpdNew[12]= -9, KnifeAtkSpdNew[13]=-10, KnifeAtkSpdNew[14]=-10,
    KnifeAtkSpdNew[72]= -8, KnifeAtkSpdNew[73]= -7, KnifeAtkSpdNew[74]= -7,
    KnifeAtkSpdNew[21]=  0, KnifeAtkSpdNew[22]= -4, KnifeAtkSpdNew[23]= -7, KnifeAtkSpdNew[24]= -7,
    KnifeAtkSpdNew[82]=  0, KnifeAtkSpdNew[83]=  0, KnifeAtkSpdNew[84]=  0,
    KnifeAtkSpdNew[31]= -8, KnifeAtkSpdNew[32]= -2, KnifeAtkSpdNew[33]= -2, KnifeAtkSpdNew[34]= -2,
    KnifeAtkSpdNew[92]= -2, KnifeAtkSpdNew[93]= -2, KnifeAtkSpdNew[94]= -2,
    KnifeAtkSpdNew[41]=-15, KnifeAtkSpdNew[42]=-13, KnifeAtkSpdNew[43]=-10, KnifeAtkSpdNew[44]=-10,
    KnifeAtkSpdNew[102]= 0, KnifeAtkSpdNew[103]= 0, KnifeAtkSpdNew[104]= 0,
    KnifeAtkSpdNew[112]= 0, KnifeAtkSpdNew[113]= 0, KnifeAtkSpdNew[114]= 0,
    KnifeAtkSpdNew[51]=  0, KnifeAtkSpdNew[52]=  0, KnifeAtkSpdNew[53]=  0, KnifeAtkSpdNew[54]=  0,
    KnifeAtkSpdNew[122]= 0, KnifeAtkSpdNew[123]= 0, KnifeAtkSpdNew[124]= 0,
    KnifeAtkSpdNew[61]=-12, KnifeAtkSpdNew[62]=-10, KnifeAtkSpdNew[63]=-20, KnifeAtkSpdNew[64]=-20,
    KnifeAtkSpdNew[132]=-10, KnifeAtkSpdNew[133]=-20, KnifeAtkSpdNew[134]=-20;
  // -- 斧头各职业对应攻速
  AxeAtkSpdNew[1]=-15,
    AxeAtkSpdNew[11]= -20, AxeAtkSpdNew[12]= -15, AxeAtkSpdNew[13]=-12, AxeAtkSpdNew[14]=-12,
    AxeAtkSpdNew[72]= -20, AxeAtkSpdNew[73]= -15, AxeAtkSpdNew[74]=-15,
    AxeAtkSpdNew[21]=  0, AxeAtkSpdNew[22]=  0, AxeAtkSpdNew[23]=  0, AxeAtkSpdNew[24]=  0,
    AxeAtkSpdNew[82]=  0, AxeAtkSpdNew[83]=  0, AxeAtkSpdNew[84]=  0,
    AxeAtkSpdNew[31]= 0, AxeAtkSpdNew[32]= 0, AxeAtkSpdNew[33]= 0, AxeAtkSpdNew[34]= 0,
    AxeAtkSpdNew[92]=  0, AxeAtkSpdNew[93]=  0, AxeAtkSpdNew[94]=  0,
    AxeAtkSpdNew[41]=  0, AxeAtkSpdNew[42]=  0, AxeAtkSpdNew[43]=  0, AxeAtkSpdNew[44]= 0,
    AxeAtkSpdNew[102]= 0, AxeAtkSpdNew[103]= 0, AxeAtkSpdNew[104]= 0,
    AxeAtkSpdNew[112]= 0, AxeAtkSpdNew[113]= 0, AxeAtkSpdNew[114]= 0,
    AxeAtkSpdNew[51]=  0, AxeAtkSpdNew[52]=  0, AxeAtkSpdNew[53]=  0, AxeAtkSpdNew[54]=  0,
    AxeAtkSpdNew[122]= 0, AxeAtkSpdNew[123]= 0, AxeAtkSpdNew[124]= 0,
    AxeAtkSpdNew[61]=-15, AxeAtkSpdNew[62]=-13, AxeAtkSpdNew[63]=-10, AxeAtkSpdNew[64]=-10,
    AxeAtkSpdNew[132]=-13, AxeAtkSpdNew[133]=-10, AxeAtkSpdNew[134]=-10;
  // -- 拳套各职业对应攻速
  FistAtkSpdNew[1] = 0,
    FistAtkSpdNew[11] = 0, FistAtkSpdNew[12] = 0, FistAtkSpdNew[13] = 0, FistAtkSpdNew[14]= 0,
    FistAtkSpdNew[72] = 0, FistAtkSpdNew[73] = 0, FistAtkSpdNew[74] = 0,
    FistAtkSpdNew[21] = 0, FistAtkSpdNew[22] = 0, FistAtkSpdNew[23] = 0, FistAtkSpdNew[24]= 0,
    FistAtkSpdNew[82] = 0, FistAtkSpdNew[83] = 0, FistAtkSpdNew[84] = 0,
    FistAtkSpdNew[31] = 0, FistAtkSpdNew[32] = 0, FistAtkSpdNew[33] = 0, FistAtkSpdNew[34]= 0,
    FistAtkSpdNew[92] = 0, FistAtkSpdNew[93] = 0, FistAtkSpdNew[94] = 0,
    FistAtkSpdNew[41] = 0, FistAtkSpdNew[42] = 0, FistAtkSpdNew[43] = 0, FistAtkSpdNew[44]= 0,
    FistAtkSpdNew[102]= 0, FistAtkSpdNew[103]= 0, FistAtkSpdNew[104]= 0,
    FistAtkSpdNew[112]= 0, FistAtkSpdNew[113]= 0, FistAtkSpdNew[114]= 0,
    FistAtkSpdNew[51] = 0, FistAtkSpdNew[52] =-20, FistAtkSpdNew[53] =-5, FistAtkSpdNew[54]=-5,
    FistAtkSpdNew[122]= 0, FistAtkSpdNew[123]= -1, FistAtkSpdNew[124]=-1,
    FistAtkSpdNew[61] = 0, FistAtkSpdNew[62] = 0, FistAtkSpdNew[63] = 0, FistAtkSpdNew[64]= 0,
    FistAtkSpdNew[132]= 0, FistAtkSpdNew[133]= 0, FistAtkSpdNew[134]= 0;

  // -- 副手各职业对应攻速(暂时修改掉副手的ASPD惩罚)
  ShieldAtkSpdNew[1]= 0,
    ShieldAtkSpdNew[11]=  0, ShieldAtkSpdNew[12]=  0, ShieldAtkSpdNew[13]=  0, ShieldAtkSpdNew[14]=  0,
    ShieldAtkSpdNew[72]=  0, ShieldAtkSpdNew[73]=  0, ShieldAtkSpdNew[74]=  0,
    ShieldAtkSpdNew[21]=  0, ShieldAtkSpdNew[22]=  0, ShieldAtkSpdNew[23]=  0, ShieldAtkSpdNew[24]=  0,
    ShieldAtkSpdNew[82]=  0, ShieldAtkSpdNew[83]=  0, ShieldAtkSpdNew[84]=  0,
    ShieldAtkSpdNew[31]=  0, ShieldAtkSpdNew[32]=  0, ShieldAtkSpdNew[33]=  0, ShieldAtkSpdNew[34]=  0,
    ShieldAtkSpdNew[92]=  0, ShieldAtkSpdNew[93]=  0, ShieldAtkSpdNew[94]=  0,
    ShieldAtkSpdNew[41]=  0, ShieldAtkSpdNew[42]=  0, ShieldAtkSpdNew[43]=  0, ShieldAtkSpdNew[44]=  0,
    ShieldAtkSpdNew[102]= 0, ShieldAtkSpdNew[103]= 0, ShieldAtkSpdNew[104]= 0,
    ShieldAtkSpdNew[112]= 0, ShieldAtkSpdNew[113]= 0, ShieldAtkSpdNew[114]= 0,
    ShieldAtkSpdNew[51]=  0, ShieldAtkSpdNew[52]=  0, ShieldAtkSpdNew[53]=  0, ShieldAtkSpdNew[54]=  0,
    ShieldAtkSpdNew[122]= 0, ShieldAtkSpdNew[123]= 0, ShieldAtkSpdNew[124]= 0,
    ShieldAtkSpdNew[61]=  0, ShieldAtkSpdNew[62]=  0, ShieldAtkSpdNew[63]=  0, ShieldAtkSpdNew[64]=  0,
    ShieldAtkSpdNew[132]= 0, ShieldAtkSpdNew[133]= 0, ShieldAtkSpdNew[134]= 0;

    Recover[1]=std::make_pair(0.5,1),
    Recover[11]=std::make_pair(0.25,2),Recover[12]=std::make_pair(0.25,3),Recover[13]=std::make_pair(0.25,3),Recover[14]=std::make_pair(0.25,5),
    Recover[72]=std::make_pair(0.25,3),Recover[73]=std::make_pair(0.25,3),Recover[74]=std::make_pair(0.25,5),
    Recover[21]=std::make_pair(0.1,6),Recover[22]=std::make_pair(0.1,9),Recover[23]=std::make_pair(0.1,9),Recover[24]=std::make_pair(0.1,11),
    Recover[82]=std::make_pair(0.1,9),Recover[83]=std::make_pair(0.1,9),Recover[84]=std::make_pair(0.1,11),
    Recover[31]=std::make_pair(0.15,2),Recover[32]=std::make_pair(0.15,4),Recover[33]=std::make_pair(0.15,4),Recover[34]=std::make_pair(0.15,5),
    Recover[92]=std::make_pair(0.15,4),Recover[93]=std::make_pair(0.15,4),Recover[94]=std::make_pair(0.15,5),
    Recover[41]=std::make_pair(0.18,2),Recover[42]=std::make_pair(0.18,4),Recover[43]=std::make_pair(0.18,4),Recover[44]=std::make_pair(0.18,6),
    Recover[102]=std::make_pair(0.18,4),Recover[103]=std::make_pair(0.18,4),Recover[104]=std::make_pair(0.18,6),
    Recover[112]=std::make_pair(0.18,4),Recover[113]=std::make_pair(0.18,4),Recover[114]=std::make_pair(0.18,6),
    Recover[51]=std::make_pair(0.2,5),Recover[52]=std::make_pair(0.2,7),Recover[53]=std::make_pair(0.2,7),Recover[54]=std::make_pair(0.2,10),
    Recover[122]=std::make_pair(0.2,7),Recover[123]=std::make_pair(0.2,7),Recover[124]=std::make_pair(0.2,7),
    Recover[61]=std::make_pair(0.25,3),Recover[62]=std::make_pair(0.25,4),Recover[63]=std::make_pair(0.25,4),Recover[64]=std::make_pair(0.25,6),
    Recover[132]=std::make_pair(0.25,4),Recover[133]=std::make_pair(0.25,4),Recover[134]=std::make_pair(0.25,6);
}

AttrFunc::~AttrFunc()
{
}

// -- 等差数列求和
DWORD AttrFunc::CalcSumNew(DWORD num)
{
  DWORD result=0;
  for (DWORD i = num; i > 0; --i)
    result=result+i;
  return result;
}

// -- 值修正
float AttrFunc::modifyValue(float value, float adjust, const std::string& action)
{
  if (action == "great")
  {
    if (value >= adjust)
      value = adjust;
  }
  else if (action == "less")
  {
    if (value <= adjust)
      value = adjust;
  }
  else if (action == "floor")
  {
    if (value != 0)
      value = floor(value * 1000 + 0.5) / 1000;
  }

  return value;
}

// -- 武器基本攻速(170--长矛;180---剑;190---法杖;200---拳刃;210---弓;220---锤子;230---斧头;240---书;250---匕首;260---乐器;270---鞭子;280---试管;290---拳套)
DWORD AttrFunc::WeaponAtkSpdNew(DWORD type, DWORD profession)
{
  if (type==170)
     return SpearAtkSpdNew[profession];
  else if (type==180)
     return SwordAtkSpdNew[profession];
  else if (type==190)
     return StaffAtkSpdNew[profession];
  else if (type==200)
     return KatarAtkSpdNew[profession];
  else if (type==210)
     return BowAtkSpdNew[profession];
  else if (type==220)
     return MaceAtkSpdNew[profession];
  else if (type== 230)
     return AxeAtkSpdNew[profession];
  else if (type==250)
     return KnifeAtkSpdNew[profession];
  else if (type==290)
     return FistAtkSpdNew[profession];
  else if (type==510 || type==511 || type==512 || type==513 || type==514 || type==515)
     return ShieldAtkSpdNew[profession];
  return 0;
}

bool AttrFunc::checkRemoteAtk(DWORD profession, DWORD WeaponType)
{
  // ---------------------------------------------------------------流氓穿戴弓时是远程职业
  if ((profession==92 || profession==93 || profession==94) && WeaponType==210)
    return true;

  TSetDWORD setIDs = TSetDWORD{41,42,43,44,102,103,104,112,113,114};
  return setIDs.find(profession) != setIDs.end();
}

// -- 刷新玩家属性
static std::vector<EAttrType> types = { EATTRTYPE_STR, EATTRTYPE_INT, EATTRTYPE_AGI, EATTRTYPE_DEX, EATTRTYPE_VIT, EATTRTYPE_LUK,
                EATTRTYPE_ATK, EATTRTYPE_DEF, EATTRTYPE_MATK, EATTRTYPE_MDEF, EATTRTYPE_MAXHP, EATTRTYPE_MAXSP,
                EATTRTYPE_HIT, EATTRTYPE_FLEE, EATTRTYPE_CRI, EATTRTYPE_CRIRES, EATTRTYPE_CRIDAMPER, EATTRTYPE_CRIDEFPER,
                EATTRTYPE_MOVESPD, EATTRTYPE_MOVESPDPER, EATTRTYPE_CASTSPD, EATTRTYPE_ATKPER, EATTRTYPE_DEFPER, EATTRTYPE_MATKPER, EATTRTYPE_MDEFPER,
                EATTRTYPE_MAXHPPER, EATTRTYPE_MAXSPPER, EATTRTYPE_REFINE, EATTRTYPE_MREFINE, EATTRTYPE_RESTORESPD, EATTRTYPE_SPRESTORESPD,
                EATTRTYPE_EQUIPASPD, EATTRTYPE_SKILLASPD, EATTRTYPE_ATKSPD, EATTRTYPE_CRIPER, EATTRTYPE_DAMREDUC, EATTRTYPE_MDAMREDUC, EATTRTYPE_RESTORESPDPER, EATTRTYPE_SPRESTORESPDPER };
void AttrFunc::calcUserAttr(SceneUser* user, DWORD index)
{
  if (user == nullptr)
    return;

  EProfession profession = user->getProfession();
  DWORD level = user->getLevel();
  DWORD map = 0;
  if (user->isOnPvp())
    map = 1;
  else if (user->isOnGvg())
    map = 2;

  for (auto &v : types)
  {
    if (user->testAttr(v) == true)
      calcAttr(user, level, profession, map, v);
  }
  XDBG << "[属性-计算]" << user->accid << user->id << user->getProfession() << user->name << "第" << index << "次计算" << XEND;
}

// -- 计算属性
void AttrFunc::calcAttr(SceneUser* user, DWORD lv, DWORD pro, DWORD map, EAttrType attr)
{
  if (user == nullptr)
    return;

  float extra = 0;
  float f = 0;

  if (attr == EATTRTYPE_STR)
  {
    calcAttr(user, lv, pro, map, EATTRTYPE_ATK);
  }
  else if (attr == EATTRTYPE_INT)
  {
    calcAttr(user, lv, pro, map, EATTRTYPE_MATK);
    calcAttr(user, lv, pro, map, EATTRTYPE_MDEF);
    calcAttr(user, lv, pro, map, EATTRTYPE_MAXSP);
  }
  else if (attr == EATTRTYPE_AGI)
  {
    calcAttr(user, lv, pro, map, EATTRTYPE_FLEE);
    calcAttr(user, lv, pro, map, EATTRTYPE_ATKSPD);
  }
  else if (attr == EATTRTYPE_DEX)
  {
    calcAttr(user, lv, pro, map, EATTRTYPE_ATK);
    calcAttr(user, lv, pro, map, EATTRTYPE_HIT);
    calcAttr(user, lv, pro, map, EATTRTYPE_CASTSPD);
  }
  else if (attr == EATTRTYPE_VIT)
  {
    calcAttr(user, lv, pro, map, EATTRTYPE_DEF);
    calcAttr(user, lv, pro, map, EATTRTYPE_MAXHP);
    calcAttr(user, lv, pro, map, EATTRTYPE_RESTORESPD);
  }
  else if (attr == EATTRTYPE_LUK)
  {
    calcAttr(user, lv, pro, map, EATTRTYPE_ATK);
    calcAttr(user, lv, pro, map, EATTRTYPE_CRI);
    calcAttr(user, lv, pro, map, EATTRTYPE_CRIRES);
  }
  else if (attr == EATTRTYPE_ATK)
  {
    float dex = user->getAttr(EATTRTYPE_DEX);
    float str = user->getAttr(EATTRTYPE_STR);
    float luk = user->getAttr(EATTRTYPE_LUK);

    // ----------玩家物理攻击计算公式
    if (checkRemoteAtk(pro, user->getWeaponType()))
      extra = dex * 2 + floor(dex * dex / 100) + floor(str / 5) + floor(luk / 5) + BaseLvAtkRate1New[pro] * lv;
    else
      extra = str * 2 + floor(str * str / 100) + floor(dex / 5) + floor(luk / 5) + BaseLvAtkRate1New[pro] * lv;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = extra + user->getOtherAttr(EATTRTYPE_ATK);
    f = modifyValue(f, 0, "less");
    user->setAttr(attr, f);

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWATK);
  }
  // ----------玩家物理防御计算公式
  else if (attr == EATTRTYPE_DEF)
  {
    extra = user->getAttr(EATTRTYPE_VIT);
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = extra + user->getOtherAttr(EATTRTYPE_DEF);
    user->setAttr(attr, modifyValue(f, 0, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWDEF);
  }
 //----------玩家魔法攻击计算公式   
  else if (attr == EATTRTYPE_MATK)
  {
    float i = user->getAttr(EATTRTYPE_INT);
    extra = i * 2 + floor(i * i / 100);
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = extra + user->getOtherAttr(EATTRTYPE_MATK);
    user->setAttr(attr, modifyValue(f, 0, ""));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMATK);
  }
 // ----------玩家魔法防御计算公式
  else if (attr == EATTRTYPE_MDEF)
  {
    extra = user->getAttr(EATTRTYPE_INT);
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = extra + user->getOtherAttr(EATTRTYPE_MDEF);
    user->setAttr(attr, f);

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMDEF);
  }
 // ----------玩家生命上限计算公式
  else if (attr == EATTRTYPE_MAXHP)
  {
    float vit = user->getAttr(EATTRTYPE_VIT);
    extra = (100 + lv * BaseLvRateNew[pro] + CalcSumNew(lv) * HpRateNew[pro] + BaseHpNew[pro]) * (1 + vit / 100);
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    if (map == 1 or map == 2)// ----map=1 表示pvp,map=2 表示gvg
      f = 4 * (extra + user->getOtherAttr(EATTRTYPE_MAXHP)) * (1 + user->getAttr(EATTRTYPE_MAXHPPER));
    else
      f = (extra + user->getOtherAttr(EATTRTYPE_MAXHP)) * (1 + user->getAttr(EATTRTYPE_MAXHPPER));

    user->setAttr(attr, f);

    calcAttr(user, lv, pro, map, EATTRTYPE_RESTORESPD);
    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMAXHP);
  }
 // ----------玩家魔法上限计算公式
  else if (attr == EATTRTYPE_MAXSP)
  {
    float i = user->getAttr(EATTRTYPE_INT);
    extra = (20 + (lv * Recover[pro].second)) * (1 + i / 100);
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = (extra + user->getOtherAttr(EATTRTYPE_MAXSP)) * (1 + user->getAttr(EATTRTYPE_MAXSPPER));
    user->setAttr(attr, f);

    calcAttr(user, lv, pro, map, EATTRTYPE_SPRESTORESPD);
    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMAXSP);
  }
 // ----------玩家命中计算公式
  else if (attr == EATTRTYPE_HIT)
  {
    extra = lv + user->getAttr(EATTRTYPE_DEX);
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = (extra + user->getOtherAttr(EATTRTYPE_HIT)) * (1 + user->getOtherAttr(EATTRTYPE_HITPER));
    user->setAttr(attr, modifyValue(f, 0, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWHIT);
  }
 // ----------玩家闪避计算公式
  else if (attr == EATTRTYPE_FLEE)
  {
    extra = lv + user->getAttr(EATTRTYPE_AGI);
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = (extra + user->getOtherAttr(EATTRTYPE_FLEE)) * (1 + user->getOtherAttr(EATTRTYPE_FLEEPER));
    user->setAttr(attr, modifyValue(f, 0, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWFLEE);
  }
 // ----------玩家暴击计算公式
  else if (attr == EATTRTYPE_CRI)
  {
    extra = 1 + floor(user->getAttr(EATTRTYPE_LUK) / 3);
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = (extra + user->getOtherAttr(EATTRTYPE_CRI)) * (1 + user->getOtherAttr(EATTRTYPE_CRIPER));
    user->setAttr(attr, modifyValue(f, 0, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWCRI);
  }
 // ----------玩家防暴计算公式
  else if (attr == EATTRTYPE_CRIRES)
  {
    extra = 1 + floor(user->getAttr(EATTRTYPE_LUK) / 5);
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = extra + user->getOtherAttr(EATTRTYPE_CRIRES);
    user->setAttr(attr, f);

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWCRIRES);
  }
 // ----------玩家暴伤计算公式
  else if (attr == EATTRTYPE_CRIDAMPER)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_CRIDAMPER);
    user->setAttr(attr, modifyValue(f, 0, "less"));
  }
 // ----------玩家暴伤防护计算公式
  else if (attr == EATTRTYPE_CRIDEFPER)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_CRIDEFPER);
    user->setAttr(attr, modifyValue(f, 0, "less"));
  }
 // ----------玩家移动速度计算公式
  else if (attr == EATTRTYPE_MOVESPD)
  {
    float extra = 1;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = (extra + user->getOtherAttr(EATTRTYPE_MOVESPD)) * (1 + user->getOtherAttr(EATTRTYPE_MOVESPDPER));
    user->setAttr(attr, modifyValue(f, 0, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMOVESPD);
  }
 // ----------玩家移动速度百分比计算公式
  else if (attr == EATTRTYPE_MOVESPDPER)
  {
    calcAttr(user, lv, pro, map, EATTRTYPE_MOVESPD);
  }
 // ----------玩家吟唱速度计算公式
  else if (attr == EATTRTYPE_CASTSPD)
  {
    float extra = user->getAttr(EATTRTYPE_DEX) / 30;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_CASTSPD);
    user->setAttr(attr, modifyValue(f, 0, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWCASTSPD);
  }
 // ----------玩家物理攻击百分比计算公式
  else if (attr == EATTRTYPE_ATKPER)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_ATKPER);
    user->setAttr(attr, modifyValue(f, -1, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWATK);
  }
 // ----------玩家物理防御百分比计算公式
  else if (attr == EATTRTYPE_DEFPER)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_DEFPER);
    user->setAttr(attr, f);

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWDEF);
  }
 // ----------玩家魔法攻击百分比计算公式
  else if (attr == EATTRTYPE_MATKPER)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_MATKPER);
    user->setAttr(attr, modifyValue(f, -1, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMATK);
  }
 // ----------玩家魔法防御百分比计算公式
  else if (attr == EATTRTYPE_MDEFPER)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_MDEFPER);
    user->setAttr(attr, modifyValue(f, -1, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWMDEF);
  }
 // ----------玩家生命上限百分比计算公式
  else if (attr == EATTRTYPE_MAXHPPER)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_MAXHPPER);
    user->setAttr(attr, modifyValue(f, -1, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_MAXHP);
  }
 // ----------玩家魔法上限百分比计算公式
  else if (attr == EATTRTYPE_MAXSPPER)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_MAXSPPER);
    user->setAttr(attr, modifyValue(f, -1, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_MAXSP);
  }
 // ----------玩家物理精炼计算公式
  else if (attr == EATTRTYPE_REFINE)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_REFINE);
    user->setAttr(attr, f);
  }
 // ----------玩家魔法精炼计算公式
  else if (attr == EATTRTYPE_MREFINE)
  {
    float extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float f = extra + user->getOtherAttr(EATTRTYPE_MREFINE);
    user->setAttr(attr, f);
  }
 // ----------玩家素质生命恢复计算公式
  else if (attr == EATTRTYPE_RESTORESPD)
  {
    extra = floor(user->getAttr(EATTRTYPE_VIT) / 5);
    user->setPointAttr(attr, extra);

    f = (extra + user->getOtherAttr(EATTRTYPE_RESTORESPD) + floor(user->getAttr(EATTRTYPE_MAXHP) / 200)) * (1 + user->getOtherAttr(EATTRTYPE_RESTORESPDPER));
    user->setAttr(attr, f);

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWRESTORESPD);
  }
  else if (attr == EATTRTYPE_RESTORESPDPER)
  {
    calcAttr(user, lv, pro, map, EATTRTYPE_RESTORESPD);
  }
 // ----------玩家素质魔法恢复计算公式
  else if (attr == EATTRTYPE_SPRESTORESPD)
  {
    extra = 1 + floor(user->getAttr(EATTRTYPE_INT) / 6);
    user->setPointAttr(attr, extra);

    f = (extra + user->getOtherAttr(EATTRTYPE_SPRESTORESPD) + floor(user->getAttr(EATTRTYPE_MAXSP) / 100)) * (1 + user->getOtherAttr(EATTRTYPE_SPRESTORESPDPER));
    user->setAttr(attr, f);
  }
  else if (attr == EATTRTYPE_SPRESTORESPDPER)
  {
    calcAttr(user, lv, pro, map, EATTRTYPE_SPRESTORESPD);
  }
  else if (attr == EATTRTYPE_EQUIPASPD)
  {
    extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = extra + user->getOtherAttr(EATTRTYPE_EQUIPASPD);
    user->setAttr(attr, f);

    calcAttr(user, lv, pro, map, EATTRTYPE_ATKSPD);
  }
  else if (attr == EATTRTYPE_SKILLASPD)
  {
    extra = 0;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    f = extra + user->getOtherAttr(EATTRTYPE_SKILLASPD);
    user->setAttr(attr, f);

    calcAttr(user, lv, pro, map, EATTRTYPE_ATKSPD);
  }
  // ----------玩家素质攻击速度计算公式
  // ----------最终ASPD=ASPD值+装备ASPD值
  // ----------ASPD值=取整((200-(200-(职业基础ASPD+盾牌ASPD-ASPD校验值+开方((敏捷总和*9.999)+(灵巧总和*0.19212))*ASPD惩罚值))*(1-ASPD药水部分-技能ASPD修正)),3)
  // ----------装备ASPD值=取整((195-ASPD值)*装备ASPD修正),2)
  // ----------ASPD校验值=IF(敏捷总和<205,向上取整((开方(205)-开方(敏捷总和))/7.15,3),0)
  // ----------ASPD惩罚值=IF(职业基础ASPD>145,1-(职业基础ASPD-144)/50,0.96)
  // ----------每秒攻击次数=50/(200-取整(最终ASPD,0))
  else if (attr == EATTRTYPE_ATKSPD)
  {
    float agi = user->getAttr(EATTRTYPE_AGI);
    float ASPD_CHECKVALUE = 0;
    if (agi < 205)
      ASPD_CHECKVALUE = floor((sqrt(205) - sqrt(agi)) * 1000 / 7.15) / 1000;
    else
      ASPD_CHECKVALUE = 0;

    float BaseJobASPD = BaseJobAtkSpdNew[pro] + WeaponAtkSpdNew(user->getWeaponType(), pro);
    float ASPD_PANISHVALUE = 0;
    if (BaseJobASPD > 145)
      ASPD_PANISHVALUE = 1 - (BaseJobASPD - 144) / 50;
    else
      ASPD_PANISHVALUE = 0.96;

    float extra = floor((200 - (200 - (BaseJobASPD - ASPD_CHECKVALUE + sqrt(agi * 9.999) * ASPD_PANISHVALUE)) * (1 - user->getAttr(EATTRTYPE_SKILLASPD))) * 1000) / 1000;
    user->setPointAttr(attr, modifyValue(extra, 0, ""));

    float EquipASPD = floor((195 - extra) * user->getAttr(EATTRTYPE_EQUIPASPD) * 100) / 100;
    float BaseASPD = extra + EquipASPD;

    if (BaseASPD >= 2275 / 12)
      BaseASPD  = 2275 / 12;
    if (BaseASPD <= 50)
      BaseASPD = 50;
    // ----------玩家最终攻击速度计算公式
    f = 50 / (200 - BaseASPD);
    user->setAttr(attr, modifyValue(f, 0, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_SHOWATKSPD);
  }
  else if (attr == EATTRTYPE_CRIPER)
  {
    f = user->getOtherAttr(EATTRTYPE_CRIPER);
    user->setAttr(attr, modifyValue(f, 0, "less"));

    calcAttr(user, lv, pro, map, EATTRTYPE_CRI);
  }
  else if (attr == EATTRTYPE_DAMREDUC)
  {
    f = user->getOtherAttr(EATTRTYPE_DAMREDUC);
    user->setAttr(attr, modifyValue(f, 1, "great"));
  }
  else if (attr == EATTRTYPE_MDAMREDUC)
  {
    f = user->getOtherAttr(EATTRTYPE_MDAMREDUC);
    user->setAttr(attr, modifyValue(f, 1, "great"));
  }
// ----------玩家面板属性计算公式
  else if (attr == EATTRTYPE_SHOWATK)
  {
    f = user->getAttr(EATTRTYPE_ATK) * (1 + user->getAttr(EATTRTYPE_ATKPER));
    user->setAttr(attr, modifyValue(f, 0, ""));
  }
  else if (attr == EATTRTYPE_SHOWMATK)
  {
    f = user->getAttr(EATTRTYPE_MATK) * (1 + user->getAttr(EATTRTYPE_MATKPER));
    user->setAttr(attr, modifyValue(f, 0, ""));
  }
  else if (attr == EATTRTYPE_SHOWMAXHP)
  {
    f = user->getAttr(EATTRTYPE_MAXHP);
    user->setAttr(attr, modifyValue(f, 0, ""));
  }
  else if (attr == EATTRTYPE_SHOWMAXSP)
  {
    f = user->getAttr(EATTRTYPE_MAXSP);
    user->setAttr(attr, modifyValue(f, 0, ""));
  }
  else if (attr == EATTRTYPE_SHOWDEF)
  {
    f = user->getPointAttr(EATTRTYPE_DEF) + user->getOtherAttr(EATTRTYPE_DEF) * (1 + user->getAttr(EATTRTYPE_DEFPER));
    user->setAttr(attr, modifyValue(f, 0, ""));
  }
  else if (attr == EATTRTYPE_SHOWMDEF)
  {
    f = user->getPointAttr(EATTRTYPE_MDEF) + user->getPointAttr(EATTRTYPE_MDEF) * (1 + user->getAttr(EATTRTYPE_MDEF));
    user->setAttr(attr, modifyValue(f, 0, ""));
  }
  else if (attr == EATTRTYPE_SHOWHIT)
  {
    f = user->getAttr(EATTRTYPE_HIT);
    user->setAttr(attr, modifyValue(f, 0, ""));
  }
  else if (attr == EATTRTYPE_SHOWFLEE)
  {
    f = modifyValue(user->getAttr(EATTRTYPE_FLEE), 0, "");
    user->setAttr(attr, f);
  }
  else if (attr == EATTRTYPE_SHOWCRI)
  {
    f = modifyValue(user->getAttr(EATTRTYPE_CRI), 0, "");
    user->setAttr(attr, f);
  }
  else if (attr == EATTRTYPE_SHOWCRIRES)
  {
    f = modifyValue(user->getAttr(EATTRTYPE_CRIRES), 0, "less");
    user->setAttr(attr, f);
  }
  else if (attr == EATTRTYPE_SHOWATKSPD)
  {
    f = modifyValue(user->getAttr(EATTRTYPE_ATKSPD), 0, "");
    user->setAttr(attr, f);
  }
  else if (attr == EATTRTYPE_SHOWMOVESPD)
  {
    f = modifyValue(user->getAttr(EATTRTYPE_MOVESPD), 0, "");
    user->setAttr(attr, f);
  }
  else if (attr == EATTRTYPE_SHOWCASTSPD)
  {
    f = modifyValue(user->getAttr(EATTRTYPE_CASTSPD), 0, "");
    user->setAttr(attr, f);
  }
  else if (attr == EATTRTYPE_SHOWRESTORESPD)
  {
    f = modifyValue(user->getAttr(EATTRTYPE_RESTORESPD), 0, "");
    user->setAttr(attr, f);
  }

  XDBG << "[属性-计算]" << user->accid << user->id << user->getProfession() << user->name << "计算" << attr << "extra =" << extra << "final =" << f << XEND;
}

