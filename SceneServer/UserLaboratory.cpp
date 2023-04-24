#include "UserLaboratory.h"
#include "SceneUser.h"
#include "RecordCmd.pb.h"

UserLaboratory::UserLaboratory(SceneUser *user):
  m_pUser(user)
{
}

UserLaboratory::~UserLaboratory()
{
}

void UserLaboratory::save(Cmd::BlobLaboratory *data)
{
  data->Clear();
  data->set_version(1);
  data->set_point(m_dwTodayMaxPoint);
  data->set_days(m_dwDays);
  XDBG << "[研究所-保存]" << m_pUser->accid << m_pUser->id << m_pUser->getProfession() << m_pUser->name << "数据大小 :" << data->ByteSize() << XEND;
}

void UserLaboratory::load(const Cmd::BlobLaboratory &data)
{
  int version = data.version();
  m_dwTodayMaxPoint = data.point();
  m_dwDays = data.days();

  if (version >= 2)
  {
  }
}

DWORD UserLaboratory::getTodayMaxPoint()
{
  if (m_pUser->getVar().getVarValue(EVARTYPE_LABORATORY_POINT) == 0)
  {
    m_dwTodayMaxPoint = 0;
    m_pUser->getVar().setVarValue(EVARTYPE_LABORATORY_POINT, 1);
  }
  /*DWORD days = getDays(now());
  if (days != m_dwDays)
  {
    m_dwDays = days;
    m_dwTodayMaxPoint = 0;
  }*/
  return m_dwTodayMaxPoint;
}

void UserLaboratory::setTodayPoint(DWORD point)
{
  if (!point) return;

  // 没超过当天最高分 则不加分
  if (point <= getTodayMaxPoint()) return;

  //DWORD add = point - m_dwTodayMaxPoint;
  //m_pUser->addMoney(EMONEYTYPE_LABORATORY, add, ESOURCE_LABORATORY);

  m_dwTodayMaxPoint = point;
}

/*DWORD UserLaboratory::getPoint()
{
  return m_pUser->getUserSceneData().getLaboratory();
}*/

