/**
 * @file Sociality.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2016-02-22
 */

#pragma once

#include <bitset>
#include <list>
#include "xSingleton.h"
#include "SessionSociality.pb.h"
#include "SocialCmd.pb.h"
#include "GCharManager.h"

using namespace Cmd;
using std::vector;
using std::map;
using std::string;
using std::set;
using std::ostringstream;
using std::stringstream;
using std::bitset;

class xRecord;

const DWORD SOCIAL_SAVE_TICK = 600;

// sociality
class Sociality
{
  friend class SocialityManager;
  friend class UserSociality;
  public:
    Sociality();
    ~Sociality();

    bool fromData(const xRecord& rRecord);
    bool toData(xRecord& rRecord, QWORD qwID);
    bool toData(SocialData* pData);
    bool toData(SocialItem* pItem);

    void setGUID(QWORD charid) { m_oGCharData.setCharID(charid); }
    QWORD getGUID() const { return m_oGCharData.getCharID(); }

    void setACCID(QWORD qwACCID) { m_oGCharData.setAccID(qwACCID); }
    QWORD getACCID() const { return m_oGCharData.getAccID(); }

    void setLevel(DWORD dwLevel) { m_oGCharData.setBaseLevel(dwLevel); m_bitmark.set(ESOCIALDATA_LEVEL); }
    DWORD getLevel() const { return m_oGCharData.getBaseLevel(); }

    void setPortrait(DWORD dwPortrait) { m_oGCharData.setPortrait(dwPortrait); m_bitmark.set(ESOCIALDATA_PORTRAIT); }
    DWORD getPortrait() const { return m_oGCharData.getPortrait(); }

    void setFrame(DWORD dwFrame) { m_oGCharData.setFrame(dwFrame); m_bitmark.set(ESOCIALDATA_FRAME); }
    DWORD getFrame() const { return m_oGCharData.getFrame(); }

    void setHair(DWORD dwHair) { m_oGCharData.setHair(dwHair); m_bitmark.set(ESOCIALDATA_HAIR); }
    DWORD getHair() const { return m_oGCharData.getHair(); }

    void setHairColor(DWORD dwHairColor) { m_oGCharData.setHairColor(dwHairColor); m_bitmark.set(ESOCIALDATA_HAIRCOLOR); }
    DWORD getHairColor() const { return m_oGCharData.getHairColor(); }

    void setBody(DWORD dwBody) { m_oGCharData.setBody(dwBody); m_bitmark.set(ESOCIALDATA_BODY); }
    DWORD getBody() const { return m_oGCharData.getBody(); }

    void setOfflineTime(DWORD dwOfflineTime) { m_oGCharData.setOfflineTime(dwOfflineTime); m_bitmark.set(ESOCIALDATA_OFFLINETIME); }
    DWORD getOfflineTime() const { return m_oGCharData.getOfflineTime(); }

    void setAdventureLv(DWORD dwLv) { m_oGCharData.setManualLv(dwLv); m_bitmark.set(ESOCIALDATA_ADVENTURELV); }
    DWORD getAdventureLv() const { return m_oGCharData.getManualLv(); }

    void setAdventureExp(DWORD dwExp) { m_oGCharData.setManualExp(dwExp); m_bitmark.set(ESOCIALDATA_ADVENTUREEXP); }
    DWORD getAdventureExp() const { return m_oGCharData.getManualExp(); }

    void setAppellation(DWORD dwID) { m_oGCharData.setTitleID(dwID); m_bitmark.set(ESOCIALDATA_APPELLATION); }
    DWORD getAppellation() const { return m_oGCharData.getTitleID(); }

    /*void setMapID(DWORD dwMapID) { m_oGCharData.setMapID(dwMapID); m_bitmark.set(ESOCIALDATA_MAP); }
    DWORD getMapID() const { return m_oGCharData.getMapID(); }*/

    void setZoneID(DWORD dwZoneID) { m_oGCharData.setZoneID(dwZoneID); m_bitmark.set(ESOCIALDATA_ZONEID); }
    DWORD getZoneID() const { return m_oGCharData.getZoneID(); }

    bool isRecall();
    bool canBeRecall();

    const string& getCreateTime();
    GCharReader& getGCharData() { return m_oGCharData; }

    void setMark(ESocialData eData) { m_bitmark.set(eData); }
  private:
    bool addRelation(ESocialRelation eRelation);
    bool removeRelation(ESocialRelation eRelation);
    void setRelation(DWORD dwRelation) { m_dwRelation = dwRelation; }
  public:
    bool checkRelation(ESocialRelation eRelation) const { return (m_dwRelation & eRelation) != 0; }
    DWORD getRelation() const { return m_dwRelation; }

    void setRelationTime(ESocialRelation eRelation, DWORD dwTime) { m_mapRelationTime[eRelation] = dwTime; }
    DWORD getRelationTime(ESocialRelation eRelation) const;

    void setProfession(EProfession eProfession) { m_oGCharData.setProfession(eProfession); m_bitmark.set(ESOCIALDATA_PROFESSION); }
    EProfession getProfession() const { return m_oGCharData.getProfession(); }

    void setGender(EGender eGender) { m_oGCharData.setGender(eGender); m_bitmark.set(ESOCIALDATA_GENDER); }
    EGender getGender() const { return m_oGCharData.getGender(); }

    void setName(const string& name) { m_oGCharData.setName(name); m_bitmark.set(ESOCIALDATA_NAME); }
    const char* getName() const { return m_oGCharData.getName(); }

    void setGuildName(const string& name) { m_oGCharData.setGuildName(name); m_bitmark.set(ESOCIALDATA_GUILDNAME); }
    const char* getGuildName() const { return m_oGCharData.getGuildName(); }

    void setGuildPortrait(const string& portrait) { m_oGCharData.setGuildPortrait(portrait); m_bitmark.set(ESOCIALDATA_GUILDPORTRAIT); }
    const char* getGuildPortrait() const { return m_oGCharData.getGuildPortrait(); }

    void setBlink(bool b) { m_oGCharData.setBlink(b); m_bitmark.set(ESOCIALDATA_BLINK); }
    bool getBlink() const { return m_oGCharData.getBlink(); }

    void setHead(DWORD dwHead) { m_oGCharData.setHead(dwHead); m_bitmark.set(ESOCIALDATA_HEAD); }
    DWORD getHead() const { return m_oGCharData.getHead(); }

    void setFace(DWORD dwFace) { m_oGCharData.setFace(dwFace); m_bitmark.set(ESOCIALDATA_FACE); }
    DWORD getFace() const { return m_oGCharData.getFace(); }

    void setMouth(DWORD dwMouth) { m_oGCharData.setMouth(dwMouth); m_bitmark.set(ESOCIALDATA_MOUTH); }
    DWORD getMouth() const { return m_oGCharData.getMouth(); };

    void setEye(DWORD dwEye) { m_oGCharData.setEye(dwEye); m_bitmark.set(ESOCIALDATA_EYE); }
    DWORD getEye() const { return m_oGCharData.getEye(); }

    void setProfic(DWORD dwProfic) { m_oGCharData.setProfic(dwProfic); m_bitmark.set(ESOCIALDATA_TUTOR_PROFIC); }
    DWORD getProfic() const { return m_oGCharData.getProfic(); }

    void fetchChangeData(SocialDataUpdate& cmd);
  private:
    GCharReader m_oGCharData;
    DWORD m_dwRelation = 0;

    map<ESocialRelation, DWORD> m_mapRelationTime;
    string m_strCreateTime;

    bool m_bInit = false;

    bitset<ESOCIALDATA_MAX> m_bitmark;
};

