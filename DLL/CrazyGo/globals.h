#pragma once
#define _CRT_SECURE_NO_WARNINGS

#ifdef __cplusplus
#define DLLEXPORT extern "C" __declspec (dllexport)
#else 
#define DLLEXPORT __declspec (dllexport)
#endif

#include <vector>
#include <algorithm>
#include <time.h>
#include <windows.h>
using namespace std;

#define BOARD_SIZE  225
#define BLANK		0
#define	MACHINE		1
#define PLAYER		2
#define OFFENSE     1
#define DEFENSE     2

// �κεط�����INFINITY������Ч����˼
#ifdef INFINITY
#undef INFINITY
#define INFINITY	10000
#endif
#define MOVE_FAIL   INFINITY
#define WIN_VALUE	5000 - 200
#define MAX_DEPTH	12

#define HASH_SIZE	1<<21
#define HASH_FAIL   INFINITY
#define HASH_PV		3
#define HASH_ALPHA	1
#define HASH_BETA	2
#define HASH_EMPTY	0	

// 3^5+3^6+...+3^15�������п�������ƥ����������
#define SIZE_MATCH	0x1486B27	
#define CNT_VECTOR	72

#define MIN(a, b) 	((a) < (b) ? (a) : (b))
#define MAX(a, b) 	((a) > (b) ? (a) : (b))

#define	DOG_FALL	    1
#define	OFFENSIVE_WIN	2
#define DEFENSIVE_WIN	3

// ����Ȩ��
#define STRAIGHT_FOUR	128
#define FOUR			80
#define STRAIGHT_THREE	66
#define THREE			3
#define STRAIGHT_TWO	2
#define ADVANCE_VALUE	15

const int weight_factor[8] =
{
    0,
    0,
    0,
    STRAIGHT_FOUR,
    FOUR,
    STRAIGHT_THREE,
    THREE,
    STRAIGHT_TWO,
};

//��ͬλ�����������е�ƫ����
#define off_fiv		0x1486A34	
#define off_six		0x148675B
#define off_sev		0x1485ED0
#define off_eig		0x148452F
#define off_nin		0x147F84C
#define off_ten		0x14711A3
#define off_ele		0x1445DA8
#define off_twe		0x13C41B7
#define off_thi		0x123EDE4
#define off_fou		0xDAF26B
#define off_fif		0

const int off_hash[11] =
{
    off_fiv,
    off_six,
    off_sev,
    off_eig,
    off_nin,
    off_ten,
    off_ele,
    off_twe,
    off_thi,
    off_fou,
    off_fif
};

// 3��0��14�η�
const int bit_index[15] =
{
    1,
    3,
    9,
    27,
    81,
    243,
    729,
    2187,
    6561,
    19683,
    59049,
    177147,
    531441,
    1594323,
    4782969
};

struct Move
{
public:
    // �׶α�־
    int phase;
    // �û����ŷ�
    int mvhash;
    // �����ŵ��ŷ�
    int wise[2];
    // �����ŷ�
    vector<int> moves;

};

/**
* ViolateException bug����
* �ڽ�int���������ָ���mvbestʱ
* ����ֽضϰ�����ŵ�ת��
�� ��ѵ��ʹ ��
*/
struct HashItem
{
    __int64 lock;
    int flag;
    int	depth;
    int value;
    int	mvbest;

};

struct Attrib
{
    // �ĸ��������ڵ���������
    // �Լ��������������λ��
    // (��ÿ����λΪһ����λ)
    int direc[4][2];
    // �õ����ڵ��ܹ���5����������
    int cnt;

};

struct RunInfo
{
    char *str;
    int len;
};

