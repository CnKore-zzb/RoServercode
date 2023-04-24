#pragma once
#include "xDefine.h"
#include "xDefine.h"
#include "xPos.h"
#include "xLuaTable.h"
//#include "SceneRecord.pb.h"
#include "ProtoCommon.pb.h"
#include "ItemConfig.h"
#include "NpcConfig.h"
#include <tuple>

using std::string;
using std::tuple;
using std::make_tuple;
using std::get;
using namespace Cmd;

enum SCENE_ENTRY_TYPE
{
  SCENE_ENTRY_USER,
  SCENE_ENTRY_NPC,
  SCENE_ENTRY_ITEM,
  SCENE_ENTRY_TRAP,
  SCENE_ENTRY_ACT,
  SCENE_ENTRY_BOOTH,
  SCENE_ENTRY_MAX
};

enum SCENE_TYPE
{
  SCENE_TYPE_STATIC,
  SCENE_TYPE_QUEST,
  SCENE_TYPE_GUILD,
  SCENE_TYPE_GUILD_RAID,
  SCENE_TYPE_GUILD_FIRE,
  SCENE_TYPE_WEDDING_SCENE,
  SCENE_TYPE_PVECARD,
  SCENE_TYPE_MVPBATTLE,
  SCENE_TYPE_SUPERGVG,
  SCENE_TYPE_TEAMPWS,
  SCENE_TYPE_MAX
};

class xSceneEntryDynamic;
typedef std::set<xSceneEntryDynamic*> DEntryPtrSet;
#define MAX_SEND_USER_NUM 10
typedef DWORD Dir;

/*enum NotifyOption
{
  NOTIFY_OPT_YES,
  NOTIFY_OPT_NO,
};*/

typedef std::vector<xPosI> xPosIVector;
#define MAX_SEND_NPC_NUM 50

inline bool isMonster(DWORD &id)
{
  //return id > 10000;
  return NpcConfig::getMe().isMonster(id);
}

struct NpcVar
{
  QWORD m_qwFollowerID = 0;
  QWORD m_qwQuestOwnerID = 0;
  QWORD m_qwNpcOwnerID = 0;
  QWORD m_qwTeamUserID = 0; /* 玩家id, 该玩家极其队友可见, 仅影响是否可见, 不用于其他逻辑判断 */
  QWORD m_qwOwnerID = 0; /*宠物等主人id*/

  QWORD m_qwTeamID = 0;
  QWORD m_qwItemUserID = 0;

  DWORD dwLayer = 0;
  //DWORD dwSpecial = 0;

  DWORD dwLabPoint = 0;
  DWORD dwLabRound = 0;

  DWORD dwSealType = 0;

  DWORD dwDojoLevel = 0;

  DWORD dwTreeType = 0;

  DWORD m_dwBuffID = 0;

  DWORD m_dwDefaultHp = 0;

  QWORD m_qwGuildID = 0;
};

enum
{
  BEHAVIOUR_NONE = 0,
  BEHAVIOUR_MOVE_ABLE = 1 << 0,
  BEHAVIOUR_ATTACK_BACK = 1 << 1,
  BEHAVIOUR_OUT_RANGE_BACK = 1 << 2,
  BEHAVIOUR_PICKUP = 1 << 3,
  BEHAVIOUR_TEAM_ATTACK = 1 << 4,
  BEHAVIOUR_SWITCH_ATTACK = 1 << 5,
  BEHAVIOUR_SWITCH_BE_ATTACK = 1 << 6,
  BEHAVIOUR_BE_ATTACK_1_MAX = 1 << 7,
  BEHAVIOUR_BE_CHANT_ATTACK = 1 << 8,
  BEHAVIOUR_GEAR = 1 << 9,
  BEHAVIOUR_NOT_SKILL_SELECT = 1 << 10,
  BEHAVIOUR_GHOST = 1 << 11,
  BEHAVIOUR_DEMON = 1 << 12,
  BEHAVIOUR_FLY = 1 << 13,
  BEHAVIOUR_STEAL_CAMERA = 1 << 14,
  BEHAVIOUR_NAUGHTY = 1 << 15,
  BEHAVIOUR_ALERT = 1 << 16,
  BEHAVIOUR_EXPEL = 1 << 17,
  BEHAVIOUR_RECKLESS = 1 << 18,
  BEHAVIOUR_GHOST_2 = 1 << 19,
  BEHAVIOUR_JEALOUS = 1 << 20,
  BEHAVIOUR_SLEEP = 1 << 21,
  BEHAVIOUR_STATUSATTACK = 1 << 22,
  BEHAVIOUR_DHIDE = 1 << 23,
  BEHAVIOUR_NIGHTWORK = 1 << 24,
  BEHAVIOUR_ENDURE = 1 << 25,
  BEHAVIOUR_NOTRANSFORM_ATT = 1 << 26, // 不会主动攻击变身玩家
  BEHAVIOUR_IMMUNETAUNT = 1 << 27,
  BEHAVIOUR_FARSEARCH = 1 << 28,
  BEHAVIOUR_MAX = 1 << 31,
};

struct NpcDefine
{
  public:
    NpcDefine()
    {
      //m_data.Clear();
      resetting();
    }
    void resetting()
    {
      //m_data.mutable_pos()->Clear();
      pos.x = pos.y = pos.z = 0.0f;
      setTerritory(5);
      setLife(1);
      setReborn(1);
      setSuperAI.clear();
      setExtraAI.clear();
      setSeatid.clear();
      //setSuperAI(0);
      setScaleMin(0.0f);
      setScaleMax(0.0f);
      DWORD id = getID();
      if (isMonster(id))
      {
        setSearch(7);
        setRange(0);
        setBehaviours(7);
      }
      else
      {
        setSearch(0);
        setRange(0);
        setBehaviours(0);
      }
    }
    bool isGear() const
    {
      return (0!=(getBehaviours() & BEHAVIOUR_GEAR));
    }
    void load(const xLuaData &data)
    {
      if (data.has("id"))
      {
        setID(data.getTableInt("id"));
      }
      if (data.has("uniqueid"))
        setUniqueID(data.getTableInt("uniqueid"));
      resetting();
      if (data.has("ignorenavmesh"))
        setIgnoreNavMesh(data.getTableInt("ignorenavmesh"));
      if (data.has("pos"))
      {
        const xLuaData &p = data.getData("pos");
        xPos pos;
        pos.x = p.getTableFloat("1");
        pos.y = p.getTableFloat("2");
        pos.z = p.getTableFloat("3");
        setPos(pos);
      }
      if (data.has("dir"))
      {
        setDir((Dir)data.getTableInt("dir"));
      }
      if (data.has("reborn"))
      {
        setReborn(data.getTableInt("reborn"));
      }

      if (data.has("scale"))
      {
        const xLuaData &p = data.getData("scale");
        setScaleMin(p.getTableFloat("1"));
        setScaleMax(p.getTableFloat("2"));
      }
      if (data.has("fix_scale"))
      {
        setScaleMin(data.getTableFloat("fix_scale"));
        setScaleMax(data.getTableFloat("fix_scale"));
      }
      if (data.has("life"))
      {
        setLife(data.getTableInt("life"));
      }
      if (data.has("behavior"))
      {
        setBehaviours(data.getTableInt("behavior"));
      }
      if (data.has("territory"))
        setTerritory(data.getTableInt("territory"));
      if (data.has("range"))
        setRange(data.getTableInt("range"));
      if (data.has("ai"))
        addSuperAI(data.getTableInt("ai"));
      //  setSuperAI(data.getTableInt("ai"));
      if (data.has("extraai"))
      {
        auto extraaif = [&](const string& key, xLuaData& data)
        {
          addExtraAI(data.getInt());
        };
        const_cast<xLuaData&>(data).getMutableData("extraai").foreach(extraaif);
      }
      if (data.has("search"))
        setSearch(data.getTableInt("search"));
      if (data.has("orgstate"))
        setGearOrgState(data.getTableInt("orgstate"));
      if (data.has("privategear"))
        setGearPrivate(data.getTableInt("privategear"));
      if (data.has("purifyValue"))
        setPurify(data.getTableInt("purifyValue"));
      if (data.has("disappeartime"))
        setDisptime(data.getTableInt("disappeartime"));
      if (data.has("protectlevel"))
        setAttSafeLv(data.getTableInt("protectlevel"));
      if (data.has("waitaction"))
        setWaitAction(data.getTableString("waitaction"));
      if (data.has("stype"))
        loadSummonType(data.getTableString("stype"));
      if (data.has("pursue"))
        setPursue(data.getTableInt("pursue"));
      if (data.has("pursuetime"))
        setPursueTime(data.getTableInt("pursuetime"));
      if (data.has("visibleinmap"))
        setVisibleInMap(data.getTableInt("visibleinmap")!=0);
      if (data.has("fadein"))
        setFadeIn(data.getTableInt("fadein"));
      if (data.has("dead_disp_time"))
        setDeadDispTime(data.getTableInt("dead_disp_time"));
      if (data.has("relive_at_dead_pos"))
        setReliveAtDeadPos(data.getTableInt("relive_at_dead_pos"));
      if(data.has("seatid"))
      {
        auto seatfunc = [&](const string& str, xLuaData& mdata)
        {
          addSeatID(mdata.getInt());
        };
        const_cast<xLuaData&>(data).getMutableData("seatid").foreach(seatfunc);
    }
    }
    void load(const Cmd::NpcDefineData &item)
    {
      fromData(item);
      //m_data.CopyFrom(item);
    }
    void save(Cmd::NpcDefineData *pData)
    {
      //pData->CopyFrom(m_data);
      toData(pData);
    }
  public:
    void setID(DWORD id)
    {
      //m_data.set_id(id);
      this->id = id;
    }
    DWORD getID() const
    {
      //return m_data.id();
      return id;
    }
    void setName(const char *name)
    {
      //m_data.set_name(name);
      this->name = name;
    }
    const string& getName() const
    {
      //return m_data.name();
      return name;
    }
    // 出生坐标
    void setPos(const xPos& p)
    {
      /*Cmd::Pos *pos = m_data.mutable_pos();
      pos->set_x(p.x);
      pos->set_y(p.y);
      pos->set_z(p.z);*/
      pos = p;
    }
    const xPos& getPos() const
    {
      return pos;
      //return xPos(m_data.pos().x(), m_data.pos().y(), m_data.pos().z());
    }
    void setRange(DWORD range)
    {
      //m_data.set_range(range);
      this->range = range;
    }
    DWORD getRange() const
    {
      //return m_data.range();
      return range;
    }
    void setDir(Dir dir)
    {
      //m_data.set_dir(dir);
      this->dir = dir;
    }
    Dir getDir() const
    {
      //return (Dir)m_data.dir();
      return (Dir)dir;
    }
    // 复活间隔
    void setReborn(DWORD reborn)
    {
      //m_data.set_reborn(reborn);
      this->reborn = reborn;
    }
    DWORD getReborn() const
    {
      //return m_data.reborn();
      return reborn;
    }
    // 活动范围
    void setTerritory(DWORD territory)
    {
      //m_data.set_territory(territory);
      this->territory = territory;
    }
    DWORD getTerritory() const
    {
      //return m_data.territory();
      return territory;
    }
    // 大小范围
    void setScaleMin(float scalemin)
    {
      //m_data.set_scalemin(scalemin);
      this->scalemin = scalemin;
    }
    float getScaleMin() const
    {
      //return m_data.scalemin();
      return scalemin;
    }
    void setScaleMax(float scalemax)
    {
      //m_data.set_scalemax(scalemax);
      this->scalemax = scalemax;
    }
    float getScaleMax() const
    {
      //return m_data.scalemax();
      return scalemax;
    }
    // 命。有n条命可以复活(n-1)次。0表示无限复活
    void setLife(DWORD life)
    {
      //m_data.set_life(life);
      this->life = life;
    }
    DWORD getLife() const
    {
      //return m_data.life();
      return life;
    }
    // 行为特性
    void setBehaviours(DWORD behaviours)
    {
      //m_data.set_behaviours(behaviours);
      this->behaviours = behaviours;
    }
    DWORD getBehaviours() const
    {
      //return m_data.behaviours();
      return behaviours;
    }
    // 高级ai
    /*void setSuperAI(DWORD superai)
    {
      //m_data.set_superai(superai);
      this->superai = superai;
    }
    DWORD getSuperAI() const
    {
      //return m_data.superai();
      return superai;
    }*/
    void addSuperAI(DWORD dwID)
    {
      if (dwID == 0)
        return;
      setSuperAI.insert(dwID);
    }
    const TSetDWORD& getSuperAI() const
    {
      return setSuperAI;
    }

    void addExtraAI(DWORD dwID)
    {
      if (dwID == 0)
        return;
      setExtraAI.insert(dwID);
    }
    const TSetDWORD& getExtraAI() const
    {
      return setExtraAI;
    }

    void addSeatID(DWORD dwID)
    {
      if (dwID == 0)
        return;
      setSeatid.insert(dwID);
    }
    const TSetDWORD& getSeat() const
    {
      return setSeatid;
    }

    // 搜索范围
    void setSearch(DWORD search)
    {
      //m_data.set_search(search);
      this->search = search;
    }
    DWORD getSearch() const
    {
      //return m_data.search();
      return search;
    }
    // 地图唯一id
    void setUniqueID(DWORD uniqueid)
    {
      //m_data.set_uniqueid(uniqueid);
      this->uniqueid = uniqueid;
    }
    DWORD getUniqueID() const
    {
      //return m_data.uniqueid();
      return uniqueid;
    }
    // 是否使用navmesh 
    void setIgnoreNavMesh(DWORD inav)
    {
      this->ignorenavmesh = inav;
    }
    DWORD getIgnoreNavMesh() const
    {
      return ignorenavmesh;
    }
    // 装置初始状态
    void setGearOrgState(DWORD gearorgstate)
    {
      //m_data.set_gearorgstate(gearorgstate);
      this->gearorgstate = gearorgstate;
    }
    DWORD getGearOrgState() const
    {
      //return m_data.gearorgstate();
      return gearorgstate;
    }
    // 是否私有装置
    void setGearPrivate(bool gearprivate)
    {
      if (gearprivate)
          //m_data.set_gearprivate(1);
          this->gearprivate = 1;
      else
          //m_data.set_gearprivate(0);
          this->gearprivate = 0;
    }
    bool isGearPrivate() const
    {
      //if (m_data.gearprivate())
      if (this->gearprivate)
        return true;
      else
        return false;
    }
    // 副本boss净化值, !=0 表示需要净化
    void setPurify(DWORD purify)
    {
      //m_data.set_purify(purify);
      this->purify = purify;
    }
    DWORD getPurify() const
    {
      //return m_data.purify();
      return purify;
    }
    // !=0 表示不会主动攻击比自身低m_dwAttSafeLv等级的敌人
    void setAttSafeLv(DWORD attsafelv)
    {
      //m_data.set_attsafelv(attsafelv);
      this->attsafelv = attsafelv;
    }
    DWORD getAttSafeLv() const
    {
      //return m_data.attsafelv();
      return attsafelv;
    }
    // !=0 表示出生到消失的时间间隔
    void setDisptime(DWORD disptime)
    {
      //m_data.set_disptime(disptime);
      this->disptime = disptime;
    }
    DWORD getDisptime() const
    {
      //return m_data.disptime();
      return disptime;
    }
    // 等级 没设置读base的值
    void setLevel(DWORD level)
    {
      //m_data.set_level(level);
      this->level = level;
    }
    DWORD getLevel() const
    {
      //return m_data.level();
      return level;
    }
    //
    void setWaitAction(const char *waitaction)
    {
      //m_data.set_waitaction(waitaction);
      this->waitaction = waitaction;
    }
    const string& getWaitAction() const
    {
      //return m_data.waitaction();
      return waitaction;
    }
    void setPursue(DWORD pursue)
    {
      this->pursue = pursue;
    }
    DWORD getPursue()
    {
      return pursue;
    }
    void setPursueTime(DWORD pursuetime)
    {
      this->pursuetime = pursuetime;
    }
    DWORD getPursueTime()
    {
      return pursuetime;
    }

    void setWeapnPetID(DWORD id) { this->weaponpetid = id; }
    DWORD getWeaponPetID() const { return weaponpetid; }

    bool getVisibleInMap() { return visibleInMap; }
    void setVisibleInMap(bool visible) { visibleInMap = visible; }

    void setSummonType(ESummonType eType)
    {
      if (eType <= ESUMMONTYPE_MIN || eType >= ESUMMONTYPE_MAX)
        return;
       eSummonType = eType;
    }
    ESummonType getSummonType() const { return eSummonType; }
    void loadSummonType(const string& str)
    {
      if (str == "branch")
        eSummonType = ESUMMONTYPE_BRANCH;
    }

    void setFadeIn(DWORD ms) { fadein = ms; }
    DWORD getFadeIn() const { return fadein; }

 
    void setDeadDispTime(DWORD sec) { dead_disptime = sec; }
    DWORD getDeadDispTime() const { return dead_disptime; }

    void setReliveAtDeadPos(bool f) { relive_at_dead_pos = f; }
    bool isReliveAtDeadPos() const { return relive_at_dead_pos; }

    void setIcon(const string& i) { icon = i; }
    string getIcon() const { return icon; }

    void setRaceType(ERaceType r) { race = r; }
    ERaceType getRaceType() const { return race; }

    void setNatureType(ENatureType n) { nature = n; }
    ENatureType getNatureType() const { return nature; }

    void setShape(DWORD s) { shape = s; }
    DWORD getShape() const { return shape; }

    void setBody(DWORD b) { body = b; }
    DWORD getBody() const { return body; }

    void setHair(DWORD h) { hair = h; }
    DWORD getHair() const { return hair; }

    void setLeftHand(DWORD l) { lefthand = l; }
    DWORD getLeftHand() const { return lefthand; }

    void setRightHand(DWORD r) { righthand = r; }
    DWORD getRightHand() const { return righthand; }

    void setHead(DWORD h) { head = h; }
    DWORD getHead() const { return head; }

    void setBack(DWORD b) { back = b; }
    DWORD getBack() const { return back; }

    void setJobExp(DWORD exp) { jobexp = exp; }
    DWORD getJobExp() const { return jobexp; }

    void setBaseExp(DWORD exp) { baseexp = exp; }
    DWORD getBaseExp() const { return baseexp; }

    void setNormalSkillID(DWORD id) { normalskillid = id; }
    DWORD getNormalSkillID() const { return normalskillid; }

    void setSuperAiNpc(bool f) { bSuperAiNpc = f; }
    bool getSuperAiNpc() const { return bSuperAiNpc; }

    void addRandomReward(DWORD id, DWORD count, DWORD weight) {
      if (id <= 0 || count <= 0)
        return;
      if (weight <= 0)
      {
        ItemInfo item;
        item.set_id(id);
        item.set_count(count);
        item.set_source(ESOURCE_ACTIVITY_EVENT);
        rewards.push_back(item);
      }
      else
      {
        randomrewards.push_back(make_tuple(id, count, weight));
        randrewardWeight += weight;
      }
    }
    void getRandomReward(TVecItemInfo& items) const
    {
      if (rewards.empty() == false)
        combinItemInfo(items, rewards);
      DWORD r = randBetween(1, randrewardWeight), w = 0;
      for (auto& v : randomrewards)
      {
        w += get<2>(v);
        if (r <= w)
        {
          ItemInfo item;
          item.set_id(get<0>(v));
          item.set_count(get<1>(v));
          item.set_source(ESOURCE_ACTIVITY_EVENT);
          combinItemInfo(items, TVecItemInfo{item});
          break;
        }
      }
    }

    void setDeadLv(DWORD dwLv) { deadlv = dwLv; }
    DWORD getDeadLv() const { return deadlv; }

    void addDeadRewardID(DWORD dwID) { setDeadRewardIDs.insert(dwID); }
    const TSetDWORD& getDeadRewardIDs() const { return setDeadRewardIDs; }

    void setBossType(DWORD dwType) { dwBossType = dwType; }
    DWORD getBossType() const { return dwBossType; }

    void toData(Cmd::NpcDefineData* pData)
    {
      if (pData == nullptr)
        return;

      pData->set_id(id);
      pData->set_name(name);
      pData->set_range(range);
      pData->set_dir(dir);
      pData->set_reborn(reborn);
      pData->set_territory(territory);
      pData->set_scalemin(scalemin);
      pData->set_scalemax(scalemax);
      pData->set_life(life);
      pData->set_behaviours(behaviours);
      //pData->set_superai(superai);
      pData->set_search(search);
      pData->set_uniqueid(uniqueid);
      pData->set_gearorgstate(gearorgstate);
      pData->set_purify(purify);
      pData->set_attsafelv(attsafelv);
      pData->set_disptime(disptime);
      pData->set_gearprivate(gearprivate);
      pData->set_level(level);
      pData->set_waitaction(waitaction);
      pData->set_stype(eSummonType);
      pData->set_pursue(pursue);
      pData->set_pursuetime(pursuetime);
      pData->set_weaponpetid(weaponpetid);
      pData->set_deadlv(deadlv);

      pData->mutable_pos()->set_x(pos.x);
      pData->mutable_pos()->set_y(pos.y);
      pData->mutable_pos()->set_z(pos.z);

      for (auto &s : setSuperAI)
        pData->add_superai(s);
      for (auto &s : setDeadRewardIDs)
        pData->add_deadrewardids(s);
    }
    void fromData(const Cmd::NpcDefineData& rData)
    {
      id = rData.id();
      name = rData.name();
      range = rData.range();
      dir = rData.dir();
      reborn = rData.reborn();
      territory = rData.territory();
      scalemin = rData.scalemin();
      scalemax = rData.scalemax();
      life = rData.life();
      behaviours = rData.behaviours();
      //superai = rData.superai();
      search = rData.search();
      uniqueid = rData.uniqueid();
      gearorgstate = rData.gearorgstate();
      purify = rData.purify();
      attsafelv = rData.attsafelv();
      disptime = rData.disptime();
      gearprivate = rData.gearprivate();
      level = rData.level();
      waitaction = rData.waitaction();
      eSummonType = rData.stype();
      pursue = rData.pursue();
      pursuetime = rData.pursuetime();
      weaponpetid = rData.weaponpetid();
      deadlv = rData.deadlv();

      pos.x = rData.pos().x();
      pos.y = rData.pos().y();
      pos.z = rData.pos().z();

      for (int i = 0; i < rData.superai_size(); ++i)
        setSuperAI.insert(rData.superai(i));
      for (int i = 0; i < rData.deadrewardids_size(); ++i)
        setDeadRewardIDs.insert(rData.deadrewardids(i));
    }
  private:
    //Cmd::NpcDefineData m_data;   // Proto

    DWORD id = 0;

    string name;

    xPos pos;
    DWORD range = 0;// = 4 [ default = 0 ];      /
    DWORD dir = 0;// [ default = 0 ];
    DWORD reborn = 0;// [ default = 0 ];      
    DWORD territory = 0;// [ default = 0 ];   
    float scalemin = 0.0f;// [ default = 0.0 ];
    float scalemax = 0.0f;// [ default = 0.0 ];
    DWORD life = 1;// [ default = 0 ];      /
    DWORD behaviours = 0;// [ default = 0 ]; 
    //DWORD superai = 0;// [ default = 0 ];    
    DWORD search = 0;// [ default = 0 ];     
    DWORD uniqueid = 0;// [ default = 0 ];   
    DWORD ignorenavmesh = 0;// [ default = 0 ];   
    DWORD gearorgstate = 0;// [ default = 0 ]
    DWORD purify = 0;// [ default = 0 ];   //
    DWORD attsafelv = 0;// [ default = 0 ];  
    DWORD disptime = 0;// [ default = 0 ];   
    DWORD gearprivate = 0;// [ default = 0 ];
    DWORD level = 0;// [ default = 0 ];   // 
    string waitaction;
    ESummonType eSummonType = ESUMMONTYPE_MIN;

    TSetDWORD setSuperAI;
    TSetDWORD setExtraAI;
    DWORD pursue = 0;
    DWORD pursuetime = 0;
    DWORD weaponpetid = 0;
    bool visibleInMap = false;
    DWORD fadein = 0; // 渐入时间(ms)
    DWORD dead_disptime = 0;
    bool relive_at_dead_pos = false; // 原地复活
    string icon;
    ERaceType race = ERACE_MIN;
    ENatureType nature = ENature_MIN;
    DWORD shape = 0;
    DWORD body = 0;
    DWORD hair = 0;
    DWORD lefthand = 0;
    DWORD righthand = 0;
    DWORD head = 0;
    DWORD back = 0;
    DWORD jobexp = 0;
    DWORD baseexp = 0;
    DWORD normalskillid = 0;
    TVecItemInfo rewards;
    TSetDWORD setDeadRewardIDs;
    vector<tuple<DWORD, DWORD, DWORD>> randomrewards;
    DWORD randrewardWeight = 0;
    TSetDWORD setSeatid;
    bool bSuperAiNpc = false;
    bool ignoreextrarwd = false;
    DWORD deadlv = 0;
    DWORD dwBossType = 0;
  public:
    NpcVar m_oVar;               // 参数  需要在添加到地图之前设置 不会保存 临时数据
};

//npc死亡到从场景删除的时间
#define NPC_DIE_DELETE_TIME 3

const float MOVE_SPEED_BASE = 1000.0f / 3.5f;    // 移动1米的时间 毫秒
const float ATTACK_SPEED_BASE = 1000.0f;    // 攻击一次的时间 毫秒
//const float BEATTACK_SPEED_BASE = 100.0f;    // 被攻击一次的时间 毫秒
const float BEATTACK_SPEED_BASE = 100.0f;    // 被攻击一次的时间 毫秒
const float BEATTACK_CONTINUE = 30.f;  //被连续攻击 延时移动 基数 毫秒
const float ATTACK_NPC_SPEED_BASE = 1500.0f;    // 攻击一次的时间 毫秒
