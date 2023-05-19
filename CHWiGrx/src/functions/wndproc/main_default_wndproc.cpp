#include "../../declarations.hpp"
#include "wndproc.h"

#include <codecvt>
#include <fstream>
#include <sstream>

void paste_board_repr(HWND wnd, std::string_view board_repr) noexcept;

LRESULT CALLBACK
main_default_wndproc(const HWND h_wnd, const UINT message, const WPARAM w_param, const LPARAM l_param) noexcept
{
    switch (message) {
        case WM_COMMAND:
        {
            switch (const int wm_id = LOWORD(w_param)) {
                case IDM_COPY_MAP:
                    copy_repr_to_clip(h_wnd);
                    break;

                case IDM_PASTE_MAP:
                {
                    paste_board_repr(h_wnd, misc::take_str_from_clip(h_wnd));
                    misc::on_game_board_change(board);
                }
                    InvalidateRect(h_wnd, nullptr, NULL);
                    break;

                case IDM_WINDOW_MAKEQUAD:
                {
                    RECT rect;
                    GetWindowRect(h_wnd, &rect);

                    const int quad_side = (rect.right - rect.left + rect.bottom - rect.top) / 2;
                    main_window.set_size(quad_side, quad_side + HEADER_HEIGHT);
                    SetWindowPos(h_wnd, nullptr, 0, 0, quad_side, quad_side + HEADER_HEIGHT, SWP_NOZORDER | SWP_NOMOVE);
                } break;

                case IDM_EXIT:
                    // Is it necessary to free memory?
                    DestroyWindow(h_wnd);
                    break;

                default:
                    break;
            }
        } break;

        case WM_CREATE:
            update_main_window_title(h_wnd);
            update_game_menu_variables(h_wnd);
            PostMessage(h_wnd, WM_COMMAND, IDM_WINDOW_MAKEQUAD, NULL);
            break;

        case WM_DROPFILES:
        {
            constexpr auto null_terminator_size = 1;
            const auto h_drop = reinterpret_cast<HDROP>(w_param);
            const auto file_size = DragQueryFile(h_drop, 0, nullptr, 0) + null_terminator_size;
            const auto file_name = new TCHAR[file_size];
            if (!DragQueryFile(h_drop, 0, file_name, file_size)) {
                debug_print("DragQueryFile", GetLastError());
            }
            debug_print("Dragged file:", file_name);
            {
                const std::ifstream file{ file_name };
                std::stringstream ss;
                ss << file.rdbuf();
                paste_board_repr(h_wnd, ss.str());
            }
            delete[] file_name;
        }
            InvalidateRect(h_wnd, nullptr, NULL);
            break;

        case WM_LBUTTONDOWN:
            on_lbutton_down(h_wnd, l_param);
            InvalidateRect(h_wnd, nullptr, NULL);
            break;

        case WM_MOVE:
            main_window.set_pos(l_param);
            break;

        case WM_SIZE:
            main_window.set_size(l_param);
            InvalidateRect(h_wnd, nullptr, NULL);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            break;
    }

    switch (window_state) {
        case WindowState::Game:
            return mainproc::main_game_state_wndproc(h_wnd, message, w_param, l_param);
        case WindowState::Edit:
            return mainproc::main_edit_state_wndproc(h_wnd, message, w_param, l_param);
    }

    return 0;
}

void paste_board_repr(const HWND wnd, const std::string_view board_repr) noexcept
{
    if (auto board_repr_sus = FromString<board_repr::BoardRepr>{}(board_repr)) {
        turn = board_repr_sus->value.turn;
        board.reset(std::move(board_repr_sus->value));
        motion_input.clear();
        misc::on_game_board_change(board);
    }
    else {
        const auto& [type, position] = board_repr_sus.error();
        const std::wstring error_message{ parse_error_type_as_wstring(type) };
        MessageBox(wnd, error_message.c_str(), L"Board repr parse error", MB_OK);

        /* Debug print */ {
            const std::string error_message_utf8{ parse_error_type_as_string(type) };
            debug_print("Error:", error_message_utf8);
            debug_print("\tBoard:", AsString<board_repr::BoardRepr>{}(board.get_repr(turn, true)));
            debug_print("\tPos:", position);
        }
    }
}
