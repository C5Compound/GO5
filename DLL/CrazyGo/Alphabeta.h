#pragma once
#include "globals.h"
#include "RC4.h"
#include "resource.h"

/*
※ 根据程序运行的结果估计
※ 五子棋的分支因子在12~15左右
*/

class AlphaBeta
{
    // 全局
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
    *  置换表相关数据及函数
    *  经测试，置换表在五子棋程序中效果很差
    *  命中几率几乎为零，去除后基本不影响效率
    */

    HashItem table[HASH_SIZE];

    // 置换表查找
    int probeHash(int depth,
        int alpha,
        int beta,
        int &mv);

    // 置换表保存
    void recordHash(int depth,
        int value,
        int flag,
        int mv);

    /**
    *  局面相关函数及数据
    */

    // 棋型匹配哈希表
    char matchTable[SIZE_MATCH];

    // 棋盘上点所在向量的索引及个数
    Attrib attrib[BOARD_SIZE];

    // 72条位向量
    int	index[72];

    // 匹配缓存
    int cache[72];

    // 统计结果
    int sort_out[2][8];

    // 局面评估
    int	evaluate();

    // 胜负判断
    int gameOver();

    // check检测
    bool inCheck();

    /**
    *  着法生成相关函数及数据
    */

    // 引用计数
    int refers[19][19];

    // 历史表
    int history[BOARD_SIZE];

    // 杀手着法
    int killer_move[MAX_DEPTH][2];

    // 着法生成
    void genMoves(Move*);

    // 初始化着法
    void initMoves(Move*, int);

    // 获得下一着法
    int nextMove(Move*);

    // 保存着法
    void setBestMove(int, int);

    /**
    *  搜索函数
    */

    // 极大极小和α,β剪枝主变例搜索
    int SearchPVS(int, int, int);

    // 根节点的搜索过程
    int SearchRoot(int);

    // 迭代加深
    void IteratDeepen();

    /*
    ※ 调试信息 ※
    */

    // 遍历的节点总数
    int node_count;
    // 遍历达到的最大深度
    int max_depth;
    // 置换表Hit次数
    int table_hits;
    // 小窗口Hit次数
    int narrow_window_hits;

public:
    // 构造
    AlphaBeta();
    // 析构
    ~AlphaBeta();

    int PutPiece(int);

    int PopPiece();

    // 落子删子
    void AddPiece(int);
    void DelPiece(int);

    // 用于直接向exe传递调试信息
    char str[1024];
    int strOffset;
};