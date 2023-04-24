#include "xTools.h"
#include "xXMLParser.h"
#include "xServer.h"
#include "ProtoCommon.pb.h"
#include "json/json2pb.h"
#include "SceneUser2.pb.h"
extern "C"
{
#include "md5/md5.h"
}

std::map<std::string, std::string> Tools::global;

bool Tools::initGlobal(const char *configFile)
{
  if (!configFile)
  {
    XERR << "[Tools]打开配置失败" << XEND;
    return false;
  }

  xXMLParser p;
  if (!p.parseDoc(configFile))
  {
    XERR << "[Tools]打开配置失败 " << configFile << XEND;
    return false;
  }
  XLOG << "[Tools] 加载 " << configFile << XEND;

  xmlNodePtr root = p.getRoot();
  if (!root)
  {
    XERR << "[Tools] 配置文件错误 没有root节点" << XEND;
    return false;
  }

  xmlNodePtr globalNode = p.getChild(root,"global");
  if (!globalNode)
  {
    XERR << "[Tools] 没有global节点" << XEND;
    return false;
  }
  std::string ss;
  xmlNodePtr node = p.getChild(globalNode);
  while (node)
  {
    ss.clear();
    if (p.getPropValue(node, "value", &ss))
    {
      global[(char *)node->name] = ss;
      XDBG << "[global] " << (char *)node->name << "," << ss.c_str() << XEND;
    }
    else
      XERR << "[global] " << (char *)node->name << " 没有 value 节点" << XEND;

    node = p.getNext(node);
  }
  XLOG << "[global],加载config.xml中的全局变量" << XEND;
  return true;
}

UINT stringTok(std::string s, std::string k, std::vector<std::string> &v)
{
  std::string::size_type len = s.length();
  std::string::size_type i = 0, j = 0;

  while (i<len)
  {
    i = s.find_first_not_of(k, i);
    if (i==std::string::npos) break;

    j = s.find_first_of(k, i);
    if (j==std::string::npos)
    {
      v.push_back(s.substr(i, s.length()-i));
      break;
    }
    else
    {
      v.push_back(s.substr(i, j-i));
      i = j+1;
    }
  }

  return v.size();
}

// 获取所有被分隔的字段,包括空
UINT stringTokAll(std::string s, std::string k, std::vector<std::string> &v)
{
  std::string::size_type len = s.length();
  std::string::size_type i = 0, j = 0;

  while (i<len)
  {
    j = s.find_first_of(k, i);
    if (j==std::string::npos)
    {
      v.push_back(s.substr(i, s.length()-i));
      break;
    }
    else
    {
      v.push_back(s.substr(i, j-i));
      i = j+1;
    }
  }

  return v.size();
}

bool base64Encode(const std::string& input, std::string* output)
{
  typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8> > Base64EncodeIterator;
  std::stringstream result;
  copy(Base64EncodeIterator(input.begin()) , Base64EncodeIterator(input.end()), std::ostream_iterator<char>(result));
  size_t equal_count = (3 - input.length() % 3) % 3;
  for (size_t i = 0; i < equal_count; i++)
  {
    result.put('=');
  }
  *output = result.str();
  return output->empty() == false;
}

bool base64Decode(const std::string& input, std::string* output)
{
  typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6> Base64DecodeIterator;
  std::stringstream result;
  try
  {
    copy(Base64DecodeIterator(input.begin()) , Base64DecodeIterator(input.end()), std::ostream_iterator<char>(result));
  }
  catch (...)
  {
    return false;
  }
  *output = result.str();
  return output->empty() == false;
}

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// bin data base64Encode
void base64Encode(const unsigned char* bindata, char* base64, int binlength )
{
  int i, j;
  unsigned char current;
  for ( i = 0, j = 0 ; i < binlength ; i += 3 )
  {
    current = (bindata[i] >> 2) ;
    current &= (unsigned char)0x3F;
    base64[j++] = base64char[(int)current];

    current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
    if ( i + 1 >= binlength )
    {
      base64[j++] = base64char[(int)current];
      base64[j++] = '=';
      base64[j++] = '=';
      break;
    }
    current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
    base64[j++] = base64char[(int)current];

    current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
    if ( i + 2 >= binlength )
    {
      base64[j++] = base64char[(int)current];
      base64[j++] = '=';
      break;
    }
    current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
    base64[j++] = base64char[(int)current];

    current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
    base64[j++] = base64char[(int)current];
  }
  base64[j] = '\0';
}

bool upyun_form_str(std::string savekey, std::string &out_policy, std::string &out_signature)
{
  DWORD timestamp = now() + 4800;
  std::stringstream signstream;
  std::stringstream policystream;
  signstream.str("");
  policystream.str("");
  policystream << "{\"bucket\":\"ro-xdcdn\",\"expiration\":"<< timestamp << ",\"save-key\":\"" << savekey << ".jpg\",\"content-lenth-range\":\"0,1024000\"}";

  base64Encode(policystream.str(), &out_policy);
  signstream << out_policy << "&" << upyun_api_key;

  char sign[1024];
  bzero(sign, sizeof(sign));

  upyun_md5(signstream.str().c_str(), signstream.str().size(), sign);
  out_signature = sign;

  XLOG << "[upyun]," << policystream.str().c_str() << XEND;
  XLOG << "[upyun]," << out_policy.c_str() << XEND;
  XLOG << "[upyun]," << signstream.str().c_str() << XEND;
  XLOG << "[upyun]," << sign << XEND;

  return true;
}

DWORD getWordCount(const std::string& str)
{
  const char* _str = str.c_str();
  int n = 0;
  char ch = 0;
  while ((ch = *_str))
  {
    if (!ch)
      break;
    if (0x80 != (0xC0 & ch))
      ++n;
    ++_str;
  }
  return n;
}

unsigned char ToHex(unsigned char x)
{
  return  x > 9 ? x + 55 : x + 48;
}

unsigned char FromHex(unsigned char x)
{
  unsigned char y = 0;
  if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
  else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
  else if (x >= '0' && x <= '9') y = x - '0';
  else assert(0);
  return y;
}

std::string UrlEncode(const std::string& str)
{
  std::string strTemp = "";
  size_t length = str.length();
  for (size_t i = 0; i < length; i++)
  {
    if (isalnum((unsigned char)str[i]) ||
      (str[i] == '-') ||
      (str[i] == '_') ||
      (str[i] == '.') ||
      (str[i] == '~'))
      strTemp += str[i];
    else if (str[i] == ' ')
      strTemp += "+";
    else
    {
      strTemp += '%';
      strTemp += ToHex((unsigned char)str[i] >> 4);
      strTemp += ToHex((unsigned char)str[i] % 16);
    }
  }
  return strTemp;
}
