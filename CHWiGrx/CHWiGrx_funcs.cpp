#include "CHWiGrx_declarations.hpp"

ATOM register_main_window_class(HINSTANCE hInstance, LPTSTR szTitle, LPTSTR szWindowClass) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = main_proc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHWIGRX));
    wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CHWIGRX);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL init_instance(HINSTANCE hInstance, LPTSTR szTitle, LPTSTR szWindowClass, int nCmdShow)
{
    hInst = hInstance; // ��������� ������ ���������� � ���������� ����������

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        window_stats.get_window_pos_x(), window_stats.get_window_pos_y(),
        window_stats.get_real_width(), window_stats.get_real_height(),
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// ���������� ��������� ��� ���� "� ���������".
INT_PTR CALLBACK about_proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void draw_figure(HDC hdc, const Figure& figure, int w_beg, int h_beg, bool is_transpanent) {
    if (h_beg == -1) h_beg = figure.position.x * window_stats.get_cell_height();
    if (w_beg == -1) w_beg = figure.position.y * window_stats.get_cell_width();
    int h_end = h_beg + window_stats.get_cell_height();
    int w_end = w_beg + window_stats.get_cell_width();
    Color col = figure.color;
    FigureType type = figure.type;
    HBITMAP hBitmap = pieces_bitmaps[col][type];
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HGDIOBJ hOldBitmap = SelectObject(hdcMem, hBitmap);
    SetStretchBltMode(hdc, STRETCH_DELETESCANS);
    if (is_transpanent) {
        TransparentBlt(hdc, w_beg, h_beg,
            window_stats.get_cell_width(), window_stats.get_cell_height(),
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, RGB(255, 0, 0));
    }
    else {
        StretchBlt(hdc, w_beg, h_beg,
            window_stats.get_cell_width(), window_stats.get_cell_height(),
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    }
    DeleteDC(hdcMem);
}

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

    #ifdef DEBUG
        std::cout << "Curr move was: " << move_rec.as_string() << '\n';
    #endif

    board.set_last_move({ *motion_input.get_in_hand(), motion_input.get_input(), turn, move_rec.ms, move_rec.promotion_choice });
    turn.to_next();
    motion_input.clear();
    InvalidateRect(hWnd, NULL, NULL);

    if (board.game_end(turn)) {
        TCHAR tmp2[] = { (TCHAR)turn.what_next(), ' ', 'w', 'i', 'n', 's', '!', '\0' };
        MessageBox(hWnd, tmp2, L"GAME END", NULL);
        restart();
        InvalidateRect(hWnd, NULL, NULL);
    }

    std::wstring curr_text = L"CHWiGrx";
    if (std::find(move_rec.ms.side_evs.begin(), move_rec.ms.side_evs.end(), SideEvent::CHECK) != move_rec.ms.side_evs.end()) {
        curr_text += L" [Check to ";
        curr_text += turn == EColor::White ? L"White" : L"Black";
        curr_text += L"]!";
    }
    SetWindowText(hWnd, curr_text.c_str());
}

void restart() {
    board.reset(start_board_repr);
    motion_input.clear();
    turn = start_board_repr.get_turn();
}

void cpy_str_to_clip(const std::string& buff)
{
    size_t len = buff.length() + 1;
    using symbol = char;
    size_t size = len * sizeof(symbol);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (hMem == NULL) return;
    memcpy(GlobalLock(hMem), buff.c_str(), len);
    GlobalUnlock(hMem);
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();
    }
}

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

HWND create_curr_choice_window(HWND parent, Figure in_hand, POINT pos, int w, int h, const WNDPROC callback, LPCWSTR class_name) {
    UnregisterClass(class_name, GetModuleHandle(nullptr));
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    HWND hWindow{};
    Figure* for_storage = new Figure(in_hand);
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(in_hand);
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.lpfnWndProc = callback;
    wc.lpszClassName = class_name;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    const auto create_window = [&hWindow, &parent, &pos, &w, &h, for_storage, &class_name]() -> HWND {
        if (hWindow = CreateWindow(class_name, L"", WS_POPUP | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            pos.x - w / 2, pos.y - h / 2, w, h, parent, nullptr, nullptr, nullptr), !hWindow)
            return nullptr
        ;
        SetWindowLongPtr(hWindow, GWLP_USERDATA, (LONG_PTR)for_storage);
        ShowWindow(hWindow, SW_SHOWDEFAULT);
        UpdateWindow(hWindow);
        return hWindow;
    };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}

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
            if (window_stats.get_prev_lbutton_click() != pos(HIWORD(lParam), LOWORD(lParam))) { // ��������� � �������� ������, �� � ������ �����
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
    const static std::regex valid_board_repr(R"(((-?\d+;){3}\w;\w;)+\[T?F?W?B?\](<(\d+\.\d+\.\d+\.\w\.\w\.\d+\.\d+\.\d+\.\d+\.\w\.\w\.\{(\w,)*\}\.\{(-?\d+,)*\}\.\{((-?\d+,){5})*\}\.\{(-?\d+,)*\}\$)*>){2}~((-?\d+,){3}\w,\w,)*)");
    const static std::regex valid_board_repr_light(R"((?:(?:-?\d+;){3}\w;\w;)+\[T?F?W?B?\](?:<(?:\d+\.\d+\.\d+\.\w\.\w\.\d+\.\d+\.\d+\.\d+\.\w\.\w\.\{(?:\w,)*\}\.\{(?:-?\d+,)*\}\.\{(?:(?:-?\d+,){5})*\}\.\{(?:-?\d+,)*\}\$)*>){2}~(?:(?:-?\d+,){3}\w,\w,)*)");
    return std::regex_match(str, valid_board_repr_light); // ��������� ������� ��� stackoverflow
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

void MotionInput::init_curr_choice_window(HWND hWnd) {
    is_curr_choice_moving = true;
    POINT cur_pos{};
    GetCursorPos(&cur_pos);
    curr_chose_window = create_curr_choice_window(hWnd, *in_hand, cur_pos, window_stats.get_cell_width(), window_stats.get_cell_height(),
        [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
            switch (uMsg) {
            case WM_EXITSIZEMOVE: // ������ ��������
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
            case WM_NCHITTEST:  // ��� ��������� ������� ���� ���������� ��������������
                return (LRESULT)HTCAPTION;
            case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);
                Figure* in_hand = (Figure*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                if (in_hand) {
                    draw_figure(hdc, *in_hand, 0, 0, false);
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
    SetWindowPos(curr_chose_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetWindowLongPtr(curr_chose_window, GWL_EXSTYLE, GetWindowLongPtr(curr_chose_window, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(curr_chose_window, RGB(255, 0, 0), 255, LWA_COLORKEY);
    SendMessage(curr_chose_window, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(cur_pos.x, cur_pos.y));
}

void MotionInput::calculate_possible_moves() {
    all_possible_moves.clear();
    for (const auto& [is_eat, move_pos] : board->get_all_possible_moves(*in_hand)) {
        if (in_hand->type == EFigureType::King
            ? (is_eat
                ? not board->check_for_when(in_hand->color, { in_hand->position, move_pos }, move_pos)
                : not board->check_for_when(in_hand->color, { in_hand->position }, move_pos)
                )
            : (is_eat
                ? not board->check_for_when(in_hand->color, { in_hand->position, move_pos }, {}, { in_hand->submit_on(move_pos) })
                : not board->check_for_when(in_hand->color, { in_hand->position }, {}, { in_hand->submit_on(move_pos) })
                )
            ) {
            all_possible_moves.push_back({ is_eat, move_pos });
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
    all_possible_moves.clear();
}

void MotionInput::prepare(Color turn) {
    in_hand = board->get_fig(input.from);
    input.target = input.from;
    if (in_hand->color == turn && in_hand->id != ERR_ID) {
        calculate_possible_moves();
    }
}