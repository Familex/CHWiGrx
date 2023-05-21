#pragma once

#include "window_stats.hpp"

class FiguresListStats final : public WindowStats
{
    size_t figures_in_row_{ 2 };
    size_t max_figures_in_row_{ PLAYABLE_FIGURES.size() };
    int curr_scroll_{};
    size_t max_scroll_{};
    long total_height_of_all_figures_{};

    // Not constexpr cause of std::ceil
    void recalculate() noexcept override
    {
        cell_size_.x = cell_size_.y = std::max(0, window_size_.x) / static_cast<int>(figures_in_row_);

        const int rows_num =
            static_cast<int>(std::ceil(static_cast<double>(max_figures_in_row_) / static_cast<double>(figures_in_row_))
            );
        total_height_of_all_figures_ = rows_num * cell_size_.y;
        max_scroll_ =
            static_cast<std::size_t>(std::max(static_cast<int>(total_height_of_all_figures_), window_size_.y));
    }

public:
    [[nodiscard]] explicit FiguresListStats(
        const Pos& window_pos,
        const Pos& window_size,
        const size_t figures_in_row,
        const size_t
    ) noexcept
      : WindowStats(window_pos, window_size)
      , figures_in_row_(figures_in_row)
    {
        recalculate();
    }

    FV set_size(const LPARAM l_param) noexcept -> void override
    {
        WindowStats::set_size(LOWORD(l_param), HIWORD(l_param));
    }

    [[nodiscard]] auto get_width_with_extra() const noexcept -> int override
    {
        // FIXME There may be no slider
        return WindowStats::get_width_with_extra() + constants::SCROLLBAR_THICKNESS;
    }

    [[nodiscard]] auto get_height_with_extra() const noexcept -> int override
    {
        return WindowStats::get_height_with_extra() + constants::SCROLLBAR_THICKNESS;
    }

    FN get_figures_in_row() const noexcept -> std::size_t { return figures_in_row_; }

    FV inc_figures_in_row() noexcept -> void
    {
        if (figures_in_row_ >= max_figures_in_row_)
            return;

        figures_in_row_++;
        recalculate();
    }

    FV dec_figures_in_row() noexcept -> void
    {
        if (figures_in_row_ <= 1)
            return;

        figures_in_row_--;
        recalculate();
    }

    FN get_max_scroll() const noexcept -> std::size_t { return max_scroll_; }

    // returns delta
    FN add_to_curr_scroll(const int val) noexcept -> int
    {
        const int old_scroll = curr_scroll_;
        curr_scroll_ = std::min(static_cast<int>(max_scroll_), std::max(0, curr_scroll_ + val));
        return curr_scroll_ - old_scroll;
    }

    FN get_curr_scroll() const noexcept -> int { return curr_scroll_; }

    FN set_curr_scroll(const int new_scroll) noexcept -> int
    {
        const int delta = new_scroll - curr_scroll_;
        curr_scroll_ = new_scroll;
        return delta;
    }

    FN get_all_figures_height() const noexcept -> std::size_t
    {
        return total_height_of_all_figures_;
    }

    FV clear_scrolling() noexcept -> void
    {
        max_scroll_ = 0;
        curr_scroll_ = 0;
    }
};
