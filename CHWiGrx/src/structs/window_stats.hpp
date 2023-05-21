#pragma once

#include "../variables/constants.hpp"
#include "../winapi/framework.hpp"

/*
 * x-axis from left to right  (→)
 * y-axis from top  to bottom (↓)
 */
class WindowStats
{
protected:
    const int units_to_move_enough_{ 2 };
    const Pos extra_window_size_{ 0, constants::HEADER_HEIGHT };
    Pos window_pos_;
    Pos prev_lbutton_click_{};
    Pos window_size_;
    Pos cell_size_{ window_size_.x / WIDTH, window_size_.y / HEIGHT };
    const int indentation_from_edges_{ 0 };

    FN virtual recalculate() noexcept -> void
    {
        cell_size_ = Pos{ window_size_.x / WIDTH, window_size_.y / HEIGHT };
    }

public:
    CTOR WindowStats(const Pos& window_pos, const Pos& window_size) noexcept
      : window_pos_(window_pos)
      , window_size_(window_size)
    {
        WindowStats::recalculate();
    }

    virtual constexpr ~WindowStats() noexcept = default;

    FV virtual set_size(const int x /* LOWORD */, const int y /* HIWORD */) noexcept -> void
    {
        this->window_size_ = Pos{ x, y };
        recalculate();
    }

    FV set_size(const Pos& new_window_size) noexcept -> void { set_size(new_window_size.x, new_window_size.y); }

    FV virtual set_size(const LPARAM l_param) noexcept -> void { set_size(LOWORD(l_param), HIWORD(l_param)); }

    FV set_rect(const RECT rect) noexcept -> void
    {
        set_pos(rect.left, rect.top);
        set_size(rect.right - rect.left, rect.bottom - rect.top);
    }

    FN get_width() const noexcept -> int { return window_size_.x; }

    FN get_height() const noexcept -> int { return window_size_.y; }

    FN virtual get_width_with_extra() const noexcept -> int
    {
        return window_size_.x + extra_window_size_.x;
    }

    FN virtual get_height_with_extra() const noexcept -> int
    {
        return window_size_.y + extra_window_size_.y;
    }

    FN get_cell_width() const noexcept -> int { return cell_size_.x; }

    FN get_cell_height() const noexcept -> int { return cell_size_.y; }

    FV set_prev_lbutton_click(const Pos& prev_lbutton_click) noexcept -> void
    {
        this->prev_lbutton_click_ = prev_lbutton_click;
    }

    FN get_prev_lbutton_click() const noexcept -> Pos { return prev_lbutton_click_; }

    FV set_pos(const Pos& wp) noexcept -> void { window_pos_ = wp; }

    FV set_pos(const int x, const int y) noexcept -> void
    {
        window_pos_.x = x;
        window_pos_.y = y;
    }

    FV set_pos(const LPARAM l_param) noexcept -> void { set_pos(LOWORD(l_param), HIWORD(l_param)); }

    FN get_window_pos_x() const noexcept -> int { return window_pos_.x; }

    FN get_window_pos_y() const noexcept -> int { return window_pos_.y; }

    // Is not used
    FN is_mouse_moved_enough(const Pos& mouse) const noexcept -> bool
    {
        const auto shift = Pos{ abs(mouse.x - prev_lbutton_click_.x), abs(mouse.y - prev_lbutton_click_.y) };
        return shift.x >= units_to_move_enough_ && shift.y >= units_to_move_enough_;
    }

    FN divide_by_cell_size(const int x, const int y) const noexcept -> Pos
    {
        return Pos{ x / cell_size_.x, y / cell_size_.y };
    }

    FN divide_by_cell_size(const LPARAM l_param) const noexcept -> Pos
    {
        return divide_by_cell_size(LOWORD(l_param), HIWORD(l_param));
    }

    FN get_cell(const Pos& start) const noexcept -> RECT
    {
        return { .left = start.x * cell_size_.x + indentation_from_edges_,
                 .top = start.y * cell_size_.y + indentation_from_edges_,
                 .right = (start.x + 1) * cell_size_.x - indentation_from_edges_ * 2,
                 .bottom = (start.y + 1) * cell_size_.y - indentation_from_edges_ * 2 };
    }

    FN get_cell(const int i, const int j) const noexcept -> RECT
    {
        return get_cell(Pos{ i, j });
    }

    FN get_figure_under_mouse(const POINT mouse) const noexcept -> Pos
    {
        return divide_by_cell_size(mouse.x - window_pos_.x, mouse.y - window_pos_.y).change_axes();
    }
};
