/**
 * @file Achieve.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-04-22
 */

#pragma once

#include <list>
#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "AchieveConfig.h"
#include "PortraitConfig.h"
#include "MiscConfig.h"

using namespace Cmd;
using std::map;
using std::list;

const DWORD MAX_PARAM = 20;
const DWORD ACHIEVE_VERSION = 14;
const DWORD ACHIEVE_CHAR_VERSION = 2;

class SceneUser;
struct SAchieveItem
{
  DWORD dwID = 0;
  DWORD dwProcess = 0;
  DWORD dwFinishTime = 0;

  bool bRewardGet = 0;

  const SAchieveCFG* pCFG = nullptr;

  QWORD arrParams[MAX_PARAM] = {0};

  bool fromData(const AchieveItem& rData);
  bool toData(AchieveItem* pData, SceneUser* pUser);

  bool fromData(const AchieveDBItem& rItem);
  bool toData(AchieveDBItem* pItem);

  bool setParams(DWORD dwIndex, QWORD qwValue) { if (dwIndex >= 20) return false; arrParams[dwIndex] = qwValue; return true; }
  QWORD getParams(DWORD dwIndex) const { if (dwIndex >= 20) return 0; return arrParams[dwIndex]; }
};
typedef map<DWORD, SAchieveItem> TMapAchieveItem;
typedef list<MaxInfo> TListMaxInfo;

class Achieve
{
  friend class PatchManager;
  public:
    Achieve(SceneUser* pUser);
    ~Achieve();

    bool load(const BlobAchieve& rAccAchieve, const BlobAchieve& rAchieve);
    bool save(BlobAchieve* pAccAchieve, BlobAchieve* pAchieve);
    bool reload();

    void queryUserResume();
    void queryAchieveData(EAchieveType eType);
    void sendAchieveData();

    SAchieveItem* getAchieveItem(DWORD dwID);
    bool getReward(DWORD dwID);
    bool isFinishAchieve(DWORD dwID);
    DWORD getUnlockedAchieveNum() const;

    // user
    void onLevelup();
    void onAttrChange();

    // social
    void onAddFriend();
    void onDemandMusic();
    void onFerrisWheel();
    void onHandTime(DWORD dwTime);
    void onExpressAdd();
    void onExpressUse();
    void onEnterGuild(bool bCreate);
    void onPhoto(EAchieveCond eCond, const TSetQWORD& setIDs = TSetQWORD{});
    void onChat(EGameChatChannel eChannel);
    void onBody();
    void onRune();

    // adventure
    void onEnterMap();
    void onPortrait();
    void onHair(bool bChange);
    void onTitle(DWORD dwTitleID);
    void onCat(bool online);

    // battle
    void onUseSkill(DWORD dwSkillID);
    void onBattleTime(DWORD dwTime);
    void onPvp(bool bWin);
    void onDead(bool bFind, DWORD dwLostExp);
    void onTrans();
    void onDamage(DWORD dwNpcID, DWORD dwDamage);
    void onHelpQuest();

    // other
    void onEquip(EAchieveCond eCond);
    void onProduceEquip(DWORD dwItemID);
    void onRefine(bool bSuccess);
    void onItemAdd(const ItemInfo& rInfo);
    void onRepairSeal();
    void onPassTower();
    void onTransWithKPL();
    void onPassDojo(bool bHelp);
    void onCarrier(DWORD dwBusID);
    void onShopBuy(QWORD qwZeny);
    void onShopSell(QWORD qwZeny);
    void onTradeBuy(QWORD qwZeny);
    void onTradeSell(QWORD qwZeny);
    void onAddMoney(QWORD qwZeny);
    void onCharge();
    void onKplConsume(QWORD qwZeny);
    void onTradeRecord();
    void onPrayLvUp();
    void onCostPrayItem(DWORD dwNum);

    // common
    void onKillMonster(DWORD dwType, DWORD dwID);
    void onMvp(EMvpScoreType eType, DWORD dwID);
    void onItemUsed(DWORD dwItemID);
    void onItemBeUsed(DWORD dwItemID);
    void onManual(EManualType eType, EAchieveCond eCond);
    void onCollection();
    void onQuestSubmit(DWORD dwQuestID);
    void onAchieveFinish(const TVecQWORD& vecFinishAchIDs = TVecQWORD{});
    void onAchieveFinishLua() { processTime(EACHIEVECOND_ACHIEVE_FINISH); }
    void onMonsterDraw(DWORD curSec);
    void onSeat();
    void onTravel();

    void onManualOpen();

    // gm
    bool finish(DWORD dwID);

    // pet
    void onPetCapture(bool bSuccess);
    void onPetBaseLvUp();
    void onPetFeed();
    void onPetTouch();
    void onPetGift();
    void onPetHand(DWORD dwDistance);
    void onPetTime(DWORD dwTime);
    //void onPetEquip();
    //void onPetFriendLvUp();
    void onPetDead();
    void onPetAdventure();
    void onPetEquip();
    void onComposePet();
    void onPetSkillAllLv(DWORD level);
    void onPetFriendLvUp(DWORD level);

    // food
    void onCookFood(DWORD dwTimes, bool bSuccess);
    void onEatFood(DWORD dwItemId, DWORD dwItemNum, bool notMine);
    void onFoodMaterialLvUp(DWORD dwItemId, DWORD dwNewLv);
    void onCookerLvUp(DWORD dwNewLv);
    void onTasterLvUp(DWORD dwNewLv);
    void onUseSaveHp(DWORD dwValue);
    void onUseSaveSp(DWORD dwValue);
    void onFoodCookLvUp(DWORD dwItemId, DWORD dwNewLv);

    // tutor
    void onTutorGuide();
    void onStudentGraduation();

    //polly
    void onGoldAppleGame(DWORD dwValue);
    void onJoinPolly();
    void onGoldAppleTotal(DWORD dwValue);

    // wedding
    void onWedding(EAchieveCond eCond, const TVecQWORD& vecParam = TVecQWORD{});

    // profession
    void onProfession();

    void timer(DWORD curSec);

    void patch_1(DWORD dwID);
    void char_version_0();
    void char_version_1();
  private:
    void initDefault();
    void refreshAchieve();

    bool collectMostUserName(EShareDataType eType, DWORD dwNum, QueryUserResumeAchCmd& cmd);

    void processOnce(EAchieveCond eCond, const TVecQWORD& vecParam = TVecQWORD{}, DWORD dwAdd = 1, bool bPatch = false, DWORD dwSource = 0);
    void processTime(EAchieveCond eCond, const TVecQWORD& vecParam = TVecQWORD{}, DWORD dwAdd = 1, bool bPatch = false, DWORD dwSource = 0);

    void version_update();
    void version_2();
    void version_3();
    void version_4();
    void version_5();
    void version_6();
    void version_7();
    void version_8();
    void version_9();
    void version_10();
    void version_11();
    void version_12();
    void version_13();
  private:
    SceneUser* m_pUser = nullptr;

    TMapAchieveItem m_mapItem;
    map<EShareDataType, TListMaxInfo> m_mapMaxCache;

    DWORD m_dwBattleTimeTmp = 0;
    DWORD m_dwTimeTick = 0;
    DWORD m_dwDataVersion = 0;
    DWORD m_dwCharVersion = 0;
};

