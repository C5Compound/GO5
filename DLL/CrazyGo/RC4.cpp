#include "RC4.h"

RC4::RC4(void)
{
    int i, j;
    char uc;
    x = y = j = 0;
    for (i = 0; i < 256; i++)
    {
        s[i] = i;
    }
    for (i = 0; i < 256; i++)
    {
        j = (j + s[i]) & 255;
        uc = s[i];
        s[i] = s[j];
        s[j] = uc;
    }
}
RC4::~RC4(void)
{
}
// 生成密码流的下一个字节
char RC4::next_char(void)
{
    char uc;
    x = (x + 1) & 255;
    y = (y + s[x]) & 255;
    uc = s[x];
    s[x] = s[y];
    s[y] = uc;
    return s[(s[x] + s[y]) & 255];
}
// 生成密码流的下四个字节
__int32 RC4::next_long(void)
{
    char uc0, uc1, uc2, uc3;
    uc0 = next_char();
    uc1 = next_char();
    uc2 = next_char();
    uc3 = next_char();
    return uc0 + (uc1 << 8) + (uc2 << 16) + (uc3 << 24);
}
__int64 RC4::next_int64(void)
{
    return next_long() + (next_long() << 32);
}

