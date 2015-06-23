#pragma once
class RC4
{
public:
    RC4(void);
    ~RC4(void);
private:
    char s[256];
    int x, y;
    // 生成密码流的下一个字节
    char next_char(void);
    // 生成密码流的下四个字节
    __int32 next_long(void);
public:
    // 生成64位数据
    __int64 next_int64(void);
};

