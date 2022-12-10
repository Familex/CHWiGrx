#include "../declarations.hpp"

LRESULT CALLBACK mainproc::main_game_state_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT ps{};
    static HBITMAP hbmMem{};
    static HGDIOBJ hOld{};
    static HDC hdcMem{};
    static HDC hdc{};
    
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
                        turn = what_next(turn);
                    }
                    break;
                    
                case IDM_RESTORE_MOVE:
                    if (board.restore_move()) {
                        motion_input.clear();
                        turn = what_next(turn);
                    }
                    break;
                
                case IDM_PASTE_START_MAP:
                {
                    std::string board_repr_aspt = take_str_from_clip();
                    if (!is_legal_board_repr(board_repr_aspt)) break;
                    start_board_repr = board_repr_aspt;
                }
                    break;
                    
                case IDM_RESET_START_MAP_IDW:
                    start_board_repr = DEFAULT_CHESS_BOARD_IDW;
                    restart();
                    break;
                    
                case IDM_RESET_START_MAP_NIDW:
                    start_board_repr = DEFAULT_CHESS_BOARD_NIDW;
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
                    // можно просто всё занулять
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
                    
                case IDM_SET_EDITOR_WINDOW_MODE:
                {
                    window_state = WindowState::EDIT;
                    SetMenu(hWnd, LoadMenu(hInst, MAKEINTRESOURCE(IDR_CHWIGRX_EDIT_MENU)));
                    motion_input.clear();
                    update_edit_menu_variables(hWnd);
                    change_checkerboard_color_theme(hWnd);
                    figures_list_window = create_figures_list_window(hWnd);
                }
                    break;
                    
                case IDM_ABOUT:
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, about_proc);
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
        
        switch (wParam)
        {
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
                Pos shift{ wParam == VK_LEFT ? Pos(0, -1) : (wParam == VK_RIGHT ? Pos(0, 1) : (wParam == VK_UP ? Pos(-1, 0) : Pos(1, 0))) };
                if (!motion_input.is_active_by_click())
                    motion_input.shift_from(shift, HEIGHT, WIDTH);
                else
                    motion_input.shift_target(shift, HEIGHT, WIDTH);
                InvalidateRect(hWnd, NULL, NULL);
                return 0;
            }
            case VK_RETURN:
                if (motion_input.is_active_by_click()) {
                    make_move(hWnd);
                    motion_input.clear();
                    InvalidateRect(hWnd, NULL, NULL);
                    return 0;
                }
                else {
                    motion_input.prepare(turn);
                    motion_input.activate_by_click();
                }
                InvalidateRect(hWnd, NULL, NULL);
                return 0;
            default:
                return 0;
            }
            switch (motion_input.get_state_by_pos()) {
            case 0:
                motion_input.set_from_x(cord);
                motion_input.by_pos_to_next();
                break;
            case 1:
                motion_input.set_from_y(cord);
                motion_input.prepare(turn);
                motion_input.activate_by_click();
                motion_input.by_pos_to_next();
                break;
            case 2:
                motion_input.set_target_x(cord);
                motion_input.by_pos_to_next();
                break;
            case 3:
                motion_input.set_target_y(cord);
                make_move(hWnd);
                motion_input.clear();
                break;
            }
        }
            InvalidateRect(hWnd, NULL, NULL);
            break;
    
    case WM_RBUTTONDOWN:
        motion_input.clear();
        InvalidateRect(hWnd, NULL, NULL);
        #ifdef _DEBUG
            std::cout << "Curr board: " << board.get_repr(true).as_string() << '\n';
        #endif // _DEBUG
        break;
        
    case WM_LBUTTONUP:
        on_lbutton_up(hWnd, wParam, lParam,
            main_window.divide_by_cell_size(lParam).change_axes()
        );
        InvalidateRect(hWnd, NULL, NULL);
        break;
        
    case WM_MOUSEMOVE:
        if (motion_input.is_drags())
        {
            if (motion_input.is_active_by_click()) {
                // Отмена ввода при перетягивании конечной клетки
                motion_input.clear();
            }
            else {
                motion_input.init_curr_choice_window(hWnd,
                    curr_choice_wndproc<true>);
            }
        }
        break;
        
    case WM_PAINT:
    {
        Input input = motion_input.get_input();
        hdc = BeginPaint(hWnd, &ps);
        hdcMem = CreateCompatibleDC(hdc);
        hbmMem = CreateCompatibleBitmap(hdc,
            main_window.get_width(),
            main_window.get_height()
        );
        hOld = SelectObject(hdcMem, hbmMem);

        draw_board(hdcMem);

        /* Возможные ходы текущей фигуры */
        {
            for (const auto& [is_eat, move_pos] : motion_input.get_possible_moves()) {
                static const HBRUSH GREEN{ CreateSolidBrush(RGB(0, 255, 0)) };
                static const HBRUSH DARK_GREEN{ CreateSolidBrush(RGB(0, 150, 0)) };
                const RECT cell = main_window.get_cell(change_axes(move_pos));
                if (is_eat) {
                    FillRect(hdcMem, &cell, DARK_GREEN);
                }
                else {
                    FillRect(hdcMem, &cell, GREEN);
                }
            }
        }

        draw_input(hdcMem, input);

        draw_figures_on_board(hdcMem);

        /* Копирование временного буфера в основной */
        BitBlt(hdc, 0, 0,
            main_window.get_width(),
            main_window.get_height(),
            hdcMem, 0, 0, SRCCOPY
        );

        SelectObject(hdcMem, hOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
    }
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return static_cast<LRESULT>(0);
}
