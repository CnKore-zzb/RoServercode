#include "ProxyUserDataThread.h"
#include "xTime.h"
#include "xServer.h"
#include "xLog.h"
#include <unistd.h>
#include "ProxyUser.h"
#include "ZoneList.h"
#include "xServer.h"
#include "base/CommonConfig.h"
#include "GCharManager.h"

ProxyUserData::ProxyUserData(QWORD accid, DWORD regionid, ProxyUserDataAction act, ProxyUser *user) : m_qwAccID(accid), m_dwRegionID(regionid), m_oAction(act), m_pUser(user)
{
}

ProxyUserData::~ProxyUserData()
{
}

bool ProxyUserData::getSnapShot()
{
  m_blRet = false;
  if (!m_qwAccID || !m_dwRegionID)
  {
    XERR_T("[请求快照],%llu,RegionID:%u", m_qwAccID, m_dwRegionID);
    return false;
  }
  XLOG_T("[请求快照],%llu,RegionID:%u", m_qwAccID, m_dwRegionID);

  bzero(&m_oAccBaseData, sizeof(m_oAccBaseData));
  bzero(m_oSnapShotDatas, sizeof(m_oSnapShotDatas));

  const char *dbname = ProxyUserDataThread::getMe().getRegionDBName(m_dwRegionID);

  {
    xField *field = ProxyUserDataThread::getMe().getDBConnPool().getField(dbname, "accbase");   // 快照  读
    if (field)
    {
      field->setValid("accid,charid,fourthchar,fifthchar,lastselect,deletecharid,deletetime,lastdeletetime,deletecount,deletecountdays,pwdresettime,nologintime,password,maincharid");
      char where[128];
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "accid = %llu", m_qwAccID);
      xRecordSet set;
      QWORD ret = ProxyUserDataThread::getMe().getDBConnPool().exeSelect(field, set, (const char *)where);
      if (QWORD_MAX == ret)
      {
        XERR_T("[请求快照],%llu,RegionID:%u,查询数据失败:%s,accbase", m_qwAccID, m_dwRegionID, dbname);
      }
      else if (ret)
      {
        m_oAccBaseData.m_qwAccID = set[0].get<QWORD>("accid");
        m_oAccBaseData.m_qwCharID = set[0].get<QWORD>("charid");
        m_oAccBaseData.m_dwFourthChar = set[0].get<DWORD>("fourthchar");
        m_oAccBaseData.m_dwFifthChar = set[0].get<DWORD>("fifthchar");
        m_oAccBaseData.m_qwLastSelect = set[0].get<DWORD>("lastselect");
        m_oAccBaseData.m_qwDeleteCharID = set[0].get<QWORD>("deletecharid");
        m_oAccBaseData.m_dwDeleteTime = set[0].get<DWORD>("deletetime");
        m_oAccBaseData.m_dwLastDeleteTime = set[0].get<DWORD>("lastdeletetime");
        m_oAccBaseData.m_dwDeleteCount = set[0].get<DWORD>("deletecount");
        m_oAccBaseData.m_dwDeleteCountDays = set[0].get<DWORD>("deletecountdays");
        m_oAccBaseData.m_dwPwdResetTime = set[0].get<DWORD>("pwdresettime");
        m_oAccBaseData.m_dwNoLoginTime = set[0].get<DWORD>("nologintime");
        strncpy(m_oAccBaseData.m_strPassword, set[0].getString("password"), LEN_256);
        m_oAccBaseData.m_qwMainCharId = set[0].get<QWORD>("maincharid");
        XLOG_T("[请求快照],%llu,%llu,帐号数据:%u,%u,%llu", m_qwAccID, m_oAccBaseData.m_qwCharID, m_oAccBaseData.m_dwFourthChar, m_oAccBaseData.m_dwFifthChar, m_oAccBaseData.m_qwLastSelect);
        XLOG_T("[请求快照],%llu,%llu,角色删除数据:%u,%u,%u,%u", m_qwAccID, m_oAccBaseData.m_qwDeleteCharID, m_oAccBaseData.m_dwDeleteTime, m_oAccBaseData.m_dwLastDeleteTime, m_oAccBaseData.m_dwDeleteCount, m_oAccBaseData.m_dwDeleteCountDays);
      }
    }
    else
    {
      XERR_T("[请求快照],%llu,RegionID:%u,查询数据失败,找不到field:%s,accbase", m_qwAccID, m_dwRegionID, dbname);
    }
  }

  {
    xField *field = ProxyUserDataThread::getMe().getDBConnPool().getField(dbname, "charbase");  // 快照  读
    if (field)
    {
      field->setValid("charid,sequence,name,zoneid,rolelv,hair,haircolor,lefthand,righthand,body,head,back,face,tail,mount,title,eye,partnerid,portrait,mouth,nologintime,gender,profession,clothcolor,maxpro");

      char where[128];
      bzero(where, sizeof(where));
      snprintf(where, sizeof(where), "accid = %llu", m_qwAccID);
      xRecordSet set;
      QWORD ret = ProxyUserDataThread::getMe().getDBConnPool().exeSelect(field, set, (const char *)where);
      if (QWORD_MAX != ret)
      {
        DWORD dwMaxTitle = 0;
        DWORD dwMaxPro = 0;
        for (DWORD i=0; i<ret; ++i)
        {
          DWORD idx = set[i].get<DWORD>("sequence");
          if (idx && idx <= MAX_CHAR_NUM)
          {
            SnapShotData& rData = m_oSnapShotDatas[idx - 1];
            rData.id = set[i].get<QWORD>("charid");
            strncpy(rData.name, set[i].getString("name"), MAX_NAMESIZE);
            rData.m_dwSequence = idx;
            rData.zoneid = set[i].get<DWORD>("zoneid");
            rData.originalzoneid = set[i].get<DWORD>("originalzoneid");
            rData.m_dwNoLoginTime = set[i].get<DWORD>("nologintime");
            rData.level = set[i].get<DWORD>("rolelv");
            rData.male = set[i].get<DWORD>("gender");
            rData.type = set[i].get<DWORD>("profession");
            rData.hair = set[i].get<DWORD>("hair");
            rData.haircolor = set[i].get<DWORD>("haircolor");
            rData.lefthand = set[i].get<DWORD>("lefthand");
            rData.righthand = set[i].get<DWORD>("righthand");
            rData.body = set[i].get<DWORD>("body");
            rData.head = set[i].get<DWORD>("head");
            rData.back = set[i].get<DWORD>("back");
            rData.face = set[i].get<DWORD>("face");
            rData.tail = set[i].get<DWORD>("tail");
            rData.mount = set[i].get<DWORD>("mount");
            rData.title = set[i].get<DWORD>("title");
            rData.eye = set[i].get<DWORD>("eye");
            rData.partnerid = set[i].get<DWORD>("partnerid");
            rData.portrait = set[i].get<DWORD>("portrait");
            rData.clothcolor = set[i].get<DWORD>("clothcolor");
            rData.mouth = set[i].get<DWORD>("mouth");
            DWORD max_pro = set[i].get<DWORD>("maxpro");
            if (rData.title > dwMaxTitle)
              dwMaxTitle = rData.title;
            if(!max_pro)
            {
              if(rData.type % 10 > dwMaxPro)
                dwMaxPro = rData.type % 10;
            }
            else
            {
              if ((max_pro % 10) > dwMaxPro)
                dwMaxPro = (max_pro % 10);
            }

            XDBG_T("[快照数据],%llu,%u,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
                rData.id, rData.zoneid, rData.name, rData.m_dwNoLoginTime, rData.level, rData.male, rData.type, rData.hair, rData.haircolor, rData.partnerid, rData.portrait, rData.clothcolor, max_pro);
          }
        }
        bool isDelete = false;
        for (DWORD i = 0; i < MAX_CHAR_NUM; ++i)
        {
          SnapShotData& rData = m_oSnapShotDatas[i];
          if (rData.id)
          {
            if (m_oAccBaseData.m_qwDeleteCharID == rData.id)
            {
              isDelete = true;
            }
            continue;
          }

          if (i < BASE_CHAR_NUM)
          {
            rData.m_dwOpen = 1;
          }
          /*
          else if (i == 4)
          {
            if (dwMaxTitle >= 1008)
              rData.m_dwOpen = 1;// m_oAccBaseData.m_dwFourthChar;
          }
          else if (i == 3)
          {
            rData.m_dwOpen = m_oAccBaseData.m_dwFifthChar;
          }
          */
          else if (i == 2)
          {
            rData.m_dwOpen = dwMaxPro >= 3;
          }
          XDBG_T("[快照数据],%u,%llu,%s,%u", i + 1, rData.id, rData.name, rData.m_dwOpen);
        }
        if (false == isDelete && m_oAccBaseData.m_qwDeleteCharID)
        {
          xField *field = ProxyUserDataThread::getMe().getDBConnPool().getField(dbname, "accbase");  // 删号  写
          if (field)
          {
            xRecord record(field);
            char where[256] = {0};
            snprintf(where, sizeof(where), "accid=%llu and deletecharid=%llu", m_qwAccID, m_oAccBaseData.m_qwDeleteCharID);
            record.put("deletecharid", 0);
            record.put("deletetime", 0);
            QWORD ret = ProxyUserDataThread::getMe().getDBConnPool().exeUpdate(record, where);
            if (QWORD_MAX == ret)
            {
            }
            else if (ret)
            {
              m_oAccBaseData.m_qwDeleteCharID = 0;
              m_oAccBaseData.m_dwDeleteTime = 0;
            }
          }
        }
        m_blRet = true;
        return true;
      }
      else
      {
        XERR_T("[请求快照],%llu,%u,查询数据失败", m_qwAccID, m_dwRegionID);
      }
    }
    else
    {
      XERR_T("[请求快照],%llu,RegionID:%u,查询数据失败,找不到field:%s,charbase", m_qwAccID, m_dwRegionID, dbname);
    }
  }
  return false;
}

void ProxyUserData::setDeleteChar()
{
  if (!getSnapShot()) return;

  DWORD cur = now();
  for (DWORD i = 0; i < MAX_CHAR_NUM; ++i)
  {
    if (m_qwDeleteCharID == m_oSnapShotDatas[i].id)
    {
      if (m_oSnapShotDatas[i].m_dwNoLoginTime && m_oSnapShotDatas[i].m_dwNoLoginTime > cur)
      {
        m_oDeleteRet = ProxyUserDeleteRet_LOCKED;
        // notifyError(REG_ERR_DELETE_ERROR_LOCKED);
        XERR_T("[删角],封号的角色不可被删除 charid:%llu,%u", m_qwDeleteCharID, m_oSnapShotDatas[i].m_dwNoLoginTime);
        return;
      }
      break;
    }
  }

  if (m_oAccBaseData.m_qwDeleteCharID && m_oAccBaseData.m_dwDeleteTime > cur)
  {
    for (DWORD i=0; i<MAX_CHAR_NUM; ++i)
    {
      if (m_oAccBaseData.m_qwDeleteCharID == m_oSnapShotDatas[i].id)
      {
        m_oDeleteRet = ProxyUserDeleteRet_ERROR;
        // notifyError(REG_ERR_DELETE_ERROR);
        return;
      }
    }
  }

  if (m_oAccBaseData.m_dwDeleteCount >= 5 && m_oAccBaseData.m_dwDeleteCountDays==getDays(cur))
  {
    if (m_oAccBaseData.m_dwLastDeleteTime)
    {
      if (m_oAccBaseData.m_dwLastDeleteTime + 24 * 60 * 60 > cur)
      {
        m_oDeleteRet = ProxyUserDeleteRet_MSG_1067;
        /*
        SysMsg cmd;
        cmd.set_id(1067);
        cmd.set_type(EMESSAGETYPE_FRAME);
        cmd.set_act(EMESSAGEACT_ADD);
        cmd.set_delay(0);

        PROTOBUF(cmd, send, len);
        forwardClientTask(send, len);
        */
        return;
      }
      else
      {
        m_oAccBaseData.m_dwDeleteCount = 0;
      }
    }
  }

  if (m_oAccBaseData.m_dwLastDeleteTime + CommonConfig::m_dwDelCharCD > cur)
  {
    m_oDeleteRet = ProxyUserDeleteRet_ERROR;
    // notifyError(REG_ERR_DELETE_ERROR);
    return;
  }

  for (DWORD i=0; i<MAX_CHAR_NUM; ++i)
  {
    SnapShotData& rData = m_oSnapShotDatas[i];
    if (rData.id == m_qwDeleteCharID)
    {
      const char *dbname = ProxyUserDataThread::getMe().getRegionDBName(m_dwRegionID);
      xField *field = ProxyUserDataThread::getMe().getDBConnPool().getField(dbname, "accbase"); // 删号 写
      if (field)
      {
        DWORD dwDeleteTime = cur;
        DWORD add = CommonConfig::getDelCharTime(rData.level);
        dwDeleteTime = cur + add * 60;
        xRecord record(field);
        char where[256] = {0};
        snprintf(where, sizeof(where), "accid=%llu and deletecharid=0", m_qwAccID);
        record.put("deletecharid", m_qwDeleteCharID);
        record.put("deletetime", dwDeleteTime);
        record.put("lastdeletetime", cur);
        QWORD ret = ProxyUserDataThread::getMe().getDBConnPool().exeUpdate(record, where);
        if (QWORD_MAX == ret)
        {
          XERR_T("[删除角色],%llu,设置失败", m_qwDeleteCharID);
        }
        else if (ret)
        {
          m_oAccBaseData.m_qwDeleteCharID = m_qwDeleteCharID;
          m_oAccBaseData.m_dwDeleteTime = dwDeleteTime;
          m_oAccBaseData.m_dwLastDeleteTime = cur;

          m_oDeleteRet = ProxyUserDeleteRet_OK;
          /*
          kickChar(m_oAccBaseData.m_qwDeleteCharID);
          if (add)
          {
            sendSnapShotToMe();
          }
          */
          XLOG_T("[删除角色],%llu,%llu,%s,设置成功", m_qwAccID, rData.id, rData.name);
        }
      }

      return;
    }
  }
  m_oDeleteRet = ProxyUserDeleteRet_ERROR;
  // notifyError(REG_ERR_DELETE_ERROR);
}

void ProxyUserData::cancelDeleteChar()
{
  m_blCancelDeleteRet = false;
  if (!getSnapShot()) return;

  const char *dbname = ProxyUserDataThread::getMe().getRegionDBName(m_dwRegionID);
  xField *field = ProxyUserDataThread::getMe().getDBConnPool().getField(dbname, "accbase");   // 取消删号 写
  if (field)
  {
    xRecord record(field);
    char where[256] = {0};
    snprintf(where, sizeof(where), "accid=%llu and deletecharid=%llu", m_qwAccID, m_oAccBaseData.m_qwDeleteCharID);
    record.put("deletecharid", 0);
    record.put("deletetime", 0);
    record.put("lastdeletetime", 0);
    QWORD ret = ProxyUserDataThread::getMe().getDBConnPool().exeUpdate(record, where);
    if (QWORD_MAX == ret)
    {
    }
    else if (ret)
    {
      m_oAccBaseData.m_qwDeleteCharID = 0;
      m_oAccBaseData.m_dwDeleteTime = 0;
      m_oAccBaseData.m_dwLastDeleteTime = 0;
  //    sendSnapShotToMe();
      XLOG_T("[删除角色],%llu,%llu,取消设置成功", m_qwAccID, m_qwDeleteCharID);
      m_blCancelDeleteRet = true;
    }
  }
}

/************************************************************************************/
/************************************************************************************/
/************************************************************************************/
/************************************************************************************/

ProxyUserDataThread::ProxyUserDataThread()
{
}

ProxyUserDataThread::~ProxyUserDataThread()
{
}

void ProxyUserDataThread::thread_stop()
{
  final();

  xThread::thread_stop();
}

void ProxyUserDataThread::final()
{
}

bool ProxyUserDataThread::thread_init()
{
  addDataBase(RO_DATABASE_NAME);

  return true;
}

void ProxyUserDataThread::addDataBase(std::string dbname)
{
  xServer::initDBConnPool("DataBase", dbname, m_oDBConnPool);
}

void ProxyUserDataThread::thread_proc()
{
  static QWORD MIN_RECURSIVE_TIME = 30 * 1000;
  static QWORD MAX_RECURSIVE_TIME = 100 * 1000;

  thread_setState(THREAD_RUN);

  while (thread_getState()==xThread::THREAD_RUN)
  {
    xTime frameTimer;

    // todo
    check();

    QWORD _e = frameTimer.uElapse();
    if (_e < MIN_RECURSIVE_TIME)
    {
      usleep(MIN_RECURSIVE_TIME - _e);
    }
    else if (_e > MAX_RECURSIVE_TIME)
    {
      XLOG_T("[ProxyUserDataThread] 帧耗时 %llu 微秒", _e);
    }
  }

  check();
  XLOG_T("[ProxyUserDataThread] final check");
}

void ProxyUserDataThread::add(ProxyUserData *pData)
{
  if (!pData) return;

  m_oPrepareQueue.put(pData);

#ifdef _LX_DEBUG
  XLOG_T("[ProxyUserDataThread-Push],队列:%u", m_oPrepareQueue.size());
#endif
}

void ProxyUserDataThread::check()
{
  ProxyUserData *data = m_oPrepareQueue.get();
  while (data)
  {
    m_oPrepareQueue.pop();
    exec(data);

    m_oFinishQueue.put(data);
    data = m_oPrepareQueue.get();

    XLOG_T("[ProxyUserDataThread] 完成队列: %u, 等待队列: %u", m_oFinishQueue.size(), m_oPrepareQueue.size());
  }
}

void ProxyUserDataThread::exec(ProxyUserData *pData)
{
  if (!pData) return;

  switch (pData->m_oAction)
  {
    case ProxyUserDataAction_SEND:
    case ProxyUserDataAction_DELETE_CHAR:
    case ProxyUserDataAction_LOGIN:
      {
        if (pData->getSnapShot())
        {
          pData->m_blRet = true;
        }
      }
      break;
    case ProxyUserDataAction_SET_DELETE:
      {
        pData->setDeleteChar();
      }
      break;
    case ProxyUserDataAction_CANCEL_DELETE:
      {
        pData->cancelDeleteChar();
      }
      break;
    default:
      break;
  }

  // todo
}

ProxyUserData* ProxyUserDataThread::get()
{
  return m_oFinishQueue.get();
}

void ProxyUserDataThread::pop()
{
  return m_oFinishQueue.pop();
}

const char* ProxyUserDataThread::getRegionDBName(DWORD regionid)
{
  auto it = m_mapRegionDB.find(regionid);
  if (it != m_mapRegionDB.end()) return it->second.c_str();

  xField *field = getDBConnPool().getField(RO_DATABASE_NAME, "region"); // 加载一次
  if (field)
  {
    field->setValid("regionname,platid");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "regionid = %u", regionid);

    xRecordSet set;
    QWORD ret = getDBConnPool().exeSelect(field, set, (const char *)where);
    if (QWORD_MAX!=ret && ret)
    {
      std::stringstream stream;
      stream.str("");
      std::string platname = getPlatName(set[0].get<DWORD>("platid"));
      stream << "ro_" << platname << "_" << set[0].getString("regionname");
      m_mapRegionDB[regionid] = stream.str();
      addDataBase(stream.str());
      XLOG_T("[ProxyUserDataThread],大服数据库添加,%u,%s", regionid, stream.str().c_str());
      return m_mapRegionDB[regionid].c_str();
    }
  }

  return "";
}

std::string ProxyUserDataThread::getPlatName(DWORD platid)
{
  auto it = m_mapPlatIDName.find(platid);
  if (it != m_mapPlatIDName.end())
  {
    return it->second;
  }
  xField *field = getDBConnPool().getField(RO_DATABASE_NAME, "platform"); // 加载一次
  if (field)
  {
    field->setValid("platname");
    char where[128];
    bzero(where, sizeof(where));
    snprintf(where, sizeof(where), "platid = %u", platid);

    xRecordSet set;
    QWORD ret = getDBConnPool().exeSelect(field, set, (const char *)where);
    if (QWORD_MAX!=ret && ret)
    {
      XDBG_T("[大服数据库],平台:%u,%s", platid, set[0].getString("platname"));
      m_mapPlatIDName[platid] = set[0].getString("platname");
      return set[0].getString("platname");
    }
  }

  return "";
}
