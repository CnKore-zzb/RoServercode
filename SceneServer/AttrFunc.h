/**
 * @file AttrFunc.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2018-07-16
 */

#pragma once

#include "xDefine.h"
#include "xSingleton.h"
#include "ProtoCommon.pb.h"

using namespace Cmd;

class SceneUser;
class AttrFunc : public xSingleton<AttrFunc>
{
  friend class xSingleton<AttrFunc>;
  private:
    AttrFunc();
  public:
    virtual ~AttrFunc();

    DWORD CalcSumNew(DWORD num);
    float modifyValue(float value, float adjust, const std::string& action);
    DWORD WeaponAtkSpdNew(DWORD type, DWORD profession);
    bool checkRemoteAtk(DWORD profession, DWORD WeaponType);

    void calcUserAttr(SceneUser* user, DWORD index);
    void calcAttr(SceneUser* user, DWORD lv, DWORD pro, DWORD map, EAttrType attr);
  private:
    // --------------------玩家属性计算公式
    // --------------------物理职业攻击成长
    std::map<DWORD, float> BaseLvAtkRate1New;
    // --------------------魔法职业攻击成长
    std::map<DWORD, float> BaseLvAtkRate2New;
    // --------------------职业物防成长
    std::map<DWORD, float> BaseLvDefRateNew;
    // --------------------职业魔防成长
    std::map<DWORD, float> BaseLvMDefRateNew;
    std::map<DWORD, float> BaseLvRateNew;
    std::map<DWORD, float> HpRateNew;
    std::map<DWORD, float> BaseHpNew;
    // ------------------------------职业空手攻速
    std::map<DWORD, float> BaseJobAtkSpdNew;
    // -- 长矛各职业对应攻速
    std::map<DWORD, float> SpearAtkSpdNew;
    // -- 长剑各职业对应攻速
    std::map<DWORD, float> SwordAtkSpdNew;
    // -- 锤子各职业对应攻速
    std::map<DWORD, float> MaceAtkSpdNew;
    // -- 拳刃各职业对应攻速
    std::map<DWORD, float> KatarAtkSpdNew;
    // -- 弓各职业对应攻速
    std::map<DWORD, float> BowAtkSpdNew;
    // -- 法杖各职业对应攻速
    std::map<DWORD, float> StaffAtkSpdNew;
    // -- 匕首各职业对应攻速
    std::map<DWORD, float> KnifeAtkSpdNew;
    // -- 斧头各职业对应攻速
    std::map<DWORD, float> AxeAtkSpdNew;
    // -- 拳套各职业对应攻速
    std::map<DWORD, float> FistAtkSpdNew;

    // -- 副手各职业对应攻速(暂时修改掉副手的ASPD惩罚)
    std::map<DWORD, float> ShieldAtkSpdNew;
    std::map<DWORD, std::pair<DWORD, DWORD>> Recover;
};

