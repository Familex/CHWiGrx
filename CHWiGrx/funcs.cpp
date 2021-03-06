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
    BOOL bRet = 0;
    while (bRet = GetMessage(&msg, nullptr, 0, 0)) {
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
        window_stats.get_window_pos_x(), window_stats.get_window_pos_y(),
        window_stats.get_real_width(), window_stats.get_real_height(),
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

void load_pieces_bitmaps(HINSTANCE hInstance) {
    pieces_bitmaps['B']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_PAWN));
    pieces_bitmaps['B']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_ROOK));
    pieces_bitmaps['B']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KING));
    pieces_bitmaps['B']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_QUEEN));
    pieces_bitmaps['B']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_BISHOP));
    pieces_bitmaps['B']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KNIGHT));
    pieces_bitmaps['W']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_PAWN));
    pieces_bitmaps['W']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_ROOK));
    pieces_bitmaps['W']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KING));
    pieces_bitmaps['W']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_QUEEN));
    pieces_bitmaps['W']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_BISHOP));
    pieces_bitmaps['W']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KNIGHT));
}

/// <summary>
/// Рисует фигуру на контекст изображения
/// </summary>
/// <param name="hdc">Контекст отображения</param>
/// <param name="figure">Фигура для отрисовки</param>
/// <param name="w_beg">Левая координата</param>
/// <param name="h_beg">Верхняя координата</param>
/// <param name="is_transpanent">Сделать ли фон прозрачным</param>
void draw_figure(HDC hdc, Color col, FigureType type, pos begin_paint, bool is_transpanent, int h, int w) {
    const int h_beg = begin_paint.x * h;
    const int w_beg = begin_paint.y * w;
    int h_end = h_beg + h;
    int w_end = w_beg + h;
    HBITMAP hBitmap = pieces_bitmaps[col][type];
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HGDIOBJ hOldBitmap = SelectObject(hdcMem, hBitmap);
    SetStretchBltMode(hdc, STRETCH_DELETESCANS);
    if (is_transpanent) {
        TransparentBlt(hdc, w_beg, h_beg,
            w, h,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, RGB(255, 0, 0));
    }
    else {
        StretchBlt(hdc, w_beg, h_beg,
            w, h,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    }
    DeleteDC(hdcMem);
}

void draw_figure(HDC hdc, Color col, FigureType type, pos begin_paint, bool is_transpanent) {
    draw_figure(hdc, col, type, begin_paint, is_transpanent, window_stats.get_cell_height(), window_stats.get_cell_width());
}

/// <summary>
/// Совершает ход
/// </summary>
/// <param name="hWnd">Дескриптор окна</param>
void make_move(HWND hWnd) {
    if (!motion_input.is_current_turn(turn)) {
        motion_input.clear();
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }

    auto [is_legal_move, move_rec] = board.provide_move(
        motion_input.get_in_hand(), motion_input.get_input(),
        turn, [c = chose] { return c; }
    );

    if (!is_legal_move) {
        motion_input.clear();
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }

    #ifdef ALLOCATE_CONSOLE
        std::cout << "Curr move was: " << move_rec.as_string() << '\n';
    #endif // ALLOCATE_CONSOLE

    board.set_last_move({ motion_input.get_in_hand(), motion_input.get_input(), turn, move_rec.ms, move_rec.promotion_choice });
    turn.to_next();
    motion_input.clear();
    InvalidateRect(hWnd, NULL, NULL);

    GameEndType curr_game_end_state = board.game_end_check(turn);

    if (curr_game_end_state != GameEndType::NotGameEnd) {
        std::wstring body = L"";
        std::wstring head = L"Game end";
        switch (curr_game_end_state) {
        case GameEndType::Checkmate: case GameEndType::Stalemate: {
            auto who_next = turn.what_next();
            body = who_next == Color::Type::White ? L"White wins!" :
                who_next == Color::Type::Black ? L"Black wins!" :
                L"None wins!";
            break;
        }
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
                throw std::runtime_error("unexpected game end");
                break;
        }
        auto result = MessageBox(hWnd, (body + L"\nCopy board to clip?").c_str(), head.c_str(), MB_YESNO);
        if (result == IDYES) {
            copy_repr_to_clip();
        }
        restart();
        InvalidateRect(hWnd, NULL, NULL);
    }

    update_check_title(hWnd);
}

void restart() {
    BoardRepr tmp_board_repr{ start_board_repr };
    board.reset(tmp_board_repr);
    motion_input.clear();
    turn = tmp_board_repr.get_turn();
}

// Копирует строку в буффер обмена
void cpy_str_to_clip(const std::string& buff)
{
    size_t len = buff.length() + 1;
    using symbol = char;
    size_t size = len * sizeof(symbol);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (hMem == NULL) return;
    LPVOID hMemLock = GlobalLock(hMem);
    if (hMemLock == NULL) return;
    memcpy(hMemLock, buff.c_str(), len);
    GlobalUnlock(hMem);
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();
    }
}

// Возвращает строку из буффера обмена
std::string take_str_from_clip() {
    if (!OpenClipboard(nullptr)) return "";
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) return "";
    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) return "";
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
HWND create_curr_choice_window(HWND parent, Figure* in_hand, POINT mouse, int w, int h, const WNDPROC callback, LPCWSTR class_name) {
    UnregisterClass(class_name, GetModuleHandle(nullptr));
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    HWND hWindow{};
    Figure* for_storage = in_hand;  // Возможно нужно копировать TODO
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(in_hand);
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(nullptr, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
    wc.hIcon = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_CHWIGRX));
    wc.hIconSm = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_CHWIGRX));
    wc.lpfnWndProc = callback;
    wc.lpszClassName = class_name;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    const auto create_window = [&hWindow, &parent, &mouse, &w, &h, for_storage, &class_name]() -> HWND {
        if (hWindow = CreateWindow(class_name, L"", WS_POPUP | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            mouse.x - w / 2, mouse.y - h / 2, w, h, parent, nullptr, nullptr, nullptr), !hWindow)
            return nullptr
        ;
        SetWindowPos(hWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        SetWindowLongPtr(hWindow, GWL_EXSTYLE, GetWindowLongPtr(hWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hWindow, RGB(255, 0, 0), 255, LWA_COLORKEY);
        SetWindowLongPtr(hWindow, GWLP_USERDATA, (LONG_PTR)for_storage);
        ShowWindow(hWindow, SW_SHOWDEFAULT);
        UpdateWindow(hWindow);
        return hWindow;
    };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}

/// <summary>
/// Функция обрабатывающая WM_LBUTTONUP
/// </summary>
/// <param name="hWnd">Дескриптор окна</param>
/// <param name="wParam"></param>
/// <param name="lParam"></param>
/// <param name="where_fig">Позиция фигуры</param>
void on_lbutton_up(HWND hWnd, WPARAM wParam, LPARAM lParam, pos where_fig) {
    motion_input.reset_lbutton_down();
    motion_input.reset_single();
    if (motion_input.is_pair()) {
        make_move(hWnd);
        motion_input.clear();
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }
    else {
        motion_input.set_target(where_fig.x, where_fig.y);
        if (motion_input.target_at_input()) {
            if (window_stats.get_prev_lbutton_click() != pos(HIWORD(lParam), LOWORD(lParam))) { // Отпустили в пределах клетки, но в другом месте
                motion_input.clear();
                InvalidateRect(hWnd, NULL, NULL);
                return;
            }
            motion_input.prepare(turn);
            motion_input.toggle_pair_input();
            InvalidateRect(hWnd, NULL, NULL);
        }
        else {
            make_move(hWnd);
        }
    }
}

bool is_legal_board_repr(const std::string& str) {
#ifdef USE_REGEX_BOARD_REPR_CHECK
    // regex не актуален, как минимум в мету были добавлены рокировки
    const static std::regex valid_board_repr(R"(((-?\d+;){3}\w;\w;)+\[T?F?W?B?\](<(\d+\.\d+\.\d+\.\w\.\w\.\d+\.\d+\.\d+\.\d+\.\w\.\w\.\{(\w,)*\}\.\{(-?\d+,)*\}\.\{((-?\d+,){5})*\}\.\{(-?\d+,)*\}\$)*>){2}~((-?\d+,){3}\w,\w,)*)");
    const static std::regex valid_board_repr_light(R"((?:(?:-?\d+;){3}\w;\w;)+\[T?F?W?B?\](?:<(?:\d+\.\d+\.\d+\.\w\.\w\.\d+\.\d+\.\d+\.\d+\.\w\.\w\.\{(?:\w,)*\}\.\{(?:-?\d+,)*\}\.\{(?:(?:-?\d+,){5})*\}\.\{(?:-?\d+,)*\}\$)*>){2}~(?:(?:-?\d+,){3}\w,\w,)*)");
    return std::regex_match(str, valid_board_repr_light); // Программа ложится под stackoverflow
#endif // USE_REGEX_BOARD_REPR_CHECK

    return (str.find('<') != str.npos &&
        str.find('>') != str.npos &&
        str.find('[') != str.npos &&
        str.find(']') != str.npos &&
        str.find('~') != str.npos);
}

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

// Создаёт окно в выбранной фигурой и привязывает к мыши
void MotionInput::init_curr_choice_window(HWND hWnd) {
    is_curr_choice_moving = true;
    POINT mouse{};
    GetCursorPos(&mouse);
    curr_chose_window = create_curr_choice_window(hWnd, in_hand, mouse, window_stats.get_cell_width(), window_stats.get_cell_height(),
        [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
            static const int TO_DESTROY_TIMER_ID{ 99 }; // не могу захватить из вне
            static const int TO_DESTROY_ELAPSE{ 5 };    // да и не надо так-то
            switch (uMsg) {
            case WM_CREATE:
                SetTimer(hWnd, TO_DESTROY_TIMER_ID, TO_DESTROY_ELAPSE, NULL);
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
                HWND parent = GetParent(hWnd);
                POINT cur_pos{};
                GetCursorPos(&cur_pos);
                RECT parent_window;
                GetWindowRect(parent, &parent_window);
                pos where_fig = window_stats.divide_by_cell_size(
                    (cur_pos.y - parent_window.top - HEADER_HEIGHT),
                    (cur_pos.x - parent_window.left)
                );
                on_lbutton_up(parent, wParam, lParam, where_fig);
                InvalidateRect(parent, NULL, NULL);
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
                    draw_figure(hdc, in_hand->get_col(), in_hand->get_type(), pos(0, 0), false);
                }
                EndPaint(hWnd, &ps);
            }
            break;
            default:
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }
            return static_cast<LRESULT>(0);
        });
    InvalidateRect(hWnd, NULL, NULL);
    SendMessage(curr_chose_window, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(mouse.x, mouse.y));
}

// Заполняет поле возможных ходов для текущей фигуры
void MotionInput::calculate_possible_moves() {
    all_moves.clear();
    for (const auto& [is_eat, move_pos] : board->get_all_possible_moves(in_hand)) {
        if (in_hand->get_type() == FigureType::Type::King) {
            if (is_eat
                ? not board->check_for_when(in_hand->get_col(), { in_hand->get_pos(), move_pos }, move_pos)
                : not board->check_for_when(in_hand->get_col(), { in_hand->get_pos() }, move_pos)
                ) {
                all_moves.push_back({ is_eat, move_pos });
            }
        }
        else {
            Figure* in_hand_in_tmp = FigureFabric::instance()->submit_on(in_hand, move_pos);
            bool check = (is_eat
                ? board->check_for_when(in_hand->get_col(), { in_hand->get_pos(), move_pos }, {}, { in_hand_in_tmp })
                : board->check_for_when(in_hand->get_col(), { in_hand->get_pos() }, {}, { in_hand_in_tmp })
                );
            if (not check) {
                all_moves.push_back({ is_eat, move_pos });
            }
        }
    }
}

void MotionInput::clear() {
    is_lbutton_down = false;
    DestroyWindow(curr_chose_window);
    is_curr_choice_moving = false;
    reset_input_order();
    in_hand = board->get_default_fig();
    input = { {0, -1}, {-1, -1} };
    all_moves.clear();
}

void MotionInput::prepare(Color turn) {
    in_hand = board->get_fig(input.from);
    input.target = input.from;
    if (in_hand->get_col() == turn && not in_hand->empty()) {
        calculate_possible_moves();
    }
}

void update_check_title(HWND hWnd) {
    std::wstring curr_text = L"CHWiGrx";
    if (board.is_empty()) {
        curr_text += L" [Empty board]";
    }
    else if (board.check_for_when(turn)) {
        curr_text += L" [Check to ";
        curr_text += turn == Color::Type::White ? L"White" : turn == Color::Type::Black ? L"Black" : L"None";
        curr_text += L"]";
    }
    SetWindowText(hWnd, curr_text.c_str());
}

void copy_repr_to_clip() {
    BoardRepr board_repr = board.get_repr(save_all_moves);
    board_repr.set_turn(turn);
    std::string board_repr_str = board_repr.as_string();
    cpy_str_to_clip(
        board_repr_str
    );
}

/* Отрисовать фоновую доску */
void draw_board(HDC hdc) {
    for (int i{}; i < HEIGHT; ++i) {
        for (int j{}; j < WIDTH; ++j) {
            const RECT cell = window_stats.get_cell(i, j);
            if ((i + j) % 2) {
                FillRect(hdc, &cell, CHECKERBOARD_ONE);
            }
            else {
                FillRect(hdc, &cell, CHECKERBOARD_TWO);
            }
        }
    }
}

/* Отрисовать фигуры на поле (та, что в руке, не рисуется) */
void draw_figures_on_board(HDC hdc) {
    for (const auto& figure : board.all_figures()) {
        if (!motion_input.is_figure_dragged(figure->get_id())) {
            draw_figure(hdc, figure->get_col(), figure->get_type(), figure->get_pos());
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

    if (turn == Color::Type::White)
        set_menu_checkbox(hWnd, IDM_WHITE_START, true);
    else if (turn == Color::Type::Black)
        set_menu_checkbox(hWnd, IDM_BLACK_START, true);

    if (board.get_idw() == true)
        set_menu_checkbox(hWnd, IDM_IDW_TRUE, true);
    else
        set_menu_checkbox(hWnd, IDM_IDW_FALSE, true);
}

HWND create_choice_window(HWND parent) {
    UnregisterClass(CHOICE_WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    HWND hWindow{};
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(hInst, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CHWIGRX));
    wc.hIconSm = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CHWIGRX));
    wc.lpfnWndProc = choice_window_proc;
    wc.lpszClassName = CHOICE_WINDOW_CLASS_NAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    const auto create_window = [&hWindow, &parent]() -> HWND {
        if (hWindow = CreateWindow(CHOICE_WINDOW_CLASS_NAME, CHOICE_WINDOW_TITLE, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
            CHOICE_WINDOW_DEFAULT_POS.x, CHOICE_WINDOW_DEFAULT_POS.y,
            CHOICE_WINDOW_DEFAULT_DIMENTIONS.x, CHOICE_WINDOW_DEFAULT_DIMENTIONS.y,
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