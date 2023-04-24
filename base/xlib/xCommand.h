#ifndef BASE_XLIB_XCOMMAND_H_
#define BASE_XLIB_XCOMMAND_H_

#include "xCmd.pb.h"

#define MAX_BUFSIZE 65535
#define TRANS_BUFSIZE 32768
#define PH_LEN 3
#define MAX_PACKSIZE (MAX_BUFSIZE-PH_LEN)
#define MAX_CMD_SET_PACKSIZE 30000
#define MIN_PACKSIZE 5  // for encrypt
//#define MAX_CMD 255
//#define MAX_USER_CMD (255-155)
// 服务器用的pb消息号从100-200
//#define MAX_PROTO_CMD (255-55)

//#define SYSTEM_CMD (MAX_CMD)
//#define SUPER_CMD (MAX_CMD-1)
//#define REG_CMD (MAX_CMD-2)
//#define SESSION_CMD (MAX_CMD-3)
//#define KUAQU_CMD (MAX_CMD-4)
//#define GATEWAY_CMD (MAX_CMD-5)
//#define RECORD_CMD 200
//#define SCENE_SESSION_CMD (MAX_CMD-7)

#define BUFFER_CMD(send, type) \
  char send##buf_3g[MAX_BUFSIZE];\
bzero(send##buf_3g, sizeof(send##buf_3g));\
type *send = (type *)send##buf_3g;\
constructInPlace<type>(send)

#define BUFFER_CMD_SIZE(send, type, size) \
  char send##buf_3g[size];\
bzero(send##buf_3g, sizeof(send##buf_3g));\
type *send = (type *)send##buf_3g;\
constructInPlace<type>(send)

#define PROTOBUF(message, send, len) \
BUFFER_CMD(send, xCommand);\
if (message.ByteSize() >= 65535) { XLOG << "[PROTOBUF],序列化失败" << message.cmd() << "" << message.param() << "len" << message.ByteSize() << XEND; } \
WORD len = message.ByteSize(); {\
if (message.SerializeToArray(send->probuf, len)) {\
  send->cmd = message.cmd();\
  send->param = message.param();\
  len += sizeof(xCommand); }\
  else { XLOG << "[PROTOBUF],序列化失败," << message.cmd() << "," << message.param() << ",len" << message.ByteSize() << XEND; } \
}

/*
    std::stringstream ss; ss.str("");\
    ss << "cmd:" << message.cmd() << "param:" << message.param() << "size:" << message.ByteSize() << "序列化失败";\
    ro_error("", ss.str());}\
    */

#define PARSE_CMD_PROTOBUF(type, message) \
  type message;{\
if (len<2 || !message.ParseFromArray((((xCommand *)buf)->probuf), len-2))\
return false;}

using namespace Cmd;
#pragma pack(1)

// 包头标志
enum PACKET_FLAG_ENUM_TYPE
{
  PACKET_FLAG_COMPRESS = 1,  // 压缩
  PACKET_FLAG_ENCRYPT = 2,  // 加密
};

struct PlatPacketHead
{
  BYTE flags;
  WORD len; // 包含cmd的长度
  BYTE cmd;
  PlatPacketHead() { flags = len = cmd = 0; }
};

struct PlatPacket
{
  PlatPacketHead ph;
  BYTE data[0];
  WORD getDataSize() { return ph.len-sizeof(BYTE); }
  WORD getFullSize() { return PH_LEN+ph.len;}
};

struct PacketHead
{
  BYTE flags;
  WORD len;
  PacketHead()
  {
    flags = len = 0;
  }
};

struct Packet
{
  PacketHead ph;
  BYTE data[0];

  WORD getDataSize() { return ph.len; }
  WORD getFullSize() { return PH_LEN+ph.len; }
};

class xCommand
{
  public:
    BYTE cmd;
    BYTE param;
    BYTE probuf[0];

    xCommand(BYTE c = 0, BYTE p = 0):cmd(c), param(p) {}
    bool is(BYTE c) { return cmd == c; }
    bool isUserCmd() { return cmd <= Cmd::MAX_USER_CMD; }

    bool operator==(const xCommand &item) const
    {
      return (cmd == item.cmd) && (param == item.param);
    }
    bool operator<(const xCommand &item) const
    {
      if (cmd == item.cmd) return param > item.param;
      return (cmd > item.cmd);
    }
};

#pragma pack()

#endif  // BASE_XLIB_XCOMMAND_H_
