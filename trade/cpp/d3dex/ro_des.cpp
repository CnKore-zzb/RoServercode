#include "PHP_API.hpp"
#include "module.h"
#include <iostream>
#include "swoole.h"

extern "C"
{
    #include "d3des.h"
    int swModule_init(swModule *);
}
#include <vector>

using namespace std;
using namespace PHP;

typedef unsigned char BYTE;

BYTE fixedkey[8] = { 95,27,5,20,131,4,8,88 };
unsigned long enckey[32] = { 0x04013922, 0x28001C08, 0x2020043B, 0x00113A18, 0x01083E14, 0x12001911, 0x00120218, 0x2100362E, 0x16011F26, 0x00000704, 0x00002803, 0x0C08043F, 0x0004312A, 0x00063B03, 0x08202D1D, 0x00012039, 0x12003D10, 0x01003506, 0x0001130F, 0x080C002E, 0x00002523, 0x04020F0D, 0x0824181B, 0x00202931, 0x20002634, 0x02013A2B, 0x00180B3C, 0x10102510, 0x11021605, 0x01001636, 0x0004312D, 0x00200C35 };
unsigned long deckey[32] = { 0x0004312D, 0x00200C35, 0x11021605, 0x01001636, 0x00180B3C, 0x10102510, 0x20002634, 0x02013A2B, 0x0824181B, 0x00202931, 0x00002523, 0x04020F0D, 0x0001130F, 0x080C002E, 0x12003D10, 0x01003506, 0x08202D1D, 0x00012039, 0x0004312A, 0x00063B03, 0x00002803, 0x0C08043F, 0x16011F26, 0x00000704, 0x00120218, 0x2100362E, 0x01083E14, 0x12001911, 0x2020043B, 0x00113A18, 0x04013922, 0x28001C08 };

std::vector<BYTE> buffer;

void ro_encrypt(Args &args, Variant &retval);
void ro_decrypt(Args &args, Variant &retval);
int sizeMod8(int len);
void encrypt(void *data, int len);
void decrypt(void *data, int len);

int swModule_init(swModule *module)
{
    buffer.resize(65535 * 2);
    module->name = (char *) "d3des";
    PHP::registerFunction(function(ro_encrypt));
    PHP::registerFunction(function(ro_decrypt));
    return SW_OK;
}

int sizeMod8(int len)
{
    return (len + 7) & ~7;
    return (len + 7) / 8 * 8;
}

void encrypt(void *data, int len)
{
    //deskey(fixedkey, EN0);
    usekey(enckey);
    for (int i = 0; i < len; i += 8)
    {
        des((BYTE *)data + i, (BYTE *)&(buffer[i]));
    }
    memcpy(data, &(buffer[0]), len);
}

void decrypt(void *data, int len)
{
    //deskey(fixedkey, DE1);
    usekey(deckey);
    for (int i = 0; i < len; i += 8)
    {
        des((BYTE *)data + i, (BYTE *)&buffer[i]);
    }
    memcpy(data, &(buffer[0]), len);
}

void ro_encrypt(Args &args, Variant &retval)
{
    Variant data = args[0];
    if (!data.isString()) {
        retval = false;
    }
    int len = sizeMod8(data.length());
    char str[len];
    bcopy(data.toCString(), str, len);
    encrypt((Variant *) str, len);

    string out(str, len);
    retval = out;
}
void ro_decrypt(Args &args, Variant &retval)
{
    Variant data = args[0];
    if (!data.isString()) {
        retval = false;
    }
    int len = sizeMod8(data.length());
    char str[len];
    bcopy(data.toCString(), str, len);
    decrypt((Variant *) str, len);

    string out(str, len);
    out.erase(out.find_last_not_of('\0') + 1);
    retval = out;
}