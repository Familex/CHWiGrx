#include "../declarations.hpp"

LRESULT CALLBACK main_default_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) 
    {
        case WM_COMMAND:
        {
            switch (int wmId = LOWORD(wParam); wmId)
            {
                case IDM_COPY_MAP:
                    copy_repr_to_clip();
                    break;
                
                case IDM_PASTE_MAP:
                {
                    std::string board_repr_str = take_str_from_clip();
                    if (!is_legal_board_repr(board_repr_str)) break;
                    BoardRepr board_repr(board_repr_str);
                    turn = board_repr.turn;
                    board.reset(std::move(board_repr));
                    motion_input.clear();
                }
                    InvalidateRect(hWnd, NULL, NULL);
                    break;

                case IDM_WINDOW_MAKEQUAD:
                {
                    RECT rect;
                    GetWindowRect(hWnd, &rect);
                    
                    int quad_side = (rect.right - rect.left + rect.bottom - rect.top) / 2;
                    main_window.set_size(quad_side, quad_side + HEADER_HEIGHT);
                    SetWindowPos(hWnd, NULL, 
                        0, 0, quad_side, quad_side + HEADER_HEIGHT, SWP_NOZORDER | SWP_NOMOVE);
                }
                    break;
                    
                case IDM_EXIT:
                    // нужно ли чистить память?
                    DestroyWindow(hWnd);
                    break;
            } 
        }
            break;
           
        case WM_CREATE:
            update_main_window_title(hWnd);
            update_game_menu_variables(hWnd);
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
