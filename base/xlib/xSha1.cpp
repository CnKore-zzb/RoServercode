extern "C"
{
#include "sha1/osha1.h"
}
#include "xSha1.h"
#include "xLog.h"

bool getSha1Result(char *result, const char *input, DWORD size)
{
  if (!result || !input || !size)
    return false;

  SHA1Context sha;
  SHA1Reset(&sha);
  SHA1Input(&sha, (const unsigned char *) input, size);

  if (SHA1Result(&sha))
  {
    //lower case
    snprintf(result, SHA1_LEN + 1, "%08x%08x%08x%08x%08x", sha.Message_Digest[0], sha.Message_Digest[1], 
        sha.Message_Digest[2], sha.Message_Digest[3], sha.Message_Digest[4]);
  }
  else
    return false;

  return true;
}

bool checkSha1(const char *sha1Str, const char *input, DWORD size)
{
  char sha1Res[SHA1_LEN + 1];
  bzero(sha1Res, sizeof(sha1Res));
  if (sha1Str && getSha1Result(sha1Res, input, size))
  {
    char sha1Str_lowerCase[SHA1_LEN + 1];
    for (DWORD i = 0; i < SHA1_LEN; ++i)
      sha1Str_lowerCase[i] = tolower(sha1Str[i]);

    if (memcmp(sha1Res, sha1Str_lowerCase, SHA1_LEN) == 0)
      return true;

    XERR << "[sha1],验证失败,input:" << input << "str:" << sha1Str << "res:" << sha1Res << XEND;
  }

  return false;
}

