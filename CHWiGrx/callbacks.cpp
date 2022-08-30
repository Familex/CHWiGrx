#include "declarations.hpp"

LRESULT CALLBACK main_window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT ps{};
    static HBITMAP hbmMem{};
    static HGDIOBJ hOld{};
    static HDC hdcMem{};
    static HDC hdc{};
    switch (window_state) {
        case WindowState::GAME:
            return mainproc::game_switch(hWnd, message, wParam, lParam, ps, hbmMem, hOld, hdcMem, hdc);
        case WindowState::EDIT:
            return mainproc::edit_switch(hWnd, message, wParam, lParam, ps, hbmMem, hOld, hdcMem, hdc);
        default:
            throw std::logic_error("Unexpected main window state: " + std::to_string(static_cast<int>(window_state)));
    }
    return static_cast<LRESULT>(0);
}

LRESULT CALLBACK mainproc::game_switch(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, PAINTSTRUCT ps, HBITMAP hbmMem, HGDIOBJ hOld, HDC hdcMem, HDC hdc) {
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
        InvalidateRect(hWnd, NULL, NULL);
        break;
    }
    case WM_RBUTTONDOWN:
        motion_input.clear();
        InvalidateRect(hWnd, NULL, NULL);
        #ifdef _DEBUG
            std::cout << "Curr board: " << board.get_repr(true).as_string() << '\n';
        #endif // _DEBUG
        break;
    case WM_LBUTTONDOWN:
        on_lbutton_down(hWnd, lParam);
        break;
    case WM_LBUTTONUP:
        on_lbutton_up(hWnd, wParam, lParam,
            main_window.divide_by_cell_size(LOWORD(lParam), HIWORD(lParam)).change_axes()
        );
        break;
    case WM_MOVE:
        main_window.set_pos(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_MOUSEMOVE:
        if (motion_input.is_drags()) {
            if (motion_input.is_active_by_click()) {
                // Отмена ввода при перетягивании конечной клетки
                motion_input.clear();
            }
            else {
                motion_input.init_curr_choice_window(hWnd,
                    curr_choice_window_proc<true>);
            }
        }
        break;
    case WM_SIZE:
        main_window.set_size(LOWORD(lParam), HIWORD(lParam));
        InvalidateRect(hWnd, NULL, NULL);
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
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return static_cast<LRESULT>(0);
}

LRESULT CALLBACK mainproc::edit_switch(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, PAINTSTRUCT ps, HBITMAP hbmMem, HGDIOBJ hOld, HDC hdcMem, HDC hdc) {
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
                    DestroyWindow(figures_list_window);
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
                    InvalidateRect(hWnd, NULL, NULL);
                    break;
                case IDM_CLEAR_BOARD:
                {
                    board.reset(BoardRepr({}, turn, board.get_idw()));
                    motion_input.clear();
                }
                    break;
                case IDM_TOGGLE_LIST_WINDOW:
                    if (figures_list_window == NULL) {
                        figures_list_window = create_figures_list_window(hWnd);
                    }
                    else {
                        DestroyWindow(figures_list_window);
                        figures_list_window = NULL;
                    }
                    set_menu_checkbox(hWnd, IDM_TOGGLE_LIST_WINDOW, figures_list_window != NULL);
                    break;
                case IDM_WHITE_START:
                    turn = Color::White;
                    update_edit_menu_variables(hWnd);
                    break;
                case IDM_BLACK_START:
                    turn = Color::Black;
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
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
            break;
        case WM_LBUTTONUP:
            on_lbutton_up(hWnd, wParam, lParam,
                main_window.divide_by_cell_size(LOWORD(lParam), HIWORD(lParam)).change_axes(),
                false
            );
            break;
        case WM_LBUTTONDOWN:
            on_lbutton_down(hWnd, lParam);
            break;
        case WM_MOUSEMOVE:
            if (!motion_input.is_active_by_click() && motion_input.is_drags()) {
                motion_input.init_curr_choice_window(hWnd, curr_choice_window_proc<false>);
            }
            InvalidateRect(hWnd, NULL, NULL);
            break;
        case WM_MOVE:
            main_window.set_pos(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_SIZE:
            main_window.set_size(LOWORD(lParam), HIWORD(lParam));
            InvalidateRect(hWnd, NULL, NULL);
            break;
        case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc,
                main_window.get_width(),
                main_window.get_height()
            );
            hOld = SelectObject(hdcMem, hbmMem);

            draw_board(hdcMem);
            draw_input(hdcMem, motion_input.get_input());
            draw_figures_on_board(hdcMem);

            // Копирование временного буфера в основной
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
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return static_cast<LRESULT>(0);
}

LRESULT CALLBACK figures_list_window_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    
    SCROLLINFO si{.cbSize=sizeof(SCROLLINFO)};
    static PAINTSTRUCT ps;
    static HBITMAP hbmMem;
    static HGDIOBJ hOld;
    static HDC hdcMem;
    static HDC hdc;
    static std::map<Color, std::vector<Figure*>> figures_prototypes;
    static Color curr_color{ Color::White };
    static bool is_resizes = true;  // also to horizontal scrollbar
    static bool is_scrolls = false;

    switch (message) {
        case WM_CREATE:
        {
            for (Color col : PLAYABLE_COLORS) {
                for (FigureType type : PLAYABLE_FIGURES) {
                    figures_prototypes[col].push_back(
                        FigureFabric::instance()->create({}, col, type)
                    );
                }
            }

            RECT window_rect{};
            GetWindowRect(hWnd, &window_rect);
            figures_list.set_rect(window_rect);
            figures_list.clear_scrolling();
            is_resizes = true;
            is_scrolls = false;

            si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            si.nPos = static_cast<int>(figures_list.get_figures_in_row());
            si.nMax = static_cast<int>(figures_prototypes[curr_color].size());
            si.nMin = 1;
            si.nPage = 1;
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

            figures_list.recalculate_dimensions();

            si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            si.nPos = 0;
            si.nMax = static_cast<int>(figures_list.get_max_scroll());
            si.nMin = 0;
            si.nPage = figures_list.get_height();
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }
            break;
        case WM_HSCROLL:
        {
            is_resizes = true;

            switch (LOWORD(wParam))
            {
                case SB_PAGELEFT: case SB_LINELEFT:
                    figures_list.dec_figures_in_row();
                    break;
                case SB_PAGERIGHT: case SB_LINERIGHT:
                    figures_list.inc_figures_in_row();
                    break;
            }
            si.fMask = SIF_POS;
            si.nPos = static_cast<int>(figures_list.get_figures_in_row());
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

            figures_list.recalculate_dimensions();

            si.fMask = SIF_RANGE | SIF_PAGE;
            si.nMin = 0;
            si.nMax = static_cast<int>(figures_list.get_max_scroll());
            si.nPage = figures_list.get_height();
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

            SetWindowPos(hWnd, NULL, NULL, NULL,
                figures_list.get_width_with_extra(), figures_list.get_height_with_extra(), SWP_NOMOVE);
        }
            break;
        case WM_VSCROLL:
        {
            is_scrolls = true;
            int delta = 0;
            switch (LOWORD(wParam))
            {
            case SB_PAGEUP:
                delta = figures_list.add_to_curr_scroll(-30);
                break;
            case SB_LINEUP:
                delta = figures_list.add_to_curr_scroll(-10);
                break;
            case SB_PAGEDOWN:
                delta = figures_list.add_to_curr_scroll(30);
                break;
            case SB_LINEDOWN:
                delta = figures_list.add_to_curr_scroll(10);
                break;
            case SB_THUMBPOSITION:
                delta = figures_list.set_curr_scroll(HIWORD(wParam));
                break;
            }

            ScrollWindowEx(hWnd, 0, -delta, (CONST RECT*) NULL,
                (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL,
                SW_INVALIDATE);
            UpdateWindow(hWnd);
            
            si.fMask = SIF_POS;
            si.nPos = figures_list.get_curr_scroll();
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }            
            break;
        case WM_SIZE:
        {
            is_resizes = true;
            figures_list.set_size(LOWORD(lParam), HIWORD(lParam));
            si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
            si.nMax = static_cast<int>(figures_list.get_all_figures_height());
            si.nPage = figures_list.get_height();
            si.nPos = figures_list.get_curr_scroll();
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }
            break;
        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = reinterpret_cast<LPMINMAXINFO>(lParam);
            lpMMI->ptMinTrackSize.y = static_cast<LONG>(figures_list.get_cell_height() + SCROLLBAR_THICKNESS);
            lpMMI->ptMaxTrackSize.y = static_cast<int>(figures_list.get_all_figures_height()) + HEADER_HEIGHT - SCROLLBAR_THICKNESS;
        }
        break;
        case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            PRECT prect = &ps.rcPaint;

            int height = figures_list.get_height();
            int width = figures_list.get_width();
            int curr_scroll = figures_list.get_curr_scroll();
            size_t all_figures_height = figures_list.get_all_figures_height();

            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc, static_cast<int>(width), static_cast<int>(all_figures_height));
            hOld = SelectObject(hdcMem, hbmMem);

            /* фон */
            RECT full_rect = { .left = prect->left, .top = prect->top + curr_scroll,
                .right = prect->right, .bottom = prect->bottom + curr_scroll };
            FillRect(hdcMem, &full_rect, CHECKERBOARD_DARK);

            /* Отрисовка фигур */
            for (int index = 0; index < figures_prototypes[curr_color].size(); ++index) {
                int x = static_cast<int>(index / figures_list.get_figures_in_row());
                int y = static_cast<int>(index % figures_list.get_figures_in_row());
                Figure* fig_to_draw = figures_prototypes[curr_color][index];
                draw_figure(hdcMem, fig_to_draw->get_col(), fig_to_draw->get_type(), { x, y },
                    true, static_cast<int>(figures_list.get_cell_height()), static_cast<int>(figures_list.get_cell_width()));
            }

            /* Копирование временного буфера в основной (взято из майкросовтовской документации) */
            if (is_scrolls) {
                BitBlt(ps.hdc,
                    prect->left, prect->top,
                    width,
                    height,
                    hdcMem,
                    prect->left,
                    prect->top + curr_scroll,
                    SRCCOPY);
                is_scrolls = false;
            }
            else if (is_resizes) {
                BitBlt(ps.hdc,
                    0, 0,
                    width, static_cast<int>(all_figures_height),
                    hdcMem,
                    0, curr_scroll,
                    SRCCOPY);
                is_resizes = false;
            }
            else {
                // При разворачивании окна заходит сюда
                BitBlt(ps.hdc,
                    0, 0,
                    width, static_cast<int>(all_figures_height),
                    hdcMem,
                    0, curr_scroll,
                    SRCCOPY);
            }

            SelectObject(hdcMem, hOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            EndPaint(hWnd, &ps);
        }
            break;
        case WM_LBUTTONDOWN:
        {
            motion_input.set_lbutton_down();
            Pos figure_to_drag = Pos(
                (LOWORD(lParam)) / static_cast<int>(figures_list.get_cell_width()),
                (HIWORD(lParam) + figures_list.get_curr_scroll()) / static_cast<int>(figures_list.get_cell_width())
            ).change_axes();
            size_t index = figure_to_drag.x * figures_list.get_figures_in_row() + figure_to_drag.y;
            if (index >= figures_prototypes[curr_color].size()) break;  // there was a click in a non-standard part of the window => ignore
            motion_input.set_in_hand(FigureFabric::instance()->create(figures_prototypes[curr_color][index], false));
        }
            break;
        case WM_RBUTTONUP:
        {
            curr_color = what_next(curr_color);
        }
            InvalidateRect(hWnd, NULL, NULL);
            break;
        case WM_MOUSEMOVE:
        {
            if (motion_input.is_drags()) {
                motion_input.init_curr_choice_window(hWnd, curr_choice_window_figures_list_proc);
            }
        }
            break;
        case WM_DESTROY:
        {
            for (Color col : PLAYABLE_COLORS) {
                for (Figure* figure : figures_prototypes[col]) {
                    delete figure;
                }
            }
            figures_prototypes.clear();
            figures_list_window = NULL;
            HWND owner = GetWindow(hWnd, GW_OWNER); // это должен быть GetParent, но оный возвращает NULL
            set_menu_checkbox(owner, IDM_TOGGLE_LIST_WINDOW, false);
        }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return static_cast<LRESULT>(0);
}

template <bool check_moves_validity>
LRESULT CALLBACK curr_choice_window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
                draw_figure(hdc, in_hand->get_col(), in_hand->get_type(), Pos(0, 0), false);
            }
            EndPaint(hWnd, &ps);
        }
        break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return static_cast<LRESULT>(0);
}

LRESULT CALLBACK curr_choice_window_figures_list_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
        Pos where_fig = main_window.get_figure_under_mouse(cur_pos);
        if (is_valid_coords(where_fig) &&
            !(figures_list.top <= cur_pos.y && cur_pos.y <= figures_list.bottom &&
                figures_list.left <= cur_pos.x && cur_pos.x <= figures_list.right)) {
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
        motion_input.set_in_hand(FigureFabric::instance()->get_default_fig());
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
        Figure* in_hand = (Figure*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (in_hand) {
            draw_figure(hdc, in_hand->get_col(), in_hand->get_type(), Pos(0, 0), false);
        }
        EndPaint(hWnd, &ps);
    }
    break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return static_cast<LRESULT>(0);
}
