#pragma once
#pragma comment(lib, "Msimg32.lib")
#include "resource.h"
#include "framework.h"
#include "../ChessCore/FigureBoard.h"

#ifdef ALLOCATE_CONSOLE
#include <stdio.h>
#include <iostream>
#include <string>
using namespace std::string_literals;
#endif // ALLOCATE_CONSOLE

/* virtual keys for numbers */
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

/* constants */
inline HINSTANCE hInst;
inline const char* DEFAULT_CHESS_BOARD_IDW =  "1;0;0;B;R;2;0;1;B;H;3;0;2;B;B;4;0;3;B;Q;5;0;4;B;K;6;0;5;B;B;7;0;6;B;H;8;0;7;B;R;9;1;0;B;P;10;1;1;B;P;11;1;2;B;P;12;1;3;B;P;13;1;4;B;P;14;1;5;B;P;15;1;6;B;P;16;1;7;B;P;17;6;0;W;P;18;6;1;W;P;19;6;2;W;P;20;6;3;W;P;21;6;4;W;P;22;6;5;W;P;23;6;6;W;P;24;6;7;W;P;25;7;0;W;R;26;7;1;W;H;27;7;2;W;B;28;7;3;W;Q;29;7;4;W;K;30;7;5;W;B;31;7;6;W;H;32;7;7;W;R;[TW]<><>~";
inline const char* DEFAULT_CHESS_BOARD_NIDW = "1;0;0;W;R;2;0;1;W;H;3;0;2;W;B;4;0;3;W;Q;5;0;4;W;K;6;0;5;W;B;7;0;6;W;H;8;0;7;W;R;9;1;0;W;P;10;1;1;W;P;11;1;2;W;P;12;1;3;W;P;13;1;4;W;P;14;1;5;W;P;15;1;6;W;P;16;1;7;W;P;17;6;0;B;P;18;6;1;B;P;19;6;2;B;P;20;6;3;B;P;21;6;4;B;P;22;6;5;B;P;23;6;6;B;P;24;6;7;B;P;25;7;0;B;R;26;7;1;B;H;27;7;2;B;B;28;7;3;B;Q;29;7;4;B;K;30;7;5;B;B;31;7;6;B;H;32;7;7;B;R;[FW]<><>~";
const int HEADER_HEIGHT = GetSystemMetrics(SM_CXPADDEDBORDER) +
                          GetSystemMetrics(SM_CYMENUSIZE)     +
                          GetSystemMetrics(SM_CYCAPTION)      +
                          GetSystemMetrics(SM_CYFRAME);

/* single mutable globals */
inline BoardRepr start_board_repr{ DEFAULT_CHESS_BOARD_IDW };
inline FigureBoard board{ start_board_repr };
inline Color turn{ EColor::White };
inline bool idw{ true };
inline char chose{ 'Q' };
inline std::map<char, std::map<char, HBITMAP>> pieces_bitmaps;
inline bool save_all_moves = true;

/* misc functions (defined in funcs.cpp) */
ATOM               register_main_window_class(HINSTANCE hInstance, LPTSTR, LPTSTR);
bool               init_instance(HINSTANCE, LPTSTR, LPTSTR, int);
INT_PTR CALLBACK   about_proc(HWND, UINT, WPARAM, LPARAM);
void               draw_figure(HDC, const Figure*, int = -1, int = -1, bool = true);
void               make_move(HWND);
void               restart();
void               cpy_str_to_clip(const std::string&);
std::string        take_str_from_clip();
HWND               create_curr_choice_window(HWND, Figure*, POINT, int, int, const WNDPROC, LPCWSTR = L"Chosen figure");
void               on_lbutton_up(HWND, WPARAM, LPARAM, pos where_fig);
bool               is_legal_board_repr(const std::string&);
void               set_menu_checkbox(HWND, UINT, bool);
void               update_check_title(HWND);
inline void        Rectangle(HDC hdc, RECT rect) { Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom); }

/* Main window WinProc func (defined in winproc.cpp) */
LRESULT CALLBACK   main_proc(HWND, UINT, WPARAM, LPARAM);

class WindowStats {
    /* Габариты окна для отрисовки и захвата ввода */
    /*  x-axis from left to right  (→)  **
    **  y-axis from top  to bottom (↓)  */
public:
    void recalculate_cell_size() {
        cell_size = { window_size.x / WIDTH, window_size.y / HEIGHT };
    }
    void set_window_size(pos window_size) {
        this->window_size = window_size;
        recalculate_cell_size();
    }
    void set_window_size(int x, int y) {
        this->window_size.x = x;
        this->window_size.y = y;
        recalculate_cell_size();
    }
    inline int get_window_height() { return window_size.x; }
    inline int get_window_width() { return window_size.y; }
    inline int get_real_height() { return window_size.x + EXTRA_WINDOW_SIZE.x; }
    inline int get_real_width() { return window_size.y + EXTRA_WINDOW_SIZE.y; }
    inline int get_cell_height() { return cell_size.x; }
    inline int get_cell_width() { return cell_size.y; }
    inline void set_prev_lbutton_click(pos plbc) { prev_lbutton_click = plbc; }
    inline pos get_prev_lbutton_click() { return prev_lbutton_click; }
    inline void set_window_pos(pos wp) { window_pos = wp; }
    inline void set_window_pos(int x, int y) { window_pos.x = x; window_pos.y = y; }
    inline int get_window_pos_x() { return window_pos.x; }
    inline int get_window_pos_y() { return window_pos.y; }
    inline bool is_mouse_moved_enough(pos mouse) {
        /* не используется */
        pos shift = { abs(mouse.x - prev_lbutton_click.x),
                      abs(mouse.y - prev_lbutton_click.y)
        };
        return shift.x >= UNITS_TO_MOVE_ENOUGH &&
               shift.y >= UNITS_TO_MOVE_ENOUGH;
    }
    inline pos divide_by_cell_size(int x, int y) {
        return {x / cell_size.x, y / cell_size.y};
    }
    inline RECT get_cell(pos start) {
        return {
            start.y * cell_size.y + INDENTATION_FROM_EDGES,
            start.x * cell_size.x + INDENTATION_FROM_EDGES,
            (start.y + 1) * cell_size.y - INDENTATION_FROM_EDGES * 2,
            (start.x + 1) * cell_size.x - INDENTATION_FROM_EDGES * 2
        };
    }
    inline RECT get_cell(int i, int j) { return get_cell({ i, j }); }
private:
    const int UNITS_TO_MOVE_ENOUGH = { 2 };
    const pos EXTRA_WINDOW_SIZE = { 59, 16 };
    pos window_pos{ 300, 300 };
    pos prev_lbutton_click{};
    pos window_size = { 498, 498 };
    pos cell_size = { window_size.x / WIDTH, window_size.y / HEIGHT };
    const int INDENTATION_FROM_EDGES{ 0 };
};

inline WindowStats window_stats{};

class MotionInput {
    /* Состояние текущего состояния ввода */
public:
    /* переименовать тут всё */
    MotionInput(FigureBoard* board) : board(board) {};
    void clear();
    void prepare(Color turn);
    void calculate_possible_moves();
    void init_curr_choice_window(HWND hWnd);
    inline void toggle_pair_input() { input_order_by_two = !input_order_by_two; }
    inline void activate_pair() { input_order_by_two = true; }
    inline void reset_single() { input_order_by_one = false; }
    inline void set_target(pos target) { input.target = target; }
    inline void set_target(int x, int y) { input.target = {x, y}; }
    inline void set_from(pos from) { input.from = from; }
    inline bool target_at_input() { return input.from == input.target; }
    inline void reset_lbutton_down() { is_lbutton_down = false; }
    inline void set_lbutton_down() { is_lbutton_down = true; }
    inline bool is_pair() { return input_order_by_two; }
    inline bool is_current_turn(Color turn) { return in_hand->is_col(turn); }
    inline auto get_in_hand() { return in_hand; }
    inline auto get_input() { return input; }
    inline void shift_from(pos shift, int max_x, int max_y) { input.from.loop_add(shift, max_x, max_y); }
    inline void shift_target(pos shift, int max_x, int max_y) { input.target.loop_add(shift, max_x, max_y); }
    inline void next_single() { ++input_order_by_one; }
    inline void set_from_x(int val) { input.from.x = val; }
    inline void set_from_y(int val) { input.from.y = val; }
    inline void set_target_x(int val) { input.target.x = val; }
    inline void set_target_y(int val) { input.target.y = val; }
    inline auto get_single_state() { return input_order_by_one; }
    inline bool is_drags() { return !is_curr_choice_moving && is_lbutton_down && not in_hand->empty(); }
    inline auto get_possible_moves() { return all_moves; }
    inline bool is_figure_dragged(Id id) { return in_hand->is(id) && is_curr_choice_moving && !input_order_by_two; }
    inline void reset_input_order() {
        input_order_by_one = 0;
        input_order_by_two = false;
    }
private:
    FigureBoard* board;
    const pos DEFAULT_INPUT_FROM{ 0, -1 };
    Input input{ DEFAULT_INPUT_FROM, {-1, -1} };
    int  input_order_by_one{ 0 };
    bool input_order_by_two{ false };
    bool is_lbutton_down{ false };
    HWND curr_chose_window{};
    bool is_curr_choice_moving{ false };
    Figure* in_hand = board->get_default_fig();
    std::list<std::pair<bool, pos>> all_moves{};
};

inline MotionInput motion_input{&board};
