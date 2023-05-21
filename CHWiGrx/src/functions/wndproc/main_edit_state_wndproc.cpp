#include "../../declarations.hpp"
#include "wndproc.h"

LRESULT CALLBACK mainproc::main_edit_state_wndproc(
    const HWND h_wnd,
    const UINT message,
    const WPARAM w_param,
    const LPARAM l_param
) noexcept
{
    static PAINTSTRUCT ps{};
    static HBITMAP hbm_mem{};
    static HGDIOBJ h_old{};
    static HDC hdc_mem{};
    static HDC hdc{};

    switch (message) {
        case WM_COMMAND:
        {
            switch (const int wm_id = LOWORD(w_param)) {
                case IDM_SET_GAME_WINDOW_MODE:
                    misc::set_window_mode(h_wnd, WindowState::Game);
                    break;

                case IDM_CLEAR_BOARD:
                {
                    mutables::board.reset(board_repr::BoardRepr({}, mutables::turn, mutables::board.get_idw()));
                    mutables::motion_input.clear();
                    InvalidateRect(h_wnd, nullptr, NULL);
                } break;

                case IDM_TOGGLE_LIST_WINDOW:
                    if (mutables::figures_list_window) {
                        destroy_window(mutables::figures_list_window);
                    }
                    else {
                        mutables::figures_list_window = misc::new_window::figures_list(h_wnd);
                    }

                    set_menu_checkbox(h_wnd, IDM_TOGGLE_LIST_WINDOW, mutables::figures_list_window != nullptr);
                    break;

                case IDM_WHITE_START:
                    mutables::turn = Color::White;
                    update_edit_menu_variables(h_wnd);
                    break;

                case IDM_BLACK_START:
                    mutables::turn = Color::Black;
                    update_edit_menu_variables(h_wnd);
                    break;

                case IDM_IDW_TRUE:
                    mutables::board.set_idw(true);
                    update_edit_menu_variables(h_wnd);
                    break;

                case IDM_IDW_FALSE:
                    mutables::board.set_idw(false);
                    update_edit_menu_variables(h_wnd);
                    break;

                default:
                    return DefWindowProc(h_wnd, message, w_param, l_param);
            }
        } break;

        case WM_LBUTTONUP:
            on_lbutton_up(
                h_wnd, w_param, l_param, mutables::main_window.divide_by_cell_size(l_param).change_axes(), false
            );
            InvalidateRect(h_wnd, nullptr, NULL);
            break;

        case WM_MOUSEMOVE:
            if (!mutables::motion_input.is_active_by_click() && mutables::motion_input.is_drags()) {
                mutables::motion_input.init_curr_choice_window(
                    h_wnd,
                    curr_choice_edit_mode_wndproc,
                    Pos{ mutables::main_window.get_cell_width(), mutables::main_window.get_cell_height() }
                );
            }
            break;

        case WM_PAINT:
        {
            hdc = BeginPaint(h_wnd, &ps);

            hdc_mem = CreateCompatibleDC(hdc);
            hbm_mem =
                CreateCompatibleBitmap(hdc, mutables::main_window.get_width(), mutables::main_window.get_height());
            h_old = SelectObject(hdc_mem, hbm_mem);

            draw_board(hdc_mem);
            draw_input(hdc_mem, mutables::motion_input.get_input());
            draw_figures_on_board(hdc_mem);

            // Copying the temporary buffer to the main one
            BitBlt(
                hdc, 0, 0, mutables::main_window.get_width(), mutables::main_window.get_height(), hdc_mem, 0, 0, SRCCOPY
            );

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
