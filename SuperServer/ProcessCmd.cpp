#include <algorithm>
#include "SuperServer.h"
#include "RegCmd.h"
#include "xNetProcessor.h"
#include "GateInfo.h"
#include "GateSuper.pb.h"
#include "MsgManager.h"
#include "BaseConfig.h"

bool SuperServer::doGateSuperCmd(xNetProcessor* np, BYTE* buf, WORD len)
{
  if (!np || !buf || !len) return false;

  using namespace Cmd;
  xCommand *cmd = (xCommand *)buf;
  switch (cmd->param)
  {
    case GATE_SUPER_USERNUM_CMD:
      {
        PARSE_CMD_PROTOBUF(GateToSuperUserNum, message);

        GateInfoM::getMe().updateUserNum(np, message.num());

        return true;
      }
    case GATEPARAM_ALTER_MSG:
      {
        PARSE_CMD_PROTOBUF(AlterMsgGateSuperCmd, message);

        alter_msg(message.title(), message.msg(), message.event());

        return true;
      }
      break;
    case GATEPARAM_PUSH_TYRANT_DB:
      {
        PARSE_CMD_PROTOBUF(PushTyrantDbGateSuperCmd, message);
        push_tyrantdb(message);
        return true;
      }
      break;
    default:
      break;
  }
  return true;
}

void SuperServer::alter_msg(const string& title, const string& message, const EPushMsg& event)
{
  xField *field = thisServer->getDBConnPool().getField(RO_DATABASE_NAME, "alter_msg");
  if (field)
  {
    xRecord record(field);
    record.put("title", title);
    record.put("message", message);
    record.put("event", event);

    QWORD ret = thisServer->getDBConnPool().exeInsert(record, true);
    if (QWORD_MAX != ret)
    {
      XLOG << "[报警]" << title << message << XEND;
    }
    else
    {
      XERR << "[报警]" << title << message << XEND;
    }
  }
}

void SuperServer::push_tyrantdb(const PushTyrantDbGateSuperCmd& rev)
{
  Json::Value root;

  stringstream ss1;
  ss1.str("");
  ss1 << rev.accid();
  std::string strAccid = ss1.str();

  Json::Value property;
  property["order_id"] = rev.orderid();
  property["amount"] = rev.amount();      //单位分
  property["virtual_currency_amount"] = rev.itemcount();
  property["currency_type"] = "CNY";
  property["product"] = rev.productid();
  property["payment"] = rev.chargetype();
  property["flag"] = 1;

  root["module"] = "GameAnalysis";
  root["name"] = "charge";

  stringstream sa;
  sa << "game_analysis-" << BaseConfig::getMe().getTapdbAppid();
  root["index"] = sa.str();
  root["identify"] = strAccid;
  root["properties"] = Json::Value(property);

  Json::FastWriter fw;
  std::string str = fw.write(root);
  string t1("\n");
  std::size_t pos = str.find_last_not_of(t1);
  string newstr = str;
  if (pos != string::npos)
  {
    newstr = str.substr(0, pos + 1);
  }

  string urlEncode = UrlEncode(newstr);
  std::stringstream ss;
  ss.str("");
  ss << "curl -d '" << urlEncode << "' http://tyrantdb.xd.com/event";
  string out = ss.str();
  system(out.c_str());

  XLOG << "[tyrantdb-推送] msg:" << out << ";json str:" << newstr << XEND;
}
