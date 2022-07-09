#include "declarations.hpp"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    load_pieces_bitmaps(hInstance);

    #ifdef ALLOCATE_CONSOLE
    if (AllocConsole()) {
        FILE* tmp;
        freopen_s(&tmp, "conin$", "r", stdin);
        freopen_s(&tmp, "conout$", "w", stdout);
        freopen_s(&tmp, "conout$", "w", stderr);
    }
    #endif // ALLOCATE_CONSOLE

    if (!prepare_main_window(hInstance, nCmdShow)) {
        return FALSE;
    }
    
    return main_window_loop(hInstance);
}
