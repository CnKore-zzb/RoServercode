#ifndef _X_JSON_PARSER_H_
#define _X_JSON_PARSER_H_
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/date_time.hpp>
#include <string.h>
#include <sstream>
#include "xDefine.h"

using namespace boost::property_tree;
using namespace boost::gregorian;
using namespace boost;

class xJsonParser
{
  public:
    xJsonParser();
    ~xJsonParser();

    bool parseFile(const char *file, ptree &pt);
    bool parse(std::string &buff, ptree &pt);

  private:
    char *buffer;
};
#endif
