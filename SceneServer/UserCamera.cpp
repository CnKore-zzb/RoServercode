#include "UserCamera.h"
#include "xSceneEntryDynamic.h"
#include "SceneUser.h"
#include "SceneNpc.h"
#include "SceneNpcManager.h"
#include "MiscConfig.h"
#include "LuaManager.h"

UserCamera::UserCamera(SceneUser* user) : m_pUser(user)
{

}

UserCamera::~UserCamera()
{

}

void UserCamera::load(const BlobCamera& data)
{
  m_mapMons2Num.clear();
  for (int i = 0; i < data.monsters_size(); ++i)
  {
    m_mapMons2Num[data.monsters(i).monsterid()] = data.monsters(i).count();
  }
}

void UserCamera::save(BlobCamera* pData)
{
  if (pData == nullptr)
    return;
  for (auto &m : m_mapMons2Num)
  {
    MonsterNum* pMons = pData->add_monsters();
    if (pMons == nullptr)
      continue;
    pMons->set_monsterid(m.first);
    pMons->set_count(m.second);
  }
}

void UserCamera::onCamera()
{
  if (m_pUser->isShootGhost() == false || m_pUser->getScene() == nullptr)
    return;
  if (m_pUser->getVar().getVarValue(EVARTYPE_CAMERA_SUMMON_DAILY) == 0)
  {
    clear();
    m_pUser->getVar().setVarValue(EVARTYPE_CAMERA_SUMMON_DAILY, 1);
  }

  const SCameraCFG& rCFG = MiscConfig::getMe().getCameraCFG();

  DWORD cur = now();
  if (cur < m_dwNextSummonTime)
    return;
  m_dwNextSummonTime = cur + rCFG.dwInterval;

  for (auto &v : rCFG.vecMonster)
  {
    DWORD num = 0;
    auto it = m_mapMons2Num.find(v.dwID);
    if (it != m_mapMons2Num.end())
      num = it->second;

    // 今日招怪已达上限
    if (v.dwDayMaxCnt != 0 && v.dwDayMaxCnt >= num)
      continue;

    bool isNight = MiscConfig::getMe().getSystemCFG().isNight(cur);
    bool randOk = LuaManager::getMe().call<bool>("CalcCameraSummonOdds", (xSceneEntryDynamic*)(m_pUser), num, v.dwOdds, isNight);

    if (!randOk)
      continue;
    if (summon(v) == false)
      continue;;
    m_mapMons2Num[v.dwID] = num + 1;
    XLOG <<"[幽灵相机-招怪], 玩家:" << m_pUser->name << m_pUser->id << "怪物:" << v.dwID << "今日已招数量:" << num + 1 << XEND;
  }
}

bool UserCamera::summon(const SCameraMonster& rCFG)
{
  if (!m_pUser || !m_pUser->getScene())
    return false;
  NpcDefine define;
  define.load(rCFG.oParams);
  define.setLife(1);

  xPos userpos = m_pUser->getPos();
  xPos pos;
  float dis = rCFG.dwDistance;
  float dir = m_pUser->getUserSceneData().getDir() / ONE_THOUSAND;
  float radian = dir / 180.0f * 3.14;

  pos.x = userpos.x - dis * sin(radian);
  pos.z = userpos.z - dis * cos(radian);
  pos.y = userpos.y;
  if (m_pUser->getScene()->getValidPos(pos, pos) == false)
  {
    if (m_pUser->getScene()->getRandPos(userpos, dis, pos) == false)
      return false;
  }
  define.setPos(pos);
  define.setDir(dir);

  SceneNpc* npc = SceneNpcManager::getMe().createNpc(define, m_pUser->getScene());
  if (!npc)
    return false;

  XLOG << "[幽灵相机-召唤怪物], 召唤成功, 玩家: "<< m_pUser->name << m_pUser->id << "怪物: "<< npc->name << npc->id << XEND;
  return true;
}

