#include "Alphabeta.h"
AlphaBeta *game = NULL;

DLLEXPORT void NewGame()
{
    if (game != NULL) {
        delete game;
    }
    game = new AlphaBeta();
}
DLLEXPORT int MakeMove(int mv)
{
    return game->PutPiece(mv);
}

// ����
DLLEXPORT void CancleMove()
{

}

DLLEXPORT void GetOffensive()
{

}

DLLEXPORT int FetchInfo(RunInfo* info)
{
    info->str = game->str;
    info->len = game->strOffset;
    return 0;
}

DLLEXPORT int GetDefensive()
{
    game->AddPiece(7 * 15 + 7);
    return (7 * 15 + 7);
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD ul_reason_for_call, PVOID pvReserved)
{
    switch (ul_reason_for_call)
    {

    case DLL_PROCESS_ATTACH:

        //�����DLL��ӳ�䵽�˽��̵ĵ�ַ�ռ�ʱ
        break;

    case DLL_THREAD_ATTACH:

        //һ���߳����ڱ�����
        break;

    case DLL_THREAD_DETACH:

        //�߳��ս�
        break;

    case DLL_PROCESS_DETACH:

        //���DLL�ӽ��̵ĵ�ַ�ռ��н��ӳ��
        if (game != NULL) {
            delete game;
        }
        break;

    }
    return TRUE;
}