#include "declarations.hpp"

int APIENTRY wWinMain(const HINSTANCE hInstance,
                      [[maybe_unused]] const HINSTANCE hPrevInstance,
                      [[maybe_unused]] const LPWSTR lpCmdLine,
                      int nCmdShow)
{
    {
        pieces_bitmaps['B']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_PAWN));
        pieces_bitmaps['B']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_ROOK));
        pieces_bitmaps['B']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KING));
        pieces_bitmaps['B']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_QUEEN));
        pieces_bitmaps['B']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_BISHOP));
        pieces_bitmaps['B']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KNIGHT));
        pieces_bitmaps['W']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_PAWN));
        pieces_bitmaps['W']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_ROOK));
        pieces_bitmaps['W']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KING));
        pieces_bitmaps['W']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_QUEEN));
        pieces_bitmaps['W']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_BISHOP));
        pieces_bitmaps['W']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KNIGHT));
    }

    {
        other_bitmaps["star"] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_STAR));
    }

    #ifdef _DEBUG
    if (AllocConsole()) {
        FILE* tmp;
        freopen_s(&tmp, "conin$", "r", stdin);
        freopen_s(&tmp, "conout$", "w", stdout);
        freopen_s(&tmp, "conout$", "w", stderr);
    }
    #endif // _DEBUG
    
    if (!prepare_window(hInstance, nCmdShow, IDS_APP_TITLE, IDC_CHWIGRX,
        {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = main_default_wndproc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = hInstance,
            .hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME_MODE_BIG)),
            .hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR)),
            .hbrBackground = reinterpret_cast<HBRUSH>((COLOR_WINDOW)),
            .hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL))
        })) {
        return FALSE;
    }

    return window_loop(hInstance);
}
