#include "../declarations.hpp"

LRESULT CALLBACK main_default_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) 
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case IDM_COPY_MAP:
                    copy_repr_to_clip();
                    break;
                
                case IDM_PASTE_MAP:
                {
                    std::string board_repr_str = take_str_from_clip();
                    if (!is_legal_board_repr(board_repr_str)) break;
                    BoardRepr board_repr(board_repr_str);
                    turn = board_repr.get_turn();
                    board.reset(board_repr);
                    motion_input.clear();
                }
                    InvalidateRect(hWnd, NULL, NULL);
                    break;

                case IDM_EXIT:
                    // нужно ли чистить память?
                    DestroyWindow(hWnd);
                    break;
            } 
        }
            break;
           
        case WM_LBUTTONDOWN:
            on_lbutton_down(hWnd, lParam);
            InvalidateRect(hWnd, NULL, NULL);
            break;

        case WM_MOVE:
            main_window.set_pos(lParam);
            break;
            
        case WM_SIZE:
            main_window.set_size(lParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    
    switch (window_state)
    {
        case WindowState::GAME:
            return mainproc::main_game_state_wndproc(hWnd, message, wParam, lParam);
        case WindowState::EDIT:
            return mainproc::main_edit_state_wndproc(hWnd, message, wParam, lParam);
        default:
            return static_cast<LRESULT>(0);
    }
}
