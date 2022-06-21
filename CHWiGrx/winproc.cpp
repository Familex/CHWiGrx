#include "declarations.hpp"

LRESULT CALLBACK main_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT ps;
    static HBITMAP hbmMem;
    static HGDIOBJ hOld;
    static HDC hdcMem;
    static HDC hdc;
    switch (message) {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_UNDO:
                if (board.undo_move()) {
                    motion_input.clear();
                    turn.to_next();
                }
                break;
            case IDM_RESTORE_MOVE:
                if (board.restore_move()) {
                    motion_input.clear();
                    turn.to_next();
                }
                break;
            case IDM_COPY_MAP:
                copy_repr_to_clip();
                break;
            case IDM_PASTE_MAP:
                do {
                    std::string board_repr_str = take_str_from_clip();
                    if (!is_legal_board_repr(board_repr_str)) break;
                    BoardRepr board_repr(board_repr_str);
                    turn = board_repr.get_turn();
                    board.reset(board_repr);
                    motion_input.clear();
                } while (0);
                break;
            case IDM_PASTE_START_MAP:
            {
                std::string board_repr_aspt = take_str_from_clip();
                if (!is_legal_board_repr(board_repr_aspt)) break;
                start_board_repr = board_repr_aspt;
            }
            break;
            case IDM_RESET_START_MAP_IDW:
                start_board_repr = { DEFAULT_CHESS_BOARD_IDW };
                restart();
                break;
            case IDM_RESET_START_MAP_NIDW:
                start_board_repr = { DEFAULT_CHESS_BOARD_NIDW };
                restart();
                break;
            case IDM_RESTART:
                restart();
                break;
            case IDM_TOGGLE_SAVE_ALL_MOVES:
            {
                HMENU hMenu = GetMenu(hWnd);
                MENUITEMINFO item_info;
                ZeroMemory(&item_info, sizeof(item_info));
                item_info.cbSize = sizeof(item_info);
                item_info.fMask = MIIM_STATE;
                if (!GetMenuItemInfo(hMenu, IDM_TOGGLE_SAVE_ALL_MOVES, FALSE, &item_info)) return 0;
                if (item_info.fState == MFS_CHECKED) {
                    save_all_moves = false;
                    item_info.fState = MFS_UNCHECKED;
                }
                else {
                    save_all_moves = true;
                    item_info.fState = MFS_CHECKED;
                }
                SetMenuItemInfoW(hMenu, IDM_TOGGLE_SAVE_ALL_MOVES, FALSE, &item_info);
            }
            break;
            case IDM_SET_CHOICE_TO_ROOK: case IDM_SET_CHOICE_TO_KNIGHT: case IDM_SET_CHOICE_TO_QUEEN: case IDM_SET_CHOICE_TO_BISHOP:
            {
                UINT item_menu_to_set_id = wmId;
                UINT curr_item_menu_state =
                    chose == 'Q' ? IDM_SET_CHOICE_TO_QUEEN
                    : chose == 'R' ? IDM_SET_CHOICE_TO_ROOK
                    : chose == 'H' ? IDM_SET_CHOICE_TO_KNIGHT
                    : chose == 'B' ? IDM_SET_CHOICE_TO_BISHOP
                    : NULL;
                set_menu_checkbox(hWnd, curr_item_menu_state, false);
                set_menu_checkbox(hWnd, item_menu_to_set_id, true);
                chose = wmId == IDM_SET_CHOICE_TO_ROOK ? 'R'
                    : wmId == IDM_SET_CHOICE_TO_KNIGHT ? 'H'
                    : wmId == IDM_SET_CHOICE_TO_QUEEN ? 'Q'
                    : wmId == IDM_SET_CHOICE_TO_BISHOP ? 'B'
                    : 'N';
            }
            break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, about_proc);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        update_check_title(hWnd);   // Для надёжности обновлю на все исходы
        InvalidateRect(hWnd, NULL, NULL);
        break;
    case WM_KEYDOWN:
    {
        int cord{ -1 };
        switch (wParam) {
        case VK_0: case VK_NUMPAD0: cord = 0; break;
        case VK_1: case VK_NUMPAD1: cord = 1; break;
        case VK_2: case VK_NUMPAD2: cord = 2; break;
        case VK_3: case VK_NUMPAD3: cord = 3; break;
        case VK_4: case VK_NUMPAD4: cord = 4; break;
        case VK_5: case VK_NUMPAD5: cord = 5; break;
        case VK_6: case VK_NUMPAD6: cord = 6; break;
        case VK_7: case VK_NUMPAD7: cord = 7; break;
        case VK_8: case VK_NUMPAD8: cord = 8; break;
        case VK_9: case VK_NUMPAD9: cord = 9; break;
        case VK_ESCAPE:
            motion_input.clear();
            InvalidateRect(hWnd, NULL, NULL);
            return 0;
        case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
        {
            pos shift{ wParam == VK_LEFT ? pos(0, -1) : (wParam == VK_RIGHT ? pos(0, 1) : (wParam == VK_UP ? pos(-1, 0) : pos(1, 0))) };
            if (!motion_input.is_pair())
                motion_input.shift_from(shift, HEIGHT, WIDTH);
            else
                motion_input.shift_target(shift, HEIGHT, WIDTH);
            InvalidateRect(hWnd, NULL, NULL);
            return 0;
        }
        case VK_RETURN:
            if (motion_input.is_pair()) {
                make_move(hWnd);
                motion_input.clear();
                InvalidateRect(hWnd, NULL, NULL);
                return 0;
            }
            else {
                motion_input.prepare(turn);
            }
            motion_input.toggle_pair_input();
            InvalidateRect(hWnd, NULL, NULL);
            return 0;
        default:
            return 0;
        }
        switch (motion_input.get_single_state()) {
        case 0:
            motion_input.set_from_x(cord);
            motion_input.next_single();
            break;
        case 1:
            motion_input.set_from_y(cord);
            motion_input.prepare(turn);
            motion_input.activate_pair();
            motion_input.next_single();
            break;
        case 2:
            motion_input.set_target_x(cord);
            motion_input.next_single();
            break;
        case 3:
            motion_input.set_target_y(cord);
            make_move(hWnd);
            motion_input.reset_input_order();
            break;
        }
        InvalidateRect(hWnd, NULL, NULL);
        break;
    }
    case WM_RBUTTONDOWN:
        motion_input.clear();
        InvalidateRect(hWnd, NULL, NULL);
        #ifdef ALLOCATE_CONSOLE
            std::cout << "Curr board: " << board.get_repr(true).as_string() << '\n';
        #endif // ALLOCATE_CONSOLE
        break;
    case WM_LBUTTONDOWN:
        {
            motion_input.set_lbutton_down();
            window_stats.set_prev_lbutton_click({ HIWORD(lParam), LOWORD(lParam) });
            motion_input.reset_single();
            if (motion_input.is_pair()) {
                motion_input.set_target(window_stats.divide_by_cell_size(HIWORD(lParam), LOWORD(lParam)));
                InvalidateRect(hWnd, NULL, NULL);
                return 0;
            }
            pos from = window_stats.divide_by_cell_size(HIWORD(lParam), LOWORD(lParam));
            if (board.cont_fig(from)) {
                motion_input.set_from(from);
                motion_input.prepare(turn);
            }
            InvalidateRect(hWnd, NULL, NULL);
        }
        break;
    case WM_LBUTTONUP:
        on_lbutton_up(hWnd, wParam, lParam, window_stats.divide_by_cell_size(HIWORD(lParam), LOWORD(lParam)));
        break;
    case WM_MOVE:
        window_stats.set_window_pos(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_MOUSEMOVE:
        if (motion_input.is_drags()) {
            motion_input.init_curr_choice_window(hWnd);
        }
        break;
    case WM_SIZE:
        window_stats.set_window_size(HIWORD(lParam), LOWORD(lParam));
        InvalidateRect(hWnd, NULL, NULL);
        break;
    case WM_PAINT:
    {
        Input input = motion_input.get_input();
        hdc = BeginPaint(hWnd, &ps);
        hdcMem = CreateCompatibleDC(hdc);
        hbmMem = CreateCompatibleBitmap(hdc,
            window_stats.get_window_width(),
            window_stats.get_window_height()
        );
        hOld = SelectObject(hdcMem, hbmMem);

        {
            /* Фоновый рисунок */
            for (int i{}; i < HEIGHT; ++i) {
                for (int j{}; j < WIDTH; ++j) {
                    static const HBRUSH CHECKERBOARDBRIGHT{ CreateSolidBrush(RGB(50, 50, 50)) };
                    static const HBRUSH CHECKERBOARDDARK{ CreateSolidBrush(RGB(128, 128, 128)) };
                    const RECT cell = window_stats.get_cell(i, j);
                    if ((i + j) % 2) {
                        FillRect(hdcMem, &cell, CHECKERBOARDBRIGHT);
                    }
                    else {
                        FillRect(hdcMem, &cell, CHECKERBOARDDARK);
                    }
                }
            }
        }

        {
            /* Возможные ходы текущей фигуры */
            for (const auto& [is_eat, move_pos] : motion_input.get_possible_moves()) {
                static const HBRUSH GREEN{ CreateSolidBrush(RGB(0, 255, 0)) };
                static const HBRUSH DARK_GREEN{ CreateSolidBrush(RGB(0, 150, 0)) };
                const RECT cell = window_stats.get_cell(move_pos);
                if (is_eat) {
                    FillRect(hdcMem, &cell, DARK_GREEN);
                }
                else {
                    FillRect(hdcMem, &cell, GREEN);
                }
            }
        }

        {
            /* Положение курсора и выделенной клетки */
            static const HBRUSH RED{ CreateSolidBrush(RGB(255, 0, 0)) };
            static const HBRUSH BLUE{ CreateSolidBrush(RGB(0, 0, 255)) };
            const RECT from_cell = window_stats.get_cell(input.from);
            const RECT targ_cell = window_stats.get_cell(input.target);
            FillRect(hdcMem, &from_cell, RED);
            FillRect(hdcMem, &targ_cell, BLUE);
        }

        {
            /* Фигуры на поле */
            for (const auto& figure : board.all_figures()) {
                if (!motion_input.is_figure_dragged(figure->get_id())) {
                    draw_figure(hdcMem, figure);
                }
            }
        }

        /* Копирование временного буфера в основной */
        BitBlt(hdc, 0, 0,
            window_stats.get_window_width(),
            window_stats.get_window_height(),
            hdcMem, 0, 0, SRCCOPY
        );

        SelectObject(hdcMem, hOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
