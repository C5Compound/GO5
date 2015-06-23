#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))

typedef struct Attrib
{
    // 四个方向所在的向量索引以及相对于向量起点的位移(以每两个位为一个单位)
    int direc[4][2];
    // 该点所在的能够成5的向量个数
    int cnt;

} Attrib;

Attrib InitPosInfo(int index)
{
    Attrib attrib = {0};
    int cnt = 2;
    int cx = index % 15;
    int cy = index / 15;
    //水平方向向量
    attrib.direc[0][0] = cy;
    attrib.direc[0][1] = cx;
    //竖直方向向量
    attrib.direc[1][0] = cx + 15;
    attrib.direc[1][1] = cy;
    //斜向右下方向向量
    if (cx + cy>3 && cx + cy<25)
    {
        attrib.direc[2][0] = cx + cy - 4 + 30;
        attrib.direc[2][1] = MIN(14 - cx, cy);
        cnt++;
    }
    //斜向左下方向向量
    if (14 + cy - cx>3 && 14 + cy - cx<25)
    {
        attrib.direc[cnt][0] = 14 + cy - cx - 4 + 51;
        attrib.direc[cnt][1] = MIN(cx, cy);
        cnt++;
    }
    attrib.cnt = cnt;
    return attrib;
}

#define SIZE_MATCH  0x1486B27
Attrib attrib[225];
char matchTable[SIZE_MATCH];

int main()
{
    memset(attrib, 0, 225 * sizeof(Attrib));
    for (int x = 0; x < 225; x++) {
        attrib[x] = InitPosInfo(x);
    }
    HANDLE hFile = CreateFile(TEXT("ATTRIBS.DAT"), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    DWORD dwWrite;
    ReadFile(hFile, attrib, 225 * sizeof(Attrib), &dwWrite, NULL);
    CloseHandle(hFile);
    return 0;
}