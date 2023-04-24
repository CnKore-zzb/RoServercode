#include "Scene.h"
#include "SceneBooth.h"
#include "SceneUser.h"
#include "xTime.h"
#include "xSceneEntryDynamic.h"


SceneBooth::SceneBooth(QWORD charid, DWORD zoneId, Scene* scene, const xPos& pos, const Cmd::MapUser& mapUser)
{
  set_id(charid);
  set_tempid(charid);
  setScene(scene);
  setPos(pos);
  m_dwZoneId = zoneId;
  m_oCmd.CopyFrom(mapUser);
}

SceneBooth::~SceneBooth()
{
  
}

void SceneBooth::sendMeToNine()
{
  if(!getScene())
    return;

  Cmd::AddMapUser cmd;
  fillData(cmd.add_users());
  PROTOBUF(cmd, send, len);
  getScene()->sendCmdToNine(getPos(), send, len);
}

void SceneBooth::delMeToNine()
{
  if(!getScene())
    return;

  Cmd::DeleteEntryUserCmd cmd;
  cmd.add_list(tempid);

  PROTOBUF(cmd, send, len);
  getScene()->sendCmdToNine(getPos(), send, len);
}

bool SceneBooth::enterScene(Scene* scene)
{
  if(!scene)
    return false;

  setScene(scene);
  if(!getScene())
    return false;

  if(!getScene()->addEntryAtPosI(this))
    return false;

  sendMeToNine();
  return true;
}

void SceneBooth::leaveScene()
{
  if(!getScene())
    return;

  getScene()->delEntryAtPosI(this);
  delMeToNine();
}

void SceneBooth::timer(QWORD curMSec)
{

}

void SceneBooth::update(const std::string& name)
{
  if(!getScene())
    return;

  m_oCmd.mutable_info()->set_name(name);

  // 同步九屏
  Cmd::BoothInfoSyncUserCmd cmd;
  cmd.set_charid(m_oCmd.guid());
  cmd.set_oper(EBOOTHOPER_UPDATE);
  cmd.mutable_info()->CopyFrom(m_oCmd.info());

  PROTOBUF(cmd, send, len);
  getScene()->sendCmdToNine(getPos(), send, len);
}

void SceneBooth::fillData(Cmd::MapUser* data)
{
  if(!data)
    return;
  data->CopyFrom(m_oCmd);
}

