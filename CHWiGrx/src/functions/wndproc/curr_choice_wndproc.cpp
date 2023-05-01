#include "../../declarations.hpp"
#include "wndproc.h"

namespace
{
template <bool CheckMovesValidity>
LRESULT CALLBACK
curr_choice_wndproc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) noexcept
{
    static constexpr int TO_DESTROY_TIMER_ID{ MAIN_WINDOW_CHOICE_TO_DESTROY_TIMER_ID };
    switch (u_msg) {
        case WM_CREATE:
            SetTimer(h_wnd, TO_DESTROY_TIMER_ID, TO_DESTROY_ELAPSE_DEFAULT_IN_MS, nullptr);
            break;
        case WM_TIMER:
            if (w_param == TO_DESTROY_TIMER_ID) {
                KillTimer(h_wnd, TO_DESTROY_TIMER_ID);
                SendMessage(h_wnd, WM_EXITSIZEMOVE, NULL, NULL);
            }
            break;
        case WM_ENTERSIZEMOVE:    // On figure start dragging
            KillTimer(h_wnd, TO_DESTROY_TIMER_ID);
            break;
        case WM_EXITSIZEMOVE:    // On figure release
        {
            const HWND parent = GetParent(h_wnd);
            POINT cur_pos{};
            GetCursorPos(&cur_pos);

            if (const Pos where_fig = main_window.get_figure_under_mouse(cur_pos);
                !CheckMovesValidity && !is_valid_coords(where_fig))
            {
                // Moved figure out of the board without checking validity => delete it from the board
                board.delete_fig(reinterpret_cast<Figure*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA))->get_pos());
                motion_input.clear();
            }
            else {
                on_lbutton_up(parent, w_param, l_param, where_fig, CheckMovesValidity);
            }
            InvalidateRect(parent, nullptr, NULL);
            DestroyWindow(h_wnd);
        } break;
        case WM_NCHITTEST:    // When intercepting mouse clicks, we simulate dragging.
            return (LRESULT)HTCAPTION;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            const HDC hdc = BeginPaint(h_wnd, &ps);
            if (const auto in_hand = reinterpret_cast<Figure*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA)); in_hand) {
                draw_figure(hdc, in_hand, Pos(0, 0), false);
            }
            EndPaint(h_wnd, &ps);
        } break;
        default:
            return DefWindowProc(h_wnd, u_msg, w_param, l_param);
    }
    return static_cast<LRESULT>(0);
}
}    // namespace

LRESULT CALLBACK
curr_choice_edit_mode_wndproc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) noexcept
{
    return curr_choice_wndproc<false>(h_wnd, u_msg, w_param, l_param);
}

LRESULT CALLBACK
curr_choice_game_mode_wndproc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) noexcept
{
    return curr_choice_wndproc<true>(h_wnd, u_msg, w_param, l_param);
}
