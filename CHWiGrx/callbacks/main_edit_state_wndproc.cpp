#include "../declarations.hpp"

LRESULT CALLBACK
mainproc::main_edit_state_wndproc(const HWND h_wnd, const UINT message, const WPARAM w_param, const LPARAM l_param)
{
    static PAINTSTRUCT ps {};
    static HBITMAP hbm_mem {};
    static HGDIOBJ h_old {};
    static HDC hdc_mem {};
    static HDC hdc {};

    switch (message) {
        case WM_COMMAND:
        {
            switch (const int wm_id = LOWORD(w_param)) {
                case IDM_SET_GAME_WINDOW_MODE:
                {
                    window_state = WindowState::Game;
                    // set icon
                    auto h_big_game_icon = LoadImage(
                        h_inst, MAKEINTRESOURCE(IDI_GAME_MODE_BIG), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED
                    );
                    auto h_mini_game_icon = LoadImage(
                        h_inst, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED
                    );
                    SendMessage(h_wnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(h_big_game_icon));
                    SendMessage(h_wnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(h_mini_game_icon));

                    DestroyWindow(figures_list_window);
                    SetMenu(h_wnd, LoadMenu(h_inst, MAKEINTRESOURCE(IDC_CHWIGRX)));
                    change_checkerboard_color_theme(h_wnd);
                    update_bot_menu_variables(h_wnd);
                } break;

                case IDM_CLEAR_BOARD:
                {
                    board.reset(board_repr::BoardRepr({}, turn, board.get_idw()));
                    motion_input.clear();
                    InvalidateRect(h_wnd, nullptr, NULL);
                } break;

                case IDM_TOGGLE_LIST_WINDOW:
                    if (figures_list_window == nullptr) {
                        figures_list_window = create_figures_list_window(h_wnd);
                    }
                    else {
                        DestroyWindow(figures_list_window);
                        figures_list_window = nullptr;
                    }
                    set_menu_checkbox(h_wnd, IDM_TOGGLE_LIST_WINDOW, figures_list_window != nullptr);
                    break;

                case IDM_WHITE_START:
                    turn = Color::White;
                    update_edit_menu_variables(h_wnd);
                    break;

                case IDM_BLACK_START:
                    turn = Color::Black;
                    update_edit_menu_variables(h_wnd);
                    break;

                case IDM_IDW_TRUE:
                    board.set_idw(true);
                    update_edit_menu_variables(h_wnd);
                    break;

                case IDM_IDW_FALSE:
                    board.set_idw(false);
                    update_edit_menu_variables(h_wnd);
                    break;

                default:
                    return DefWindowProc(h_wnd, message, w_param, l_param);
            }
        } break;

        case WM_LBUTTONUP:
            on_lbutton_up(h_wnd, w_param, l_param, main_window.divide_by_cell_size(l_param).change_axes(), false);
            InvalidateRect(h_wnd, nullptr, NULL);
            break;

        case WM_MOUSEMOVE:
            if (!motion_input.is_active_by_click() && motion_input.is_drags()) {
                motion_input.init_curr_choice_window(h_wnd, curr_choice_wndproc<false>);
            }
            break;

        case WM_PAINT:
        {
            hdc = BeginPaint(h_wnd, &ps);

            hdc_mem = CreateCompatibleDC(hdc);
            hbm_mem = CreateCompatibleBitmap(hdc, main_window.get_width(), main_window.get_height());
            h_old = SelectObject(hdc_mem, hbm_mem);

            draw_board(hdc_mem);
            draw_input(hdc_mem, motion_input.get_input());
            draw_figures_on_board(hdc_mem);

            // Копирование временного буфера в основной
            BitBlt(hdc, 0, 0, main_window.get_width(), main_window.get_height(), hdc_mem, 0, 0, SRCCOPY);

            SelectObject(hdc_mem, h_old);
            DeleteObject(hbm_mem);
            DeleteDC(hdc_mem);

            EndPaint(h_wnd, &ps);
        } break;

        default:
            return DefWindowProc(h_wnd, message, w_param, l_param);
    }
    return static_cast<LRESULT>(0);
}
