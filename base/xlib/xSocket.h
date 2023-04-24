#pragma once
#include "xDefine.h"
#include "xByteBuffer.h"
#include "xCmdQueue.h"
#include "xLog.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "xlib/xMutex.h"

#define SOCKET int
#define INVALID_SOCKET -1

class xNetProcessor;

inline void SAFE_CLOSE_SOCKET(SOCKET &fd)
{
  ::shutdown(fd, SHUT_RDWR);
  ::close(fd);

  fd = INVALID_SOCKET;
}

class xSocket
{
  public:
    xSocket(xNetProcessor *n);
    ~xSocket();

  public:
    void setName();
    SOCKET getSockFD() const { return m_nSockFD; }

    bool valid() { return INVALID_SOCKET != m_nSockFD; }
    bool connect(const char *ip, INT port);
    bool accept(SOCKET sockfd, const sockaddr_in &addr);
    void close();

    bool setNonBlock();
    bool setSockOpt();
    void setComp(bool flag) { m_blCompFlag = flag; }
    void setEnc(bool flag) { m_blEncFlag = flag; }

    bool getPlatCmd(xCmdQueue &q);
    bool getFluentCmd(xCmdQueue &queue);
    bool getCmd(xCmdQueue &q);

    //数据放进缓冲区，等待发送
    bool sendCmd(const void *data, WORD len);
    bool sendNoDataCmd();
    int sendData();

    int recvData();
    bool writeToBuf(void *data, DWORD len);

  protected:
    WORD sizeMod8(WORD len);//resize by 8 bytes, for encrypt

    void compressAll();
    WORD compress(void *data, QWORD len);
    WORD uncompress(void *dest, QWORD destLen, void *src, QWORD srcLen);

    void encrypt(void *data, WORD len);
    void decrypt(void *data, WORD len);
    bool needEnc() { return m_blEncFlag; }
    bool needDec() { return m_blEncFlag; }

    SOCKET m_nSockFD;
    sockaddr_in m_oSockAddr;

  public:
    in_addr& getIP()
    {
      return m_oSockAddr.sin_addr;
    }
    WORD getPort()
    {
      return m_oSockAddr.sin_port;
    }

  private:
    DWORD getSize(BYTE *pBuf, BYTE delim, DWORD len);
  private:
    // 发送缓存
    // 实际发送的数据
    // sendData
    xByteBuffer m_oSendBuffer;
    // 发送缓存
    // sendData
    xByteBuffer m_oTempSendBuffer;
    // 需要发送的消息
    // sendCmd sendData
    xByteBuffer m_oSendCmdBuffer;
    // 临时缓存
    // sendData
    xByteBuffer m_oTempSendCmdBuffer;
    // 发送锁
    xRWLock m_oSendLock;

    // 接收缓存
    // 实际接收到的数据
    // xNetProcesser::recvData
    xByteBuffer m_oRecvBuffer;
    // 解密解压缓存
    // xNetProcesser::recvData
    // sendData
    xByteBuffer m_oBuffer;
    // 接收到的消息长度
    DWORD m_dwRecvSize;


    bool m_blEncFlag;   //加密、解密标志
    bool m_blCompFlag;  //压缩标志

    xNetProcessor *m_pTask;
};
