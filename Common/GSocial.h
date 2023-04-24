/**
 * @file GSocial.h
 * @brief 
 * @author sunhuiwei, sunhuiwei@xindong.com
 * @version v1
 * @date 2017-09-21
 */

#pragma once

#include "xDefine.h"
#include "SocialCmd.pb.h"

using std::set;
using std::map;
using std::string;
using namespace Cmd;

const DWORD TUTOR_ITEM_ADVENTURE_ACTIONID = 9999;

struct SGSocialItem
{
  DWORD dwRelation = 0;
  map<ESocialRelation, DWORD> mapRelationTime;
};
typedef map<QWORD, SGSocialItem> TMapGSocialList;

class GSocial
{
  public:
    GSocial();
    ~GSocial();

    static void parseRelationTime(const string& str, map<ESocialRelation, DWORD>& mapResult);
    static void serialRelationTime(const map<ESocialRelation, DWORD>& mapTime, string& result);

    bool toData(SyncSocialListSocialCmd& cmd);
    bool checkRelation(QWORD qwCharID, ESocialRelation eRelation) const;

    DWORD getRelationCount(ESocialRelation eRelation) const;
    DWORD getRelationTime(QWORD qwCharID, ESocialRelation eRelation) const;

    void initSocial(const SyncSocialListSocialCmd& cmd);
    void updateSocial(const SocialListUpdateSocialCmd& cmd);
    bool isInit() { return m_bInit; }

    QWORD getTutorCharID();
    void collectRelation(ESocialRelation eRelation, TSetQWORD& setIDs);
  private:
    TMapGSocialList m_mapSocialList;

    QWORD m_qwCharID = 0;
    bool m_bInit = false;
};

