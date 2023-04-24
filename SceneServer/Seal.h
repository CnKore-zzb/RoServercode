/**
 * @file Seal.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2015-11-17
 */

#pragma once

#include "xDefine.h"
#include "xPos.h"
#include "SealRepair.h"

namespace Cmd
{
  class BlobSeal;
};
using namespace Cmd;
using std::string;
using std::vector;
using std::set;
// seal
class SceneUser;
class SceneNpc;
class Scene;
class Seal
{
  public:
    Seal(SceneUser* pUser);
    ~Seal();

    bool load(const BlobSeal& oBlob);
    bool save(BlobSeal* pBlob);

    void sendAllSealInfo();
    void sendAllSealToTeam();

    const SSealItem* getSealItem(DWORD dwMapID, QWORD qwID);
    bool removeSealItemToTeam(DWORD dwMapID, QWORD qwID);

    void onQuestSubmit(DWORD dwQuestID);
    void timer(DWORD curTime);

    bool beginSeal(QWORD qwID, EFinishType etype = EFINISHTYPE_NORMAL);

    const TVecSealData& getSealData() { return m_vecData; }
    void setDataMark();

    void addUpdateNewSeal(DWORD mapid, const SSealItem& sItem);
    void addUpdateDelSeal(DWORD mapid, const SSealItem& sItem);

    void addTeamNewSeal(DWORD mapid, const SSealItem& sItem);
    void addTeamDelSeal(DWORD mapid, const SSealItem& sItem);

    void addMySealToUser(SceneUser* pUser);
    void delMySealToUser(SceneUser* pUser);

    void onUserOffline();
    void onLeaveScene();
    void onEnterScene();

    const TSetDWORD& getOpenSeals() { return m_setOpenSealIDs; }
  public:
    void refreshSealItem(DWORD curTime);
    void checkAddSeal(DWORD curTime);
    void update();
  public:
    void testOpenSeal();
    void openSeal(DWORD dwID);
    void closeSeal(DWORD dwID);
  private:
    SceneNpc* createSeal(Scene* pScene, const SSealItem& sItem, bool needNewPos);
    SceneNpc* createSelfSeal(const SSealItem& sItem);
    void getTeamOtherUser(set<SceneUser*>& userSet);
    bool haveEnoughSeal() const;
    bool getRandomPos(Scene* pScene, const SSealItem& sItem, xPos& outPos);

  private:
    SceneUser* m_pUser = nullptr;

    // self data
    TVecSealData m_vecData;

    // handle teamers and self data change
    TVecSealData m_vecNewData;
    TVecSealData m_vecDelData;

    DWORD m_dwNextProduceTime = 0;
    TSetDWORD m_setOpenSealIDs;

  // ------- 个人封印修复 ---------
  public:
    void addSeal(DWORD dwID); // for quest
    bool beginSelfSeal(QWORD id);
    void onBeBreakSkill();
    void onMonsterDie(SceneNpc* npc);
  private:
    void process(DWORD curTime);
    void finishSeal(QWORD sealid);
    void failSeal(QWORD sealid);
    bool rebeginSeal(QWORD id);
    void selfSealLeaveScene();
    void selfSealEnterScene();
  private:
    void delSceneImage();
  private:
    // quest data
    TVecSealData m_vecQuestData;
    TVecRepairSeal m_vecSealingData;
    TVecRepairSeal m_vecFinishSealData;
    DWORD m_dwExitDMapTime = 0;
    DWORD m_dwDMapID = 0;
    DWORD m_dwCountDownTime = 0;
};
