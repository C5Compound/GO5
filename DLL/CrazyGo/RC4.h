#pragma once
class RC4
{
public:
    RC4(void);
    ~RC4(void);
private:
    char s[256];
    int x, y;
    // ��������������һ���ֽ�
    char next_char(void);
    // ���������������ĸ��ֽ�
    __int32 next_long(void);
public:
    // ����64λ����
    __int64 next_int64(void);
};

