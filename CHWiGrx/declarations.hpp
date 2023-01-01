#pragma once
#pragma comment(lib, "Msimg32.lib")
#include "resource.hpp"
#include "framework.hpp"
#include "../ChessCore/FigureBoard.h"
#include "../ChessCore/bot.h"
#include <optional>
#include <string>

using namespace std::string_literals;

#ifdef _DEBUG
    #include <stdio.h>
    #include <iostream>
    #include <string>
#endif // _DEBUG

template <typename ...Ts>
void debug_print(Ts ...args)
{
    #ifdef _DEBUG
        (std::cout << ... << args) << std::endl;
    #endif
}

/* virtual keys for numbers */
constexpr auto VK_0 = 48;
constexpr auto VK_1 = 49;
constexpr auto VK_2 = 50;
constexpr auto VK_3 = 51;
constexpr auto VK_4 = 52;
constexpr auto VK_5 = 53;
constexpr auto VK_6 = 54;
constexpr auto VK_7 = 55;
constexpr auto VK_8 = 56;
constexpr auto VK_9 = 57;

enum class WindowState { GAME, EDIT };

/* constants */
inline HINSTANCE hInst;
inline const HBRUSH CHECKERBOARD_DARK { CreateSolidBrush(RGB(0x32, 0x32, 0x32)) };
inline const HBRUSH CHECKERBOARD_BRIGHT { CreateSolidBrush(RGB(0x80, 0x80, 0x80)) };
inline const BoardRepr DEFAULT_CHESS_BOARD_IDW = BoardRepr("1;0;0;B;R;2;0;1;B;H;3;0;2;B;B;4;0;3;B;Q;5;0;4;B;K;6;0;5;B;B;7;0;6;B;H;8;0;7;B;R;9;1;0;B;P;10;1;1;B;P;11;1;2;B;P;12;1;3;B;P;13;1;4;B;P;14;1;5;B;P;15;1;6;B;P;16;1;7;B;P;17;6;0;W;P;18;6;1;W;P;19;6;2;W;P;20;6;3;W;P;21;6;4;W;P;22;6;5;W;P;23;6;6;W;P;24;6;7;W;P;25;7;0;W;R;26;7;1;W;H;27;7;2;W;B;28;7;3;W;Q;29;7;4;W;K;30;7;5;W;B;31;7;6;W;H;32;7;7;W;R;[TW1;8;25;32;]<><>~");
inline const BoardRepr DEFAULT_CHESS_BOARD_NIDW = BoardRepr("1;0;0;W;R;2;0;1;W;H;3;0;2;W;B;4;0;3;W;Q;5;0;4;W;K;6;0;5;W;B;7;0;6;W;H;8;0;7;W;R;9;1;0;W;P;10;1;1;W;P;11;1;2;W;P;12;1;3;W;P;13;1;4;W;P;14;1;5;W;P;15;1;6;W;P;16;1;7;W;P;17;6;0;B;P;18;6;1;B;P;19;6;2;B;P;20;6;3;B;P;21;6;4;B;P;22;6;5;B;P;23;6;6;B;P;24;6;7;B;P;25;7;0;B;R;26;7;1;B;H;27;7;2;B;B;28;7;3;B;Q;29;7;4;B;K;30;7;5;B;B;31;7;6;B;H;32;7;7;B;R;[FW1;8;25;32;]<><>~");
inline const BoardRepr EMPTY_REPR = BoardRepr({}, Color::White, true);
inline const int HEADER_HEIGHT = GetSystemMetrics(SM_CXPADDEDBORDER) +
                                 GetSystemMetrics(SM_CYMENUSIZE)     +
                                 GetSystemMetrics(SM_CYCAPTION)      +
                                 GetSystemMetrics(SM_CYFRAME);
inline const int SCROLLBAR_THICKNESS = GetSystemMetrics(SM_CXVSCROLL);
inline const LPCTSTR FIGURES_LIST_WINDOW_CLASS_NAME = L"CHWIGRX:LIST";
inline const LPCTSTR FIGURES_LIST_WINDOW_TITLE = L"Figures list";
inline const LPCTSTR CURR_CHOICE_WINDOW_CLASS_NAME = L"CHWIGRX:CHOICE";
inline const Pos FIGURES_LIST_WINDOW_DEFAULT_POS = { CW_USEDEFAULT, CW_USEDEFAULT };
inline const Pos FIGURES_LIST_WINDOW_DEFAULT_DIMENTIONS = { 300, 300 };
inline constexpr auto MAIN_WINDOW_CHOICE_TO_DESTROY_TIMER_ID = 1;
inline constexpr auto FIGURES_LIST_CHOICE_TO_DESTROY_TIMER_ID = 2;
inline constexpr auto TO_DESTROY_ELAPSE_DEFAULT_IN_MS = 5;
inline constexpr COLORREF TRANSPARENCY_PLACEHOLDER = RGB(0xFF, 0x0, 0x0);

/* single mutable globals */
inline WindowState window_state = WindowState::GAME;
inline BoardRepr start_board_repr{ DEFAULT_CHESS_BOARD_IDW };
inline FigureBoard board{ start_board_repr };
inline Color turn{ Color::White };
inline char chose{ 'Q' };
inline std::map<char, std::map<char, HBITMAP>> pieces_bitmaps;
inline std::map<const char*, HBITMAP> other_bitmaps;
inline bool save_all_moves = true;
inline HBRUSH CHECKERBOARD_ONE = CHECKERBOARD_DARK;
inline HBRUSH CHECKERBOARD_TWO = CHECKERBOARD_BRIGHT;
inline HWND figures_list_window = NULL;
inline bot::Type bot_type = bot::Type::Random;
inline bot::Difficulty bot_difficulty = bot::Difficulty::D0;
inline Color bot_turn = Color::Black;

/* misc functions */
bool init_instance(HINSTANCE, LPTSTR, LPTSTR, int);
INT_PTR CALLBACK about_proc(HWND, UINT, WPARAM, LPARAM);
void cpy_str_to_clip(const std::string&);
std::string take_str_from_clip();
HWND create_curr_choice_window(HWND, Figure*, POINT, int, int, const WNDPROC);
bool prepare_window(HINSTANCE, int, UINT, UINT, WNDCLASSEX);
int window_loop(HINSTANCE);
void change_checkerboard_color_theme(HWND);
HWND create_figures_list_window(HWND);

/* menu */
void set_menu_checkbox(HWND, UINT, bool);
void update_edit_menu_variables(HWND);
void update_game_menu_variables(HWND);
void update_bot_menu_variables(HWND);

/* board logic */
void update_main_window_title(HWND);
void on_lbutton_up(HWND, WPARAM, LPARAM, Pos, bool=true);
void on_lbutton_down(HWND, LPARAM);
void restart();
void copy_repr_to_clip();
void make_move(HWND, std::optional<Input> = std::nullopt);
bool is_legal_board_repr(const std::string&);
bool is_bot_move();

/* draw */
void draw_figure(HDC, const Figure*, const Pos, const bool = true);
void draw_figure(HDC, const Figure*, const Pos, const bool, const int, const int);
void draw_board(HDC);
void draw_figures_on_board(HDC);
void draw_input(HDC, Input);
inline void Rectangle(HDC hdc, RECT rect) { Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom); }

/* ---- WNDPROC functions ---------------------------------- */
namespace mainproc {
    LRESULT main_game_state_wndproc(HWND, UINT, WPARAM, LPARAM);
    LRESULT main_edit_state_wndproc(HWND, UINT, WPARAM, LPARAM);
}
LRESULT CALLBACK main_default_wndproc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK figures_list_wndproc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK curr_choice_figures_list_wndproc(HWND, UINT, WPARAM, LPARAM);
template <bool> LRESULT CALLBACK curr_choice_wndproc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK moves_list_wndproc(HWND, UINT, WPARAM, LPARAM);

class WindowStats {
    /* Габариты окна для отрисовки и захвата ввода */
    /*  x-axis from left to right  (→)  **
    **  y-axis from top  to bottom (↓)  */
public:
    WindowStats(Pos window_pos, Pos window_size) : window_pos(window_pos), window_size(window_size) {};
    void virtual recalculate_cell_size() {
        cell_size = { window_size.x / WIDTH, window_size.y / HEIGHT };
    }
    void virtual set_size(int x /* LOWORD */, int y /* HIWORD */) {
        this->window_size = { x, y };
        recalculate_cell_size();
    }
    void set_size(Pos window_size) { set_size(window_size.x, window_size.y); }
    void set_size(LPARAM lParam) { set_size(LOWORD(lParam), HIWORD(lParam)); }
    inline void set_rect(RECT rect) {
        set_pos({ rect.left, rect.top });
        set_size(rect.right - rect.left, rect.bottom - rect.top);
    }
    inline int get_width() { return window_size.x; }
    inline int get_height() { return window_size.y; }
    inline virtual int get_width_with_extra() { return window_size.x + EXTRA_WINDOW_SIZE.x; }
    inline virtual int get_height_with_extra() { return window_size.y + EXTRA_WINDOW_SIZE.y; }
    inline int get_cell_width() { return cell_size.x; }
    inline int get_cell_height() { return cell_size.y; }
    inline void set_prev_lbutton_click(Pos plbc) { prev_lbutton_click = plbc; }
    inline Pos get_prev_lbutton_click() { return prev_lbutton_click; }
    inline void set_pos(Pos wp) { window_pos = wp; }
    inline void set_pos(int x, int y) { window_pos.x = x; window_pos.y = y; }
    inline void set_pos(LPARAM lParam) { set_pos(LOWORD(lParam), HIWORD(lParam)); }
    inline int get_window_pos_x() { return window_pos.x; }
    inline int get_window_pos_y() { return window_pos.y; }
    inline bool is_mouse_moved_enough(Pos mouse) {
        /* не используется */
        Pos shift = { abs(mouse.x - prev_lbutton_click.x),
                      abs(mouse.y - prev_lbutton_click.y)
        };
        return shift.x >= UNITS_TO_MOVE_ENOUGH &&
               shift.y >= UNITS_TO_MOVE_ENOUGH;
    }
    inline Pos divide_by_cell_size(int x, int y) {
        return {x / cell_size.x, y / cell_size.y};
    }
    inline Pos divide_by_cell_size(LPARAM lParam) { return divide_by_cell_size(LOWORD(lParam), HIWORD(lParam)); }
    inline RECT get_cell(Pos start) {
        return {
            .left = start.x * cell_size.x + INDENTATION_FROM_EDGES,
            .top = start.y * cell_size.y + INDENTATION_FROM_EDGES,
            .right = (start.x + 1) * cell_size.x - INDENTATION_FROM_EDGES * 2,
            .bottom = (start.y + 1) * cell_size.y - INDENTATION_FROM_EDGES * 2
        };
    }
    inline RECT get_cell(int i, int j) { return get_cell({ i, j }); }
    inline Pos get_figure_under_mouse(POINT mouse) {
        return divide_by_cell_size(
            mouse.x - window_pos.x,
            mouse.y - window_pos.y
        ).change_axes();
    }
    virtual ~WindowStats() {};
protected:
    const int UNITS_TO_MOVE_ENOUGH = { 2 };
    const Pos EXTRA_WINDOW_SIZE = { 0, HEADER_HEIGHT };
    Pos window_pos;
    Pos prev_lbutton_click{};
    Pos window_size;
    Pos cell_size = { window_size.x / WIDTH, window_size.y / HEIGHT };
    const int INDENTATION_FROM_EDGES{ 0 };
} inline main_window{ { 300, 300 }, { 498, 498 } };

class FiguresListStats : public WindowStats {
public:
    FiguresListStats(Pos window_pos, Pos window_size, size_t figures_in_row, size_t figures_num)
        : WindowStats(window_pos, window_size), figures_in_row(figures_in_row) {
        int rows_num = static_cast<int>(ceil(figures_num / static_cast<double>(figures_in_row)));
        total_height_of_all_figures = static_cast<int>(rows_num * cell_size.y);
    }
    void set_size(int w, int h) override {
        WindowStats::set_size(w, h);
        recalculate_dimensions();
    }
    void set_size(LPARAM lParam) { set_size(LOWORD(lParam), HIWORD(lParam)); }
    void recalculate_cell_size() override {
        cell_size.x = cell_size.y = static_cast<int>(std::max(0, window_size.x) / figures_in_row);
    }
    inline int get_width_with_extra() override {
        // Может не быть слайдера TODO
        return WindowStats::get_width_with_extra() + SCROLLBAR_THICKNESS;
    }
    inline int get_height_with_extra() override {
        return WindowStats::get_height_with_extra() + SCROLLBAR_THICKNESS;
    }
    void recalculate_dimensions() {
        recalculate_cell_size();
        int rows_num = static_cast<int>(ceil(MAX_FIGURES_IN_ROW / static_cast<double>(figures_in_row)));
        total_height_of_all_figures = static_cast<int>(rows_num * cell_size.y);
        max_scroll = std::max(static_cast<int>(total_height_of_all_figures), window_size.y);
    }
    inline size_t get_figures_in_row() const { return figures_in_row; }
    inline void inc_figures_in_row() { if (figures_in_row < MAX_FIGURES_IN_ROW) figures_in_row++; }
    inline void dec_figures_in_row() { if (figures_in_row != 1) figures_in_row--; }
    inline size_t get_max_scroll() const { return max_scroll; }
    // returns delta
    inline int add_to_curr_scroll(int val) {
        int old_scroll = curr_scroll;
        curr_scroll = std::min(static_cast<int>(max_scroll), std::max(0, curr_scroll + val));
        return curr_scroll - old_scroll;
    }
    inline int get_curr_scroll() const { return curr_scroll; }
    inline int set_curr_scroll(int new_scroll) {
        int delta = new_scroll - curr_scroll;
        curr_scroll = new_scroll;
        return delta;
    }
    inline size_t get_all_figures_height() const { return total_height_of_all_figures; }
    inline void clear_scrolling() {
        max_scroll = 0;
        curr_scroll = 0;
    }
private:
    size_t figures_in_row{ 2 };
    size_t MAX_FIGURES_IN_ROW{ PLAYABLE_FIGURES.size() };
    int curr_scroll{ 0 };
    size_t max_scroll{ 0 };
    long total_height_of_all_figures;
} inline figures_list{
    FIGURES_LIST_WINDOW_DEFAULT_POS, FIGURES_LIST_WINDOW_DEFAULT_DIMENTIONS,
    2, PLAYABLE_FIGURES.size()
};

/* Текущее состояние ввода */
class MotionInput {
public:
    MotionInput(FigureBoard* board) : board(board) {};
    void clear();
    void prepare(Color turn);
    void calculate_possible_moves();
    void init_curr_choice_window(HWND, WNDPROC);
    inline void activate_by_click() { 
        input_order_by_two = true;
        input_order_by_one = 2;
    }
    inline void deactivate_by_click() { input_order_by_two = false; }
    inline void deactivate_by_pos() { input_order_by_one = false; }
    inline void set_target(Pos target) { input.target = target; }
    inline void set_target(int x, int y) { input.target = {x, y}; }
    inline void set_from(Pos from) { input.from = from; }
    inline void set_in_hand(Figure* in_hand) { this->in_hand = in_hand; }
    inline bool is_target_at_input() { return input.from == input.target; }
    inline void set_lbutton_up() { is_lbutton_down = false; }
    inline void set_lbutton_down() { is_lbutton_down = true; }
    inline bool is_active_by_click() { return input_order_by_two; }
    inline bool is_current_turn(Color turn) { return in_hand->is_col(turn); }
    inline auto get_in_hand() { return in_hand; }
    inline auto get_input() { return input; }
    inline void shift_from(Pos shift, int max_x, int max_y) { input.from.loop_add(shift, max_x, max_y); }
    inline void shift_target(Pos shift, int max_x, int max_y) { input.target.loop_add(shift, max_x, max_y); }
    inline void by_pos_to_next() { ++input_order_by_one; }
    inline void set_from_x(int val) { input.from.x = val; }
    inline void set_from_y(int val) { input.from.y = val; }
    inline void set_target_x(int val) { input.target.x = val; }
    inline void set_target_y(int val) { input.target.y = val; }
    inline auto get_state_by_pos() { return input_order_by_one; }
    inline bool is_drags() { return !is_curr_choice_moving && is_lbutton_down && not in_hand->empty(); }
    inline auto get_possible_moves() { return all_moves; }
    inline bool is_figure_dragged(Id id) { return in_hand->is(id) && is_curr_choice_moving && !input_order_by_two; }
private:
    FigureBoard* board;
    const Pos DEFAULT_INPUT_FROM{ 0, -1 };
    Input input{ DEFAULT_INPUT_FROM, {-1, -1} };
    int  input_order_by_one{ 0 };
    bool input_order_by_two{ false };
    bool is_lbutton_down{ false };
    HWND curr_chose_window{};
    bool is_curr_choice_moving{ false };
    Figure* in_hand = board->get_default_fig();
    std::vector<std::pair<bool, Pos>> all_moves{};
} inline motion_input{ &board };
