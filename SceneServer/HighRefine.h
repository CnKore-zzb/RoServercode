#pragma once

#include "xDefine.h"
#include "SceneItem.pb.h"
#include "RecordCmd.pb.h"
#include "Var.pb.h"

class SLotteryCFG;
class SceneUser;

typedef std::map<Cmd::EEquipPos, TVecDWORD> TMapPos2Level;

class HighRefine
{
  public:
    HighRefine(SceneUser* pUser);
    ~HighRefine();
    void load(const Cmd::BlobHighRefine& data);
    void save(Cmd::BlobHighRefine* data);
    void sendHighRefineData();
    
    bool matCompose(Cmd::HighRefineMatComposeCmd& rev);
    bool highRefine(Cmd::HighRefineCmd& rev);
    
    bool checkLevel(Cmd::EEquipPos pos, DWORD lv);
    
    //void collectEquipAttr(Cmd::EEquipPos pos, DWORD refineLv, TVecAttrSvrs& vecAttrs);
    void collectEquipAttr(Cmd::EEquipPos pos, DWORD refineLv);

  private:
    SceneUser* m_pUser = nullptr; 
    TMapPos2Level m_mapHighRefineData;
};

