#pragma once

#include <map>
#include <string>
#include "xDefine.h"
#include "xSingleton.h"
#include "UserCmd.h"
#include "xLuaTable.h"

using namespace Cmd;
using std::string;

class xSceneEntryDynamic;
class SceneUser;
class Scene;
typedef bool (*GmFun)(xSceneEntryDynamic *, const xLuaData &);
struct GmCommand
{
  char cmd[MAX_NAMESIZE];
  GmFun p;
  BYTE pri;
  char desc[256];
};

typedef bool (*GmSceneFun)(Scene *, const xLuaData &);
struct GmSceneCmd
{
  char cmd[MAX_NAMESIZE];
  GmSceneFun p;
  BYTE pri;
  char desc[256];
};

class GMCommandRuler : public xSingleton<GMCommandRuler>
{
  public:
    GMCommandRuler();
    ~GMCommandRuler();
    bool execute(xSceneEntryDynamic* entry, std::string command);
    bool execute(xSceneEntryDynamic* entry, const xLuaData &data);
    bool execute(std::string command);
    bool checkGMRight(SceneUser* user, const char* command, GmFun &type);
    bool luaGMCmd(xSceneEntryDynamic* entry, const char* command);
  private:
    void getParam(const std::vector<std::string> &sVec, xLuaData &params);

  public:
    bool scene_execute(Scene* pScene, const xLuaData &data);
    static bool scene_summon(Scene* pScene, const xLuaData& params);
    static bool scene_clearnpc(Scene* pScene, const xLuaData& params);
    static bool dscene_summon(Scene* pScene, const xLuaData& params);
    static bool dscene_clearnpc(Scene* pScene, const xLuaData& params);

    static bool scene_clearvisiblenpc(Scene* pScene, const xLuaData& params);
    static bool scene_hidenpc(Scene* pScene, const xLuaData& params);
    static bool scene_shownpc(Scene* pScene, const xLuaData& params);
    static bool scene_weather(Scene* pScene, const xLuaData& params);
    static bool scene_sky(Scene* pScene, const xLuaData& params);
    static bool setenv(Scene* pScene, const xLuaData& params);
    static bool setbgm(Scene* pScene, const xLuaData& params);
    static bool scene_se(Scene* pScene, const xLuaData& params);    //场景播放音效

    static bool scene_replace_reward(Scene* pScene, const xLuaData& params);
    static bool scene_recover_reward(Scene* pScene, const xLuaData& params);
    static bool scene_add_extra_reward(Scene* pScene, const xLuaData& params);
    static bool scene_del_extra_reward(Scene* pScene, const xLuaData& params);

    static bool scene_add_event_reward(Scene* pScene, const xLuaData& params);
    static bool scene_del_event_reward(Scene* pScene, const xLuaData& params);

    static bool scene_add_double_event_reward(Scene* pScene, const xLuaData& params);
    static bool scene_del_double_event_reward(Scene* pScene, const xLuaData& params);

    static bool scene_replace_board_reward(Scene* pScene, const xLuaData& params);
    static bool scene_recover_board_reward(Scene* pScene, const xLuaData& params);

    static bool startactivity(Scene* pScene, const xLuaData& params);
    static bool stopactivity(Scene* pScene, const xLuaData& params);  

    static bool startglobalactivity(Scene* pScene, const xLuaData& params);
    static bool stopglobalactivity(Scene* pScene, const xLuaData& params);  

    static bool scene_open_npcfunc(Scene* pScene, const xLuaData& params);
    static bool scene_close_npcfunc(Scene* pScene, const xLuaData& params);

    static bool resetGuildRaid(Scene* pScene, const xLuaData& params);
    static bool recoverGuildRaid(Scene* pScene, const xLuaData& params);

    static bool scene_changebody(Scene* pScene, const xLuaData& params);
    static bool scene_showseat(Scene* pScene, const xLuaData& params);
    static bool scene_gvg(Scene* pScene, const xLuaData& params);
    static bool scene_rangebgm(Scene* pScene, const xLuaData& params);
    static bool scene_rangesky(Scene* pScene, const xLuaData& params);
    static bool scene_rangeweather(Scene* pScene, const xLuaData& params);
    static bool scene_effect(Scene* pScene, const xLuaData& params);
    static bool scene_dropitem(Scene* pScene, const xLuaData& params);

    static bool scene_gear(Scene* pScene, const xLuaData& params);
    static bool scene_kickalluser(Scene* pScene, const xLuaData& params);
    static bool scene_shakecreen(Scene* pScene, const xLuaData& params);
    static bool scene_addquest(Scene* pScene, const xLuaData& params);
    static bool scene_delquest(Scene* pScene, const xLuaData& params);

  public:
    static bool loadXml(xSceneEntryDynamic* user, const xLuaData &params);
    static bool cmdfilter(xSceneEntryDynamic* user, const xLuaData &params);
    static bool captain(xSceneEntryDynamic* user, const xLuaData &params);
    static bool loadlua(xSceneEntryDynamic* user, const xLuaData &params);
    static bool loadtable(xSceneEntryDynamic* user, const xLuaData &params);
    static bool patch(xSceneEntryDynamic* user, const xLuaData &params);

    // gm command
    static bool additem(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool subitem(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool addmoney(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool submoney(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool money(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool removecard(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool addattrpoint(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool addbaseexp(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool addjobexp(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool exchangejob(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool reward(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool setattr(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool dmap(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool gocity(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool follower(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool rideon(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool rideoff(xSceneEntryDynamic* pUser, const xLuaData& params);
    //static bool stage(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool buff(xSceneEntryDynamic* pUser, const xLuaData& params);
    //static bool changeskill(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool heal(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool god(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool killer(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool hideme(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool normal(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool levelup(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool joblevelup(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool basetolevel(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool jobtolevel(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool killnine(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool effect(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool quest(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool summon(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool setColor(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool setSysTime(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool menu(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool restart(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool carrier(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addmount(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool getHair(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool getEye(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool testBuff(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool gear(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool showboss(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool openui(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool exit_go(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool exit_visible(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool cleancorpse(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addpurify(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool delbuff(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool finishQuest(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool finishGroup(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addskill(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addpartner(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool removepartner(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool npcfunction(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool modelshow(xSceneEntryDynamic* entry, const xLuaData& params);
    //static bool followleader(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool sound_effect(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool clearpack(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool sound_bgm(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool gametime(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool resetgametime(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool playaction(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool checkgametime(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool checkweather(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool setsky(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool setweather(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool setenv(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool randPos(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool savemap(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool shakescreen(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool entertower(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool useskill(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool delstatus(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool gopvp(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool rewards(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool fakedead(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool addinterlocution(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool resetattrpoint(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool setattrpoint(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool resetskillpoint(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool setskillpoint(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool npcscale(xSceneEntryDynamic * entry, const xLuaData &params);

    static bool changecarrier(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool scenery(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool npctalk(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool openseal(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool activemap(xSceneEntryDynamic * entry, const xLuaData &params);
    /*test use hair style CM 测试用*/
    static bool usehair(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool npcdie(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool setSearchRange(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool passtower(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool resettower(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool barrage(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool shownpc(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool hidenpc(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool addmotion(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool handinhand(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool image(xSceneEntryDynamic * entry, const xLuaData &params);
    static bool misc(xSceneEntryDynamic * entry, const xLuaData &params);

    static bool setdaily(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool initdaily(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool refreshquest(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool dropItem(xSceneEntryDynamic* entry, const xLuaData &params);

    static bool manual(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool portrait(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool delchar(xSceneEntryDynamic* entry, const xLuaData &params);

    //trade
    static bool tradePrice(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool tradeBriefSearch(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool tradeDetailSearch(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool tradeMyPending(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool tradeMyLog(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool tradeSell(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool tradeBuy(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool tradeCancel(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool forwardRecord(const Cmd::UserCmd* cmd, unsigned short len);    

    static bool refreshtower(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool compensateuser(xSceneEntryDynamic* entry, const xLuaData &params);

    static bool speffect(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool changearrow(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool addasset(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool movetrack(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool setzone(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool itemimage(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool itemmusic(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool addhandnpc(xSceneEntryDynamic* entry, const xLuaData &params);

    static bool toy_smile(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool activity(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool use_gift_code(xSceneEntryDynamic* entry, const xLuaData &params);

    static bool jumpzone(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool activity_reward(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool sendmsg(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool dropReward(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool playdialog(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool unlockmanual(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool usedepositcard(xSceneEntryDynamic* entry, const xLuaData &params);

    static bool pickup(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool npcmove(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool testtool(xSceneEntryDynamic* entry, const xLuaData& params);

    static bool equip(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool resetShopSkill(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool randzeny(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool loveletter(xSceneEntryDynamic* entry, const xLuaData& params);
    //充值战斗时长
    static bool clearBattletime(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool setCredit(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool playcharge(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool chat(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool addweaponpet(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool refinetest(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool lotterytest(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool unlockweaponpet(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool seenpc(xSceneEntryDynamic* entry, const xLuaData &params);

    static bool guildlevelup(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool questitem(xSceneEntryDynamic* entry, const xLuaData &params);
    static bool reducehp(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool decsealcount(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool changequest(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addquota(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool subquota(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool clearquota(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool joinpvp(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool kickpvpuser(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool resetpvp(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool switchpvp(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool setcardslot(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool firework(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool deletePassword(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool questnpc(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool optguildraid(xSceneEntryDynamic* entry, const xLuaData& params);

    static bool oneclickrefine(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool astrolabe(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool showmini(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool resetaugury(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool setvar(xSceneEntryDynamic* entry, const xLuaData& params);

    static bool charge(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool photo(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool delspeffect(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool achieve(xSceneEntryDynamic* entry, const xLuaData& params);

    static bool catchpet(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool precatchpet(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool pet(xSceneEntryDynamic* entry, const xLuaData& params);

    static bool cookerlvup(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool tasterlvup(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addcookerexp(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addtasterexp(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addfooddataexp(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool sceneeffect(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool clearsatiety(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool advancepro(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool unlockrecipe(xSceneEntryDynamic* pUser, const xLuaData& params);
    static bool tutor(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool tutorskill(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool finishboardquest(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool settowermaxlayer(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool being(xSceneEntryDynamic* entry, const xLuaData& params);

    static bool lottery(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addjoyvalue(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addbattletime(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool beingbody(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addguildpray(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool breakequip(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool guild(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool callteamer(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool guildchallenge(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool genderreward(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool addmanualattributes(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool manualleveldown(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool deltitle(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool codeused(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool codeitem(xSceneEntryDynamic* entry, const xLuaData& params);  //纯粹测试用
    static bool addmaxjoblv(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool lotterygive(xSceneEntryDynamic* entry, const xLuaData& params);  

    static bool setlotterycnt(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool clearlotterylimit(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool marry(xSceneEntryDynamic* entry, const xLuaData& params);  //纯粹测试用
    static bool divorce(xSceneEntryDynamic* entry, const xLuaData& params);  //纯粹测试用
    static bool marriageproposal(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool divorcemail(xSceneEntryDynamic* entry, const xLuaData& params); //强制离婚邮件通知对方，任务用
    static bool forcedivorce(xSceneEntryDynamic* entry, const xLuaData& params); //强制离婚，任务用
    static bool rmcodeitem(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool tower(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool laboratory(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool carddecompose(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool tutorgrowreward(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool pvecardeffect(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool user(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool servant(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool branchremove(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool branchrestore(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool branchcmd(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool resetmainchar(xSceneEntryDynamic* entry, const xLuaData& params);

    static bool slotcmd(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool modifycharge(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool inviteteammates(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool unlockpetwear(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool lockquota(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool unlockquota(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool rewardsafety(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool setshopgotcount(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool activatetransfer(xSceneEntryDynamic* entry, const xLuaData& params);
    static bool boss(xSceneEntryDynamic* entry, const xLuaData& params);

    static bool addresist(xSceneEntryDynamic* entry, const xLuaData& params);
};

