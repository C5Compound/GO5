#pragma once
#include "globals.h"
#include "RC4.h"
#include "resource.h"

/*
�� ���ݳ������еĽ������
�� ������ķ�֧������12~15����
*/

class AlphaBeta
{
    // ȫ��
private:
    int chessboard[BOARD_SIZE];
    int turn;
    int distance;
    vector<int> all_moves;
    __int64	key;
    __int64 lock;
    __int64 rc4[2][BOARD_SIZE];
    int next_move;

    /**
    *  �û���������ݼ�����
    *  �����ԣ��û����������������Ч���ܲ�
    *  ���м��ʼ���Ϊ�㣬ȥ���������Ӱ��Ч��
    */

    HashItem table[HASH_SIZE];

    // �û������
    int probeHash(int depth,
        int alpha,
        int beta,
        int &mv);

    // �û�����
    void recordHash(int depth,
        int value,
        int flag,
        int mv);

    /**
    *  ������غ���������
    */

    // ����ƥ���ϣ��
    char matchTable[SIZE_MATCH];

    // �����ϵ���������������������
    Attrib attrib[BOARD_SIZE];

    // 72��λ����
    int	index[72];

    // ƥ�仺��
    int cache[72];

    // ͳ�ƽ��
    int sort_out[2][8];

    // ��������
    int	evaluate();

    // ʤ���ж�
    int gameOver();

    // check���
    bool inCheck();

    /**
    *  �ŷ�������غ���������
    */

    // ���ü���
    int refers[19][19];

    // ��ʷ��
    int history[BOARD_SIZE];

    // ɱ���ŷ�
    int killer_move[MAX_DEPTH][2];

    // �ŷ�����
    void genMoves(Move*);

    // ��ʼ���ŷ�
    void initMoves(Move*, int);

    // �����һ�ŷ�
    int nextMove(Move*);

    // �����ŷ�
    void setBestMove(int, int);

    /**
    *  ��������
    */

    // ����С�ͦ�,�¼�֦����������
    int SearchPVS(int, int, int);

    // ���ڵ����������
    int SearchRoot(int);

    // ��������
    void IteratDeepen();

    /*
    �� ������Ϣ ��
    */

    // �����Ľڵ�����
    int node_count;
    // �����ﵽ��������
    int max_depth;
    // �û���Hit����
    int table_hits;
    // С����Hit����
    int narrow_window_hits;

public:
    // ����
    AlphaBeta();
    // ����
    ~AlphaBeta();

    int PutPiece(int);

    int PopPiece();

    // ����ɾ��
    void AddPiece(int);
    void DelPiece(int);

    // ����ֱ����exe���ݵ�����Ϣ
    char str[1024];
    int strOffset;
};