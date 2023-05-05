#pragma once

#include "figure.hpp"
#include "pos.hpp"

#include <map>
#include <ranges>

// FIXME order of methods
struct FigureBoard
{
    std::map<Pos, Figure*> figures;
    std::vector<Figure*> captured_figures;

    FigureBoard() noexcept = default;

    FigureBoard(const FigureBoard&) noexcept = delete;

    FigureBoard(FigureBoard&& other) noexcept
    {
        figures = std::move(other.figures);
        captured_figures = std::move(other.captured_figures);
    }

    FigureBoard& operator=(const FigureBoard&) noexcept = delete;

    FigureBoard& operator=(FigureBoard&& other) noexcept
    {
        figures = std::move(other.figures);
        captured_figures = std::move(other.captured_figures);
        return *this;
    }

    ~FigureBoard() noexcept { clear(); }

    void clear() noexcept
    {
        for (const auto& fig : figures | std::views::values) {
            delete fig;
        }
        figures.clear();
        for (const auto& fig : captured_figures) {
            delete fig;
        }
        captured_figures.clear();
    }

    template <typename Self>
    [[nodiscard]] auto get_figures(this Self&& self) noexcept
    {
        return self.figures | std::views::values | std::ranges::to<std::vector<Figure*>>();
    }

    template <typename Self>
    [[nodiscard]] auto get_fig(this Self&& self, const Pos position) noexcept
        -> std::optional<typename decltype(self.figures)::mapped_type>
    {
        if (self.figures.contains(position)) {
            return self.figures.at(position);
        }
        return std::nullopt;
    }

    template <typename Self>
    [[nodiscard]] auto
    get_fig(this Self&& self, const Id id) noexcept -> std::optional<typename decltype(self.figures)::mapped_type>
    {
        for (auto fig : self.figures | std::views::values) {
            if (fig->is(id)) {
                return fig;
            }
        }
        return std::nullopt;
    }

    template <typename Self>
    [[nodiscard]] auto
    get_fig_unsafe(this Self&& self, const Pos position) noexcept -> typename decltype(self.figures)::mapped_type
    {
        const auto out = self.get_fig(position);
        return out.has_value() ? out.value() : nullptr;
    }

    template <typename Self>
    [[nodiscard]] auto
    get_fig_unsafe(this Self&& self, const Id id) noexcept -> typename decltype(self.figures)::mapped_type
    {
        const auto out = self.get_fig(id);
        return out.has_value() ? out.value() : nullptr;
    }

    [[nodiscard]] auto find_king(Color col) const noexcept -> std::optional<const Figure*>
    {
        if (const auto map_ptr = std::ranges::find_if(
                figures,
                [col](const auto& it) { return it.second->is_col(col) && it.second->get_type() == FigureType::King; }
            );
            map_ptr != figures.end())
        {
            return map_ptr->second;
        }
        return std::nullopt;
    }

    void place_figure(Figure* const fig) noexcept { figures[fig->get_pos()] = fig; }

    [[nodiscard]] bool empty() const noexcept { return figures.empty(); }

    [[nodiscard]] bool cont_fig(const Pos& position) const noexcept { return figures.contains(position); }

    [[nodiscard]] bool capture_figure(Figure* const it)
    {
        captured_figures.push_back(it);
        figures.erase(it->get_pos());
        return true;
    }

    [[nodiscard]] bool capture_figure(const Id& id)
    {
        if (const auto fig = get_fig(id)) {
            captured_figures.push_back(fig.value());
            figures.erase(fig.value()->get_pos());
            return true;
        }
        return false;
    }

    void recapture_figure(const Id& id)
    {
        const auto to_resurrect_id =
            std::ranges::find_if(captured_figures, [&id](auto&& val) { return id == val->get_id(); });
        if (to_resurrect_id == captured_figures.end()) {
            assert(!"This figure can't be resurrected");
        }
        Figure* to_resurrect = *to_resurrect_id;
        place_figure(to_resurrect);
        captured_figures.erase(to_resurrect_id);
    }

    bool delete_fig(const Pos& pos)
    {
        if (cont_fig(pos)) {
            delete_fig_unsafe(pos);
            return true;
        }
        return false;
    }

    void delete_fig_unsafe(const Pos& pos)
    {
        delete figures[pos];
        figures.erase(pos);
    }

    void place_fig(Figure* const fig)
    {
        if (cont_fig(fig->get_pos()))
            delete_fig(fig->get_pos());
        figures[fig->get_pos()] = fig;
    }

    [[nodiscard]] auto get_figures_of(const Color col) const noexcept -> std::vector<Figure*>
    {
        std::vector<Figure*> acc{};
        for (const auto& fig : figures | std::views::values) {
            if (fig->is_col(col)) {
                acc.push_back(fig);
            }
        }
        return acc;
    }

    void swap_fig(Figure* fig1, Figure* fig2)
    {
        figures.erase(fig1->get_pos());
        figures.erase(fig2->get_pos());

        const Pos fig1_prev_pos = fig1->get_pos();
        fig1->move_to(fig2->get_pos());
        fig2->move_to(fig1_prev_pos);

        figures[fig1->get_pos()] = fig1;
        figures[fig2->get_pos()] = fig2;
    }

    void move_fig(Figure* fig, const Pos& to, const bool capture = true)
    {
        if (fig->get_pos() == to) {
            return;
        }
        if (const auto maybe_eat = get_fig(to); maybe_eat.has_value()) {
            if (capture) {
                [[maybe_unused]] const auto res = capture_figure(maybe_eat.value());
            }
            else {
                figures.erase(maybe_eat.value()->get_pos());
                delete maybe_eat.value();
            }
        }
        figures.erase(fig->get_pos());
        fig->move_to(to);
        figures[fig->get_pos()] = fig;
    }
};
