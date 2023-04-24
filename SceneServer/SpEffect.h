#pragma once

#include "xNoncopyable.h"
#include "xDefine.h"
#include "RecordCmd.pb.h"
#include "ProtoCommon.pb.h"
#include "SceneMap.pb.h"
#include <unordered_map>
#include <set>

class xSceneEntryDynamic;
struct SSpEffect;
typedef std::vector<Cmd::SpEffectData> TVecSpEffectData;
typedef std::map<DWORD, TVecSpEffectData> TMapSpEffectData;

class SpEffect : private xNoncopyable
{
  public:
    SpEffect(xSceneEntryDynamic *u);
    ~SpEffect();

  public:
    void save(Cmd::BlobSpEffect *data);
    void load(const Cmd::BlobSpEffect &data);
    void timer(DWORD curTime);

    bool add(DWORD id, TSetQWORD& target, bool bTarLined = false);
    void del(DWORD id, const std::string& guid);
    void del(DWORD id, QWORD qwIgnoreId);
    void checkTarget(TSetQWORD& target);
    void collectSpEffectData(Cmd::MapUser* pData);
    void collectSpEffectData(Cmd::MapNpc* pData);
    bool hasLine(DWORD id);
    bool hasLineEntry(DWORD lineid, QWORD entryid);
    bool beLineByOther(DWORD lineid);
    
    void beLined(QWORD charId, DWORD expireTime);    
    DWORD whoLinedMeCnt();
    void onLeaveScene();

  private:
    void onAdd(const SSpEffect* pCFG, const Cmd::SpEffectData& blobData);
    void onDel(const SSpEffect* pCFG, const Cmd::SpEffectData& blobData);
    void send(const Cmd::SpEffectData& blobData, bool isAdd = true);

  private:
    xSceneEntryDynamic *m_pEntry = NULL;
    TMapSpEffectData m_mapSpEffectData;
    std::map<QWORD, DWORD/*expiretime*/> m_mapWhoLineMe;
};
