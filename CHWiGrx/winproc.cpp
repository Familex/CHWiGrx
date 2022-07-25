#include "declarations.hpp"

LRESULT CALLBACK main_window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT ps;
    static HBITMAP hbmMem;
    static HGDIOBJ hOld;
    static HDC hdcMem;
    static HDC hdc;
    switch (window_state) {
        case WindowState::GAME:
            return mainproc::game_switch(hWnd, message, wParam, lParam, ps, hbmMem, hOld, hdcMem, hdc);
        case WindowState::EDIT:
            return mainproc::edit_switch(hWnd, message, wParam, lParam, ps, hbmMem, hOld, hdcMem, hdc);
        default:
            assert(false);
            break;
    }
    return static_cast<LRESULT>(0);
}

LRESULT mainproc::game_switch(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, PAINTSTRUCT ps, HBITMAP hbmMem, HGDIOBJ hOld, HDC hdcMem, HDC hdc) {
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
            {
                std::string board_repr_str = take_str_from_clip();
                if (!is_legal_board_repr(board_repr_str)) break;
                BoardRepr board_repr(board_repr_str);
                turn = board_repr.get_turn();
                board.reset(board_repr);
                motion_input.clear();
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
            BoardRepr empty_repr = { EMPTY_BOARD };
            board.reset(empty_repr);
            turn = empty_repr.get_turn();
            motion_input.clear();
            update_edit_menu_variables(hWnd);
            change_checkerboard_color_theme(hWnd);
            choice_window = create_choice_window(hWnd);
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

        draw_board(hdcMem);

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

        draw_figures_on_board(hdcMem);

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
    return static_cast<LRESULT>(0);
}

LRESULT mainproc::edit_switch(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, PAINTSTRUCT ps, HBITMAP hbmMem, HGDIOBJ hOld, HDC hdcMem, HDC hdc) {
    switch (message)
    {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case IDM_COPY_MAP:
                    copy_repr_to_clip();
                    break;
                case IDM_SET_GAME_WINDOW_MODE:
                    window_state = WindowState::GAME;
                    DestroyWindow(choice_window);
                    SetMenu(hWnd, LoadMenu(hInst, MAKEINTRESOURCE(IDC_CHWIGRX)));
                    change_checkerboard_color_theme(hWnd);
                    break;
                case IDM_PASTE_MAP:
                {
                    std::string board_repr_str = take_str_from_clip();
                    if (!is_legal_board_repr(board_repr_str)) break;
                    BoardRepr board_repr(board_repr_str);
                    turn = board_repr.get_turn();
                    board.reset(board_repr);
                }
                    break;
                case IDM_TOGGLE_LIST_WINDOW:
                    if (choice_window == NULL) {
                        choice_window = create_choice_window(hWnd);
                    }
                    else {
                        DestroyWindow(choice_window);
                        choice_window = NULL;
                    }
                    set_menu_checkbox(hWnd, IDM_TOGGLE_LIST_WINDOW, choice_window != NULL);
                    break;
                case IDM_WHITE_START:
                    turn = Color::Type::White;
                    update_edit_menu_variables(hWnd);
                    break;
                case IDM_BLACK_START:
                    turn = Color::Type::Black;
                    update_edit_menu_variables(hWnd);
                    break;
                case IDM_IDW_TRUE:
                    board.set_idw(true);
                    update_edit_menu_variables(hWnd);
                    break;
                case IDM_IDW_FALSE:
                    board.set_idw(false);
                    update_edit_menu_variables(hWnd);
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
            break;
        case WM_MOVE:
            window_stats.set_window_pos(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_SIZE:
            window_stats.set_window_size(HIWORD(lParam), LOWORD(lParam));
            InvalidateRect(hWnd, NULL, NULL);
            break;
        case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc,
                window_stats.get_window_width(),
                window_stats.get_window_height()
            );
            hOld = SelectObject(hdcMem, hbmMem);

            draw_board(hdcMem);
            draw_figures_on_board(hdcMem);

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
    return static_cast<LRESULT>(0);
}

LRESULT CALLBACK choice_window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    
    SCROLLINFO si{.cbSize=sizeof(SCROLLINFO)};

    static PAINTSTRUCT ps;
    static HBITMAP hbmMem;
    static HGDIOBJ hOld;
    static HDC hdcMem;
    static HDC hdc;
    static std::vector<Figure*> unique_figures;
    static size_t figures_cell_lenght = 30;
    static size_t figures_in_row = 2;
    static int curr_scroll = 0;

    switch (message) {

        case WM_CREATE:
        {
            RECT rect;
            GetClientRect(hWnd, &rect);

            for (char type : ALL_FIGURES) {
                unique_figures.push_back(
                    FigureFabric::instance()->create({}, Color::Type::White, FigureType(type))
                );
            }

            si.cbSize = sizeof(si);
            si.fMask = SIF_POS | SIF_RANGE;
            si.nPos = figures_in_row;
            si.nMax = unique_figures.size();
            si.nMin = 1;
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

            figures_cell_lenght = (rect.right - rect.left) / figures_in_row;
            int rows_num = static_cast<int>(ceil(unique_figures.size() / static_cast<double>(figures_in_row)));
            long total_height_of_all_figures = rows_num * figures_cell_lenght;

            si.cbSize = sizeof(si);
            si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            si.nPos = 0;
            si.nMax = std::max(total_height_of_all_figures, rect.bottom - rect.top);
            si.nMin = 0;
            si.nPage = rect.bottom - rect.top;
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }
            break;
        case WM_HSCROLL:
        {
            RECT rect;
            GetClientRect(hWnd, &rect);

            switch (LOWORD(wParam))
            {
            case SB_PAGELEFT: case SB_LINELEFT:
                if (figures_in_row > 1) {
                    figures_in_row--;
                    InvalidateRect(hWnd, NULL, NULL);
                }
                break;
            case SB_PAGERIGHT: case SB_LINERIGHT:
                if (figures_in_row < unique_figures.size()) {
                    figures_in_row++;
                    InvalidateRect(hWnd, NULL, NULL);
                }
                break;
            }

            si.cbSize = sizeof(si);
            si.fMask = SIF_POS;
            si.nPos = figures_in_row;
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

            figures_cell_lenght = (rect.right - rect.left) / figures_in_row;
            int rows_num = static_cast<int>(ceil(unique_figures.size() / static_cast<double>(figures_in_row)));
            long total_height_of_all_figures = rows_num * figures_cell_lenght;

            si.cbSize = sizeof(si);
            si.fMask = SIF_RANGE | SIF_PAGE;
            si.nMin = 0;
            si.nMax = std::max(total_height_of_all_figures, rect.bottom - rect.top);
            si.nPage = rect.bottom - rect.top;
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

        }
            InvalidateRect(hWnd, NULL, NULL);
            break;
        case WM_VSCROLL:
        {
            int delta = 0;
            switch (LOWORD(wParam))
            {
            case SB_PAGEUP:
                delta = -50; break;
            case SB_LINEUP:
                delta = -10;
                break;
            case SB_PAGEDOWN:
                delta = 50;
                break;
            case SB_LINEDOWN:
                delta = 10;
                break;
            }
            si.fMask = SIF_ALL;
            GetScrollInfo(hWnd, SB_VERT, &si);
            si.fMask = SIF_POS;
            si.nPos = std::min(si.nMax, si.nPos + delta);
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }            
            InvalidateRect(hWnd, NULL, NULL);
            break;
        case WM_SIZE:
        {
            int height = HIWORD(lParam);
            int width = LOWORD(lParam);
            figures_cell_lenght = width / figures_in_row;
            int rows_num = static_cast<int>(ceil(unique_figures.size() / static_cast<double>(figures_in_row)));
            long total_height_of_all_figures = rows_num * figures_cell_lenght;
            int max_scroll = std::max(0L, total_height_of_all_figures - height);
            int curr_scroll = std::min(curr_scroll, max_scroll);
            si.cbSize = sizeof(si);
            si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
            si.nMax = total_height_of_all_figures;
            si.nPage = height;
            si.nPos = curr_scroll;
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
            
            /*
            * Scroll bar dissapears...
            si.cbSize = sizeof(si);
            si.fMask = SIF_PAGE;
            si.nPage = width;
            SetScrollInfo(hWnd, SB_HORZ, &si, FALSE);
            */
        }
            InvalidateRect(hWnd, NULL, NULL);
            break;
        case WM_PAINT:
        {
            RECT rect;
            GetClientRect(hWnd, &rect);

            size_t height = rect.bottom - rect.top;
            size_t width = rect.right - rect.left;

            hdc = BeginPaint(hWnd, &ps);
            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc, width, height);
            hOld = SelectObject(hdcMem, hbmMem);

            /* фон */
            FillRect(hdcMem, &rect, CHECKERBOARD_DARK);

            /* рисовать фигуры (не забыть про отступ) */
            for (int index = 0; index < unique_figures.size(); ++index) {
                int x = index / figures_in_row;
                int y = index % figures_in_row;
                Figure* fig_to_draw = unique_figures[index];
                int a = figures_cell_lenght;
                draw_figure(hdcMem, fig_to_draw->get_col(), fig_to_draw->get_type(), { x, y }, true, a, a);
            }
             
            /* Копирование временного буфера в основной */
            BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY
            );

            SelectObject(hdcMem, hOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            EndPaint(hWnd, &ps);
        }
            break;
        case WM_DESTROY:
        {
            for (Figure* figure : unique_figures) {
                delete figure;
            }
            unique_figures.clear();
            choice_window = NULL;
            HWND owner = GetWindow(hWnd, GW_OWNER); // это должен быть GetParent, но оный возвращает NULL
            set_menu_checkbox(owner, IDM_TOGGLE_LIST_WINDOW, false);
        }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return static_cast<LRESULT>(0);
}
