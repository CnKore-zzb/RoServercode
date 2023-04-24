#include "UserHandNpc.h"
#include "SceneUser.h"
#include "MiscConfig.h"
#include "GuidManager.h"
#include "MsgManager.h"

void SHandNpcData::toData(HandNpcData* pData)
{
  if (pData == nullptr)
    return;
  pData->set_body(dwBody);
  pData->set_head(dwHead);
  pData->set_hair(dwHair);
  pData->set_haircolor(dwHairColor);
  pData->set_guid(qwGuid);
  pData->set_speffect(dwSpEffectID);
  pData->set_name(strName);
  pData->set_eye(dwEye);
}

UserHandNpc::UserHandNpc(SceneUser* user):m_pUser(user)
{

}

UserHandNpc::~UserHandNpc()
{

}

void UserHandNpc::load(const BlobHandNpc& data)
{
  if (!data.has_data() || !data.has_endtime())
    return;
  DWORD cur = now();
  if (cur >= data.endtime())
    return;

  m_stHandData.dwBody = data.data().body();
  m_stHandData.dwHead = data.data().head();
  m_stHandData.dwHair = data.data().hair();
  m_stHandData.dwHairColor = data.data().haircolor();
  m_stHandData.dwSpEffectID = data.data().speffect();
  m_stHandData.strName = data.data().name();
  m_stHandData.dwEye = data.data().eye();

  QWORD qwGUID = GuidManager::getMe().getNextNpcID();
  m_stHandData.qwGuid = qwGUID;

  m_dwEndTime = data.endtime();
}

void UserHandNpc::save(BlobHandNpc* pData)
{
  if (!haveHandNpc())
    return;

  m_stHandData.toData(pData->mutable_data());
  pData->set_endtime(m_dwEndTime);
}

bool UserHandNpc::checkAdd()
{
  if (m_pUser->getUserSceneData().getBodyScale() != 100)
    return false;

  if (haveHandNpc())
  {
    MsgManager::sendMsg(m_pUser->id, 880);
    return false;
  }

  return true;
}

bool UserHandNpc::addHandNpc()
{
  if (!checkAdd())
    return false;

  if (m_pUser->m_oHands.has())
  {
    m_pUser->m_oHands.breakup();
  }
  if (m_pUser->getWeaponPet().haveHandCat())
  {
    m_pUser->getWeaponPet().breakHand();
  }
  if (m_pUser->getUserPet().handPet())
    m_pUser->getUserPet().breakHand();

  const SHandNpcCFG& rCFG = MiscConfig::getMe().getHandNpcCFG();

  auto getData = [&](DWORD& value, const TVecDWORD& svec) -> bool
  {
    DWORD size = svec.size();
    if (size == 0)
      return false;
    DWORD index = randBetween(0, size - 1);
    value = svec[index];
    return true;
  };

  bool bCorrcet = true;

  if (getData(m_stHandData.dwBody, rCFG.vecBody) == false)
    bCorrcet = false;
  if (getData(m_stHandData.dwHead, rCFG.vecHead) == false)
    bCorrcet = false;
  if (getData(m_stHandData.dwHair, rCFG.vecHair) == false)
    bCorrcet = false;
  if (getData(m_stHandData.dwHairColor, rCFG.vecHairColor) == false)
    bCorrcet = false;
  if (getData(m_stHandData.dwEye, rCFG.vecEye) == false)
    bCorrcet = false;

  m_stHandData.strName = "艾娃"; // 临时处理, 后续优化

  if (!bCorrcet)
  {
    XERR << "[HandNpc], 配置错误, 无法生成npc 形象" << XEND;
    return false;
  }

  QWORD qwGUID = GuidManager::getMe().getNextNpcID();
  if (qwGUID == 0)
  {
    XERR << "[HandNpc], 生成guid失败" << XEND;
    return false;
  }

  m_stHandData.qwGuid = qwGUID;

  m_dwEndTime = now() + rCFG.dwContinueTime;

  UserHandNpcCmd cmd;
  m_stHandData.toData(cmd.mutable_data());
  cmd.set_ishand(true);
  cmd.set_userid(m_pUser->id);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  playEmoji(rCFG.getRandomOne(rCFG.stBirthEmoji));
  playDialog(rCFG.getRandomOne(rCFG.stBirthDialog));

  XLOG << "[HandNpc], 玩家" << m_pUser->name << ", " << m_pUser->id << "添加艾娃" << XEND;
  return true;
}

bool UserHandNpc::addHandNpc(const SHandNpcData& data, DWORD time)
{
  if (!checkAdd())
    return false;

  m_stHandData.copyFrom(data);

  QWORD qwGUID = GuidManager::getMe().getNextNpcID();
  if (qwGUID == 0)
  {
    XERR << "[HandNpc], 生成guid失败" << XEND;
    return false;
  }

  m_stHandData.qwGuid = qwGUID;
  m_dwEndTime = time + now();

  UserHandNpcCmd cmd;
  m_stHandData.toData(cmd.mutable_data());
  cmd.set_ishand(true);
  cmd.set_userid(m_pUser->id);

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  XLOG << "[HandNpc], 玩家" << m_pUser->name << ", " << m_pUser->id << "添加兔子灯" << XEND;
  return true;
}


void UserHandNpc::delHandNpc()
{
  if (!haveHandNpc())
    return;
  const SHandNpcCFG& rCFG = MiscConfig::getMe().getHandNpcCFG();
  playEmoji(rCFG.getRandomOne(rCFG.stDispEmoji));
  playDialog(rCFG.getRandomOne(rCFG.stDispDialog));

  UserHandNpcCmd cmd;
  cmd.set_ishand(false);
  cmd.set_userid(m_pUser->id);
  m_stHandData.toData(cmd.mutable_data());

  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  m_dwEndTime = 0;

  XLOG << "[HandNpc], 玩家" << m_pUser->name << ", " << m_pUser->id << "删除艾娃" << XEND;
}

void UserHandNpc::timer(DWORD dwCurTime)
{
  if (!haveHandNpc())
    return;

  if (dwCurTime >= m_dwEndTime)
  {
    delHandNpc();
    return;
  }
  if (dwCurTime >= m_dwNextPlayEmoji)
  {
    const SHandNpcCFG& rCFG = MiscConfig::getMe().getHandNpcCFG();
    playEmoji(rCFG.getRandomOne(rCFG.stNormalEmoji));
  }
  if (dwCurTime >= m_dwNextPlayDialog)
  {
    const SHandNpcCFG& rCFG = MiscConfig::getMe().getHandNpcCFG();
    playDialog(rCFG.getRandomOne(rCFG.stNormalDialog));
  }
}

void UserHandNpc::getHandNpcData(HandNpcData* pData)
{
  if (!haveHandNpc())
    return;
  m_stHandData.toData(pData);
}

void UserHandNpc::onLeaveScene()
{
  //delHandNpc();
}

void UserHandNpc::playDialog(DWORD id)
{
  if (canTalk() == false)
    return;
  if (!haveHandNpc() || id == 0)
    return;
  TalkInfo cmd;
  cmd.set_guid(m_stHandData.qwGuid);
  cmd.set_talkid(id);
  PROTOBUF(cmd, send, len);
  m_pUser->sendCmdToNine(send, len);

  m_dwNextPlayDialog = MiscConfig::getMe().getHandNpcCFG().dwDialogInterval + now();
}

void UserHandNpc::playEmoji(DWORD id)
{
  if (canTalk())
    return;
  if (!haveHandNpc() || id == 0)
    return;
  UserActionNtf message;
  message.set_type(EUSERACTIONTYPE_EXPRESSION);
  message.set_value(id);
  message.set_charid(m_stHandData.qwGuid);
  PROTOBUF(message, send, len);
  m_pUser->sendCmdToNine(send, len);

  m_dwNextPlayEmoji = MiscConfig::getMe().getHandNpcCFG().dwEmojiInterval + now();
}

void UserHandNpc::onUseSkill()
{
  if (canTalk() == false)
    return;
  if (!haveHandNpc())
    return;
  const SHandNpcCFG& rCFG = MiscConfig::getMe().getHandNpcCFG();
  DWORD cur = now();
  if (cur >= m_dwNextPlayEmoji)
  {
    playEmoji(rCFG.getRandomOne(rCFG.stAttackEmoji));
  }
  if (cur >= m_dwNextPlayDialog)
  {
    playDialog(rCFG.getRandomOne(rCFG.stAttackDialog));
  }
}


