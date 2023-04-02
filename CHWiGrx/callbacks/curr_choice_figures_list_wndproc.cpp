#include "../declarations.hpp"

LRESULT CALLBACK curr_choice_figures_list_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // выбранная фигура временная => удаляется при закрытии окна
    static const int TO_DESTROY_TIMER_ID{ FIGURES_LIST_CHOICE_TO_DESTROY_TIMER_ID };
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
    case WM_ENTERSIZEMOVE:
        KillTimer(hWnd, TO_DESTROY_TIMER_ID);
        break;
    case WM_EXITSIZEMOVE: // Фигуру отпустил
    {
        HWND hmain_window = GetWindow(GetParent(hWnd), GW_OWNER);
        POINT cur_pos{};
        GetCursorPos(&cur_pos);
        RECT figures_list;
        GetWindowRect(GetParent(hWnd), &figures_list);
        
        if (Pos where_fig = main_window.get_figure_under_mouse(cur_pos); 
                is_valid_coords(where_fig) &&
                !(figures_list.top <= cur_pos.y && cur_pos.y <= figures_list.bottom &&
                    figures_list.left <= cur_pos.x && cur_pos.x <= figures_list.right)
            ) {
            auto to_place = reinterpret_cast<Figure*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
            to_place->move_to(where_fig);
            board.place_fig(to_place);
            if (to_place->is(FigureType::Rook)) {
                board.on_castling(to_place->get_id());
            }
            InvalidateRect(hmain_window, NULL, NULL);
        }
        else {
            delete reinterpret_cast<Figure*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        }
        motion_input.clear_hand();
        motion_input.clear();
        DestroyWindow(hWnd);
    }
    break;
    case WM_NCHITTEST:  // При перехвате нажатий мыши симулируем перетаскивание
        return (LRESULT)HTCAPTION;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
       
        if (Figure* in_hand = (Figure*)GetWindowLongPtr(hWnd, GWLP_USERDATA); 
                in_hand
            ) {
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
