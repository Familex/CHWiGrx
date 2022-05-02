#include "declarations.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    constexpr auto MAX_LOADSTRING = 100;    // Похоже, от этого не избавиться

    WCHAR szTitle[MAX_LOADSTRING];
    WCHAR szWindowClass[MAX_LOADSTRING];

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CHWIGRX, szWindowClass, MAX_LOADSTRING);
    register_main_window_class(hInstance, szTitle, szWindowClass);

    pieces_bitmaps['B']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_PAWN   ));
    pieces_bitmaps['B']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_ROOK   ));
    pieces_bitmaps['B']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KING   ));
    pieces_bitmaps['B']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_QUEEN  ));
    pieces_bitmaps['B']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_BISHOP ));
    pieces_bitmaps['B']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KNIGHT ));
    pieces_bitmaps['W']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_PAWN   ));
    pieces_bitmaps['W']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_ROOK   ));
    pieces_bitmaps['W']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KING   ));
    pieces_bitmaps['W']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_QUEEN  ));
    pieces_bitmaps['W']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_BISHOP ));
    pieces_bitmaps['W']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KNIGHT ));

    board.get_fig({ 1, 1 })->submit_on({ 5, 5 });

    #ifdef ALLOCATE_CONSOLE
    if (AllocConsole()) {
        FILE* tmp;
        freopen_s(&tmp, "conin$", "r", stdin);
        freopen_s(&tmp, "conout$", "w", stdout);
        freopen_s(&tmp, "conout$", "w", stderr);
    }
    #endif // ALLOCATE_CONSOLE

    // Выполнить инициализацию приложения:
    if (!init_instance (hInstance, szTitle, szWindowClass, nCmdShow))
    {
        return FALSE;
    }
    
    // Цикл основного сообщения:
    MSG msg;
    HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHWIGRX));
    BOOL bRet = 0;
    while (bRet = GetMessage(&msg, nullptr, 0, 0)) {
        if (-1 == bRet) break;
        if (!TranslateAccelerator(msg.hwnd, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}
