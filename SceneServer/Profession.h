#pragma once

#include "xDefine.h"
#include "RecordCmd.pb.h"

#define BRANCH_BACKUP 9999 // 主分支备份
#define ONE_GRADE_PRO_MAX_JOBLV 50 // 一转最大job等级
#define TWO_GRADE_PRO_MAX_JOBLV 90 // 二转最大job等级
#define THREE_GRADE_PRO_MAX_JOBLV 130 // 三转最大job等级
#define PEAK_PRO_MAX_JOBLV 160 // 巅峰最大job等级

typedef std::map<DWORD, Cmd::ProfessionData> TMapBranchProfession;
typedef std::map<DWORD, Cmd::ProfessionSvrData> TMapBranchSvrProfession;

namespace Cmd
{
  class ProfessionUserInfo;
};

class SceneUser;
class Profession
{
  public:
    Profession(SceneUser* pUser);
    ~Profession();

  public:
    bool load(const Cmd::BlobProfession& oBlob);
    bool save(Cmd::BlobProfession* pBlob);

    bool checkLoadTime();
    bool sendBranchData(DWORD branch = 0);

  public:
    bool addBranch(DWORD dwBranch);
    bool addBranch(DWORD dwProfession, DWORD dwJobLv);
    bool hasBranch(DWORD dwBranch);
    bool initRoleData(DWORD dwBranch, DWORD dwProfession, DWORD dwJobLv);

    void setJobLv(DWORD dwBranch, DWORD dwJoblv);
    DWORD getJobLv(DWORD dwBranch);
    DWORD getMaxJobLv();

    DWORD getTotalSkillPoint(DWORD dwBranch);
    DWORD getBrotherBranchMax(const TVecDWORD& vecBrother);

    void queryBranchs(Cmd::ProfessionQueryUserCmd& cmd);
    DWORD getProfessionMax();
    DWORD getBaseProfession(); // 获取初始角色基础职业
    DWORD getProfessionCount(); // 获取购买职业数
    DWORD getBuyBaseProfessionCount(); // 获取购买同系职业数

    Cmd::ProfessionData* getProfessionData(DWORD dwBranch);
  private:
    SceneUser* m_pUser = nullptr;
    TMapBranchProfession m_mapBranch; // 已购买分支职业列表
    TMapBranchSvrProfession m_mapSvrBranch;

    DWORD m_dwLastLoadTime; // 上次切换时间

  public:
    bool changeBranch(DWORD dwBranch); // 切换职业分支

    bool loadProfessionData(Cmd::ProfessionData& data, bool isFirst = false);
    bool saveProfessionData(Cmd::ProfessionData& data);

    // 切换存档时用到的接口 by lzq
    // 保存当前职业分支
    bool saveCurProfessionData();
    // 加载存档记录
    bool loadRecordProfessionData(Cmd::ProfessionData& record_data);
    // 转换ProfessionData->ProfessionUserInfo  发送给客户端的结构
    bool collectProfessionUserInfo(const Cmd::ProfessionData& src_data, Cmd::ProfessionUserInfo* dest_data);

  public:
    void syncBranchJoblv(DWORD dwBranch, DWORD dwJoblv);
    void onEquipExchange(const std::string& guidOld, const std::string& guidNew);

  public:
    // 获取已有职业列表(二转以上)
    void getProfessions(TVecDWORD& vecProfessions);
    //void collectAttr(TVecAttrSvrs& attrs);
    void collectAttr();

  public: // 意外处理
    bool remove(DWORD branch); // 移除分支
    void backup(Cmd::ProfessionData& data); // 备份
    void restore(); // 加载备份数据，以防万一
    void fixBranch(Cmd::ProfessionQueryRecordCmd& cmd);

  private:
    bool resetAstrolabesData(std::vector<std::pair<DWORD, DWORD>>& vecStars);

    bool loadAstrolabesData(const Cmd::BlobAstrolabe& data, const std::vector<std::pair<DWORD, DWORD>>& vecStars, bool& isReset, bool isFirst);
    bool saveAstrolabesData(Cmd::BlobAstrolabe* pData);

    bool loadRoleData(const Cmd::UserRoleData& data, bool isFirst);
    bool saveRoleData(Cmd::UserRoleData* pData);

    bool loadPackageData(const Cmd::ProfessionData& data, bool isFirst);
    bool savePackageData(Cmd::ProfessionData& data);

    bool loadBeingData(const Cmd::BlobUserBeing& data);
    bool saveBeingData(Cmd::BlobUserBeing* pData);

    bool loadPartnerData(const Cmd::BlobPet& data);
    bool savePartnerData(Cmd::BlobPet* pData);

    bool loadExchangeShopData(const Cmd::ProfessionData& data);
    bool saveExchangeShopData(Cmd::ProfessionData& data);

    void exchangeSkillcut(Cmd::ProfessionData& data);
  public:
    bool setExchangeTime(DWORD branch, DWORD evo, DWORD dwTime);
    bool getExchangeTime(DWORD branch, DWORD evo, DWORD& rTime);
};
