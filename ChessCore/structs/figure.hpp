#pragma once

#include "../stuff/macro.h"
#include "../stuff/stuff.hpp"

#include <format>

class Figure {
    Id id{ ERR_ID };
    Pos position{ };
    Color color{ Color::None };
    FigureType type{ FigureType::None };
    
public:
    CTOR Figure() noexcept {};

    CTOR Figure(const Id id, const Pos& position, const Color color, const FigureType type) noexcept
        : id{ id }
        , position{ position }
        , color{ color }
        , type{ type } {};
        
    FN move_to(const Pos& p) noexcept {
        position = p;
    }
    
    FN move_to(const int x, const int y) noexcept {
        position.x = x; position.y = y;
    }
    
    FN operator <=>(const Figure& r) const noexcept {
        return this->id <=> r.id;
    }
    
    [[nodiscard]] auto
        as_string() const noexcept -> std::string
    {
        // x-axis from top  to bottom (↓)
        // y-axis from left to right  (→)
        return std::format("{}{}{}{}",
            id, position.as_string(),
            col_to_char(color), figure_type_to_char(type)
        );
    }
    
    FN get_id() const noexcept -> Id {
        return id;
    }
    
    FN get_pos() const noexcept -> Pos {
        return position;
    }
    
    FN get_col() const noexcept -> Color {
        return color;
    }
    
    FN get_type() const noexcept -> FigureType {
        return type;
    }
    
    FN is_col(const Color col) const noexcept -> bool {
        return color == col;
    }
    
    FN is_col(const Figure* const fig) const noexcept -> bool {
        return color == fig->get_col();
    }

    FN is(const Id id) const noexcept -> bool {
        return this->id == id;
    }
    
    FN is(FigureType type) const noexcept -> bool {
        return this->type == type;
    }
    
    FN at(Pos p) const noexcept -> bool {
        return position == p;
    }
};

FN to_pos_vector(const std::vector<Figure*>& lst) noexcept -> std::vector<Pos> {
    std::vector<Pos> acc{};
    for (const auto& fig : lst) {
        acc.emplace_back(fig->get_pos());
    }
    return acc;
}
