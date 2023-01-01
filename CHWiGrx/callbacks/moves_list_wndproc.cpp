#include "../declarations.hpp"

LRESULT CALLBACK moves_list_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static std::vector<MoveRec> display_prev_move_recs;
    static std::vector<MoveRec> display_future_move_recs;

    switch (uMsg) 
    {
        

        default:
            break;
    }
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}