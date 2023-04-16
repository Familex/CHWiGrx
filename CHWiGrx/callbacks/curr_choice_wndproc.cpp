#include "../declarations.hpp"

template <bool CheckMovesValidity>
LRESULT CALLBACK curr_choice_wndproc<CheckMovesValidity>(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) {
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
        case WM_ENTERSIZEMOVE: // Фигуру начали передвигать
            KillTimer(h_wnd, TO_DESTROY_TIMER_ID);
            break;
        case WM_EXITSIZEMOVE: // Фигуру отпустил
        {
            const HWND parent = GetParent(h_wnd);
            POINT cur_pos{};
            GetCursorPos(&cur_pos);
            
            if (const Pos where_fig = main_window.get_figure_under_mouse(cur_pos); 
                    !CheckMovesValidity && !is_valid_coords(where_fig)
                ) {
                // Вынесли фигуру за пределы доски без проверки валидности => удаляем с доски
                board.delete_fig(
                    reinterpret_cast<Figure*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA))->get_pos()
                );
                motion_input.clear();
            }
            else {
                on_lbutton_up(parent, w_param, l_param, where_fig, CheckMovesValidity);
            }
            InvalidateRect(parent, nullptr, NULL);
            DestroyWindow(h_wnd);
        }
        break;
        case WM_NCHITTEST:
            // При перехвате нажатий мыши симулируем перетаскивание
            return (LRESULT)HTCAPTION;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            const HDC hdc = BeginPaint(h_wnd, &ps);
            if (const auto in_hand = reinterpret_cast<Figure*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA)); 
                    in_hand
                ) {
                draw_figure(hdc, in_hand, Pos(0, 0), false);
            }
            EndPaint(h_wnd, &ps);
        }
        break;
        default:
            return DefWindowProc(h_wnd, u_msg, w_param, l_param);
    }
    return static_cast<LRESULT>(0);
}

template LRESULT CALLBACK curr_choice_wndproc<true>(HWND, UINT, WPARAM, LPARAM);
template LRESULT CALLBACK curr_choice_wndproc<false>(HWND, UINT, WPARAM, LPARAM);
