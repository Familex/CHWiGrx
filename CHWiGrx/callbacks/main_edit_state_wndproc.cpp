#include "../declarations.hpp"

LRESULT CALLBACK mainproc::main_edit_state_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT ps{};
    static HBITMAP hbmMem{};
    static HGDIOBJ hOld{};
    static HDC hdcMem{};
    static HDC hdc{};
    
    switch (message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case IDM_SET_GAME_WINDOW_MODE:
                {
                    window_state = WindowState::GAME;
                    // set icon
                    auto hBigGameIcon = LoadImage(hInst, MAKEINTRESOURCE(IDI_GAME_MODE_BIG), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
                    auto hMiniGameIcon = LoadImage(hInst, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
                    SendMessage(hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hBigGameIcon));
                    SendMessage(hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hMiniGameIcon));

                    DestroyWindow(figures_list_window);
                    SetMenu(hWnd, LoadMenu(hInst, MAKEINTRESOURCE(IDC_CHWIGRX)));
                    change_checkerboard_color_theme(hWnd);
                }
                    break;

                case IDM_CLEAR_BOARD:
                {
                    board.reset(BoardRepr({}, turn, board.get_idw()));
                    motion_input.clear();
                    InvalidateRect(hWnd, NULL, NULL);
                }
                    break;
                    
                case IDM_TOGGLE_LIST_WINDOW:
                    if (figures_list_window == NULL) {
                        figures_list_window = create_figures_list_window(hWnd);
                    }
                    else {
                        DestroyWindow(figures_list_window);
                        figures_list_window = NULL;
                    }
                    set_menu_checkbox(hWnd, IDM_TOGGLE_LIST_WINDOW, figures_list_window != NULL);
                    break;
                    
                case IDM_WHITE_START:
                    turn = Color::White;
                    update_edit_menu_variables(hWnd);
                    break;

                case IDM_BLACK_START:
                    turn = Color::Black;
                    update_edit_menu_variables(hWnd);
                    break;

                case IDM_IDW_TRUE:
                    board.set_idw(true);
                    update_edit_menu_variables(hWnd);
                    break;

                case IDM_IDW_FALSE:
                    board.set_idw(false);
                    update_edit_menu_variables(hWnd);
                    break;
                    
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
            break;
            
        case WM_LBUTTONUP:
            on_lbutton_up(hWnd, wParam, lParam,
                main_window.divide_by_cell_size(lParam).change_axes(), 
                false
            );
            InvalidateRect(hWnd, NULL, NULL);
            break;

        case WM_MOUSEMOVE:
            if (!motion_input.is_active_by_click() && motion_input.is_drags()) {
                motion_input.init_curr_choice_window(hWnd, curr_choice_wndproc<false>);
            }
            break;

        case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            
            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc,
                main_window.get_width(),
                main_window.get_height()
            );
            hOld = SelectObject(hdcMem, hbmMem);

            draw_board(hdcMem);
            draw_input(hdcMem, motion_input.get_input());
            draw_figures_on_board(hdcMem);

            // Копирование временного буфера в основной
            BitBlt(hdc, 0, 0,
                main_window.get_width(),
                main_window.get_height(),
                hdcMem, 0, 0, SRCCOPY
            );

            SelectObject(hdcMem, hOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hWnd, &ps);
        }
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return static_cast<LRESULT>(0);
}
