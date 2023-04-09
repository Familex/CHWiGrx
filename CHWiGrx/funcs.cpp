#include "declarations.hpp"

bool prepare_window(HINSTANCE hInstance, int nCmdShow, UINT title_id, UINT window_class_id, WNDCLASSEX wcex) {
    constexpr auto MAX_LOADSTRING = 100;

    WCHAR szTitle[MAX_LOADSTRING];
    WCHAR szWindowClass[MAX_LOADSTRING];

    // Инициализация глобальных строк
    LoadStringW(hInstance, title_id, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, window_class_id, szWindowClass, MAX_LOADSTRING);

    wcex.lpszClassName = szWindowClass;
    wcex.lpszMenuName = MAKEINTRESOURCE(window_class_id);   // ... = szWindowClass не работает

    RegisterClassExW(&wcex);

    if (!init_instance(hInstance, szTitle, szWindowClass, nCmdShow))
    {
        return false;
    }
    return true;
}

// Цикл сообщений
int window_loop(HINSTANCE hInstance) {
    MSG msg;
    HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHWIGRX));
    while (BOOL bRet = GetMessage(&msg, nullptr, 0, 0)) {
        if (-1 == bRet) break;
        if (!TranslateAccelerator(msg.hwnd, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

// Инициализация окна
bool init_instance(HINSTANCE hInstance, LPTSTR szTitle, LPTSTR szWindowClass, int nCmdShow)
{
    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        main_window.get_window_pos_x(), main_window.get_window_pos_y(),
        main_window.get_width_with_extra(), main_window.get_height_with_extra(),
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hWnd) {
        return false;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return true;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK about_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return static_cast<INT_PTR>(TRUE);

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return static_cast<INT_PTR>(TRUE);
        }
        break;
    }
    return static_cast<INT_PTR>(FALSE);
}

void draw_figure(HDC hdc, const Figure* figure, const Pos begin_paint, const bool is_transpanent, const int w, const int h) {
    const int w_beg = begin_paint.y * w;
    const int h_beg = begin_paint.x * h;
    int h_end = h_beg + h;
    int w_end = w_beg + h;
    HBITMAP hBitmap = pieces_bitmaps[col_to_char(figure->get_col())][figure_type_to_char(figure->get_type())];
    BITMAP bm{};
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HGDIOBJ hOldBitmap = SelectObject(hdcMem, hBitmap);
    SetStretchBltMode(hdc, STRETCH_DELETESCANS);
    if (is_transpanent) {
        TransparentBlt(hdc, w_beg, h_beg,
            w, h,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, TRANSPARENCY_PLACEHOLDER);
    }
    else {
        StretchBlt(hdc, w_beg, h_beg,
            w, h,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    }
    
    // if it's a castring rook, add a star to right top corner
    // FIXME draws rook instead of star.
    if (board.has_castling(figure->get_id()))
    {
        HBITMAP hStarBitmap = other_bitmaps["star"];
        GetObject(hStarBitmap, sizeof(BITMAP), &bm);
        HGDIOBJ hOldBitmap = SelectObject(hdcMem, hStarBitmap); // <- new hOldBitmap?
        SetStretchBltMode(hdc, STRETCH_DELETESCANS);
        TransparentBlt(hdc, 
            w_beg + w * 2 / 3, h_beg,
            w / 3, h / 3,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, TRANSPARENCY_PLACEHOLDER);
    }
    
    DeleteDC(hdcMem);
}

void draw_figure(HDC hdc, const Figure* figure, const Pos begin_paint, const bool is_transpanent) {
    draw_figure(hdc, figure, begin_paint, is_transpanent, main_window.get_cell_width(), main_window.get_cell_height());
}

/// <summary>
/// Совершает ход
/// </summary>
/// <param name="hWnd">Дескриптор окна</param>
void make_move(HWND hWnd, std::optional<Input> input_) {
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
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }
    
    const auto move_message_sus = board.provide_move(
        in_hand.value(), input,
        turn, [c = chose] { return c; }
    );

    if (!move_message_sus.has_value()) {
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }

    const auto& move_message = move_message_sus.value();

    debug_print("Curr move was:", 
        as_string<mvmsg::MoveMessage>{}(move_message, AsStringMeta{ 0_id, 2, 2 })
    );

    board.set_last_move(move_message);
    turn = what_next(turn);
    InvalidateRect(hWnd, NULL, NULL);
    UpdateWindow(hWnd);
    
    if (GameEndType curr_game_end_state = board.game_end_check(turn); 
            curr_game_end_state != GameEndType::NotGameEnd
        ) {
        std::wstring body = L"";
        std::wstring head = L"Game end";
        switch (curr_game_end_state) 
        {
            case GameEndType::Checkmate:
            {
                auto who_next = what_next(turn);
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
        auto result = MessageBox(hWnd, (body + L"\nCopy board to clip?").c_str(), head.c_str(), MB_YESNO);
        if (result == IDYES) {
            copy_repr_to_clip(hWnd);
        }
        restart();
        InvalidateRect(hWnd, NULL, NULL);
    }

    update_main_window_title(hWnd);

    if (is_bot_move())
    {
        make_move(hWnd, bot::create_move(bot_type, board, turn));
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

/// FIXME doesn't work
bool cpy_str_to_clip(HWND hwnd, std::string_view s) {
    OpenClipboard(hwnd);
    EmptyClipboard();
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
    if (!hg) {
        CloseClipboard();
        return false;
    }
    LPVOID p = GlobalLock(hg);
    if (!p) {
        GlobalFree(hg);
        CloseClipboard();
        return false;
    }
    memcpy(p, s.data(), s.size() + 1);
    GlobalUnlock(hg);
    SetClipboardData(CF_TEXT, hg);
    CloseClipboard();
    GlobalFree(hg);
    return true;
}

// Возвращает строку из буффера обмена
std::string take_str_from_clip() {
    if (!OpenClipboard(NULL)) return "";
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (!hData) return "";
    const char* pszText = static_cast<const char*>(GlobalLock(hData));
    if (!pszText) return "";
    std::string text(pszText);
    GlobalUnlock(hData);
    CloseClipboard();
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
/// <param name="class_name">Имя класса окна</param>
/// <returns>Дескриптор окна</returns>
HWND create_curr_choice_window(HWND parent, Figure* in_hand, POINT mouse, int w, int h, const WNDPROC callback) {
    UnregisterClass(CURR_CHOICE_WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    Figure* for_storage = in_hand;  // Возможно нужно копировать TODO
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(in_hand);
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(nullptr, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
    wc.hIcon = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_GAME_MODE_BIG));
    wc.hIconSm = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL));
    wc.lpfnWndProc = callback;
    wc.lpszClassName = CURR_CHOICE_WINDOW_CLASS_NAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    const auto create_window = [&parent, &mouse, &w, &h, for_storage]() -> HWND {
        if (HWND hWindow = CreateWindow(CURR_CHOICE_WINDOW_CLASS_NAME, L"", WS_POPUP | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            mouse.x - w / 2, mouse.y - h / 2, w, h, parent, nullptr, nullptr, nullptr);
                hWindow) {
            SetWindowPos(hWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetWindowLongPtr(hWindow, GWL_EXSTYLE, GetWindowLongPtr(hWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(hWindow, TRANSPARENCY_PLACEHOLDER, 0xFF, LWA_COLORKEY);
            SetWindowLongPtr(hWindow, GWLP_USERDATA, (LONG_PTR)for_storage);
            ShowWindow(hWindow, SW_SHOWDEFAULT);
            UpdateWindow(hWindow);
            return hWindow;
        }
        else {
            return nullptr;
        }
    };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}

void on_lbutton_up(HWND hWnd, WPARAM wParam, LPARAM lParam, Pos where_fig, bool use_move_check_and_log) {
    motion_input.set_lbutton_up();
    motion_input.deactivate_by_pos();
    if (motion_input.is_active_by_click()) {
        if (use_move_check_and_log) {
            make_move(hWnd);
        }
        else {
            if (!motion_input.is_target_at_input()) {
                board.move_fig(motion_input.get_input(), false);
            }
        }
        motion_input.clear();
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }
    else {
        motion_input.set_target(where_fig.x, where_fig.y);
        if (motion_input.is_target_at_input()) {
            if (main_window.get_prev_lbutton_click() != Pos(LOWORD(lParam), HIWORD(lParam))) { // Отпустили в пределах клетки, но в другом месте
                motion_input.clear();
                InvalidateRect(hWnd, NULL, NULL);
                return;
            }
            motion_input.prepare(turn);
            motion_input.activate_by_click();
            InvalidateRect(hWnd, NULL, NULL);
        }
        else {
            if (motion_input.get_in_hand().has_value()) {
                if (use_move_check_and_log) {
                    make_move(hWnd);
                }
                else if (is_valid_coords(motion_input.get_input().target)) {
                    board.move_fig(motion_input.get_input(), false);
                }
            }
            motion_input.clear();
        }
    }
}

void on_lbutton_down(HWND hWnd, LPARAM lParam) {
    // bot move guard
    if (window_state == WindowState::GAME && bot_type != bot::Type::None && turn == bot_turn) {
        return;
    }
    
    motion_input.set_lbutton_down();
    main_window.set_prev_lbutton_click( Pos{ LOWORD(lParam), HIWORD(lParam) });
    motion_input.deactivate_by_pos();
    if (motion_input.is_active_by_click()) {
        motion_input.set_target(main_window.divide_by_cell_size(lParam).change_axes());
        InvalidateRect(hWnd, NULL, NULL);
    }
    else {
        Pos from = main_window.divide_by_cell_size(lParam).change_axes();
        if (board.cont_fig(from)) {
            motion_input.set_from(from);
            motion_input.prepare(turn);
        }
    }
    InvalidateRect(hWnd, NULL, NULL);
};

void set_menu_checkbox(HWND hWnd, UINT menu_item, bool state) {
    HMENU hMenu = GetMenu(hWnd);
    MENUITEMINFO item_info;
    ZeroMemory(&item_info, sizeof(item_info));
    item_info.cbSize = sizeof(item_info);
    item_info.fMask = MIIM_STATE;
    if (!GetMenuItemInfo(hMenu, menu_item, FALSE, &item_info)) return;
    item_info.fState = state ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfoW(hMenu, menu_item, FALSE, &item_info);
}

// Создаёт окно c выбранной фигурой и привязывает к мыши
void MotionInput::init_curr_choice_window(HWND hWnd, WNDPROC callback) {
    if (!in_hand.has_value()) return;
    
    is_curr_choice_moving = true;
    POINT mouse{};
    GetCursorPos(&mouse);
    curr_chose_window = create_curr_choice_window(hWnd, in_hand.value(), mouse,
        main_window.get_cell_width(), main_window.get_cell_height(),
        callback);
    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);  // Форсирую перерисовку, т.к. появляется артефакт
    SendMessage(curr_chose_window, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(mouse.x, mouse.y));
}

// Заполняет поле возможных ходов для текущей фигуры
void MotionInput::calculate_possible_moves() {
    if (in_hand.has_value()) {
        all_moves = board->get_all_possible_moves(in_hand.value());
    }
}

void MotionInput::clear() {
    is_lbutton_down = false;
    DestroyWindow(curr_chose_window);
    is_curr_choice_moving = false;
    deactivate_by_click();
    deactivate_by_pos();
    in_hand = std::nullopt;
    input = Input{ Pos{0, -1}, Pos{-1, -1} };
    all_moves.clear();
}

void MotionInput::prepare(Color turn) {
    in_hand = board->get_fig(input.from);
    input.target = input.from;
    if (in_hand.has_value() && in_hand.value()->get_col() == turn) {
        calculate_possible_moves();
    }
}

void update_main_window_title(HWND hWnd) {
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
    
    SetWindowText(hWnd, title.c_str());
}

bool copy_repr_to_clip(HWND hWnd) {
    board_repr::BoardRepr board_repr{ board.get_repr(turn, save_all_moves) };
    auto board_repr_str = as_string<board_repr::BoardRepr>{}(board_repr);
    return cpy_str_to_clip(
        hWnd,
        board_repr_str
    );
}

/* Отрисовать фоновую доску */
void draw_board(HDC hdc) {
    for (int i{}; i < HEIGHT; ++i) {
        for (int j{}; j < WIDTH; ++j) {
            const RECT cell = main_window.get_cell(i, j);
            if ((i + j) % 2) {
                FillRect(hdc, &cell, CHECKERBOARD_ONE);
            }
            else {
                FillRect(hdc, &cell, CHECKERBOARD_TWO);
            }
        }
    }
}

// Положение курсора и выделенной клетки
void draw_input(HDC hdcMem, Input input) {
    static const HBRUSH RED{ CreateSolidBrush(RGB(255, 0, 0)) };
    static const HBRUSH BLUE{ CreateSolidBrush(RGB(0, 0, 255)) };
    const RECT from_cell = main_window.get_cell(change_axes(input.from));
    const RECT targ_cell = main_window.get_cell(change_axes(input.target));
    FillRect(hdcMem, &from_cell, RED);
    FillRect(hdcMem, &targ_cell, BLUE);
}

/* Отрисовать фигуры на поле (та, что в руке, не рисуется) */
void draw_figures_on_board(HDC hdc) {
    for (const auto& figure : board.get_all_figures()) {
        if (!motion_input.is_figure_dragged(figure->get_id())) {
            draw_figure(hdc, figure, figure->get_pos());
        }
    }
}

void change_checkerboard_color_theme(HWND hWnd) {
    if (window_state == WindowState::EDIT) {
        std::swap(CHECKERBOARD_ONE, CHECKERBOARD_TWO);
    }
    else if (window_state == WindowState::GAME) {
        std::swap(CHECKERBOARD_ONE, CHECKERBOARD_TWO);
    }
    else {
        assert(false);
    }
    InvalidateRect(hWnd, NULL, NULL);
}

void update_edit_menu_variables(HWND hWnd) {
    for (auto menu_id : { IDM_WHITE_START, IDM_BLACK_START, IDM_IDW_TRUE, IDM_IDW_FALSE }) {
        set_menu_checkbox(hWnd, menu_id, false);
    }

    if (turn == Color::White)
        set_menu_checkbox(hWnd, IDM_WHITE_START, true);
    else if (turn == Color::Black)
        set_menu_checkbox(hWnd, IDM_BLACK_START, true);

    if (board.get_idw() == true)
        set_menu_checkbox(hWnd, IDM_IDW_TRUE, true);
    else
        set_menu_checkbox(hWnd, IDM_IDW_FALSE, true);
}

void update_bot_menu_variables(HWND hWnd)
{
    for (auto menu_id : {
            IDM_TOGGLE_BOT,
            IDM_BOTCOLOR_WHITE, IDM_BOTCOLOR_BLACK,
            IDM_BOTDIFFICULTY_EASY, IDM_BOTDIFFICULTY_NORMAL, IDM_BOTDIFFICULTY_HARD,
            IDM_BOTTYPE_RANDOM,
        }) {
        set_menu_checkbox(hWnd, menu_id, false);
    }
    
    if (bot_type != bot::Type::None) {
        set_menu_checkbox(hWnd, IDM_TOGGLE_BOT, true);

        if (bot_turn == Color::White)
            set_menu_checkbox(hWnd, IDM_BOTCOLOR_WHITE, true);
        else if (bot_turn == Color::Black)
            set_menu_checkbox(hWnd, IDM_BOTCOLOR_BLACK, true);
        
        if (bot_difficulty == bot::Difficulty::D0)
            set_menu_checkbox(hWnd, IDM_BOTDIFFICULTY_EASY, true);
        else if (bot_difficulty == bot::Difficulty::D1)
            set_menu_checkbox(hWnd, IDM_BOTDIFFICULTY_NORMAL, true);
        else if (bot_difficulty == bot::Difficulty::D2)
            set_menu_checkbox(hWnd, IDM_BOTDIFFICULTY_HARD, true);
        else if (bot_difficulty == bot::Difficulty::D3)
            set_menu_checkbox(hWnd, IDM_BOTDIFFICULTY_VERYHARD, true);
            
        if (bot_type == bot::Type::Random)
            set_menu_checkbox(hWnd, IDM_BOTTYPE_RANDOM, true);
        else if (bot_type == bot::Type::NeuralNetwork)
            ;   // placeholder;
    }
    else {
        // without bot
    }
}

void update_game_menu_variables(HWND hWnd)
{
    update_bot_menu_variables(hWnd);

    for (auto menu_id : { 
            IDM_TOGGLE_SAVE_ALL_MOVES,
            IDM_SET_CHOICE_TO_QUEEN, IDM_SET_CHOICE_TO_ROOK, IDM_SET_CHOICE_TO_BISHOP, IDM_SET_CHOICE_TO_KNIGHT,
        }) {
        set_menu_checkbox(hWnd, menu_id, false);
    }

    if (save_all_moves)
        set_menu_checkbox(hWnd, IDM_TOGGLE_SAVE_ALL_MOVES, true);

    switch (chose)
    {
        case FigureType::Queen:
            set_menu_checkbox(hWnd, IDM_SET_CHOICE_TO_QUEEN, true);
            break;
            
        case FigureType::Rook:
            set_menu_checkbox(hWnd, IDM_SET_CHOICE_TO_ROOK, true);
            break;
           
        case FigureType::Bishop:
            set_menu_checkbox(hWnd, IDM_SET_CHOICE_TO_BISHOP, true);
            break;
            
        case FigureType::Knight:
            set_menu_checkbox(hWnd, IDM_SET_CHOICE_TO_KNIGHT, true);
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
    HWND hWindow{};
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_FIGURES_LIST));
    wc.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_FIGURES_LIST));
    wc.lpfnWndProc = figures_list_wndproc;
    wc.lpszClassName = FIGURES_LIST_WINDOW_CLASS_NAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    const auto create_window = [&hWindow, &parent]() -> HWND {
        if (hWindow = CreateWindow(FIGURES_LIST_WINDOW_CLASS_NAME, FIGURES_LIST_WINDOW_TITLE, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
            FIGURES_LIST_WINDOW_DEFAULT_POS.x, FIGURES_LIST_WINDOW_DEFAULT_POS.y,
            figures_list.get_width_with_extra(), figures_list.get_height_with_extra(),
            parent, nullptr, hInst, nullptr), !hWindow)
            return nullptr;
        ShowWindow(hWindow, SW_SHOWDEFAULT);
        UpdateWindow(hWindow);
        return hWindow;
    };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}