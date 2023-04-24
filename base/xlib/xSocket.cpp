#include "zlib/zlib.h"
extern "C"
{
#include "des/d3des.h"
}

#include "xSocket.h"
#include "xNetProcessor.h"
#include <errno.h>
#include <fcntl.h>
#include "xServer.h"

//压缩等级: 0 ~ 9, 数字越大，压缩率越高，耗时越长
#define COMPRESS_LEVEL 6
#define MIN_COMPRESS_SIZE 48

BYTE fixedkey[8] = { 95,27,5,20,131,4,8,88 };
unsigned long enckey[32] = { 0x04013922, 0x28001C08, 0x2020043B, 0x00113A18, 0x01083E14, 0x12001911, 0x00120218, 0x2100362E, 0x16011F26, 0x00000704, 0x00002803, 0x0C08043F, 0x0004312A, 0x00063B03, 0x08202D1D, 0x00012039, 0x12003D10, 0x01003506, 0x0001130F, 0x080C002E, 0x00002523, 0x04020F0D, 0x0824181B, 0x00202931, 0x20002634, 0x02013A2B, 0x00180B3C, 0x10102510, 0x11021605, 0x01001636, 0x0004312D, 0x00200C35 };
unsigned long deckey[32] = { 0x0004312D, 0x00200C35, 0x11021605, 0x01001636, 0x00180B3C, 0x10102510, 0x20002634, 0x02013A2B, 0x0824181B, 0x00202931, 0x00002523, 0x04020F0D, 0x0001130F, 0x080C002E, 0x12003D10, 0x01003506, 0x08202D1D, 0x00012039, 0x0004312A, 0x00063B03, 0x00002803, 0x0C08043F, 0x16011F26, 0x00000704, 0x00120218, 0x2100362E, 0x01083E14, 0x12001911, 0x2020043B, 0x00113A18, 0x04013922, 0x28001C08 };

xSocket::xSocket(xNetProcessor *n):m_pTask(n)
{
  m_nSockFD = INVALID_SOCKET;

  m_dwRecvSize = 0;
  m_blEncFlag = false;
  m_blCompFlag = false;

  m_oSendBuffer.m_strType = "SendBuffer";
  m_oTempSendBuffer.m_strType = "TempSendBuffer";
  m_oSendCmdBuffer.m_strType = "SendCmdBuffer";
  m_oTempSendCmdBuffer.m_strType = "TempSendCmdBuffer";
  m_oBuffer.m_strType = "Buffer";
  m_oRecvBuffer.m_strType = "RecvBuffer";
}

xSocket::~xSocket()
{
  close();
}

void xSocket::setName()
{
  m_oSendBuffer.m_strName = m_pTask->name;
  m_oTempSendBuffer.m_strName = m_pTask->name;
  m_oSendCmdBuffer.m_strName = m_pTask->name;
  m_oTempSendCmdBuffer.m_strName = m_pTask->name;
  m_oBuffer.m_strName = m_pTask->name;
  m_oRecvBuffer.m_strName = m_pTask->name;
}

bool xSocket::accept(SOCKET sockfd, const sockaddr_in &addr)
{
  if (m_nSockFD == sockfd) return false;

  if (INVALID_SOCKET != m_nSockFD)
  {
    XERR_T("[Socket],accept error,%s,%u,%s:%u", m_pTask->name, m_nSockFD, inet_ntoa(getIP()), ntohs(getPort()));
    return false;
  }
  m_nSockFD = sockfd;
  m_oSockAddr = addr;

  XLOG_T("[Socket],accept,%s,%u,%s:%u", m_pTask->name, m_nSockFD, inet_ntoa(getIP()), ntohs(getPort()));

  setSockOpt();

  return setNonBlock();
}

/*
bool xSocket::connect(const char *ip, INT port)
{
  m_nSockFD = socket(AF_INET, SOCK_STREAM, 0);
  if (m_nSockFD < 0)
  {
    XERR << "[Socket],connect() " << ip << ":" << port << " socket failed " << this << XEND;
    return false;
  }
  bzero(&m_oSockAddr, sizeof(m_oSockAddr));
  m_oSockAddr.sin_family = AF_INET;
  m_oSockAddr.sin_addr.s_addr = inet_addr(ip);
  m_oSockAddr.sin_port = htons(port);

  if (setNonBlock() == false)
  {
    return false;
  }

  int ret = ::connect(m_nSockFD, (sockaddr *)&m_oSockAddr, sizeof(sockaddr_in));
  if (0 != ret)
  {
    if (EINPROGRESS != errno)
    {
      XERR << "[Socket]" << "connect error" << ip << ":" << port << "error:" << strerror(errno) << XEND;
      return false;
    }
    else
    {
      int error = -1;
      timeval tm;
      tm.tv_sec = 0; //TIME_OUT_TIME;
      tm.tv_usec = 100000;
      socklen_t len = sizeof(tm);

      fd_set wset,rset;
      FD_ZERO(&wset);
      FD_ZERO(&rset);
      FD_SET(m_nSockFD, &wset);
      FD_SET(m_nSockFD, &rset);

      int ret = select(m_nSockFD + 1, &rset, &wset, NULL, &tm);
      if (ret > 0)
      {
        getsockopt(m_nSockFD, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
        if (error == 0)
        {
          XLOG << "[Socket]" << "connect" << ip << ":" << port << "fd:" << m_nSockFD << XEND;
          return true;
        }
        else
        {
          XERR << "[Socket]" << "connect error" << ip << ":" << port << "other error:" << strerror(errno) << XEND;
          return false;
        }
      }
      else if (ret == 0)
      {
        XERR << "[Socket]" << "connect error" << ip << ":" << port << "connect time out:" << strerror(errno) << XEND;
        return false;
      }
      else
      {
        XERR << "[Socket]" << "connect error" << ip << ":" << port << "network error:" << strerror(errno) << XEND;
        return false;
      }
    }
    XERR << "[Socket]" << "connect error" << ip << ":" << port << "error:" << strerror(errno) << XEND;
    return false;
  }

  XLOG << "[Socket]" << "connect" << ip << ":" << port << "fd:" << m_nSockFD << XEND;
  return true;
}
*/

bool xSocket::connect(const char *ip, INT port)
{
  if (INVALID_SOCKET != m_nSockFD)
  {
    XERR_T("[Socket],connect error, has m_nSockFD:%u,%p", m_nSockFD, this);
    return false;
  }

  m_nSockFD = socket(AF_INET, SOCK_STREAM, 0);
  if (m_nSockFD < 0)
  {
    XERR_T("[Socket],connect error,%s:%u,socket failed,%p", ip, port, this);
    return false;
  }
  struct timeval timeo = {0, 100000};
  socklen_t len = sizeof(timeo);
  setsockopt(m_nSockFD, SOL_SOCKET, SO_SNDTIMEO, &timeo, len);

  bzero(&m_oSockAddr, sizeof(m_oSockAddr));
  m_oSockAddr.sin_family = AF_INET;
  m_oSockAddr.sin_addr.s_addr = inet_addr(ip);
  m_oSockAddr.sin_port = htons(port);

  int ret = ::connect(m_nSockFD, (sockaddr *)&m_oSockAddr, sizeof(sockaddr_in));
  if (0 != ret)
  {
    if (errno == EINPROGRESS)
    {
      XERR_T("[Socket],connect,%s:%u,time out", ip, port);
      return false;
    }
    XERR_T("[Socket],connect,%s,%u,failed with error:%u,%p", ip, port, ret, this);
    return false;
  }

  setSockOpt();

  XLOG_T("[Socket],connect,%s:%u,%u", ip, port, m_nSockFD);
  return setNonBlock();
}

void xSocket::close()
{
  if (valid())
  {
    SAFE_CLOSE_SOCKET(m_nSockFD);
  }
}

bool xSocket::setNonBlock()
{
  int flags = fcntl(m_nSockFD, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (-1 == fcntl(m_nSockFD, F_SETFL, flags))
  {
    XERR_T("[Socket],setNonBlock failed,%s,%u", m_pTask->name, m_nSockFD);
    return false;
  }
  return true;
}

bool xSocket::setSockOpt()
{
  int nBuf = MAX_BUFSIZE * 2;
  setsockopt(m_nSockFD,SOL_SOCKET,SO_RCVBUF,(const char*)&nBuf,sizeof(int));
  setsockopt(m_nSockFD,SOL_SOCKET,SO_SNDBUF,(const char*)&nBuf,sizeof(int));
  return true;
}

WORD xSocket::sizeMod8(WORD len)
{
  // return len;
  return (len + 7) & ~7;
  return (len + 7) / 8 * 8;
}

int xSocket::recvData()
{
  int final_ret = 0;
  {
    m_oRecvBuffer.wr_reserve(MAX_BUFSIZE);

    int ret = ::recv(m_nSockFD, m_oRecvBuffer.getWriteBuf(), MAX_BUFSIZE, 0);
    if (ret < 0)
    {
      if ((errno!=EAGAIN) && (errno!=EWOULDBLOCK) && (errno!=EINTR))
      {
        XERR_T("[xSocket],fd:%u,接收错误,errno:%s", m_nSockFD, strerror(errno));
        final_ret = -1;
      }
      else
      {
#ifdef _LX_DEBUG
        //XERR("[SOCKET],接收成功");
#endif
      }
    }
    else if (0==ret)//peer shutdown
    {
      XERR_T("[xSocket],fd:%u,接收错误,peer shutdown", m_nSockFD);
      final_ret = -1;
    }
    else
    {
      m_oRecvBuffer.write(ret);
      final_ret = 1;
    }
  }

  return final_ret;
}

int xSocket::sendData()
{
  if (!valid()) return -1;

  compressAll();

  int all = m_oSendBuffer.getReadSize();
  while (all)
  {
    int realsend = std::min(all, MAX_BUFSIZE);
    int ret = ::send(m_nSockFD, m_oSendBuffer.getReadBuf(), realsend, 0);
    if (ret > 0)
    {
      m_oSendBuffer.read(ret);
      if (ret < realsend)
      {
#ifdef _LX_DEBUG
        XLOG << "[SOCKET-send]" << "发送,fd:" << m_nSockFD << "ret:" << ret << "real:" << realsend << XEND;
#endif
        return realsend;
      }
      all = m_oSendBuffer.getReadSize();
    }
    else if (ret==0)
    {
   //   all = m_oSendBuffer.getReadSize();
      XERR_T("[SOCKET],发送异常,fd:%u,ret:%u,real:%u", m_nSockFD, ret, realsend);
      return realsend;
    }
    else
    {
      if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
      {
        XERR_T("[SOCKET],发送异常,fd:%u,ret:%u,real:%u,%s", m_nSockFD, ret, realsend, strerror(errno));
        return realsend;
      }
      else
      {
        XERR_T("[SOCKET],发送错误,fd:%u,ret:%u,real:%u,%s", m_nSockFD, ret, realsend, strerror(errno));
        return -1;
      }
    }
  }
  //m_oSendBuffer.read_flip();

  return all;
}

bool xSocket::sendCmd(const void *data, WORD len)
{
  if (!valid()) return false;

  PacketHead ph;
  ph.len = len;
  ScopeWriteLock swl(m_oSendLock);
  m_oSendCmdBuffer.write(&ph, PH_LEN);
  m_oSendCmdBuffer.write(data, len);
  return true;
}

bool xSocket::sendNoDataCmd()
{
  if (!valid()) return false;

  PacketHead ph;
  ph.len = 0;
  ScopeWriteLock swl(m_oSendLock);
  m_oSendCmdBuffer.write(&ph, PH_LEN);
  return true;
}

bool xSocket::writeToBuf(void *data, DWORD len)
{
  m_oSendBuffer.write(data, len);

  return true;
}

WORD xSocket::compress(void *data, QWORD len)
{
  QWORD newLen = m_oBuffer.size();
  if (Z_OK == ::compress2((Bytef *)m_oBuffer.getBegin(), (uLongf *)&newLen, (Bytef *)data, (uLong)len, COMPRESS_LEVEL))
  {
    memcpy(data, m_oBuffer.getBegin(), newLen);
    return newLen;
  }
  XLOG_T("[xSocket],%s,fd:%u,压缩错误", m_pTask->name, m_nSockFD);
  return 0;
}

WORD xSocket::uncompress(void *dest, QWORD destLen, void *src, QWORD srcLen)
{
  QWORD newLen = destLen;
  int ret = ::uncompress((Bytef *)dest, (uLongf *)&newLen, (Bytef *)src, (uLong)srcLen);
  if (Z_OK == ret)
  {
    return newLen;
  }

  XLOG_T("[xSocket],%s,fd:%u,解压失败:%u,%u,%u", m_pTask->name, m_nSockFD, ret, srcLen, destLen);
  return 0;
}

void xSocket::encrypt(void *data, WORD len)
{
  // deskey(fixedkey, EN0);
  usekey(enckey);
  for (WORD i = 0; i < len; i += 8)
  {
    des((BYTE *)data + i, (BYTE *)m_oBuffer.getBegin(i));
  }
  memcpy(data, m_oBuffer.getBegin(), len);
}

void xSocket::decrypt(void *data, WORD len)
{
  // deskey(fixedkey, DE1);
  usekey(deckey);
  for (WORD i = 0; i < len; i += 8)
  {
    des((BYTE *)data + i, (BYTE *)m_oBuffer.getBegin(i));
  }
  memcpy(data, m_oBuffer.getBegin(), len);
}

// 平台数据接收
bool xSocket::getPlatCmd(xCmdQueue &queue)
{
  DWORD all = m_oRecvBuffer.getReadSize();
  while(all >= MIN_PACKSIZE)
  {
    PlatPacket *p = (PlatPacket*)m_oRecvBuffer.getReadBuf();
    if(nullptr == p || 1 != p->ph.flags) return false;

    m_dwRecvSize = p->getFullSize();
    memcpy(m_oBuffer.getBegin(), p, m_dwRecvSize);
    queue.put((BYTE*)m_oBuffer.getBegin(), m_dwRecvSize);
    m_oRecvBuffer.read(m_dwRecvSize);
    all = m_oRecvBuffer.getReadSize();
  }

  return true;
}

//fluent ack: {"ack":"_1_4"}
bool xSocket::getFluentCmd(xCmdQueue &queue)
{
  DWORD all = m_oRecvBuffer.getReadSize();
  while (all >= MIN_PACKSIZE)
  {
    BYTE* p = m_oRecvBuffer.getReadBuf(); 
    DWORD size = getSize(p, '}', all);
    if (size == 0)
    {
      return false;
    }    
    m_dwRecvSize = size;

    memcpy(m_oBuffer.getBegin(), p, m_dwRecvSize);
    queue.put((BYTE*)m_oBuffer.getBegin(), m_dwRecvSize);
    m_oRecvBuffer.read(m_dwRecvSize);
    // XDBG << "[fluent-日志] 收到fluent返回，大小" <<m_dwRecvSize << XEND;
    all = m_oRecvBuffer.getReadSize();
  }
  return true;
}

DWORD xSocket::getSize(BYTE *pBuf, BYTE delim, DWORD len)
{
  if (!pBuf)
    return 0;
  for (DWORD i = 0; i < len; i++)
  {
    if (pBuf[i] == delim)
      return (i + 1);
  }
  return 0;
}

bool xSocket::getCmd(xCmdQueue &queue)
{
  DWORD all = m_oRecvBuffer.getReadSize();
  while (all >= MIN_PACKSIZE)
  {
    Packet *p = (Packet *)m_oRecvBuffer.getReadBuf();
    WORD wCmdSize = p->ph.len;

    if (p->ph.flags & PACKET_FLAG_ENCRYPT)
    {
      wCmdSize = sizeMod8(wCmdSize);
    }

    m_dwRecvSize = wCmdSize + PH_LEN;

    if (m_dwRecvSize > all)
    {
    //  m_oRecvBuffer.read_flip();
      return false;
    }

    if (p->ph.flags & PACKET_FLAG_ENCRYPT)
    {
      decrypt(p->data, wCmdSize);
    }

    if (p->ph.flags & PACKET_FLAG_COMPRESS)
    {
      WORD len = uncompress(m_oBuffer.getBegin(), m_oBuffer.size(), p->data, p->ph.len);
      if (len > 0)
      {
        queue.put((BYTE *)m_oBuffer.getBegin(), len);
      }
      else
      {
        XLOG_T("[xSocket],%s,fd:%u,解压错误,消息长度:%u", m_pTask->name, m_nSockFD, p->ph.len);
      }
    }
    else
    {
      queue.put(p->data, p->ph.len);
    }
    m_oRecvBuffer.read(m_dwRecvSize);
    all = m_oRecvBuffer.getReadSize();
  }
 // m_oRecvBuffer.read_flip();
  return true;
}

void xSocket::compressAll()
{
  {
    m_oTempSendCmdBuffer.reset();
    ScopeWriteLock swl(m_oSendLock);
    m_oTempSendCmdBuffer.copy(m_oSendCmdBuffer);
    m_oSendCmdBuffer.reset();
  }

  while (m_oTempSendCmdBuffer.getReadSize() > 0)
  {
    PacketHead *pPacketHead = (PacketHead *)m_oTempSendCmdBuffer.getReadBuf();
    m_oTempSendBuffer.reset();
    PacketHead ph;
    m_oTempSendBuffer.write(&ph, PH_LEN);
    m_oTempSendBuffer.write(m_oTempSendCmdBuffer.getReadBuf(PH_LEN), pPacketHead->len);
    PacketHead *p = (PacketHead *)m_oTempSendBuffer.getBegin();

    WORD wDataSize = pPacketHead->len;
    if (m_blCompFlag && pPacketHead->len >= MIN_COMPRESS_SIZE)
    {
      wDataSize = compress(m_oTempSendBuffer.getBegin(PH_LEN), pPacketHead->len);
      if (wDataSize > 0)
      {
        p->flags |= PACKET_FLAG_COMPRESS;
      }
      else
      {
        wDataSize = pPacketHead->len;
        XLOG_T("[压缩-失败],%s,fd:%u,%u", m_pTask->name, m_nSockFD, pPacketHead->len);
      }
    }
    p->len = wDataSize;
    if (needEnc())
    {
      wDataSize = sizeMod8(wDataSize);
      p->flags |= PACKET_FLAG_ENCRYPT;
      encrypt(m_oTempSendBuffer.getBegin(PH_LEN), wDataSize);
    }

    if (!writeToBuf(m_oTempSendBuffer.getBegin(), wDataSize + PH_LEN))
    {
      XLOG_T("[xSocket],%s,fd:%u,push cmd error", m_pTask->name, m_nSockFD);
    }
    m_oTempSendCmdBuffer.read(pPacketHead->len + PH_LEN);
  }
 // m_oTempSendCmdBuffer.read_flip();
}
