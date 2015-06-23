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

// 悔棋
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

        //当这个DLL被映射到了进程的地址空间时
        break;

    case DLL_THREAD_ATTACH:

        //一个线程正在被创建
        break;

    case DLL_THREAD_DETACH:

        //线程终结
        break;

    case DLL_PROCESS_DETACH:

        //这个DLL从进程的地址空间中解除映射
        if (game != NULL) {
            delete game;
        }
        break;

    }
    return TRUE;
}