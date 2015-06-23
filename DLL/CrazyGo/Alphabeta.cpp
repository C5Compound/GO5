#include "Alphabeta.h"

AlphaBeta::AlphaBeta()
{
    // 初始化RC4密码流
    RC4 rc;
    key = rc.next_int64();
    lock = rc.next_int64();
    for (int i = 0; i < BOARD_SIZE; i++) {
        rc4[0][i] = rc.next_int64();
        rc4[1][i] = rc.next_int64();
    }

    // 初始化各向量在字典中的初始偏移
    for (int cx = 0; cx < 30; cx++) {
        index[cx] = off_hash[10];
    }
    for (int cx = 30; cx < 51; cx++) {
        index[cx] = off_hash[10 - abs(cx - 40)];
    }
    for (int cx = 51; cx < 72; cx++) {
        index[cx] = off_hash[10 - abs(cx - 61)];
    }

    // 初始化棋盘
    memset(chessboard, 0, BOARD_SIZE * sizeof(int));

    // 初始化匹配结果
    memset(cache, 0, 72 * sizeof(int));

    // 初始化棋型统计信息
    memset(sort_out, 0, 2 * 8 * sizeof(int));

    // 不这么设置问题应该不大
    // 只是统计结果中可能会出现负值
    // TODO
    sort_out[0][0] = 72;

    // 清空置换表
    memset(table, 0, HASH_SIZE * sizeof(HashItem));

    // 清空历史表和引用表
    memset(refers, 0, 19 * 19 * sizeof(int));
    memset(history, 0, 15 * 15 * sizeof(int));

    /**
     * 获取模块句柄
     * 不使用这个参数无法获得句柄
     */
    HMODULE hModule = GetModuleHandle(TEXT("CrazyGo.dll"));

    HRSRC hSrc;
    HGLOBAL hGlobal;

    // 初始化匹配字典
    hSrc = FindResource(hModule, MAKEINTRESOURCE(IDR_RT_RCDATA1), TEXT("RT_RCDATA"));
    hGlobal = LoadResource(hModule, hSrc);
    memcpy(matchTable, LockResource(hGlobal), SIZE_MATCH * sizeof(char));

    // 初始化各点所在的向量的信息
    hSrc = FindResource(hModule, MAKEINTRESOURCE(IDR_RT_RCDATA2), TEXT("RT_RCDATA"));
    hGlobal = LoadResource(hModule, hSrc);
    memcpy(attrib, LockResource(hGlobal), 225 * sizeof(Attrib));

    // 先手
    turn = OFFENSE;

    // 初始化落子
    all_moves.clear();
}

AlphaBeta::~AlphaBeta()
{
}

// 落子函数
void AlphaBeta::AddPiece(int mv)
{
    // 更新棋盘、key和lock
    chessboard[mv] = turn;
    key ^= rc4[turn - 1][mv];
    lock ^= rc4[turn - 1][mv];

    //  距离加一
    distance += 1;

    // 统计信息
    node_count += 1;
    max_depth = distance > max_depth ? distance : max_depth;

    int entry, tmp;

    // 局面评估数据更新
    for (int cx = 0; cx < attrib[mv].cnt; cx++) {

        // 获取罗字典所在向量的索引号
        entry = attrib[mv].direc[cx][0];

        // 更新向量在字典中的索引
        index[entry] += turn * bit_index[attrib[mv].direc[cx][1]];

        // 获取落子前的匹配结果
        tmp = cache[entry];

        // 从统计结果中剔除原来的匹配结果
        sort_out[tmp / 8][tmp % 8] -= 1;

        // 获取新的匹配结果
        tmp = matchTable[index[entry]];

        // 向统计结果中添加新的结果
        sort_out[tmp / 8][tmp % 8] += 1;

        // 保存新的匹配结果
        cache[entry] = tmp;
    }

    // 更新引用计数
    for (int x = mv / 15; x <= mv / 15 + 4; x++) {
        for (int y = mv % 15; y <= mv % 15 + 4; y++) {
            refers[x][y] += 1;
        }
    }

    // 交换走子方
    turn ^= 3;

    // 记录走子
    all_moves.push_back(mv);
}

// 删子函数
void AlphaBeta::DelPiece(int mv)
{
    turn ^= 3;

    chessboard[mv] = BLANK;
    key ^= rc4[turn - 1][mv];
    lock ^= rc4[turn - 1][mv];

    distance -= 1;

    int tmp, entry;

    for (int cx = 0; cx < attrib[mv].cnt; cx++) {
        entry = attrib[mv].direc[cx][0];
        index[entry] -= turn * bit_index[attrib[mv].direc[cx][1]];
        tmp = cache[entry];
        sort_out[tmp / 8][tmp % 8] -= 1;
        tmp = matchTable[index[entry]];
        sort_out[tmp / 8][tmp % 8] += 1;
        cache[entry] = tmp;
    }

    for (int x = mv / 15; x <= mv / 15 + 4; x++) {
        for (int y = mv % 15; y <= mv % 15 + 4; y++) {
            refers[x][y] -= 1;
        }
    }

    all_moves.pop_back();
}

int AlphaBeta::evaluate()
{
    // 平局
    if (all_moves.size() >= BOARD_SIZE) {
        return 0;
    }

    // 己方
    int x = turn - 1;
    // 对方
    int y = 2 - turn;

    // 长联和成五
    if (sort_out[x][1] || sort_out[x][2]) {
        return WIN_VALUE;
    }
    if (sort_out[y][1] || sort_out[y][2]) {
        return -WIN_VALUE;
    }

    // 活四和己方冲四
    if (sort_out[x][3] || sort_out[x][4]) {
        return WIN_VALUE - 100;
    }
    if (sort_out[y][3]) {
        return -WIN_VALUE + 100;
    }

    // 己方活三而对方不冲四
    if (sort_out[x][5] && !sort_out[y][4]) {
        return WIN_VALUE - 400;
    }

    // 其余情况
    int score_of_x =
        sort_out[x][5] * weight_factor[5] +
        sort_out[x][6] * weight_factor[6] +
        sort_out[x][7] * weight_factor[7];

    int score_of_y =
        sort_out[y][4] * weight_factor[4] +
        sort_out[y][5] * weight_factor[5] +
        sort_out[y][6] * weight_factor[6] +
        sort_out[y][7] * weight_factor[7];

    return score_of_x - score_of_y/* + ADVANCE_VALUE*/;
}

// 检测是否被check
bool AlphaBeta::inCheck()
{
    return sort_out[2 - turn][4] || sort_out[2 - turn][5];
}

// 判断是否胜负已分
int AlphaBeta::gameOver()
{
    // 平局
    if (all_moves.size() == BOARD_SIZE) {
        return DOG_FALL;
    }

    // 先手黑棋胜 
    if (sort_out[0][1] ||
        sort_out[0][2]) {
        return OFFENSIVE_WIN;
    }

    // 后手白棋胜
    if (sort_out[1][1] ||
        sort_out[1][2]) {
        return DEFENSIVE_WIN;
    }
    return 0;
}

// 保存最佳着法
void AlphaBeta::setBestMove(int mvbest, int depth)
{
    history[mvbest] += depth * depth;
    if (mvbest != killer_move[distance][1]) {
        killer_move[distance][0] = killer_move[distance][1];
        killer_move[distance][1] = mvbest;
    }
}

// 超出边界的PVS主变例搜索
int AlphaBeta::SearchPVS(int depth, int alpha, int beta)
{
    if (depth == 0 ||                               // 深度已达到
        distance >= MAX_DEPTH ||                    // 最大距离已达到
        gameOver()) {                               // 胜负已分
        return evaluate();
    }
    int vl, mv;
    /**
     * 越界bug所在
     * 进行如下初始化后可解决
     */
    int mvhash = MOVE_FAIL;

    // 置换表探查
    if ((vl = probeHash(depth, alpha, beta, mvhash)) != HASH_FAIL) {
        table_hits += 1;
        return vl;
    }

    int mvbest = MOVE_FAIL;
    int vlbest = -INFINITY;
    int hash_flag = HASH_ALPHA;

    // 初始化着法
    Move mvs;
    initMoves(&mvs, mvhash);

    while ((mv = nextMove(&mvs)) != MOVE_FAIL) {
        AddPiece(mv);
        if (vlbest == -INFINITY) {
            vl = -SearchPVS(depth - 1, -beta, -alpha);
        } else {
            narrow_window_hits += 1;
            vl = -SearchPVS(depth - 1, -alpha - 1, -alpha);
            if (vl > alpha && vl < beta) {
                narrow_window_hits -= 1;
                vl = -SearchPVS(depth - 1, -beta, -alpha);
            }
        }
        DelPiece(mv);
        if (vl > vlbest) {
            vlbest = vl;
            if (vl >= beta) {
                hash_flag = HASH_BETA;
                mvbest = mv;
                break;
            }
            if (vl > alpha) {
                hash_flag = HASH_PV;
                mvbest = mv;
                alpha = vl;
            }
        }
    }
    if (hash_flag != HASH_ALPHA) {
        setBestMove(mvbest, depth);
    }
    // 如果是ALPHA着法，mvbest为MOVE_FAIL
    recordHash(depth, vlbest, hash_flag, mvbest);
    return vlbest;
}

int AlphaBeta::SearchRoot(int depth)
{
    int vl, mv;
    /**
    * 越界bug所在
    * 进行如下初始化后可解决
    */
    int mvbest = MOVE_FAIL;
    int vlbest = -INFINITY;
    Move mvs;
    // 尽管没有把上一次搜索得到的最好点通过mvhash传递
    // 但是killer_move已经保存了这个点
    initMoves(&mvs, MOVE_FAIL);
    while ((mv = nextMove(&mvs)) != MOVE_FAIL) {
        AddPiece(mv);
        if (vlbest == -INFINITY) {
            vl = -SearchPVS(depth - 1, -INFINITY, INFINITY);
        } else {
            narrow_window_hits += 1;
            vl = -SearchPVS(depth - 1, -vlbest - 1, -vlbest);
            if (vl > vlbest) {
                narrow_window_hits -= 1;
                vl = -SearchPVS(depth - 1, -INFINITY, -vlbest);
            }
        }
        DelPiece(mv);
        if (vl > vlbest) {
            vlbest = vl;
            mvbest = mv;
        }
    }
    setBestMove(mvbest, depth);
    recordHash(depth, vlbest, HASH_PV, mvbest);
    next_move = mvbest;
    return vlbest;
}

// 迭代加深
void AlphaBeta::IteratDeepen()
{
    memset(str, 0, 1024 * sizeof(char));
    strOffset = 0;

    time_t ct;
    time(&ct);

    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>%s", ctime(&ct));
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>---------- 搜索开始 ----------\n1>...\n");

    // 初始化时间起点
    clock_t total;
    total = clock();

    // 初始化各统计值
    node_count = 0;
    max_depth = 0;
    table_hits = 0;
    narrow_window_hits = 0;

    distance = 0;
    clock_t t;
    int value_return;
    // 清空置换表
    memset(table, 0, HASH_SIZE * sizeof(HashItem));
    // 清空杀手着法
    for (int cx = 0; cx < MAX_DEPTH; cx++) {
        killer_move[cx][0] = killer_move[cx][1] = MOVE_FAIL;
    }
    // 迭代加深和超时检测
    for (int depth = 1; depth < MAX_DEPTH; depth++) {
        t = clock();
        value_return = SearchRoot(depth);
        if (value_return >= WIN_VALUE - 500 || value_return <= -WIN_VALUE + 500) {
            break;
        }
        if (clock() - t >= CLOCKS_PER_SEC / 2) {
            break;
        }
    }

    // 向C#传递
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>搜索节点数　　　:%d\n", node_count);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>最大搜索深度为　:%d\n", max_depth);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>置换表Ｈｉｔ　　:%d\n", table_hits);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>小窗口Ｈｉｔ　　:%d\n", narrow_window_hits);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>用时　　　　　　:%fs\n", (float)(clock() - total) / CLOCKS_PER_SEC);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>最佳落子点为　　:%d\n", next_move);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>====== 搜索结束 ======\n\n\n");
}

int AlphaBeta::PutPiece(int mv)
{
    int tmp;

    if (chessboard[mv] == BLANK) {

        strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>玩家落子于　　　:%d\n", mv);

        AddPiece(mv);

        tmp = gameOver();
        if (tmp == OFFENSIVE_WIN) {
            return 1024;
        }
        if (tmp == DEFENSIVE_WIN) {
            return 2048;
        }
        if (tmp == DOG_FALL) {
            return 4096;
        }

        IteratDeepen();
        AddPiece(next_move);

        tmp = gameOver();
        if (tmp == OFFENSIVE_WIN) {
            return 1024 + next_move;
        }
        if (tmp == DEFENSIVE_WIN) {
            return 2048 + next_move;
        }
        if (tmp == DOG_FALL) {
            return 4096 + next_move;
        }
        return next_move;
    }
    return INFINITY;
}

int AlphaBeta::PopPiece()
{
    if (all_moves.size() >= 2) {
        DelPiece(all_moves.back());
        DelPiece(all_moves.back());
        // 置换表回退
        // TODO
        return 1;
    }
    return 0;
}

// 着法生成函数
void AlphaBeta::genMoves(Move *p)
{
    p->moves.clear();
    for (int mv = 0; mv < 225; mv++) {
        if (refers[mv / 15 + 2][mv % 15 + 2] && !chessboard[mv] &&
            mv != p->mvhash && mv != p->wise[0] && mv != p->wise[1]) {
            p->moves.push_back(mv);
        }
    }
    sort(p->moves.begin(), p->moves.end(), [this](int i, int j){return history[i] > history[j]; });
}

// 初始化着法
void AlphaBeta::initMoves(Move *p, int mvhash)
{
    p->phase = 0;
    p->mvhash = mvhash;
    //这里是否考虑杀手着法不存在的情况
    p->wise[0] = 
        (killer_move[distance][0] != MOVE_FAIL) ? 
        killer_move[distance][0] : mvhash;
    p->wise[1] = 
        (killer_move[distance][1] != MOVE_FAIL) ? 
        killer_move[distance][1] : mvhash;
}

// 生成下一着法
int AlphaBeta::nextMove(Move *p)
{
    switch (p->phase)
    {
    case 0:
        p->phase += 1;
        if (p->mvhash != MOVE_FAIL)
        {
            return p->mvhash;
        }
    case 1:
        p->phase += 1;
        if (p->wise[0] != p->mvhash &&
            !chessboard[p->wise[0]]) {
            return killer_move[distance][0];
        }
    case 2:
        p->phase += 1;
        if (p->wise[1] != p->mvhash &&
            p->wise[1] != p->wise[0] && 
            !chessboard[p->wise[1]]) {
            return p->wise[1];
        }
    case 3:
        p->phase += 1;
        genMoves(p);
    case 4:
        while (!p->moves.empty()) {
            int mv = p->moves.back();
            p->moves.pop_back();
            return mv;
        }
    default:
        return MOVE_FAIL;
    }
}

// 检索置换表
// 置换表着法提取有问题
// TODO
int AlphaBeta::probeHash(int depth, int alpha, int beta, int &mv)
{
    HashItem item = table[key & (HASH_SIZE - 1)];
    if (item.lock != lock) {
        mv = MOVE_FAIL;
        return HASH_FAIL;
    }
    // 是否胜负已分
    bool over = false;
    mv = item.mvbest;
    //如果是必胜的局面，即使深度不够依然成功返回
    if (item.value >= WIN_VALUE) {
        over = true;
        item.value -= distance;
    }
    else if (item.value <= -WIN_VALUE) {
        over = true;
        item.value += distance;
    }
    if (item.depth >= depth || over) {
        if (item.flag == HASH_ALPHA) {
            return item.value <= alpha ? item.value : HASH_FAIL;
        }
        else if (item.flag == HASH_BETA) {
            return item.value >= beta ? item.value : HASH_FAIL;
        }
        return item.value;
    }
    return HASH_FAIL;
}

/**
* 记录到置换表时如果是ALPHA着法时
* 是否记录到置换表？
* 暂时才取的是连同着法都记录！
* 至于如果是胜利的着法而深度不够则不记录
*/
void AlphaBeta::recordHash(int depth, int value, int flag, int mv)
{
    HashItem item = table[key & (HASH_SIZE - 1)];

    if (item.flag != HASH_EMPTY && depth < item.depth) {
        return;
    }
    if (value >= WIN_VALUE) {
        item.value = value - distance;
    }
    else if (item.value <= -WIN_VALUE) {
        item.value = value + distance;
    }
    else {
        item.value = value;
    }
    item.lock = lock;
    item.flag = flag;
    item.depth = depth;

    /**
     * mv = 16 吃子bug所在地
     * HashItem的mvbest为char类型，当所要记录的为ALPHA 着法时
     * mv = 10000，赋值后截断为16。可以将mvbest改为int型或者
     * 将 MOVE_FAIL 改为225
     */
    item.mvbest = mv;

    table[key & (HASH_SIZE - 1)] = item;
}