#include "../../declarations.hpp"
#include "wndproc.h"

LRESULT CALLBACK curr_choice_figures_list_wndproc(
    const HWND h_wnd,
    const UINT u_msg,
    const WPARAM w_param,
    const LPARAM l_param
) noexcept
{
    // выбранная фигура временная => удаляется при закрытии окна
    static constexpr int TO_DESTROY_TIMER_ID { FIGURES_LIST_CHOICE_TO_DESTROY_TIMER_ID };
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
        case WM_ENTERSIZEMOVE:
            KillTimer(h_wnd, TO_DESTROY_TIMER_ID);
            break;
        case WM_EXITSIZEMOVE:    // Фигуру отпустил
        {
            const HWND hmain_window = GetWindow(GetParent(h_wnd), GW_OWNER);
            POINT cur_pos {};
            GetCursorPos(&cur_pos);
            RECT figures_list;
            GetWindowRect(GetParent(h_wnd), &figures_list);

            if (const Pos where_fig = main_window.get_figure_under_mouse(cur_pos);
                is_valid_coords(where_fig) && !(figures_list.top <= cur_pos.y && cur_pos.y <= figures_list.bottom &&
                                                figures_list.left <= cur_pos.x && cur_pos.x <= figures_list.right))
            {
                const auto to_place = reinterpret_cast<Figure*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA));
                to_place->move_to(where_fig);
                board.place_fig(to_place);
                if (to_place->is(FigureType::Rook)) {
                    board.on_castling(to_place->get_id());
                }
                InvalidateRect(hmain_window, nullptr, NULL);
            }
            else {
                delete reinterpret_cast<Figure*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA));
            }
            motion_input.clear_hand();
            motion_input.clear();
            DestroyWindow(h_wnd);
        } break;
        case WM_NCHITTEST:    // При перехвате нажатий мыши симулируем перетаскивание
            return (LRESULT)HTCAPTION;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            const HDC hdc = BeginPaint(h_wnd, &ps);

            if (const auto* in_hand = reinterpret_cast<Figure*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA)); in_hand) {
                draw_figure(hdc, in_hand, Pos(0, 0), false);
            }
            EndPaint(h_wnd, &ps);
        } break;
        default:
            return DefWindowProc(h_wnd, u_msg, w_param, l_param);
    }
    return static_cast<LRESULT>(0);
}
