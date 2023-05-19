#include "../../declarations.hpp"
#include "wndproc.h"

LRESULT CALLBACK mainproc::main_game_state_wndproc(
    const HWND h_wnd,
    const UINT message,
    const WPARAM w_param,
    const LPARAM l_param
) noexcept
{
    static PAINTSTRUCT ps{};
    static HBITMAP hbm_mem{};
    static HGDIOBJ h_old{};
    static HDC hdc_mem{};
    static HDC hdc{};

    switch (message) {
        case WM_COMMAND:
        {
            switch (const int wm_id = LOWORD(w_param)) {
                case IDM_UNDO:
                    if (board.undo_move()) {
                        motion_input.clear();
                        turn = what_next(turn);
                        on_game_board_change(board);
                    }
                    break;

                case IDM_RESTORE_MOVE:
                    if (board.restore_move()) {
                        motion_input.clear();
                        turn = what_next(turn);
                        on_game_board_change(board);
                        InvalidateRect(h_wnd, nullptr, NULL);
                        UpdateWindow(h_wnd);
                        game_end_check(h_wnd, what_next(turn));
                    }
                    break;

                case IDM_PASTE_START_MAP:
                {
                    if (const auto br = take_repr_from_clip(h_wnd)) {
                        start_board_repr = br->value;
                    }
                } break;

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
                    const HMENU h_menu = GetMenu(h_wnd);
                    MENUITEMINFO item_info;
                    ZeroMemory(&item_info, sizeof(item_info));
                    item_info.cbSize = sizeof(item_info);
                    item_info.fMask = MIIM_STATE;
                    if (!GetMenuItemInfo(h_menu, IDM_TOGGLE_SAVE_ALL_MOVES, FALSE, &item_info))
                        return 0;
                    if (item_info.fState == MFS_CHECKED) {
                        save_all_moves = false;
                        item_info.fState = MFS_UNCHECKED;
                    }
                    else {
                        save_all_moves = true;
                        item_info.fState = MFS_CHECKED;
                    }
                    SetMenuItemInfoW(h_menu, IDM_TOGGLE_SAVE_ALL_MOVES, FALSE, &item_info);

                    break;
                }

                case IDM_WINDOW_MOVELOG:
                {
                    if (!moves_list_window) {
                        moves_list_window = *create_window(CreateWindowArgsBuilder{}
                                                               .set_class_name(TEXT("CHWIGRX:MOVES_LOG"))
                                                               .set_style(WS_OVERLAPPEDWINDOW)
                                                               .set_title(TEXT("Moves Log"))
                                                               .set_wc_wndproc(moves_list_wndproc)
                                                               .set_parent(h_wnd)
                                                               .build(h_inst));
                    }
                    else {
                        destroy_window(moves_list_window);
                    }

                    update_game_menu_variables(h_wnd);

                    break;
                }

                case IDM_SET_CHOICE_TO_ROOK:
                case IDM_SET_CHOICE_TO_KNIGHT:
                case IDM_SET_CHOICE_TO_QUEEN:
                case IDM_SET_CHOICE_TO_BISHOP:
                {
                    // FIXME move to menu.cpp
                    const UINT item_menu_to_set_id = wm_id;
                    const UINT curr_item_menu_state = chose == FigureType::Queen    ? IDM_SET_CHOICE_TO_QUEEN
                                                      : chose == FigureType::Rook   ? IDM_SET_CHOICE_TO_ROOK
                                                      : chose == FigureType::Knight ? IDM_SET_CHOICE_TO_KNIGHT
                                                      : chose == FigureType::Bishop ? IDM_SET_CHOICE_TO_BISHOP
                                                                                    : NULL;
                    set_menu_checkbox(h_wnd, curr_item_menu_state, false);
                    set_menu_checkbox(h_wnd, item_menu_to_set_id, true);
                    chose = wm_id == IDM_SET_CHOICE_TO_QUEEN    ? FigureType::Queen
                            : wm_id == IDM_SET_CHOICE_TO_ROOK   ? FigureType::Rook
                            : wm_id == IDM_SET_CHOICE_TO_KNIGHT ? FigureType::Knight
                                                                : FigureType::Bishop;
                } break;

                case IDM_SET_EDITOR_WINDOW_MODE:
                {
                    window_state = WindowState::Edit;
                    // set icon
                    auto h_edit_icon =
                        LoadImage(h_inst, MAKEINTRESOURCE(IDI_EDIT_MODE), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
                    SendMessage(h_wnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(h_edit_icon));
                    SendMessage(h_wnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(h_edit_icon));
                    // set menu
                    SetMenu(h_wnd, LoadMenu(h_inst, MAKEINTRESOURCE(IDR_CHWIGRX_EDIT_MENU)));
                    // board prepare
                    motion_input.clear();
                    update_edit_menu_variables(h_wnd);
                    change_checkerboard_color_theme(h_wnd);
                    board.reset_move_logger();
                    figures_list_window = new_window::figures_list(h_wnd);
                    destroy_window(moves_list_window);

                    break;
                }

                /* ---- Bot ------------------------------------------- */
                case IDM_TOGGLE_BOT:
                    if (bot_type) {
                        bot_type = std::nullopt;
                    }
                    else {
                        bot_type = bot::Type::Random;
                    }
                    update_bot_menu_variables(h_wnd);
                    break;

                case IDM_BOTDIFFICULTY_EASY:
                    bot_difficulty = bot::Difficulty::D0;
                    update_bot_menu_variables(h_wnd);
                    break;

                case IDM_BOTDIFFICULTY_NORMAL:
                    bot_difficulty = bot::Difficulty::D1;
                    update_bot_menu_variables(h_wnd);
                    break;

                case IDM_BOTDIFFICULTY_HARD:
                    bot_difficulty = bot::Difficulty::D2;
                    update_bot_menu_variables(h_wnd);
                    break;

                case IDM_BOTDIFFICULTY_VERYHARD:
                    bot_difficulty = bot::Difficulty::D3;
                    update_bot_menu_variables(h_wnd);
                    break;

                case IDM_BOTTYPE_RANDOM:
                    bot_type = bot::Type::Random;
                    update_bot_menu_variables(h_wnd);
                    break;

                case IDM_BOTCOLOR_WHITE:
                    bot_turn = Color::White;
                    update_bot_menu_variables(h_wnd);
                    break;

                case IDM_BOTCOLOR_BLACK:
                    bot_turn = Color::Black;
                    update_bot_menu_variables(h_wnd);
                    break;

                case IDM_ABOUT:
                    DialogBox(h_inst, MAKEINTRESOURCE(IDD_ABOUTBOX), h_wnd, about_proc);
                    break;

                default:
                    return DefWindowProc(h_wnd, message, w_param, l_param);
            }
        }
            update_main_window_title(h_wnd);
            InvalidateRect(h_wnd, nullptr, NULL);
            break;

        case WM_KEYDOWN:
        {
            int cord{ -1 };

            switch (w_param) {
                case VK_0:
                case VK_1:
                case VK_2:
                case VK_3:
                case VK_4:
                case VK_5:
                case VK_6:
                case VK_7:
                case VK_8:
                case VK_9:
                    cord = static_cast<int>(w_param) - VK_0;
                    break;

                case VK_NUMPAD0:
                case VK_NUMPAD1:
                case VK_NUMPAD2:
                case VK_NUMPAD3:
                case VK_NUMPAD4:
                case VK_NUMPAD5:
                case VK_NUMPAD6:
                case VK_NUMPAD7:
                case VK_NUMPAD8:
                case VK_NUMPAD9:
                    cord = static_cast<int>(w_param) - VK_NUMPAD0;
                    break;

                case VK_ESCAPE:
                    motion_input.clear();
                    InvalidateRect(h_wnd, nullptr, NULL);
                    return 0;

                case VK_LEFT:
                case VK_RIGHT:
                case VK_UP:
                case VK_DOWN:
                {
                    const Pos shift{ w_param == VK_LEFT
                                         ? Pos(0, -1)
                                         : (w_param == VK_RIGHT ? Pos(0, 1)
                                                                : (w_param == VK_UP ? Pos(-1, 0) : Pos(1, 0))) };
                    if (!motion_input.is_active_by_click())
                        motion_input.shift_from(shift, HEIGHT, WIDTH);
                    else
                        motion_input.shift_target(shift, HEIGHT, WIDTH);
                    InvalidateRect(h_wnd, nullptr, NULL);

                    return 0;
                }

                case VK_RETURN:
                    if (motion_input.is_active_by_click()) {
                        make_move(h_wnd);
                        motion_input.clear();
                        InvalidateRect(h_wnd, nullptr, NULL);
                        return 0;
                    }
                    motion_input.prepare(turn);
                    motion_input.activate_by_click();
                    InvalidateRect(h_wnd, nullptr, NULL);
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
                    make_move(h_wnd);
                    motion_input.clear();
                    break;
                default:
                    std::unreachable();
            }
        }
            InvalidateRect(h_wnd, nullptr, NULL);
            break;

        case WM_RBUTTONDOWN:
            motion_input.clear();
            InvalidateRect(h_wnd, nullptr, NULL);

            debug_print("Curr board:", AsString<board_repr::BoardRepr>{}(board.get_repr(turn, true)));

            break;

        case WM_LBUTTONUP:
            on_lbutton_up(h_wnd, w_param, l_param, main_window.divide_by_cell_size(l_param).change_axes());
            InvalidateRect(h_wnd, nullptr, NULL);
            break;

        case WM_MOUSEMOVE:
            if (motion_input.is_drags()) {
                if (motion_input.is_active_by_click()) {
                    // Move cancel on target cell drag
                    motion_input.clear();
                }
                else {
                    motion_input.init_curr_choice_window(
                        h_wnd,
                        curr_choice_game_mode_wndproc,
                        Pos{ main_window.get_cell_width(), main_window.get_cell_height() }
                    );
                }
            }
            break;

        case WM_PAINT:
        {
            const Input input = motion_input.get_input();
            hdc = BeginPaint(h_wnd, &ps);
            hdc_mem = CreateCompatibleDC(hdc);
            hbm_mem = CreateCompatibleBitmap(hdc, main_window.get_width(), main_window.get_height());
            h_old = SelectObject(hdc_mem, hbm_mem);

            draw_board(hdc_mem);

            /* Возможные ходы текущей фигуры */
            {
                for (const auto& [is_eat, move_pos] : motion_input.get_possible_moves()) {
                    static const HBRUSH GREEN{ CreateSolidBrush(RGB(0, 255, 0)) };
                    static const HBRUSH DARK_GREEN{ CreateSolidBrush(RGB(0, 150, 0)) };
                    const RECT cell = main_window.get_cell(change_axes(move_pos));
                    if (is_eat) {
                        FillRect(hdc_mem, &cell, DARK_GREEN);
                    }
                    else {
                        FillRect(hdc_mem, &cell, GREEN);
                    }
                }
            }

            draw_input(hdc_mem, input);

            draw_figures_on_board(hdc_mem);

            /* Копирование временного буфера в основной */
            BitBlt(hdc, 0, 0, main_window.get_width(), main_window.get_height(), hdc_mem, 0, 0, SRCCOPY);

            SelectObject(hdc_mem, h_old);
            DeleteObject(hbm_mem);
            DeleteDC(hdc_mem);

            EndPaint(h_wnd, &ps);
        } break;

        default:
            return DefWindowProc(h_wnd, message, w_param, l_param);
    }
    return static_cast<LRESULT>(0);
}
