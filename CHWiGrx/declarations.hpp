#pragma once
#pragma comment(lib, "Msimg32.lib")
//      ^^^^^^^^^^^^^^^^^^^^^^^^^^^ for TransparentBlt
#include "resource.hpp"
#include "framework.hpp"
#include "../ChessCore/ChessGame.hpp"
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
void debug_print([[maybe_unused]] Ts ...args)
{
    #ifdef _DEBUG
        ((std::cout << args << " "), ...);
        std::cout << std::endl;
    #else
        ((void)args, ...);
    #endif // _DEBUG
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

enum class WindowState { Game, Edit };

/* constants */
inline HINSTANCE h_inst;
inline const HBRUSH CHECKERBOARD_DARK { CreateSolidBrush(RGB(0x32, 0x32, 0x32)) };
inline const HBRUSH CHECKERBOARD_BRIGHT { CreateSolidBrush(RGB(0x80, 0x80, 0x80)) };
inline const auto DEFAULT_CHESS_BOARD_IDW{ 
    FromString<board_repr::BoardRepr>{}(
        "02H8W8TWC0,7,25,32!"
        "0.00BR1.01BH2.02BB3.03BK4.04BQ5.05BB6.06BH7.07BR8.08BP9.09BP10.10BP11.11BP13.12BP14.13BP15.14BP16.15BP17.48WP18.49WP19.50WP20.51WP21.52WP22.53WP23.54WP24.55WP25.56WR26.57WH27.58WB28.59WK29.60WQ30.61WB31.62WH32.63WR<><>"
    ).value().value
};
inline const auto DEFAULT_CHESS_BOARD_NIDW{
    FromString<board_repr::BoardRepr>{}(
        "02H8W8FWC0,7,25,32!"
        "0.00WR1.01WH2.02WB3.03WQ4.04WK5.05WB6.06WH7.07WR8.08WP9.09WP10.10WP11.11WP12.12WP13.13WP14.14WP15.15WP16.48BP17.49BP18.50BP19.51BP20.52BP21.53BP22.54BP23.55BP25.56BR26.57BH27.58BB28.59BQ29.60BK30.61BB31.62BH32.63BR<><>"
    ).value().value
};
inline const auto EMPTY_REPR = board_repr::BoardRepr({}, Color::White, true);
inline const int HEADER_HEIGHT{ GetSystemMetrics(SM_CXPADDEDBORDER) +
                                GetSystemMetrics(SM_CYMENUSIZE) +
                                GetSystemMetrics(SM_CYCAPTION) +
                                GetSystemMetrics(SM_CYFRAME) };
inline const int SCROLLBAR_THICKNESS{ GetSystemMetrics(SM_CXVSCROLL) };
inline const LPCTSTR FIGURES_LIST_WINDOW_CLASS_NAME{ L"CHWIGRX:LIST" };
inline const LPCTSTR FIGURES_LIST_WINDOW_TITLE{ L"Figures list" };
inline const LPCTSTR CURR_CHOICE_WINDOW_CLASS_NAME{ L"CHWIGRX:CHOICE" };
inline constexpr Pos FIGURES_LIST_WINDOW_DEFAULT_POS{ CW_USEDEFAULT, CW_USEDEFAULT };
inline constexpr Pos FIGURES_LIST_WINDOW_DEFAULT_DIMENSIONS{ 300, 300 };
inline constexpr auto MAIN_WINDOW_CHOICE_TO_DESTROY_TIMER_ID{ 1 };
inline constexpr auto FIGURES_LIST_CHOICE_TO_DESTROY_TIMER_ID{ 2 };
inline constexpr auto TO_DESTROY_ELAPSE_DEFAULT_IN_MS{ 5 };
inline constexpr COLORREF TRANSPARENCY_PLACEHOLDER{ RGB(0xFF, 0x0, 0x0) };

/* single mutable globals */
inline WindowState window_state = WindowState::Game;
inline board_repr::BoardRepr start_board_repr{ DEFAULT_CHESS_BOARD_IDW };
inline ChessGame board{ board_repr::BoardRepr{ start_board_repr } /* <- explicit copy */ };
inline Color turn{ Color::White };
inline FigureType chose{ FigureType::Queen };
inline std::map<char, std::map<char, HBITMAP>> pieces_bitmaps;
inline std::map<const char*, HBITMAP> other_bitmaps;
inline bool save_all_moves = true;
inline HBRUSH checkerboard_one = CHECKERBOARD_DARK;
inline HBRUSH checkerboard_two = CHECKERBOARD_BRIGHT;
inline HWND figures_list_window = nullptr;
inline bot::Type bot_type = bot::Type::Random;
inline bot::Difficulty bot_difficulty = bot::Difficulty::D0;
inline Color bot_turn = Color::Black;

/* misc functions */
bool init_instance(HINSTANCE, LPTSTR, LPTSTR, int);
INT_PTR CALLBACK about_proc(HWND, UINT, WPARAM, LPARAM);
bool cpy_str_to_clip(HWND, std::string_view);
std::string take_str_from_clip(HWND);
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
bool copy_repr_to_clip(HWND);
auto take_repr_from_clip(HWND)
    -> ParseEither<board_repr::BoardRepr, ParseErrorType>;
void make_move(const HWND);
bool is_bot_move();

/* draw */
void draw_figure(HDC, const Figure*, const Pos, const bool = true);
void draw_figure(HDC, const Figure*, const Pos, const bool, const int, const int);
void draw_board(HDC);
void draw_figures_on_board(HDC);
void draw_input(HDC, Input);
inline void rectangle(const HDC hdc, const RECT rect) { Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom); }

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
    WindowStats(const Pos window_pos, const Pos window_size)
        : window_pos_(window_pos)
        , window_size_(window_size)
    { }

    void virtual recalculate_cell_size() {
        cell_size_ = Pos{ window_size_.x / WIDTH, window_size_.y / HEIGHT };
    }
    void virtual set_size(const int x /* LOWORD */, const int y /* HIWORD */) {
        this->window_size_ = Pos{ x, y };
        recalculate_cell_size();
    }
    void set_size(const Pos new_window_size) { set_size(new_window_size.x, new_window_size.y); }
    void virtual set_size(const LPARAM l_param) { set_size(LOWORD(l_param), HIWORD(l_param)); }
    inline void set_rect(const RECT rect) {
        set_pos(rect.left, rect.top);
        set_size(rect.right - rect.left, rect.bottom - rect.top);
    }
    inline int get_width() const { return window_size_.x; }
    inline int get_height() const { return window_size_.y; }
    inline virtual int get_width_with_extra() { return window_size_.x + extra_window_size_.x; }
    inline virtual int get_height_with_extra() { return window_size_.y + extra_window_size_.y; }
    inline int get_cell_width() const { return cell_size_.x; }
    inline int get_cell_height() const { return cell_size_.y; }
    inline void set_prev_lbutton_click(const Pos prev_lbutton_click) { this->prev_lbutton_click_ = prev_lbutton_click; }
    inline Pos get_prev_lbutton_click() const { return prev_lbutton_click_; }
    inline void set_pos(const Pos wp) { window_pos_ = wp; }
    inline void set_pos(const int x, const int y) { window_pos_.x = x; window_pos_.y = y; }
    inline void set_pos(const LPARAM l_param) { set_pos(LOWORD(l_param), HIWORD(l_param)); }
    inline int get_window_pos_x() const { return window_pos_.x; }
    inline int get_window_pos_y() const { return window_pos_.y; }
    inline bool is_mouse_moved_enough(const Pos mouse) const
    {
        /* не используется */
        const auto shift = Pos{
            abs(mouse.x - prev_lbutton_click_.x),
            abs(mouse.y - prev_lbutton_click_.y)
        };
        return shift.x >= units_to_move_enough_ &&
               shift.y >= units_to_move_enough_;
    }
    inline Pos divide_by_cell_size(const int x, const int y) const { return Pos{ x / cell_size_.x, y / cell_size_.y }; }
    inline Pos divide_by_cell_size(const LPARAM l_param) const { return divide_by_cell_size(LOWORD(l_param), HIWORD(l_param)); }
    inline RECT get_cell(const Pos start) const
    {
        return {
            .left = start.x * cell_size_.x + indentation_from_edges_,
            .top = start.y * cell_size_.y + indentation_from_edges_,
            .right = (start.x + 1) * cell_size_.x - indentation_from_edges_ * 2,
            .bottom = (start.y + 1) * cell_size_.y - indentation_from_edges_ * 2
        };
    }
    inline RECT get_cell(const int i, const int j) const { return get_cell(Pos{ i, j }); }
    inline Pos get_figure_under_mouse(const POINT mouse) const
    {
        return divide_by_cell_size(
            mouse.x - window_pos_.x,
            mouse.y - window_pos_.y
        ).change_axes();
    }
    virtual ~WindowStats() = default;
protected:
    const int units_to_move_enough_{ 2 };
    const Pos extra_window_size_{ 0, HEADER_HEIGHT };
    Pos window_pos_;
    Pos prev_lbutton_click_{};
    Pos window_size_;
    Pos cell_size_{ window_size_.x / WIDTH, window_size_.y / HEIGHT };
    const int indentation_from_edges_{ 0 };
} inline main_window{ Pos{ 300, 300 }, Pos{ 498, 498 } };

class FiguresListStats final : public WindowStats {
public:
    FiguresListStats(const Pos window_pos, const Pos window_size, const size_t figures_in_row, const size_t figures_num)
            : WindowStats(window_pos, window_size)
            , figures_in_row_(figures_in_row)
    {
        const int rows_num = static_cast<int>(ceil(figures_num / static_cast<double>(figures_in_row)));
        total_height_of_all_figures_ = static_cast<int>(rows_num * cell_size_.y);
    }
    void set_size(const int w, const int h) override {
        WindowStats::set_size(w, h);
        recalculate_dimensions();
    }
    void set_size(const LPARAM l_param) override { set_size(LOWORD(l_param), HIWORD(l_param)); }
    void recalculate_cell_size() override {
        cell_size_.x = cell_size_.y = static_cast<int>(std::max(0, window_size_.x) / figures_in_row_);
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
        const int rows_num = static_cast<int>(ceil(max_figures_in_row_ / static_cast<double>(figures_in_row_)));
        total_height_of_all_figures_ = static_cast<int>(rows_num * cell_size_.y);
        max_scroll_ = std::max(static_cast<int>(total_height_of_all_figures_), window_size_.y);
    }
    inline size_t get_figures_in_row() const { return figures_in_row_; }
    inline void inc_figures_in_row() { if (figures_in_row_ < max_figures_in_row_) figures_in_row_++; }
    inline void dec_figures_in_row() { if (figures_in_row_ != 1) figures_in_row_--; }
    inline size_t get_max_scroll() const { return max_scroll_; }
    // returns delta
    inline int add_to_curr_scroll(const int val) {
        const int old_scroll = curr_scroll_;
        curr_scroll_ = std::min(static_cast<int>(max_scroll_), std::max(0, curr_scroll_ + val));
        return curr_scroll_ - old_scroll;
    }
    inline int get_curr_scroll() const { return curr_scroll_; }
    inline int set_curr_scroll(const int new_scroll) {
        const int delta = new_scroll - curr_scroll_;
        curr_scroll_ = new_scroll;
        return delta;
    }
    inline size_t get_all_figures_height() const { return total_height_of_all_figures_; }
    inline void clear_scrolling() {
        max_scroll_ = 0;
        curr_scroll_ = 0;
    }
private:
    size_t figures_in_row_{ 2 };
    size_t max_figures_in_row_{ PLAYABLE_FIGURES.size() };
    int curr_scroll_{ 0 };
    size_t max_scroll_{ 0 };
    long total_height_of_all_figures_;
} inline figures_list{
    FIGURES_LIST_WINDOW_DEFAULT_POS, FIGURES_LIST_WINDOW_DEFAULT_DIMENSIONS,
    2, PLAYABLE_FIGURES.size()
};

/* Текущее состояние ввода */
class MotionInput {
public:
    explicit MotionInput(ChessGame* board)
        : board_(board)
    { }

    void clear();
    void prepare(Color turn);
    void calculate_possible_moves();
    void init_curr_choice_window(HWND, WNDPROC);
    inline void activate_by_click() { 
        input_order_by_two_ = true;
        input_order_by_one_ = 2;
    }
    inline void deactivate_by_click() { input_order_by_two_ = false; }
    inline void deactivate_by_pos() { input_order_by_one_ = 0; }
    inline void set_target(const Pos target) { input_.target = target; }
    inline void set_target(const int x, const int y) { input_.target = Pos{x, y}; }
    inline void set_from(const Pos from) { input_.from = from; }
    inline void set_in_hand(Figure* in_hand) { this->in_hand_ = in_hand; }
    inline void clear_hand() { this->in_hand_ = std::nullopt; }
    inline bool is_target_at_input() const { return input_.from == input_.target; }
    inline void set_lbutton_up() { is_lbutton_down_ = false; }
    inline void set_lbutton_down() { is_lbutton_down_ = true; }
    inline bool is_active_by_click() const { return input_order_by_two_; }
    inline bool is_current_turn(const Color turn) const { return in_hand_.has_value() && in_hand_.value()->is_col(turn); }
    inline auto get_in_hand() const { return in_hand_; }
    inline auto get_input() const { return input_; }
    inline void shift_from(const Pos shift, const int max_x, const int max_y) { input_.from.loop_add(shift, max_x, max_y); }
    inline void shift_target(const Pos shift, const int max_x, const int max_y) { input_.target.loop_add(shift, max_x, max_y); }
    inline void by_pos_to_next() { ++input_order_by_one_; }
    inline void set_from_x(const int val) { input_.from.x = val; }
    inline void set_from_y(const int val) { input_.from.y = val; }
    inline void set_target_x(const int val) { input_.target.x = val; }
    inline void set_target_y(const int val) { input_.target.y = val; }
    inline auto get_state_by_pos() const { return input_order_by_one_; }
    inline bool is_drags() const { return !is_curr_choice_moving_ && is_lbutton_down_ && in_hand_.has_value(); }
    inline auto get_possible_moves() { return all_moves_; }
    inline bool is_figure_dragged(const Id id) const { return in_hand_.has_value() && in_hand_.value()->is(id) && is_curr_choice_moving_ && !input_order_by_two_; }
private:
    ChessGame* board_;
    const Pos default_input_from_{ 0, -1 };
    Input input_{ default_input_from_, Pos{-1, -1} };
    int  input_order_by_one_{ 0 };
    bool input_order_by_two_{ false };
    bool is_lbutton_down_{ false };
    HWND curr_chose_window_{ };
    bool is_curr_choice_moving_{ false };
    std::optional<Figure*> in_hand_{ std::nullopt };
    std::vector<std::pair<bool, Pos>> all_moves_{};
} inline motion_input{ &board };
