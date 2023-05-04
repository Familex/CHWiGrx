#pragma once

#include "../declarations.hpp"
#include "chess_game.hpp"

class MotionInput
{
public:
    CTOR MotionInput(ChessGame& board)
      : board_{ board }
    { }

    void clear()
    {
        is_lbutton_down_ = false;
        destroy_window(curr_choice_window_);
        is_curr_choice_moving_ = false;
        deactivate_by_click();
        deactivate_by_pos();
        clear_hand();
        input_ = Input{ Pos{ 0, -1 }, Pos{ -1, -1 } };
        all_moves_.clear();
    }

    FN prepare(const Color turn) noexcept -> void
    {
        in_hand_ = board_.get_fig(input_.from);
        input_.target = input_.from;
        if (in_hand_.has_value() && in_hand_.value()->is_col(turn)) {
            calculate_possible_moves();
        }
    }

    FN calculate_possible_moves() noexcept -> void
    {
        if (in_hand_.has_value()) {
            all_moves_ = board_.get_all_possible_moves(in_hand_.value());
        }
    }

    void init_curr_choice_window(const HWND h_wnd, const WNDPROC callback, Pos cell_size) noexcept
    {
        if (!in_hand_.has_value())
            return;

        is_curr_choice_moving_ = true;
        POINT mouse{};
        GetCursorPos(&mouse);
        curr_choice_window_ =
            create_curr_choice_window(h_wnd, in_hand_.value(), mouse, cell_size.x, cell_size.y, callback);
        // Force redraw, because there is an artifact
        RedrawWindow(h_wnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
        SendMessage(curr_choice_window_, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(mouse.x, mouse.y));
    }

    FN activate_by_click() noexcept -> void
    {
        input_order_by_two_ = true;
        input_order_by_one_ = 2;
    }

    FN deactivate_by_click() noexcept -> void { input_order_by_two_ = false; }

    FN deactivate_by_pos() noexcept -> void { input_order_by_one_ = 0; }

    FN set_target(const Pos target) noexcept -> void { input_.target = target; }

    FN set_target(const int x, const int y) noexcept -> void { input_.target = Pos{ x, y }; }

    FN set_from(const Pos from) noexcept -> void { input_.from = from; }

    FN set_in_hand(Figure* in_hand) noexcept -> void { this->in_hand_ = in_hand; }

    FN clear_hand() noexcept -> void { this->in_hand_ = std::nullopt; }

    FN is_target_at_input() const noexcept -> bool { return input_.from == input_.target; }

    FN set_lbutton_up() noexcept -> void { is_lbutton_down_ = false; }

    FN set_lbutton_down() noexcept -> void { is_lbutton_down_ = true; }

    FN is_active_by_click() const noexcept -> bool { return input_order_by_two_; }

    FN is_current_turn(const Color turn) const noexcept -> bool
    {
        return in_hand_.has_value() && in_hand_.value()->is_col(turn);
    }

    FN get_in_hand() const noexcept { return in_hand_; }

    FN get_input() const noexcept -> Input { return input_; }

    FN shift_from(const Pos shift, const int max_x, const int max_y) noexcept -> void
    {
        input_.from.loop_add(shift, max_x, max_y);
    }

    FN shift_target(const Pos shift, const int max_x, const int max_y) noexcept -> void
    {
        input_.target.loop_add(shift, max_x, max_y);
    }

    FN by_pos_to_next() noexcept -> void { ++input_order_by_one_; }

    FN set_from_x(const int val) noexcept -> void { input_.from.x = val; }

    FN set_from_y(const int val) noexcept -> void { input_.from.y = val; }

    FN set_target_x(const int val) noexcept -> void { input_.target.x = val; }

    FN set_target_y(const int val) noexcept -> void { input_.target.y = val; }

    FN get_state_by_pos() const noexcept -> int { return input_order_by_one_; }

    FN is_drags() const noexcept -> bool
    {
        return !is_curr_choice_moving_ && is_lbutton_down_ && in_hand_.has_value();
    }

    FN get_possible_moves() const noexcept { return all_moves_; }

    FN is_figure_dragged(const Id id) const noexcept -> bool
    {
        return in_hand_.has_value() && in_hand_.value()->is(id) && is_curr_choice_moving_ && !input_order_by_two_;
    }

private:
    ChessGame& board_;
    const Pos default_input_from_{ 0, -1 };
    Input input_{ default_input_from_, Pos{ -1, -1 } };
    int input_order_by_one_{ 0 };
    bool input_order_by_two_{ false };
    bool is_lbutton_down_{ false };
    HWND curr_choice_window_{};
    bool is_curr_choice_moving_{ false };
    std::optional<Figure*> in_hand_{ std::nullopt };
    std::vector<std::pair<bool, Pos>> all_moves_{};
};
