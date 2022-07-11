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

    if (!prepare_window(hInstance, nCmdShow, IDS_APP_TITLE, IDC_CHWIGRX,
        {
            .cbSize = sizeof(WNDCLASSEX),
            .style = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc = main_proc,
            .cbClsExtra = 0,
            .cbWndExtra = 0,
            .hInstance = hInstance,
            .hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHWIGRX)),
            .hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR)),
            .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
            .hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL))
        })) {
        return FALSE;
    }

    return window_loop(hInstance);
}
