#include "../declarations.hpp"

LRESULT CALLBACK moves_list_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SCROLLINFO si{ .cbSize = sizeof(SCROLLINFO) };
    static PAINTSTRUCT ps;
    static HBITMAP hbmMem;
    static HGDIOBJ hOld;
    static HDC hdcMem;
    static HDC hdc;
    static std::vector<MoveRec> display_move_recs;
    static bool is_resizes = true;  // also to horizontal scrollbar
    static bool is_scrolls = false;

    switch (uMsg) 
    {
        default:
            break;
    }
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}