#ifndef BASE_XLIB_XDBMETA_H_
#define BASE_XLIB_XDBMETA_H_

enum
{
  DBDATA_TYPE_NULL,
  DBDATA_TYPE_CHAR,
  DBDATA_TYPE_BYTE,
  DBDATA_TYPE_INT,
  DBDATA_TYPE_WORD,
  DBDATA_TYPE_DWORD,
  DBDATA_TYPE_QWORD,
  DBDATA_TYPE_BIN,
  DBDATA_TYPE_BIN2,
};

#define MAX_BIN2_SIZE 1024*10000

struct DBDataBin2
{
  DWORD size;
  BYTE data[0];
};

typedef struct
{
  const char *name;
  int type;
  unsigned int size;
}dbCol;
#endif  // BASE_XLIB_XDBMETA_H_
