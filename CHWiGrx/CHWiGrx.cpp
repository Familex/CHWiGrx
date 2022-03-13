// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
// CHWiGrx.cpp : Определяет точку входа для приложения.
//
// #define DEBUG

#ifdef DEBUG
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#endif // DEBUG

#include "framework.h"
#include "CHWiGrx.h"
#define MAX_LOADSTRING 100
#define VK_0 48
#define VK_1 49
#define VK_2 50
#define VK_3 51
#define VK_4 52
#define VK_5 53
#define VK_6 54
#define VK_7 55
#define VK_8 56
#define VK_9 57


// Глобальные переменные:
HDC hdc;
HDC hdcMem;
HGDIOBJ hOld;
HBITMAP hbmMem;
PAINTSTRUCT ps;
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HBITMAP black_pawn, old_bmap;
const int EXTRA_WINDOW_WIDTH = 16;
const int EXTRA_WINDOW_HEIGHT = 59;
int window_width = 500;
int window_height = 500;
pos window_pos{ 300, 300 };
pos grab_error{ 0, 0 };
Input input{ {-1, -1}, {-1, -1} };
int cell_width = window_width / WIDTH;
int cell_height = window_height / HEIGHT;
const HBRUSH CHECKERBOARDBRIGHT { CreateSolidBrush(RGB(50,  50,  50 )) };
const HBRUSH CHECKERBOARDDARK   { CreateSolidBrush(RGB(128, 128, 128)) };
const HBRUSH BLACK              { CreateSolidBrush(RGB(0,   0,   0  )) };
const HBRUSH WHITE              { CreateSolidBrush(RGB(255, 255, 255)) };
const HBRUSH RED                { CreateSolidBrush(RGB(255, 0,   0  )) };
const HBRUSH BLUE               { CreateSolidBrush(RGB(0,   0,   255)) };
const HBRUSH GREEN              { CreateSolidBrush(RGB(0,   255, 0  )) };
const HBRUSH DARK_GREEN         { CreateSolidBrush(RGB(0,   150, 0  )) };
const int INDENTATION_FROM_EDGES{ 1 };
FigureBoard board{};
MoveMessage ms{};
char chose{ 'Q' };
Color turn{ EColor::White };
POINT cursor{};
pos prev_lbutton_click{};
int  input_order_by_one{ 0 };
bool input_order_by_two{ false };
std::list<Figure>::iterator in_hand = board.get_default_fig();
std::list<std::pair<bool, pos>> all_possible_moves{};
std::map<char, std::map<char, HBITMAP>> pieces_bitmaps;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM               MyRegisterClass(HINSTANCE hInstance);
BOOL               InitInstance(HINSTANCE, int);
LRESULT CALLBACK   WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK   About(HWND, UINT, WPARAM, LPARAM);
void               draw_figure(HDC, const Figure&, int=-1, int=-1);
void               clear_current_globals();
void               make_move(HWND);
void               init_globals(pos, Color);
void reset_input_order() { input_order_by_one = 0; input_order_by_two = false; }
void               restart();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CHWIGRX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    pieces_bitmaps['B']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_PAWN   ));
    pieces_bitmaps['B']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_ROOK   ));
    pieces_bitmaps['B']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KING   ));
    pieces_bitmaps['B']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_QUEEN  ));
    pieces_bitmaps['B']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_BISHOP ));
    pieces_bitmaps['B']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KNIGHT ));
    pieces_bitmaps['W']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_PAWN   ));
    pieces_bitmaps['W']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_ROOK   ));
    pieces_bitmaps['W']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KING   ));
    pieces_bitmaps['W']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_QUEEN  ));
    pieces_bitmaps['W']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_BISHOP ));
    pieces_bitmaps['W']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KNIGHT ));


    #ifdef DEBUG
    if (AllocConsole()) {
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
    }
    #endif // DEBUG

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    
    // Цикл основного сообщения:
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHWIGRX));
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

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHWIGRX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CHWIGRX);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       window_pos.x, window_pos.y, window_width + EXTRA_WINDOW_WIDTH, window_height + EXTRA_WINDOW_HEIGHT, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_RESTART:
                restart();
                InvalidateRect(hWnd, NULL, NULL);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
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
                    reset_input_order();
                    clear_current_globals();
                    InvalidateRect(hWnd, NULL, NULL);
                    return 0;
                case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
                {
                    pos shift{ wParam == VK_LEFT ? pos(0, -1) : (wParam == VK_RIGHT ? pos(0, 1) : (wParam == VK_UP ? pos(-1, 0) : pos(1, 0))) };
                    if (!input_order_by_two)
                        input.from.loop_add(shift, HEIGHT, WIDTH);
                    else
                        input.target.loop_add(shift, HEIGHT, WIDTH);
                    InvalidateRect(hWnd, NULL, NULL);
                    return 0;
                }
                case VK_RETURN:
                    if (input_order_by_two) {
                        make_move(hWnd);
                        clear_current_globals();
                        InvalidateRect(hWnd, NULL, NULL);
                        return 0;
                    }
                    else {
                        init_globals(input.from, turn);
                    }
                    input_order_by_two = !input_order_by_two;
                    InvalidateRect(hWnd, NULL, NULL);
                    return 0;
            }
            switch (input_order_by_one) {
            case 0:
                input.from.x = cord;
                ++input_order_by_one;
                break;
            case 1:
                input.from.y = cord;
                init_globals(input.from, turn);
                ++input_order_by_one;
                break;
            case 2:
                input.target.x = cord;
                ++input_order_by_one;
                break;
            case 3:
                input.target.y = cord;
                make_move(hWnd);
                reset_input_order();
                break;
            }
            InvalidateRect(hWnd, NULL, NULL);
            break;
        }
    case WM_RBUTTONDOWN: 
        reset_input_order();
        clear_current_globals();
        InvalidateRect(hWnd, NULL, NULL);
        break;
    case WM_LBUTTONDOWN:
        prev_lbutton_click = { HIWORD(lParam), LOWORD(lParam) };
        input_order_by_one = 0;
        if (input_order_by_two) {
            input.target = { HIWORD(lParam) / cell_height, LOWORD(lParam) / cell_width };
            InvalidateRect(hWnd, NULL, NULL);
            return 0;
        }
        input.from = { HIWORD(lParam) / cell_height, LOWORD(lParam) / cell_width };
        init_globals(input.from, turn);
        /* grab_error = { // на память
            (int)(cursor.x - (in_hand->position.y + .5) * cell_width)  - (cursor.x - in_hand->position.y * cell_width),
            (int)(cursor.y - (in_hand->position.x + .5) * cell_height) - (cursor.y - in_hand->position.x * cell_height)
        }; */
        grab_error = {
            -cell_width / 2  - (in_hand->position.y - cursor.x / cell_width ) * cell_width,
            -cell_height / 2 - (in_hand->position.x - cursor.y / cell_height) * cell_height
        };
        InvalidateRect(hWnd, NULL, NULL);
        break;
    case WM_LBUTTONUP:
        input_order_by_one = 0;
        if (input_order_by_two) {
            make_move(hWnd);
            clear_current_globals();
            InvalidateRect(hWnd, NULL, NULL);
            return 0;
        }
        else {
            input.target = { HIWORD(lParam) / cell_height, LOWORD(lParam) / cell_width };
            if (input.from == input.target) {
                if (prev_lbutton_click != pos(HIWORD(lParam), LOWORD(lParam))) { // Отпустили в пределах клетки, но в другом месте
                    clear_current_globals();
                    InvalidateRect(hWnd, NULL, NULL);
                    return 0;
                }
                init_globals(input.from, turn);
                input_order_by_two = !input_order_by_two;
                InvalidateRect(hWnd, NULL, NULL);
            }
            else {
                make_move(hWnd);
            }
        }
        break;
    case WM_MOVE:
        window_pos = { LOWORD(lParam), HIWORD(lParam) };
        return 0;
    case WM_MOUSEMOVE:
        GetCursorPos(&cursor); // ScreenToClient не работает почему-то
        cursor.x -= window_pos.x;
        cursor.y -= window_pos.y;
        if (in_hand->id != ERR_ID) {
            InvalidateRect(hWnd, NULL, NULL);
        }
        break;
    case WM_SIZE:
        window_width = LOWORD(lParam);
        window_height = HIWORD(lParam);
        cell_width = window_width / WIDTH;
        cell_height = window_height / HEIGHT;
        InvalidateRect(hWnd, NULL, NULL);
        break;
    case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);

            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc, window_width, window_height);
            hOld = SelectObject(hdcMem, hbmMem);

            {
                for (int i{}; i < HEIGHT; ++i) {
                    for (int j{}; j < WIDTH; ++j) {
                        int h_beg = i * cell_height + INDENTATION_FROM_EDGES;
                        int w_beg = j * cell_width + INDENTATION_FROM_EDGES;
                        int h_end = h_beg + cell_height + INDENTATION_FROM_EDGES;
                        int w_end = w_beg + cell_width + INDENTATION_FROM_EDGES;
                        if ((i + j) % 2) {
                            SelectObject(hdcMem, CHECKERBOARDBRIGHT);
                        }
                        else {
                            SelectObject(hdcMem, CHECKERBOARDDARK);
                        }
                        Rectangle(hdcMem, w_beg, h_beg, w_end, h_end);
                    }
                }
            }
            {
                for (const auto& [is_eat, move_pos] : all_possible_moves) {
                    if (is_eat) {
                        SelectObject(hdcMem, DARK_GREEN);
                    }
                    else {
                        SelectObject(hdcMem, GREEN);
                    }
                    Rectangle(hdcMem,
                        move_pos.y * cell_width,
                        move_pos.x * cell_height,
                        (move_pos.y + 1) * cell_width + INDENTATION_FROM_EDGES + INDENTATION_FROM_EDGES,
                        (move_pos.x + 1) * cell_height + INDENTATION_FROM_EDGES + INDENTATION_FROM_EDGES
                    );
                }
            }
            {
                SelectObject(hdcMem, RED);
                Rectangle(hdcMem,
                    input.from.y * cell_width + INDENTATION_FROM_EDGES,
                    input.from.x * cell_height + INDENTATION_FROM_EDGES,
                    (input.from.y + 1)* cell_width + INDENTATION_FROM_EDGES * 2,
                    (input.from.x + 1)* cell_height + INDENTATION_FROM_EDGES * 2
                );
                SelectObject(hdcMem, BLUE);
                Rectangle(hdcMem,
                    input.target.y* cell_width + INDENTATION_FROM_EDGES,
                    input.target.x* cell_height + INDENTATION_FROM_EDGES,
                    (input.target.y + 1)* cell_width + INDENTATION_FROM_EDGES * 2,
                    (input.target.x + 1)* cell_height + INDENTATION_FROM_EDGES * 2
                );
            }
            {
                for (const auto& figure : board.all_figures()) {
                    if (not input_order_by_two && in_hand->id == figure.id) {
                        draw_figure(hdcMem, figure, cursor.x + grab_error.x, cursor.y + grab_error.y);
                    }
                    else {
                        draw_figure(hdcMem, figure);
                    }
                }
            }

            BitBlt(hdc, 0, 0, window_width, window_height, hdcMem, 0, 0, SRCCOPY);

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

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

void draw_figure(HDC hdc, const Figure& figure, int w_beg, int h_beg) {
    if (h_beg == -1) h_beg = figure.position.x * cell_height;
    if (w_beg == -1) w_beg = figure.position.y * cell_width;
    int h_end = h_beg + cell_height;
    int w_end = w_beg + cell_width;
    Color col = figure.color;
    FigureType type = figure.type;
    HBITMAP hBitmap = pieces_bitmaps[col][type];
    BITMAP bm;
    GetObject(hBitmap, sizeof(BITMAP), &bm);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HGDIOBJ hOldBitmap = SelectObject(hdcMem, hBitmap);
    SetStretchBltMode(hdc, STRETCH_HALFTONE);
    TransparentBlt(hdc, w_beg, h_beg, cell_width, cell_height,
        hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, RGB(255, 0, 0));
    DeleteDC(hdcMem);
}

void clear_current_globals() {
    reset_input_order();
    in_hand = board.get_default_fig();
    input = { {0, -1}, {-1, -1} };
    all_possible_moves.clear();
}

void make_move(HWND hWnd) {
    if (in_hand->color != turn) {
        clear_current_globals();
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }

    try {
        ms = board.move_check(in_hand, input);
    }
    catch (std::invalid_argument) {
        clear_current_globals();
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }

    switch (ms.main_ev)
    {
    case MainEvent::MOVE: case MainEvent::LMOVE:
        in_hand->move_to(input.target);
        break;
    case MainEvent::EN_PASSANT: case MainEvent::EAT:
        in_hand->move_to(input.target);
        for (const auto& it : ms.to_eat) board.capture_figure(it);
        break;
    case MainEvent::CASTLING:
        if (board.has_castling(turn, ms.to_move.back().first->id)) {
            in_hand->move_to(input.target);
            for (auto& [who, whereinto] : ms.to_move) {
                who->move_to(whereinto);
            }
        }
        else {
            goto continue_flag;
        }
        break;
    case MainEvent::E:
        MessageBox(hWnd, L"MainEvent::E", L"Error", NULL);
    }

    InvalidateRect(hWnd, NULL, NULL);

    for (const auto& s_ev : ms.side_evs) {
        switch (s_ev)
        {
        case SideEvent::CASTLING_BREAK:
            if (not ms.what_castling_breaks.empty() &&
                board.has_castling(turn, ms.what_castling_breaks.back())
                ) {
                for (const Id& id : ms.what_castling_breaks) {
                    board.off_castling(turn, id);
                }
            }
            break;
        case SideEvent::PROMOTION:
            chose = 'Q';
            do {
                // TODO
            } while (std::string("QRHB").find(chose) == std::string::npos);
            in_hand->type = { chose };
            break;
        case SideEvent::CHECK:
            MessageBox(hWnd, L"Check", L"Check", NULL);
            break;
        case SideEvent::E:
            MessageBox(hWnd, L"SideEvent::E", L"Error", NULL);
        }

    }

    turn.to_next();
    board.set_last_move({ ms, in_hand });
    clear_current_globals();
    if (board.game_end(turn)) {
        TCHAR tmp2[] = { turn.what_next(), ' ', 'w', 'i', 'n', 's', '!', '\0'};
        MessageBox(hWnd, tmp2, L"GAME END", NULL);
        restart();
        InvalidateRect(hWnd, NULL, NULL);
    }
continue_flag:
    InvalidateRect(hWnd, NULL, NULL);
}

void init_globals(pos from, Color turn) {
    in_hand = board.get_fig(from);
    input.target = from;
    if (in_hand->color == turn) {
        all_possible_moves.clear();
        for (const auto& [is_eat, move_pos] : board.get_all_possible_moves(*in_hand)) {
            if (in_hand->type == EFigureType::King
                ? (is_eat
                    ? not board.check_for_when(in_hand->color, { in_hand->position, move_pos }, move_pos)
                    : not board.check_for_when(in_hand->color, { in_hand->position }, move_pos)
                    )
                : (is_eat
                    ? not board.check_for_when(in_hand->color, { in_hand->position, move_pos }, {}, { in_hand->submit_on(move_pos) })
                    : not board.check_for_when(in_hand->color, { in_hand->position }, {}, { in_hand->submit_on(move_pos) })
                    )
                ) {
                all_possible_moves.push_back({ is_eat, move_pos });
            }
        }
    }
}

void restart() {
    board.reset();
    in_hand = board.get_default_fig();
    all_possible_moves.clear();
    reset_input_order();
    input = { {-1, -1}, {-1, -1} };
    turn = EColor::White;
    ms = {};
    input_order_by_one = 0;
}
