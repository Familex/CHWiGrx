#include "../declarations.hpp"

template <bool check_moves_validity>
LRESULT CALLBACK curr_choice_wndproc<check_moves_validity>(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static const int TO_DESTROY_TIMER_ID{ MAIN_WINDOW_CHOICE_TO_DESTROY_TIMER_ID };
    switch (uMsg) {
        case WM_CREATE:
            SetTimer(hWnd, TO_DESTROY_TIMER_ID, TO_DESTROY_ELAPSE_DEFAULT_IN_MS, NULL);
            break;
        case WM_TIMER:
            if (wParam == TO_DESTROY_TIMER_ID) {
                KillTimer(hWnd, TO_DESTROY_TIMER_ID);
                SendMessage(hWnd, WM_EXITSIZEMOVE, NULL, NULL);
            }
            break;
        case WM_ENTERSIZEMOVE: // Фигуру начали передвигать
            KillTimer(hWnd, TO_DESTROY_TIMER_ID);
            break;
        case WM_EXITSIZEMOVE: // Фигуру отпустил
        {
            HWND parent = GetParent(hWnd);
            POINT cur_pos{};
            GetCursorPos(&cur_pos);
            Pos where_fig = main_window.get_figure_under_mouse(cur_pos);
            if (!check_moves_validity && !is_valid_coords(where_fig)) {
                // Вынесли фигуру за пределы доски без проверки валидности => удаляем с доски
                board.delete_fig(
                    reinterpret_cast<Figure*>(GetWindowLongPtr(hWnd, GWLP_USERDATA))->get_pos()
                );
                motion_input.clear();
            }
            else {
                on_lbutton_up(parent, wParam, lParam, where_fig, check_moves_validity);
            }
            InvalidateRect(parent, NULL, NULL);
            DestroyWindow(hWnd);
        }
        break;
        case WM_NCHITTEST:
            // При перехвате нажатий мыши симулируем перетаскивание
            return (LRESULT)HTCAPTION;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            Figure* in_hand = (Figure*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (in_hand) {
                draw_figure(hdc, in_hand, Pos(0, 0), false);
            }
            EndPaint(hWnd, &ps);
        }
        break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return static_cast<LRESULT>(0);
}

template LRESULT CALLBACK curr_choice_wndproc<true>(HWND, UINT, WPARAM, LPARAM);
template LRESULT CALLBACK curr_choice_wndproc<false>(HWND, UINT, WPARAM, LPARAM);
