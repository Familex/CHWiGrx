// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

// #define DEBUG

#ifdef DEBUG
#define DEBUG_TIMER
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <string>
using namespace std::string_literals;
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
#define HEADER_HEIGHT 53


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
const pos DEFAULT_INPUT_FROM{ 0, -1 };
Input input{ DEFAULT_INPUT_FROM, {-1, -1} };
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
const char* default_chess_board = "[TW]1;0;0;B;R;2;0;1;B;H;3;0;2;B;B;4;0;3;B;Q;5;0;4;B;K;6;0;5;B;B;7;0;6;B;H;8;0;7;B;R;9;1;0;B;P;10;1;1;B;P;11;1;2;B;P;12;1;3;B;P;13;1;4;B;P;14;1;5;B;P;15;1;6;B;P;16;1;7;B;P;17;6;0;W;P;18;6;1;W;P;19;6;2;W;P;20;6;3;W;P;21;6;4;W;P;22;6;5;W;P;23;6;6;W;P;24;6;7;W;P;25;7;0;W;R;26;7;1;W;H;27;7;2;W;B;28;7;3;W;Q;29;7;4;W;K;30;7;5;W;B;31;7;6;W;H;32;7;7;W;R;;<><>~";
BoardRepr start_board_repr{ default_chess_board };
FigureBoard board{ start_board_repr };
bool save_all_moves = true;
Color start_turn{ EColor::White };
char chose{ 'Q' };
Color turn{ EColor::White };
POINT cursor{};
bool is_hooked{ false };
pos prev_lbutton_click{};
bool is_curr_choice_moving{ false };
bool is_lbutton_down{ false };
HWND curr_chose_window{};
int  input_order_by_one{ 0 };
bool input_order_by_two{ false };
std::list<Figure>::iterator in_hand = board.get_default_fig();
std::list<std::pair<bool, pos>> all_possible_moves{};
std::map<char, std::map<char, HBITMAP>> pieces_bitmaps;
const DWORD compression_algorithm = COMPRESS_ALGORITHM_MSZIP;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM               MyRegisterClass(HINSTANCE hInstance);
BOOL               InitInstance(HINSTANCE, int);
LRESULT CALLBACK   WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK   About(HWND, UINT, WPARAM, LPARAM);
void               draw_figure(HDC, const Figure&, int=-1, int=-1, bool=true);
void               clear_current_globals();
void               make_move(HWND);
void               init_globals(pos, Color);
void               reset_input_order() { input_order_by_one = 0; input_order_by_two = false; }
void               restart();
void               cpy_str_to_clip(const std::string&);
std::string        take_str_from_clip();
std::string        compress_string(const std::string&);
std::string        decompress_string(const std::string&);
HWND               createWindow(HWND, POINT, int, int, const WNDPROC);
void               initCurrChoiceWindow(HWND);
void               on_lbutton_up(HWND, WPARAM, LPARAM, pos where_fig);


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
    wcex.hCursor        = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
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

#ifdef DEBUG_TIMER
    case WM_CREATE:
        SetTimer(hWnd, 1, 1000, NULL);
        break;
    case WM_TIMER:
        std::cout << "1.Is moving: " << std::hex << is_curr_choice_moving << ". Lbutton " << is_lbutton_down << "." << std::endl;
        break;
#endif // DEBUG

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_UNDO:
                if (board.undo_move()) {
                    clear_current_globals();
                    turn.to_next();
                }
                break;
            case IDM_RESTORE_MOVE:
                if (board.restore_move()) {
                    clear_current_globals();
                    turn.to_next();
                }
                break;
            case IDM_COPY_MAP:
                {
                    BoardRepr board_repr = board.get_repr(save_all_moves);
                    board_repr.set_turn(turn);
                    cpy_str_to_clip(
                        board_repr.as_string()
                    );
                }
                break;
            case IDM_PASTE_MAP:
                do {
                    BoardRepr board_repr(take_str_from_clip());
                    turn = board_repr.get_turn();
                    board.reset(board_repr);
                    clear_current_globals();
                } while (0);
                break;
            case IDM_PASTE_START_MAP:
                start_board_repr = take_str_from_clip();
                start_turn = start_board_repr.get_turn();
                break;
            case IDM_RESET_START_MAP:
                start_board_repr = { default_chess_board };
                start_turn = start_board_repr.get_turn();
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
                default:
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
                input_order_by_two = true;
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
        #ifdef DEBUG
            std::cout << "Curr board: " << board.get_repr(true).as_string() << '\n';
        #endif // DEBUG
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
        InvalidateRect(hWnd, NULL, NULL);
        break;
    case WM_LBUTTONUP:
        on_lbutton_up(hWnd, wParam, lParam, {HIWORD(lParam) / cell_height, LOWORD(lParam) / cell_width});
        break;
    case WM_MOVE:
        window_pos = { LOWORD(lParam), HIWORD(lParam) };
        break;
    case WM_MOUSEMOVE:
        if (!is_curr_choice_moving && is_lbutton_down) {
            is_curr_choice_moving = true;
            initCurrChoiceWindow(hWnd);
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
                    if (in_hand->id != figure.id || !is_curr_choice_moving || input_order_by_two) {
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

void draw_figure(HDC hdc, const Figure& figure, int w_beg, int h_beg, bool is_transpanent) {
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
    if (is_transpanent) {
        TransparentBlt(hdc, w_beg, h_beg, cell_width, cell_height,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, RGB(255, 0, 0));
    }
    else {
        StretchBlt(hdc, w_beg, h_beg, cell_width, cell_height,
            hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    }
    DeleteDC(hdcMem);
}

void clear_current_globals() {
    is_lbutton_down = false;
    is_curr_choice_moving = false;
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

    auto [is_legal_move, move_rec] = board.provide_move(in_hand, input, turn, [c = chose] { return c; });

    if (!is_legal_move) {
        clear_current_globals();
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }

    #ifdef DEBUG
        std::cout << "Curr move was: " << move_rec.as_string() << '\n';
    #endif

    board.set_last_move({ *in_hand, input, turn, move_rec.ms, move_rec.promotion_choice });
    turn.to_next();
    clear_current_globals();
    InvalidateRect(hWnd, NULL, NULL);

    if (board.game_end(turn)) {
        TCHAR tmp2[] = { turn.what_next(), ' ', 'w', 'i', 'n', 's', '!', '\0'};
        MessageBox(hWnd, tmp2, L"GAME END", NULL);
        restart();
        InvalidateRect(hWnd, NULL, NULL);
    }
}

void init_globals(pos from, Color turn) {
    is_lbutton_down = true;
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
    board.reset(start_board_repr);
    in_hand = board.get_default_fig();
    all_possible_moves.clear();
    reset_input_order();
    input = { DEFAULT_INPUT_FROM, {} };
    turn = start_turn;
    input_order_by_one = 0;
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

HWND createWindow(HWND parent, POINT pos, int w, int h, const WNDPROC callback) {
    UnregisterClass(L"Chosen figure", GetModuleHandle(nullptr));
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    HWND hWindow{};
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.lpfnWndProc = callback;
    wc.lpszClassName = L"Chosen figure";
    wc.style = CS_VREDRAW | CS_HREDRAW;

    const auto create_window = [&hWindow, &parent, &pos, &w, &h]() -> HWND {
        if (hWindow = CreateWindow(L"Chosen figure", L"", WS_POPUP | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            pos.x - w / 2, pos.y - h / 2, w, h, parent, nullptr, nullptr, nullptr), !hWindow
            )
            return nullptr;
        ShowWindow(hWindow, SW_SHOWDEFAULT);
        UpdateWindow(hWindow);
        return hWindow;
    };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}

void initCurrChoiceWindow(HWND hWnd) {
    is_curr_choice_moving = true;
    POINT cur_pos{};
    GetCursorPos(&cur_pos);
    curr_chose_window = createWindow(hWnd, cur_pos, cell_width, cell_height, [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg)
        {
        case WM_EXITSIZEMOVE: // Фигуру отпустил
            {
                HWND parent = GetParent(hWnd);
                POINT cur_pos{};
                GetCursorPos(&cur_pos);
                RECT parent_window;
                GetWindowRect(parent, &parent_window);
                pos where_fig{
                    (cur_pos.y - parent_window.top - HEADER_HEIGHT) / cell_height,
                    (cur_pos.x - parent_window.left) / cell_width
                };
                on_lbutton_up(hWnd, wParam, lParam, where_fig);
                InvalidateRect(parent, NULL, NULL);
                clear_current_globals();
                DestroyWindow(hWnd);
            }
            break;
        case WM_NCHITTEST:  // При перехвате нажатий мыши симулируем перетаскивание
            return (LRESULT)HTCAPTION;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            draw_figure(hdc, *in_hand, 0, 0, false);
            EndPaint(hWnd, &ps);
            break;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }});
    InvalidateRect(hWnd, NULL, NULL);
    SetWindowPos(curr_chose_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetWindowLongPtr(curr_chose_window, GWL_EXSTYLE, GetWindowLongPtr(curr_chose_window, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(curr_chose_window, RGB(255, 0, 0), 255, LWA_COLORKEY);
    SendMessage(curr_chose_window, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(cur_pos.x, cur_pos.y));
}

void on_lbutton_up(HWND hWnd, WPARAM wParam, LPARAM lParam, pos where_fig) {
    is_lbutton_down = false;
    input_order_by_one = 0;
    if (input_order_by_two) {
        make_move(hWnd);
        clear_current_globals();
        InvalidateRect(hWnd, NULL, NULL);
        return;
    }
    else {
        input.target = { where_fig.x, where_fig.y };
        if (input.from == input.target) {
            if (prev_lbutton_click != pos(HIWORD(lParam), LOWORD(lParam))) { // Отпустили в пределах клетки, но в другом месте
                clear_current_globals();
                InvalidateRect(hWnd, NULL, NULL);
                return;
            }
            init_globals(input.from, turn);
            input_order_by_two = !input_order_by_two;
            InvalidateRect(hWnd, NULL, NULL);
        }
        else {
            make_move(hWnd);
        }
    }
}