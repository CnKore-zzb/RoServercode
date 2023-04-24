#include "xJsonParser.h"
#include "xTools.h"
#include <string>

xJsonParser::xJsonParser()
{
  buffer = NULL;
}

xJsonParser::~xJsonParser()
{
  if (buffer) free(buffer);
}

bool xJsonParser::parseFile(const char *file, ptree &pt)
{
  FILE *in = NULL;

  if (in == NULL)
    in = fopen(file, "r");

  if (in == NULL) return false;

  fseek(in, 0L, SEEK_END);
  DWORD numBytes = ftell(in);
  fseek(in, 0L, SEEK_SET);

  if (buffer)
  {
    free(buffer);
    buffer = NULL;
  }

  do
  {
    buffer = (char *)calloc(numBytes, sizeof(char));
    if (buffer == NULL)
      break;

    size_t readsize = fread(buffer, sizeof(char), numBytes, in);
    if (readsize != numBytes)
      break;

    fclose(in);

    std::string buff(buffer);

#ifdef _LX_DEBUG
 //   XLOG("[Json],加载,%s,buffsize:%u,numBytes:%u", file, buff.size(), numBytes);
   // XLOG("[Json],%s", buff.c_str());
#endif

    /*
    std::size_t pos = buff.find("\"");
    while (pos != std::string::npos)
    {
      buff.replace(pos, 1, "\\\"");
      pos = buff.find("\"", pos + 2);
    }
    */

#ifdef _LX_DEBUG
   // XLOG("[Json],加载,%s,buffsize:%u,numBytes:%u", file, buff.size(), numBytes);
   // XLOG("[Json],%s", buff.c_str());
#endif

    std::stringstream stream;

    stream << buff;

#ifdef _LX_DEBUG
   // XLOG("[Json],加载,%s,%u", file, stream.str().size());
#endif

    read_json<ptree>(stream, pt);


    return true;

  } while (false);

  fclose(in);

  return false;
}

bool xJsonParser::parse(std::string &buff, ptree &pt)
{
  std::stringstream stream;

  stream << buff;

  try
  {
    read_json<ptree>(stream, pt);
  }
  catch (ptree_error &e)
  {
    return false;
  }
  return true;
}
