#include "declarations.hpp"

bool prepare_window(const HINSTANCE h_instance, const int n_cmd_show, const UINT title_id, const UINT window_class_id, WNDCLASSEX wcex) {
    constexpr auto max_load_string = 100;

    WCHAR sz_title[max_load_string];
    WCHAR sz_window_class[max_load_string];

    // Инициализация глобальных строк
    LoadStringW(h_instance, title_id, sz_title, max_load_string);
    LoadStringW(h_instance, window_class_id, sz_window_class, max_load_string);

    wcex.lpszClassName = sz_window_class;
    wcex.lpszMenuName = MAKEINTRESOURCE(window_class_id);   // ... = szWindowClass не работает

    RegisterClassExW(&wcex);

    if (!init_instance(h_instance, sz_title, sz_window_class, n_cmd_show))
    {
        return false;
    }
    return true;
}

// Цикл сообщений
int window_loop(const HINSTANCE h_instance) {
    MSG msg;
    const HACCEL h_accelerators = LoadAccelerators(h_instance, MAKEINTRESOURCE(IDC_CHWIGRX));
    while (const BOOL b_ret = GetMessage(&msg, nullptr, 0, 0)) {
        if (-1 == b_ret) break;
        if (!TranslateAccelerator(msg.hwnd, h_accelerators, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

// Инициализация окна
bool init_instance(const HINSTANCE h_instance, const LPTSTR sz_title, const LPTSTR sz_window_class, const int n_cmd_show)
{
    h_inst = h_instance; // Сохранить маркер экземпляра в глобальной переменной

    const HWND h_wnd = CreateWindowW(sz_window_class, sz_title, WS_OVERLAPPEDWINDOW,
        main_window.get_window_pos_x(), main_window.get_window_pos_y(),
        main_window.get_width_with_extra(), main_window.get_height_with_extra(),
        nullptr, nullptr, h_instance, nullptr
    );

    if (!h_wnd) {
        return false;
    }

    ShowWindow(h_wnd, n_cmd_show);
    UpdateWindow(h_wnd);

    return true;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK about_proc(const HWND h_dlg, const UINT message, const WPARAM w_param, const LPARAM l_param)
{
    UNREFERENCED_PARAMETER(l_param);
    switch (message)
    {
    case WM_INITDIALOG:
        return static_cast<INT_PTR>(TRUE);

    case WM_COMMAND:
        if (LOWORD(w_param) == IDOK || LOWORD(w_param) == IDCANCEL)
        {
            EndDialog(h_dlg, LOWORD(w_param));
            return static_cast<INT_PTR>(TRUE);
        }
        break;
    }
    return static_cast<INT_PTR>(FALSE);
}

void draw_figure(const HDC hdc, const Figure* figure, const Pos begin_paint, const bool is_transparent, const int w, const int h) {
    const int w_beg = begin_paint.y * w;
    const int h_beg = begin_paint.x * h;
    int h_end = h_beg + h;
    int w_end = w_beg + h;
    const HBITMAP h_bitmap = pieces_bitmaps[col_to_char(figure->get_col())][figure_type_to_char(figure->get_type())];
    BITMAP bm{};
    GetObject(h_bitmap, sizeof(BITMAP), &bm);
    const HDC hdc_mem = CreateCompatibleDC(hdc);
    SelectObject(hdc_mem, h_bitmap);
    SetStretchBltMode(hdc, STRETCH_DELETESCANS);
    if (is_transparent) {
        TransparentBlt(hdc, w_beg, h_beg,
            w, h,
            hdc_mem, 0, 0, bm.bmWidth, bm.bmHeight, TRANSPARENCY_PLACEHOLDER);
    }
    else {
        StretchBlt(hdc, w_beg, h_beg,
            w, h,
            hdc_mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    }
    
    // if it's a castling rook, add a star to right top corner
    // FIXME draws rook instead of star.
    if (board.has_castling(figure->get_id()))
    {
        const HBITMAP h_star_bitmap = other_bitmaps["star"];
        GetObject(h_star_bitmap, sizeof(BITMAP), &bm);
        SelectObject(hdc_mem, h_star_bitmap); // <- new hOldBitmap?
        SetStretchBltMode(hdc, STRETCH_DELETESCANS);
        TransparentBlt(hdc, 
            w_beg + w * 2 / 3, h_beg,
            w / 3, h / 3,
            hdc_mem, 0, 0, bm.bmWidth, bm.bmHeight, TRANSPARENCY_PLACEHOLDER);
    }
    
    DeleteDC(hdc_mem);
}

void draw_figure(const HDC hdc, const Figure* figure, const Pos begin_paint, const bool is_transparent) {
    draw_figure(hdc, figure, begin_paint, is_transparent, main_window.get_cell_width(), main_window.get_cell_height());
}

/// <summary>
/// Совершает ход
/// </summary>
/// <param name="h_wnd">Дескриптор окна</param>
void make_move(const HWND h_wnd) {
    if (!is_bot_move() && !motion_input.is_current_turn(turn))
        return;     // Запрет хода вне очереди

    auto in_hand = motion_input.get_in_hand();
    Input input = motion_input.get_input();
    
    if (is_bot_move())
    {
        input = bot::create_move(bot_type, board, turn);
        in_hand = board.get_fig(input.from);
    }

    if (!in_hand.has_value()) {
        InvalidateRect(h_wnd, nullptr, NULL);
        return;
    }
    
    const auto move_message_sus = board.provide_move(
        in_hand.value(), input,
        turn, [c = chose] { return c; }
    );

    if (!move_message_sus.has_value()) {
        InvalidateRect(h_wnd, nullptr, NULL);
        return;
    }

    const auto& move_message = move_message_sus.value();

    debug_print("Curr move was:", 
        AsString<mvmsg::MoveMessage>{}(move_message, AsStringMeta{ 0_id, 2, 2 })
    );

    board.set_last_move(move_message);
    turn = what_next(turn);
    InvalidateRect(h_wnd, nullptr, NULL);
    UpdateWindow(h_wnd);
    
    if (const GameEndType curr_game_end_state = board.game_end_check(turn); 
            curr_game_end_state != GameEndType::NotGameEnd
        ) {
        std::wstring body;
        const std::wstring head = L"Game end";
        switch (curr_game_end_state) 
        {
            case GameEndType::Checkmate:
            {
                const auto who_next = what_next(turn);
                body = who_next == Color::White ? L"White wins!" :
                    who_next == Color::Black ? L"Black wins!" :
                    L"None wins!";
            }
                break;

            case GameEndType::Stalemate:
                body = turn == Color::White ? L"Stalemate to white!" :
                    turn == Color::Black ? L"Stalemate to black!" :
                    L"Stalemate?";
                break;
                
            case GameEndType::FiftyRule:
                body = L"Fifty rule";
                break;
                
            case GameEndType::InsufficientMaterial:
                body = L"Insufficient material";
                break;

            case GameEndType::MoveRepeat:
                body = L"Move repeat rule";
                break;
                
            default:
                assert(!"unexpected game end");
                break;
        }
        if (const auto result = MessageBox(h_wnd, (body + L"\nCopy board to clip?").c_str(), head.c_str(), MB_YESNO);
                result == IDYES) 
        {
            copy_repr_to_clip(h_wnd);
        }
        restart();
        InvalidateRect(h_wnd, nullptr, NULL);
    }

    update_main_window_title(h_wnd);

    if (is_bot_move())
    {
        make_move(h_wnd);
    }
}

bool is_bot_move()
{
    return bot_type != bot::Type::None && turn == bot_turn;
}

void restart() {
    board_repr::BoardRepr start_board_repr_copy{ start_board_repr }; // explicit copy constructor
    board.reset(std::move(start_board_repr_copy));
    motion_input.clear();
    turn = start_board_repr.turn;
}

bool cpy_str_to_clip(const HWND h_wnd, const std::string_view s) {
    if (!OpenClipboard(h_wnd)) {
        debug_print("Failed to open clipboard");
        return false;
    }

    if (!EmptyClipboard()) {
        debug_print("Failed to empty clipboard");
        CloseClipboard();
        return false;
    }

    const HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
    if (!hg) {
        debug_print("Failed to allocate memory");
        CloseClipboard();
        return false;
    }

    const LPVOID p = GlobalLock(hg);
    if (!p) {
        debug_print("Failed to lock memory");
        GlobalFree(hg);
        CloseClipboard();
        return false;
    }

    memcpy(p, s.data(), s.size() + 1);
    GlobalUnlock(hg);

    if (!SetClipboardData(CF_TEXT, hg)) {
        debug_print("Failed to set clipboard data");
        GlobalFree(hg);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

std::string take_str_from_clip(const HWND h_wnd) {
    if (!IsClipboardFormatAvailable(CF_TEXT)) {
        debug_print("CF_TEXT format not available");
        return "";
    }

    if (!OpenClipboard(h_wnd)) {
        debug_print("Failed to open clipboard");
        return "";
    }

    const HANDLE h_data = GetClipboardData(CF_TEXT);
    if (!h_data) {
        debug_print("Failed to get clipboard data");
        CloseClipboard();
        return "";
    }

    const auto psz_text = static_cast<const char*>(GlobalLock(h_data));
    if (!psz_text) {
        debug_print("Failed to lock memory");
        CloseClipboard();
        return "";
    }

    std::string text(psz_text);
    GlobalUnlock(h_data);

    if (!CloseClipboard()) {
        debug_print("Failed to close clipboard");
        return "";
    }

    return text;
}



/// <summary>
/// Создаёт экземпляр окна для выбранной фигуры
/// </summary>
/// <param name="parent">Основное окно</param>
/// <param name="in_hand">Выбранная фигура</param>
/// <param name="mouse">Позиция мыши</param>
/// <param name="w">Ширина фигуры</param>
/// <param name="h">Высота фигуры</param>
/// <param name="callback">Функция окна</param>
/// <returns>Дескриптор окна</returns>
HWND create_curr_choice_window(HWND parent, Figure* in_hand, POINT mouse, int w, int h, const WNDPROC callback) {
    UnregisterClass(CURR_CHOICE_WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    Figure* for_storage = in_hand;  // Нужно копировать TODO (В колбеке тоже тогда не забыть удалить)
                                    // т.к. в основной программе тоже будет использоваться in_hand
                                    // который может быть уничтожен.
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(in_hand);
    wc.hbrBackground = nullptr;
    wc.hCursor = LoadCursor(nullptr, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
    wc.hIcon = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_GAME_MODE_BIG));
    wc.hIconSm = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL));
    wc.lpfnWndProc = callback;
    wc.lpszClassName = CURR_CHOICE_WINDOW_CLASS_NAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    const auto create_window = [&parent, &mouse, &w, &h, for_storage]() -> HWND {
        if (const HWND h_window = CreateWindow(CURR_CHOICE_WINDOW_CLASS_NAME, L"", WS_POPUP | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            mouse.x - w / 2, mouse.y - h / 2, w, h, parent, nullptr, nullptr, nullptr);
                h_window) {
            SetWindowPos(h_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetWindowLongPtr(h_window, GWL_EXSTYLE, GetWindowLongPtr(h_window, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(h_window, TRANSPARENCY_PLACEHOLDER, 0xFF, LWA_COLORKEY);
            SetWindowLongPtr(h_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(for_storage));
            ShowWindow(h_window, SW_SHOWDEFAULT);
            UpdateWindow(h_window);
            return h_window;
        }
        else {
            return nullptr;
        }
    };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}

void on_lbutton_up(const HWND h_wnd, [[maybe_unused]] WPARAM w_param, const LPARAM l_param, const Pos where_fig, const bool use_move_check_and_log) {
    motion_input.set_lbutton_up();
    motion_input.deactivate_by_pos();
    if (motion_input.is_active_by_click()) {
        if (use_move_check_and_log) {
            make_move(h_wnd);
        }
        else {
            if (!motion_input.is_target_at_input()) {
                board.move_fig(motion_input.get_input(), false);
            }
        }
        motion_input.clear();
        InvalidateRect(h_wnd, nullptr, NULL);
        return;
    }
    else {
        motion_input.set_target(where_fig.x, where_fig.y);
        if (motion_input.is_target_at_input()) {
            if (main_window.get_prev_lbutton_click() != Pos(LOWORD(l_param), HIWORD(l_param))) { // Отпустили в пределах клетки, но в другом месте
                motion_input.clear();
                InvalidateRect(h_wnd, nullptr, NULL);
                return;
            }
            motion_input.prepare(turn);
            motion_input.activate_by_click();
            InvalidateRect(h_wnd, nullptr, NULL);
        }
        else {
            if (motion_input.get_in_hand().has_value()) {
                if (use_move_check_and_log) {
                    make_move(h_wnd);
                }
                else if (is_valid_coords(motion_input.get_input().target)) {
                    board.move_fig(motion_input.get_input(), false);
                }
            }
            motion_input.clear();
        }
    }
}

void on_lbutton_down(const HWND h_wnd, const LPARAM l_param) {
    // bot move guard
    if (window_state == WindowState::Game && bot_type != bot::Type::None && turn == bot_turn) {
        return;
    }
    
    motion_input.set_lbutton_down();
    main_window.set_prev_lbutton_click( Pos{ LOWORD(l_param), HIWORD(l_param) });
    motion_input.deactivate_by_pos();
    if (motion_input.is_active_by_click()) {
        motion_input.set_target(main_window.divide_by_cell_size(l_param).change_axes());
        InvalidateRect(h_wnd, nullptr, NULL);
    }
    else {
        if (const Pos from = main_window.divide_by_cell_size(l_param).change_axes();
                board.cont_fig(from))
        {
            motion_input.set_from(from);
            motion_input.prepare(turn);
        }
    }
    InvalidateRect(h_wnd, nullptr, NULL);
};

void set_menu_checkbox(const HWND h_wnd, const UINT menu_item, const bool value) {
    const HMENU h_menu = GetMenu(h_wnd);
    MENUITEMINFO item_info;
    ZeroMemory(&item_info, sizeof(item_info));
    item_info.cbSize = sizeof(item_info);
    item_info.fMask = MIIM_STATE;
    if (!GetMenuItemInfo(h_menu, menu_item, FALSE, &item_info)) return;
    item_info.fState = value ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfoW(h_menu, menu_item, FALSE, &item_info);
}

// Создаёт окно c выбранной фигурой и привязывает к мыши
void MotionInput::init_curr_choice_window(const HWND h_wnd, const WNDPROC callback) {
    if (!in_hand_.has_value()) return;
    
    is_curr_choice_moving_ = true;
    POINT mouse{};
    GetCursorPos(&mouse);
    curr_chose_window_ = create_curr_choice_window(h_wnd, in_hand_.value(), mouse,
        main_window.get_cell_width(), main_window.get_cell_height(),
        callback);
    RedrawWindow(h_wnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);  // Форсирую перерисовку, т.к. появляется артефакт
    SendMessage(curr_chose_window_, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(mouse.x, mouse.y));
}

// Заполняет поле возможных ходов для текущей фигуры
void MotionInput::calculate_possible_moves() {
    if (in_hand_.has_value()) {
        all_moves_ = board_->get_all_possible_moves(in_hand_.value());
    }
}

void MotionInput::clear() {
    is_lbutton_down_ = false;
    DestroyWindow(curr_chose_window_);
    is_curr_choice_moving_ = false;
    deactivate_by_click();
    deactivate_by_pos();
    in_hand_ = std::nullopt;
    input_ = Input{ Pos{0, -1}, Pos{-1, -1} };
    all_moves_.clear();
}

void MotionInput::prepare(const Color turn) {
    in_hand_ = board_->get_fig(input_.from);
    input_.target = input_.from;
    if (in_hand_.has_value() && in_hand_.value()->get_col() == turn) {
        calculate_possible_moves();
    }
}

void update_main_window_title(HWND h_wnd) {
    /* CHWiGrx vs bot [check] */
    std::wstring title = L"CHWiGrx ";
    title.reserve(64);    // hardcoded (CHWiGrx vs NeuralNetwork bot diff.NaN [Check to Black])

    if (bot_type != bot::Type::None) {
        title += L"vs ";
        
        switch (bot_type)
        {
            case bot::Type::Unselected:    title += L"Unselected ";    break;
            case bot::Type::Random:        title += L"Random ";        break;
            case bot::Type::Minimax:       title += L"Minimax ";       break;
            case bot::Type::AlphaBeta:     title += L"AlphaBeta ";     break;
            case bot::Type::MonteCarlo:    title += L"MonteCarlo ";    break;
            case bot::Type::NeuralNetwork: title += L"NeuralNetwork "; break;
            default:                       title += L"Undefined ";     break;
        }
        
        title += L"bot ";

        switch (bot_difficulty)
        {
            case bot::Difficulty::D0: title += L"diff.0 "; break;
            case bot::Difficulty::D1: title += L"diff.1 "; break;
            case bot::Difficulty::D2: title += L"diff.2 "; break;
            case bot::Difficulty::D3: title += L"diff.3 "; break;
            default: title += L"diff.NaN "; break;
        }
    }

    if (board.is_empty()) {
        title += L"[Empty board]";
    }
    else if (board.check_for_when(turn)) {
        title += L"[Check to ";
        title += turn == Color::White ? L"White" : turn == Color::Black ? L"Black" : L"None";
        title += L"]";
    }
    
    SetWindowText(h_wnd, title.c_str());
}

bool copy_repr_to_clip(const HWND h_wnd) {
    const board_repr::BoardRepr board_repr{ board.get_repr(turn, save_all_moves) };
    const auto board_repr_str = AsString<board_repr::BoardRepr>{}(board_repr);
    return cpy_str_to_clip(
        h_wnd,
        board_repr_str
    );
}

auto take_repr_from_clip(const HWND h_wnd)
-> ParseEither<board_repr::BoardRepr, ParseErrorType>
{
    return FromString<board_repr::BoardRepr>{}(take_str_from_clip(h_wnd));
}

/* Отрисовать фоновую доску */
void draw_board(const HDC hdc) {
    for (int i{}; i < HEIGHT; ++i) {
        for (int j{}; j < WIDTH; ++j) {
            const RECT cell = main_window.get_cell(i, j);
            if ((i + j) % 2) {
                FillRect(hdc, &cell, checkerboard_one);
            }
            else {
                FillRect(hdc, &cell, checkerboard_two);
            }
        }
    }
}

// Положение курсора и выделенной клетки
void draw_input(const HDC hdc_mem, const Input input) {
    static const HBRUSH RED{ CreateSolidBrush(RGB(255, 0, 0)) };
    static const HBRUSH BLUE{ CreateSolidBrush(RGB(0, 0, 255)) };
    const RECT from_cell = main_window.get_cell(change_axes(input.from));
    const RECT target_cell = main_window.get_cell(change_axes(input.target));
    FillRect(hdc_mem, &from_cell, RED);
    FillRect(hdc_mem, &target_cell, BLUE);
}

/* Отрисовать фигуры на поле (та, что в руке, не рисуется) */
void draw_figures_on_board(const HDC hdc) {
    for (const auto& figure : board.get_all_figures()) {
        if (!motion_input.is_figure_dragged(figure->get_id())) {
            draw_figure(hdc, figure, figure->get_pos());
        }
    }
}

void change_checkerboard_color_theme(const HWND h_wnd) {
    if (window_state == WindowState::Edit) {
        std::swap(checkerboard_one, checkerboard_two);
    }
    else if (window_state == WindowState::Game) {
        std::swap(checkerboard_one, checkerboard_two);
    }
    else {
        assert(false);
    }
    InvalidateRect(h_wnd, nullptr, NULL);
}

void update_edit_menu_variables(const HWND h_wnd) {
    for (const auto menu_id : { IDM_WHITE_START, IDM_BLACK_START, IDM_IDW_TRUE, IDM_IDW_FALSE }) {
        set_menu_checkbox(h_wnd, menu_id, false);
    }

    if (turn == Color::White)
        set_menu_checkbox(h_wnd, IDM_WHITE_START, true);
    else if (turn == Color::Black)
        set_menu_checkbox(h_wnd, IDM_BLACK_START, true);

    if (board.get_idw() == true)
        set_menu_checkbox(h_wnd, IDM_IDW_TRUE, true);
    else
        set_menu_checkbox(h_wnd, IDM_IDW_FALSE, true);
}

void update_bot_menu_variables(const HWND h_wnd)
{
    for (const auto menu_id : {
             IDM_TOGGLE_BOT,
             IDM_BOTCOLOR_WHITE, IDM_BOTCOLOR_BLACK,
             IDM_BOTDIFFICULTY_EASY, IDM_BOTDIFFICULTY_NORMAL, IDM_BOTDIFFICULTY_HARD,
             IDM_BOTTYPE_RANDOM,
         }) {
        set_menu_checkbox(h_wnd, menu_id, false);
    }
    
    if (bot_type != bot::Type::None) {
        set_menu_checkbox(h_wnd, IDM_TOGGLE_BOT, true);

        if (bot_turn == Color::White)
            set_menu_checkbox(h_wnd, IDM_BOTCOLOR_WHITE, true);
        else if (bot_turn == Color::Black)
            set_menu_checkbox(h_wnd, IDM_BOTCOLOR_BLACK, true);
        
        if (bot_difficulty == bot::Difficulty::D0)
            set_menu_checkbox(h_wnd, IDM_BOTDIFFICULTY_EASY, true);
        else if (bot_difficulty == bot::Difficulty::D1)
            set_menu_checkbox(h_wnd, IDM_BOTDIFFICULTY_NORMAL, true);
        else if (bot_difficulty == bot::Difficulty::D2)
            set_menu_checkbox(h_wnd, IDM_BOTDIFFICULTY_HARD, true);
        else if (bot_difficulty == bot::Difficulty::D3)
            set_menu_checkbox(h_wnd, IDM_BOTDIFFICULTY_VERYHARD, true);
            
        if (bot_type == bot::Type::Random)
            set_menu_checkbox(h_wnd, IDM_BOTTYPE_RANDOM, true);
        else if (bot_type == bot::Type::NeuralNetwork)
            ;   // placeholder;
    }
    else {
        // without bot
    }
}

void update_game_menu_variables(const HWND h_wnd)
{
    update_bot_menu_variables(h_wnd);

    for (const auto menu_id : { 
             IDM_TOGGLE_SAVE_ALL_MOVES,
             IDM_SET_CHOICE_TO_QUEEN, IDM_SET_CHOICE_TO_ROOK, IDM_SET_CHOICE_TO_BISHOP, IDM_SET_CHOICE_TO_KNIGHT,
         }) {
        set_menu_checkbox(h_wnd, menu_id, false);
    }

    if (save_all_moves)
        set_menu_checkbox(h_wnd, IDM_TOGGLE_SAVE_ALL_MOVES, true);

    switch (chose)
    {
        case FigureType::Queen:
            set_menu_checkbox(h_wnd, IDM_SET_CHOICE_TO_QUEEN, true);
            break;
            
        case FigureType::Rook:
            set_menu_checkbox(h_wnd, IDM_SET_CHOICE_TO_ROOK, true);
            break;
           
        case FigureType::Bishop:
            set_menu_checkbox(h_wnd, IDM_SET_CHOICE_TO_BISHOP, true);
            break;
            
        case FigureType::Knight:
            set_menu_checkbox(h_wnd, IDM_SET_CHOICE_TO_KNIGHT, true);
            break;

        default:
            #ifdef _DEBUG
                assert(("Unknown chose", false));
            #endif
            break;
    }
}

HWND create_figures_list_window(HWND parent) {
    UnregisterClass(FIGURES_LIST_WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    HWND h_window{};
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = nullptr;
    wc.hCursor = LoadCursor(h_inst, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
    wc.hIcon = LoadIcon(h_inst, MAKEINTRESOURCE(IDI_FIGURES_LIST));
    wc.hIconSm = LoadIcon(h_inst, MAKEINTRESOURCE(IDI_FIGURES_LIST));
    wc.lpfnWndProc = figures_list_wndproc;
    wc.lpszClassName = FIGURES_LIST_WINDOW_CLASS_NAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    const auto create_window = [&h_window, &parent]() -> HWND {
        if (h_window = CreateWindow(FIGURES_LIST_WINDOW_CLASS_NAME, FIGURES_LIST_WINDOW_TITLE, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
            FIGURES_LIST_WINDOW_DEFAULT_POS.x, FIGURES_LIST_WINDOW_DEFAULT_POS.y,
            figures_list.get_width_with_extra(), figures_list.get_height_with_extra(),
            parent, nullptr, h_inst, nullptr), !h_window)
            return nullptr;
        ShowWindow(h_window, SW_SHOWDEFAULT);
        UpdateWindow(h_window);
        return h_window;
    };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}