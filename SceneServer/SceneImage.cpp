#include "SceneImage.h"
#include "Scene.h"
#include "SceneUser.h"
#include "SceneUserManager.h"
#include "MapConfig.h"
#include "SceneManager.h"
#include "SceneNpcManager.h"
#include "SceneNpc.h"
#include "MsgManager.h"

SceneImage::SceneImage(Scene *s):m_pScene(s)
{
}

SceneImage::~SceneImage()
{
}

bool SceneImage::add(DWORD raidid, SceneUser *user, xPos center, DWORD range, DWORD sealid/*=0*/, DWORD npcId/*=0*/)
{
  if (m_pScene->isDScene() || !user) return false;

  const SRaidCFG *base = MapConfig::getMe().getRaidCFG(raidid);
  if (base)
  {
    if (base->dwMapID != m_pScene->getMapID()) return false;
    switch (base->eRestrict)
    {
      case ERAIDRESTRICT_PRIVATE:// RaidMapRestrict::Private:
        {
          ImageItem &image = m_oPrivateList[user->id];
          image.dwRaidID = raidid;
          image.oCenterPos = center;
          image.dwRange = range;
          image.dwSealConfig = sealid;

          check(user);
        }
        break;
      case ERAIDRESTRICT_TEAM://RaidMapRestrict::Team:
        {
          //SceneTeam *pTeam = user->getTeam();
          //if (!pTeam) return false;

          QWORD teamid = user->getTeamID();
          TVecImageItem& vecImage = m_oTeamList[teamid];
          auto it = find_if(vecImage.begin(), vecImage.end(), [raidid](const ImageItem& r) -> bool{
              return raidid == r.dwRaidID;
              });
          if (it != vecImage.end()) return false;

          ImageItem image;
          image.dwRaidID = raidid;
          image.oCenterPos = center;
          image.dwRange = range;
          image.dwSealConfig = sealid; 
          image.dwNpcId = npcId;

          if (base->eRaidType == ERAIDTYPE_ITEMIMAGE)
          {            
            NpcDefine oDefine;
            oDefine.setID(npcId);
            oDefine.resetting();
            oDefine.setPos(center);
            oDefine.m_oVar.m_qwTeamID = teamid;
            oDefine.setBehaviours(oDefine.getBehaviours() | BEHAVIOUR_NOT_SKILL_SELECT);

            SceneNpc* pNpc = SceneNpcManager::getMe().createNpc(oDefine, m_pScene);
            if (!pNpc)
            {
              MsgManager::sendMsg(user->id, 863);
              return false;
            }        
   
            UserActionNtf message;
            message.set_type(EUSERACTIONTYPE_GEAR_ACTION);
            message.set_value(1001);
            message.set_charid(pNpc->id);
            pNpc->setGearStatus(1002);
            PROTOBUF(message, send, len);
            pNpc->sendCmdToNine(send, len);

            image.m_vecNpcs.push_back(pNpc);
            //pTeam->setImageCreator(user->id);
          }
          
          vecImage.push_back(image);
          check(user);
          user->m_blCheckTeamImage = true;
        }
        break;
      case ERAIDRESTRICT_USER_TEAM:
        {
          for (auto &m : m_oUserTeamList)
          {
            if (!user->isMyTeamMember(m.first))
              continue;
            if (m.second.dwRaidID == raidid)
              return false;
          }
          ImageItem& image = m_oUserTeamList[user->id];
          image.dwRaidID = raidid;
          image.oCenterPos = center;
          image.dwRange = range;
          image.dwSealConfig = sealid;

          check(user);
          user->m_blCheckTeamImage = true;
        }
        break;
      default:
        return false;
        break;
    }
  }
  return true;
}

void SceneImage::checkTeam(SceneUser *user)
{
  if (!user) return;

  if (user->getTeamID() == 0)
    return;
  const GTeam& rTeam = user->getTeam();
  for (auto &m : rTeam.getTeamMemberList())
  {
    if (m.second.charid() == user->id)
      continue;

    SceneUser *pUser = SceneUserManager::getMe().getUserByID(m.second.charid());
    if (pUser)
    {
      check(pUser);
    }
  }
}

void SceneImage::check(SceneUser *user)
{
  if (!user || !user->getScene() || user->getScene()!=m_pScene) return;

  if (m_pScene->isDScene())
  {
    if (m_oRaidData.dwRange)
    {
      DScene* pDScene = dynamic_cast<DScene*> (m_pScene);
      if (!pDScene || !pDScene->getRaidCFG()) return;

      if (!checkRadius(m_oRaidData.oCenterPos, user->getPos(), m_oRaidData.dwRange))
      {
        if (pDScene->getRaidCFG()->dwLeaveImageTime == 0)
        {
          user->gomap(m_pScene->getMapID(), GoMapType::Image, user->getPos());
        }
        else
        {
          auto it = m_mapUser2Outtime.find(user->id);
          if (it == m_mapUser2Outtime.end())
          {
            m_mapUser2Outtime[user->id] = now() + pDScene->getRaidCFG()->dwLeaveImageTime;
            return;
          }
        }
        return;
      }
      else
      {
        auto it = m_mapUser2Outtime.find(user->id);
        if (it != m_mapUser2Outtime.end())
        {
          m_mapUser2Outtime.erase(user->id);
        }
      }
    }
  }
  else
  {
    {
      auto it = m_oPrivateList.find(user->id);
      if (it != m_oPrivateList.end())
      {
        if (checkRadius(it->second.oCenterPos, user->getPos(), it->second.dwRange))
        {
          CreateDMapParams params;
          params.dwRaidID = it->second.dwRaidID;
          params.m_oEnterPos = user->getPos();
          params.m_oImageCenter = it->second.oCenterPos;
          params.m_dwImageRange = it->second.dwRange;
          params.qwCharID = user->id;

          SceneManager::getMe().createDScene(params);
          //user->createDMap(it->second.dwRaidID, params);
          return;
        }
      }
    }
    {
      for (auto &m : m_oUserTeamList)
      {
        if (user->isMyTeamMember(m.first) && checkRadius(m.second.oCenterPos, user->getPos(), m.second.dwRange))
        {
          CreateDMapParams params;
          params.dwRaidID = m.second.dwRaidID;
          params.m_oEnterPos = user->getPos();
          params.m_oImageCenter = m.second.oCenterPos;
          params.m_dwImageRange = m.second.dwRange;
          params.m_dwSealID = m.second.dwSealConfig;
          params.qwCharID = m.first;
          params.m_dwNpcId = m.second.dwNpcId;
          params.vecMembers.push_back(user->id);
          SceneManager::getMe().createDScene(params);
          return;
        }
      }
    }
    if (user->getTeamID())
    {
      const GTeam& rTeam = user->getTeam();
      auto it = m_oTeamList.find(rTeam.getTeamID());
      if (it != m_oTeamList.end())
      {
        for (auto &v : it->second)
        {
          if (checkRadius(v.oCenterPos, user->getPos(), v.dwRange))
          {
            CreateDMapParams params;
            params.dwRaidID = v.dwRaidID;
            params.m_oEnterPos = user->getPos();
            params.m_oImageCenter = v.oCenterPos;
            params.m_dwImageRange = v.dwRange;
            params.m_dwSealID = v.dwSealConfig;
            params.qwCharID = user->id;
            params.m_dwNpcId = v.dwNpcId;
            SceneManager::getMe().createDScene(params);
            //user->createDMap(it->second.dwRaidID, params);
            return;
          }
        }
      }
    }
  }
}

void SceneImage::del(QWORD guid, DWORD raidid)
{
  auto it = m_oPrivateList.find(guid);
  if (it != m_oPrivateList.end())
    m_oPrivateList.erase(guid);

  auto k = m_oUserTeamList.find(guid);
  if (k != m_oPrivateList.end())
    m_oUserTeamList.erase(guid);

  auto m = m_oTeamList.find(guid);
  if (m == m_oTeamList.end()) return;
  auto v = find_if(m->second.begin(), m->second.end(), [raidid](const ImageItem& r) ->bool{
      return r.dwRaidID == raidid;
      });
  if (v == m->second.end())
    return;
  onImageClose(*v);
  m->second.erase(v);
}

void SceneImage::goRealMap(SceneUser* pUser)
{
  if (pUser == nullptr)
    return;
  if (m_pScene->isDScene())
    pUser->gomap(m_pScene->getMapID(), GoMapType::Image, pUser->getPos());
}

void SceneImage::onImageClose(const ImageItem& imageItem)
{
  const SRaidCFG *base = MapConfig::getMe().getRaidCFG(imageItem.dwRaidID);
  if (!base)
    return;
 
  if (base->eRaidType == ERAIDTYPE_ITEMIMAGE)
  {
    //clear npc, scenenpcmanager will del 
    for (auto &v : imageItem.m_vecNpcs)
      if (v)
        v->removeAtonce();
  }
}

void SceneImage::timer(DWORD dwCurTime)
{
  if (m_pScene->isDScene())
  {
    if (m_mapUser2Outtime.size() == 0)
      return;
    for (auto m = m_mapUser2Outtime.begin(); m != m_mapUser2Outtime.end(); )
    {
      if (dwCurTime < m->second)
      {
        ++m;
        continue;
      }
      SceneUser* user = SceneUserManager::getMe().getUserByID(m->first);
      if (!user || user->getScene() != m_pScene)
      {
        m = m_mapUser2Outtime.erase(m);
        continue;
      }
      m = m_mapUser2Outtime.erase(m);
      user->gomap(m_pScene->getMapID(), GoMapType::Image, user->getPos());
    }
  }
}

bool SceneImage::userCanEnter(SceneUser* user)
{
  if (user == nullptr)
    return false;

  if (m_qwImageIndex == 0)
    return true;

  DScene* pDScene = dynamic_cast<DScene*> (m_pScene);
  if (pDScene == nullptr)
    return true;

  // 个人
  if (m_qwImageIndex == user->id)
    return true;

  if (user->getTeamID() == 0)
    return false;

  // 组队
  if (m_qwImageIndex == user->getTeamID())
    return true;

  // 个人 | 组队
  const GTeam& rTeam = user->getTeam();
  for (auto &m : rTeam.getTeamMemberList())
  {
    if (m.second.charid() == m_qwImageIndex)
      return true;
  }
  return false;
}
