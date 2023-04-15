#include "../declarations.hpp"

#include <codecvt>

LRESULT CALLBACK main_default_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) 
    {
        case WM_COMMAND:
        {
            switch (int wmId = LOWORD(wParam); wmId)
            {
                case IDM_COPY_MAP:
                    copy_repr_to_clip(hWnd);
                    break;
                
                case IDM_PASTE_MAP:
                {
                    auto board_repr_sus = take_repr_from_clip(hWnd);
                    if (board_repr_sus.has_value()) {
                        turn = board_repr_sus.value().value.turn;
                        board.reset(std::move(board_repr_sus.value().value));
                        motion_input.clear();
                    }
                    else {
                        std::wstring error_message{ parse_error_type_as_wstring(board_repr_sus.error().type ) };
                        MessageBox(hWnd, error_message.c_str(), L"Board repr parse error", MB_OK);

                        /* Debug print */ {
                            std::string error_message_utf8{ parse_error_type_as_string(board_repr_sus.error().type) };
                            debug_print("Error:", error_message_utf8);
                            debug_print("\tBoard:", as_string<board_repr::BoardRepr>{}(board.get_repr(turn, true)));
                            debug_print("\tPos:", board_repr_sus.error().position);
                        }
                    }
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
