#include "Alphabeta.h"

AlphaBeta::AlphaBeta()
{
    // ��ʼ��RC4������
    RC4 rc;
    key = rc.next_int64();
    lock = rc.next_int64();
    for (int i = 0; i < BOARD_SIZE; i++) {
        rc4[0][i] = rc.next_int64();
        rc4[1][i] = rc.next_int64();
    }

    // ��ʼ�����������ֵ��еĳ�ʼƫ��
    for (int cx = 0; cx < 30; cx++) {
        index[cx] = off_hash[10];
    }
    for (int cx = 30; cx < 51; cx++) {
        index[cx] = off_hash[10 - abs(cx - 40)];
    }
    for (int cx = 51; cx < 72; cx++) {
        index[cx] = off_hash[10 - abs(cx - 61)];
    }

    // ��ʼ������
    memset(chessboard, 0, BOARD_SIZE * sizeof(int));

    // ��ʼ��ƥ����
    memset(cache, 0, 72 * sizeof(int));

    // ��ʼ������ͳ����Ϣ
    memset(sort_out, 0, 2 * 8 * sizeof(int));

    // ����ô��������Ӧ�ò���
    // ֻ��ͳ�ƽ���п��ܻ���ָ�ֵ
    // TODO
    sort_out[0][0] = 72;

    // ����û���
    memset(table, 0, HASH_SIZE * sizeof(HashItem));

    // �����ʷ������ñ�
    memset(refers, 0, 19 * 19 * sizeof(int));
    memset(history, 0, 15 * 15 * sizeof(int));

    /**
     * ��ȡģ����
     * ��ʹ����������޷���þ��
     */
    HMODULE hModule = GetModuleHandle(TEXT("CrazyGo.dll"));

    HRSRC hSrc;
    HGLOBAL hGlobal;

    // ��ʼ��ƥ���ֵ�
    hSrc = FindResource(hModule, MAKEINTRESOURCE(IDR_RT_RCDATA1), TEXT("RT_RCDATA"));
    hGlobal = LoadResource(hModule, hSrc);
    memcpy(matchTable, LockResource(hGlobal), SIZE_MATCH * sizeof(char));

    // ��ʼ���������ڵ���������Ϣ
    hSrc = FindResource(hModule, MAKEINTRESOURCE(IDR_RT_RCDATA2), TEXT("RT_RCDATA"));
    hGlobal = LoadResource(hModule, hSrc);
    memcpy(attrib, LockResource(hGlobal), 225 * sizeof(Attrib));

    // ����
    turn = OFFENSE;

    // ��ʼ������
    all_moves.clear();
}

AlphaBeta::~AlphaBeta()
{
}

// ���Ӻ���
void AlphaBeta::AddPiece(int mv)
{
    // �������̡�key��lock
    chessboard[mv] = turn;
    key ^= rc4[turn - 1][mv];
    lock ^= rc4[turn - 1][mv];

    //  �����һ
    distance += 1;

    // ͳ����Ϣ
    node_count += 1;
    max_depth = distance > max_depth ? distance : max_depth;

    int entry, tmp;

    // �����������ݸ���
    for (int cx = 0; cx < attrib[mv].cnt; cx++) {

        // ��ȡ���ֵ�����������������
        entry = attrib[mv].direc[cx][0];

        // �����������ֵ��е�����
        index[entry] += turn * bit_index[attrib[mv].direc[cx][1]];

        // ��ȡ����ǰ��ƥ����
        tmp = cache[entry];

        // ��ͳ�ƽ�����޳�ԭ����ƥ����
        sort_out[tmp / 8][tmp % 8] -= 1;

        // ��ȡ�µ�ƥ����
        tmp = matchTable[index[entry]];

        // ��ͳ�ƽ��������µĽ��
        sort_out[tmp / 8][tmp % 8] += 1;

        // �����µ�ƥ����
        cache[entry] = tmp;
    }

    // �������ü���
    for (int x = mv / 15; x <= mv / 15 + 4; x++) {
        for (int y = mv % 15; y <= mv % 15 + 4; y++) {
            refers[x][y] += 1;
        }
    }

    // �������ӷ�
    turn ^= 3;

    // ��¼����
    all_moves.push_back(mv);
}

// ɾ�Ӻ���
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
    // ƽ��
    if (all_moves.size() >= BOARD_SIZE) {
        return 0;
    }

    // ����
    int x = turn - 1;
    // �Է�
    int y = 2 - turn;

    // �����ͳ���
    if (sort_out[x][1] || sort_out[x][2]) {
        return WIN_VALUE;
    }
    if (sort_out[y][1] || sort_out[y][2]) {
        return -WIN_VALUE;
    }

    // ���ĺͼ�������
    if (sort_out[x][3] || sort_out[x][4]) {
        return WIN_VALUE - 100;
    }
    if (sort_out[y][3]) {
        return -WIN_VALUE + 100;
    }

    // �����������Է�������
    if (sort_out[x][5] && !sort_out[y][4]) {
        return WIN_VALUE - 400;
    }

    // �������
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

// ����Ƿ�check
bool AlphaBeta::inCheck()
{
    return sort_out[2 - turn][4] || sort_out[2 - turn][5];
}

// �ж��Ƿ�ʤ���ѷ�
int AlphaBeta::gameOver()
{
    // ƽ��
    if (all_moves.size() == BOARD_SIZE) {
        return DOG_FALL;
    }

    // ���ֺ���ʤ 
    if (sort_out[0][1] ||
        sort_out[0][2]) {
        return OFFENSIVE_WIN;
    }

    // ���ְ���ʤ
    if (sort_out[1][1] ||
        sort_out[1][2]) {
        return DEFENSIVE_WIN;
    }
    return 0;
}

// ��������ŷ�
void AlphaBeta::setBestMove(int mvbest, int depth)
{
    history[mvbest] += depth * depth;
    if (mvbest != killer_move[distance][1]) {
        killer_move[distance][0] = killer_move[distance][1];
        killer_move[distance][1] = mvbest;
    }
}

// �����߽��PVS����������
int AlphaBeta::SearchPVS(int depth, int alpha, int beta)
{
    if (depth == 0 ||                               // ����Ѵﵽ
        distance >= MAX_DEPTH ||                    // �������Ѵﵽ
        gameOver()) {                               // ʤ���ѷ�
        return evaluate();
    }
    int vl, mv;
    /**
     * Խ��bug����
     * �������³�ʼ����ɽ��
     */
    int mvhash = MOVE_FAIL;

    // �û���̽��
    if ((vl = probeHash(depth, alpha, beta, mvhash)) != HASH_FAIL) {
        table_hits += 1;
        return vl;
    }

    int mvbest = MOVE_FAIL;
    int vlbest = -INFINITY;
    int hash_flag = HASH_ALPHA;

    // ��ʼ���ŷ�
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
    // �����ALPHA�ŷ���mvbestΪMOVE_FAIL
    recordHash(depth, vlbest, hash_flag, mvbest);
    return vlbest;
}

int AlphaBeta::SearchRoot(int depth)
{
    int vl, mv;
    /**
    * Խ��bug����
    * �������³�ʼ����ɽ��
    */
    int mvbest = MOVE_FAIL;
    int vlbest = -INFINITY;
    Move mvs;
    // ����û�а���һ�������õ�����õ�ͨ��mvhash����
    // ����killer_move�Ѿ������������
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

// ��������
void AlphaBeta::IteratDeepen()
{
    memset(str, 0, 1024 * sizeof(char));
    strOffset = 0;

    time_t ct;
    time(&ct);

    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>%s", ctime(&ct));
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>---------- ������ʼ ----------\n1>...\n");

    // ��ʼ��ʱ�����
    clock_t total;
    total = clock();

    // ��ʼ����ͳ��ֵ
    node_count = 0;
    max_depth = 0;
    table_hits = 0;
    narrow_window_hits = 0;

    distance = 0;
    clock_t t;
    int value_return;
    // ����û���
    memset(table, 0, HASH_SIZE * sizeof(HashItem));
    // ���ɱ���ŷ�
    for (int cx = 0; cx < MAX_DEPTH; cx++) {
        killer_move[cx][0] = killer_move[cx][1] = MOVE_FAIL;
    }
    // ��������ͳ�ʱ���
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

    // ��C#����
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>�����ڵ���������:%d\n", node_count);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>����������Ϊ��:%d\n", max_depth);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>�û���ȣ������:%d\n", table_hits);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>С���ڣȣ������:%d\n", narrow_window_hits);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>��ʱ������������:%fs\n", (float)(clock() - total) / CLOCKS_PER_SEC);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>������ӵ�Ϊ����:%d\n", next_move);
    strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>====== �������� ======\n\n\n");
}

int AlphaBeta::PutPiece(int mv)
{
    int tmp;

    if (chessboard[mv] == BLANK) {

        strOffset += sprintf_s(str + strOffset, 1024 - strOffset, "1>��������ڡ�����:%d\n", mv);

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
        // �û������
        // TODO
        return 1;
    }
    return 0;
}

// �ŷ����ɺ���
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

// ��ʼ���ŷ�
void AlphaBeta::initMoves(Move *p, int mvhash)
{
    p->phase = 0;
    p->mvhash = mvhash;
    //�����Ƿ���ɱ���ŷ������ڵ����
    p->wise[0] = 
        (killer_move[distance][0] != MOVE_FAIL) ? 
        killer_move[distance][0] : mvhash;
    p->wise[1] = 
        (killer_move[distance][1] != MOVE_FAIL) ? 
        killer_move[distance][1] : mvhash;
}

// ������һ�ŷ�
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

// �����û���
// �û����ŷ���ȡ������
// TODO
int AlphaBeta::probeHash(int depth, int alpha, int beta, int &mv)
{
    HashItem item = table[key & (HASH_SIZE - 1)];
    if (item.lock != lock) {
        mv = MOVE_FAIL;
        return HASH_FAIL;
    }
    // �Ƿ�ʤ���ѷ�
    bool over = false;
    mv = item.mvbest;
    //����Ǳ�ʤ�ľ��棬��ʹ��Ȳ�����Ȼ�ɹ�����
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
* ��¼���û���ʱ�����ALPHA�ŷ�ʱ
* �Ƿ��¼���û���
* ��ʱ��ȡ������ͬ�ŷ�����¼��
* ���������ʤ�����ŷ�����Ȳ����򲻼�¼
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
     * mv = 16 ����bug���ڵ�
     * HashItem��mvbestΪchar���ͣ�����Ҫ��¼��ΪALPHA �ŷ�ʱ
     * mv = 10000����ֵ��ض�Ϊ16�����Խ�mvbest��Ϊint�ͻ���
     * �� MOVE_FAIL ��Ϊ225
     */
    item.mvbest = mv;

    table[key & (HASH_SIZE - 1)] = item;
}