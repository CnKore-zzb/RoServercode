#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "xLuaTable.h"

using std::string;
using std::map;
using namespace Cmd;

#define GCHAR_VERSION 1

typedef map<QWORD, DWORD> TMapSocial;

class GChar
{
  public:
    GChar(DWORD dwRegionID, QWORD charid);
    GChar& operator=(const GChar& rChar);
    virtual ~GChar() = 0;

    void setCharID(QWORD qwCharID);
    QWORD getCharID() const { return m_qwCharID; }

  public:
    // GChar 增加version字段
    // version 1 增加 sequence,mount,title,partnerid,nologintime
    DWORD getVersion() const { return m_oData.getTableInt("version"); }
    void setVersion() { m_oData.setData("version", GCHAR_VERSION); }

    // redis 里面读出的数据
    // get reader use
    // set writer use
  public:
    QWORD getAccID() const { return m_oData.getTableQWORD("accid"); }
    void setAccID(QWORD qwAccID) { m_oData.setData("accid", qwAccID);; }

    const char* getName() const { return (strcmp(m_oData.getTableString("name"), "null") == 0) ? "" : m_oData.getTableString("name");  }
    void setName(const string& name) { m_oData.setData("name", name); }

    DWORD getJobLevel() const { return m_oData.getTableInt("joblv"); }
    void setJobLevel(DWORD dwJobLv) { m_oData.setData("joblv", dwJobLv); }

    DWORD getBaseLevel() const { return m_oData.getTableInt("rolelv"); }
    void setBaseLevel(DWORD dwBaseLv) { m_oData.setData("rolelv", dwBaseLv); }

    DWORD getMapID() const { return m_oData.getTableInt("mapid"); }
    void setMapID(DWORD dwMapID) { m_oData.setData("mapid", dwMapID); }

    DWORD getPortrait() const { return m_oData.getTableInt("portrait"); }
    void setPortrait(DWORD dwPortrait) { m_oData.setData("portrait", dwPortrait); }

    DWORD getBody() const { return m_oData.getTableInt("body"); }
    void setBody(DWORD dwBody) { m_oData.setData("body", dwBody); }

    DWORD getHead() const { return m_oData.getTableInt("head"); }
    void setHead(DWORD dwHead) { m_oData.setData("head", dwHead); }

    DWORD getFace() const { return m_oData.getTableInt("face"); }
    void setFace(DWORD dwFace) { m_oData.setData("face", dwFace); }

    DWORD getBack() const { return m_oData.getTableInt("back"); }
    void setBack(DWORD dwBack) { m_oData.setData("back", dwBack); }

    DWORD getTail() const { return m_oData.getTableInt("tail"); }
    void setTail(DWORD dwTail) { m_oData.setData("tail", dwTail); }

    DWORD getHair() const { return m_oData.getTableInt("hair"); }
    void setHair(DWORD dwHair) { m_oData.setData("hair", dwHair); }

    DWORD getHairColor() const { return m_oData.getTableInt("haircolor"); }
    void setHairColor(DWORD dwHairColor) { m_oData.setData("haircolor", dwHairColor); }

    DWORD getClothColor() const { return m_oData.getTableInt("clothcolor"); }
    void setClothColor(DWORD dwClothColor) { m_oData.setData("clothcolor", dwClothColor); }

    DWORD getLeftHand() const { return m_oData.getTableInt("lefthand"); }
    void setLeftHand(DWORD dwLeftHand) { m_oData.setData("lefthand", dwLeftHand); }

    DWORD getRightHand() const { return m_oData.getTableInt("righthand"); }
    void setRightHand(DWORD dwRightHand) { m_oData.setData("righthand", dwRightHand); }

    DWORD getFrame() const { return m_oData.getTableInt("frame"); }
    void setFrame(DWORD dwFrame) { m_oData.setData("frame", dwFrame); }

    EProfession getProfession() const { return static_cast<EProfession>(m_oData.getTableInt("profession")); }
    void setProfession(EProfession eProfession) { m_oData.setData("profession", eProfession); }

    EGender getGender() const { return static_cast<EGender>(m_oData.getTableInt("gender")); }
    void setGender(EGender eGender) { m_oData.setData("gender", eGender); }

    bool getBlink() const { return m_oData.getTableInt("blink"); }
    void setBlink(bool blink) { m_oData.setData("blink", blink); }

    DWORD getOnlineTime() const { return m_oData.getTableInt("onlinetime"); }
    void setOnlineTime(DWORD dwTime) { m_oData.setData("onlinetime", dwTime); }

    DWORD getOfflineTime() const { return m_oData.getTableInt("offlinetime"); }
    void setOfflineTime(DWORD dwTime) { m_oData.setData("offlinetime", dwTime); }

    DWORD getEye() const { return m_oData.getTableInt("eye"); }
    void setEye(DWORD dwEye) { m_oData.setData("eye", dwEye); }

    DWORD getMouth() const { return m_oData.getTableInt("mouth"); }
    void setMouth(DWORD dwMouth) { m_oData.setData("mouth", dwMouth); }

    DWORD getZoneID() const { return m_oData.getTableInt("zoneid"); }
    void setZoneID(DWORD dwZoneID) { m_oData.setData("zoneid", dwZoneID); }

    DWORD getOriginalZoneID() const { return m_oData.getTableInt("originalzoneid"); }
    void setOriginalZoneID(DWORD dwZoneID) { m_oData.setData("originalzoneid", dwZoneID); }

    DWORD getManualLv() const { return m_oData.getTableInt("manuallv"); }
    void setManualLv(DWORD dwLv) { m_oData.setData("manuallv", dwLv); }

    DWORD getManualExp() const { return m_oData.getTableInt("manualexp"); }
    void setManualExp(DWORD dwExp) { m_oData.setData("manualexp", dwExp); }

    DWORD getTitleID() const { return m_oData.getTableInt("titleid"); }
    void setTitleID(DWORD dwID) { m_oData.setData("titleid", dwID); }

    DWORD getQueryType() const { return m_oData.getTableInt("querytype"); }
    void setQueryType(DWORD dwType) { m_oData.setData("querytype", dwType); }
    DWORD getQueryWeddingType() const { return m_oData.getTableInt("queryweddingtype"); }
    void setQueryWeddingType(DWORD dwType) { m_oData.setData("queryweddingtype", dwType); }
    // 选角界面格子编号
    DWORD getSequence() const { return m_oData.getTableInt("sequence"); }
    void setSequence(DWORD dwSequence) { m_oData.setData("sequence", dwSequence); }

    // 坐骑
    DWORD getMount() const { return m_oData.getTableInt("mount"); }
    void setMount(DWORD dwMount) { m_oData.setData("mount", dwMount); }

    // 称号 冒险登记
    //DWORD getTitle() const { return m_oData.getTableInt("title"); }
    //void setTitle(DWORD dwTitle) { m_oData.setData("title", dwTitle); }

    // 宠物
    DWORD getPartnerID() const { return m_oData.getTableInt("partnerid"); }
    void setPartnerID(DWORD dwPartnerID) { m_oData.setData("partnerid", dwPartnerID); }

    // 封号时间
    DWORD getNologinTime() const { return m_oData.getTableInt("nologintime"); }
    void setNologinTime(DWORD dwNologinTime) { m_oData.setData("nologintime", dwNologinTime); }

    // SocialServer Set
    QWORD getGuildID() const { return m_oData.getTableQWORD("guild_id"); }
    void setGuildID(QWORD qwGuildID) { m_oData.setData("guild_id", qwGuildID); }

    const char* getGuildName() const { return (strcmp(m_oData.getTableString("guild_name"), "null") == 0) ? "" : m_oData.getTableString("guild_name"); }
    void setGuildName(const string& name) { m_oData.setData("guild_name", name); }

    const char* getGuildPortrait() const { return (strcmp(m_oData.getTableString("guild_portrait"), "null") == 0) ? "" : m_oData.getTableString("guild_portrait"); }
    void setGuildPortrait(const string& portrait) { m_oData.setData("guild_portrait", portrait); }

    //交易所最大挂单限制
    DWORD getPendingLimit() const { return m_oData.getTableInt("pendinglimit"); }
    void setPendingLimit(DWORD limit) { m_oData.setData("pendinglimit", limit); }

    //摊位最大挂单限制
    DWORD getBoothPendingLimit() const { return m_oData.getTableInt("boothPendinglimit"); }
    void setBoothPendingLimit(DWORD limit) { m_oData.setData("boothPendinglimit", limit); }

    //摊位开启状态 0:关闭；1开启
    DWORD getBoothOpenStatus() const { return m_oData.getTableInt("boothOpenStatus"); }
    void setBoothOpenStatus(DWORD status) { m_oData.setData("boothOpenStatus", status); }

    //交易所撤单返回上架费比例，千分比
    DWORD getReturnRate() const { return m_oData.getTableInt("returnrate"); }
    void setReturnRate(DWORD rate) { m_oData.setData("returnrate", rate); }

    //赠送额度
    DWORD getQuota() const { return m_oData.getTableInt("quota"); }
    void setQuota(QWORD quota) { m_oData.setData("quota", quota); }

    // 导师
    void setTutor(bool b) { m_oData.setData("tutor", b); }
    bool getTutor() const { return m_oData.getTableInt("tutor"); }

    void setProfic(DWORD d) { m_oData.setData("profic", d); }
    DWORD getProfic() const { return m_oData.getTableInt("profic"); }

    //设置结婚对方名字
    void setWeddingPartner(const std::string& name) { m_oData.setData("wedding_partner", name); }
    const char* getWeddingPartner() const { return (strcmp(m_oData.getTableString("wedding_partner"), "null") == 0) ? "" : m_oData.getTableString("wedding_partner"); }

    // 设置服装

    const TMapSocial& getSocial() { return m_mapSocial; }
    void clearSocial() { m_mapSocial.clear(); }
    void setSocial()
    {
      if (m_mapSocial.empty())
      {
        m_oData.setData("social", "");
        return;
      }

      stringstream sstr;
      sstr.str("");
      for (auto &m : m_mapSocial)
        sstr << m.first << ":" << m.second << ";";

      m_oData.setData("social", sstr.str());
    }

    bool checkRelation(QWORD qwCharID, DWORD dwRelation);
    bool updateRelation(QWORD qwCharID, DWORD dwRelation);
    bool addRelation(QWORD qwCharID, DWORD dwRelation);
    bool delRelation(QWORD qwCharID);
    DWORD getRelationCount(DWORD dwRelation);
    DWORD getRecallCount();

    void debug_log();
    const string& getKey() const { return m_strRedisKey; }
  protected:
    void parseSocial();
  protected:
    TMapSocial m_mapSocial;

  protected:
    QWORD m_qwCharID = 0;
    DWORD m_dwRegionID = 0;

    std::string m_strRedisKey;

  protected:
    xLuaData m_oData;
};

class GCharReader : public GChar
{
  public:
    GCharReader(DWORD dwRegionID, QWORD charid);
    virtual ~GCharReader();

  public:
    bool get();
    EError getBySocial();
    bool getByGuild();
    bool getByTeam();
    bool getByPvpTeam();
    bool getNameOnly();
    bool getByTutor();
    bool getByQuery();
    bool getByWedding();

    DWORD fetchMapID();

  private:
    bool get(xLuaData &data);
};

class GCharWriter : public GChar
{
  public:
    GCharWriter(DWORD dwRegionID, QWORD charid);
    virtual ~GCharWriter();

  public:
    bool save();
};
