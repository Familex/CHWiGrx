#include "../declarations.hpp"

#include <codecvt>

LRESULT CALLBACK main_default_wndproc(const HWND h_wnd, const UINT message, const WPARAM w_param, const LPARAM l_param)
{
    switch (message) {
        case WM_COMMAND:
        {
            switch (const int wm_id = LOWORD(w_param); wm_id) {
                case IDM_COPY_MAP:
                    copy_repr_to_clip(h_wnd);
                    break;

                case IDM_PASTE_MAP:
                {
                    if (auto board_repr_sus = take_repr_from_clip(h_wnd); board_repr_sus.has_value()) {
                        turn = board_repr_sus.value().value.turn;
                        board.reset(std::move(board_repr_sus.value().value));
                        motion_input.clear();
                    }
                    else {
                        const std::wstring error_message { parse_error_type_as_wstring(board_repr_sus.error().type) };
                        MessageBox(h_wnd, error_message.c_str(), L"Board repr parse error", MB_OK);

                        /* Debug print */ {
                            const std::string error_message_utf8 {
                                parse_error_type_as_string(board_repr_sus.error().type)
                            };
                            debug_print("Error:", error_message_utf8);
                            debug_print("\tBoard:", AsString<board_repr::BoardRepr> {}(board.get_repr(turn, true)));
                            debug_print("\tPos:", board_repr_sus.error().position);
                        }
                    }
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
                    // нужно ли чистить память?
                    DestroyWindow(h_wnd);
                    break;

                default:
                    break;
            }
        } break;

        case WM_CREATE:
            update_main_window_title(h_wnd);
            update_game_menu_variables(h_wnd);
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

    return static_cast<LRESULT>(0);
}
