#pragma once

#include "stuff.h"

class Figure {
    Id id{ ERR_ID };
    Pos position{ };
    Color color{ Color::None };
    FigureType type{ FigureType::None };
    
public:
    [[nodiscard]] constexpr Figure() noexcept
        {};
        
    [[nodiscard]] constexpr Figure(const Id id, 
                                   const Pos& position,      
                                   const Color color, 
                                   const FigureType type) noexcept
        : id{ id }
        , position{ position }
        , color{ color }
        , type{ type } {};
        
    constexpr void move_to(const Pos& p) noexcept 
        { position = p; }
    
    constexpr void move_to(const int x, const int y) noexcept
        { position.x = x; position.y = y; }
    
    [[nodiscard]] constexpr bool operator ==(const Figure& r) const
        { return this->id == r.id; }
    
    [[nodiscard]] std::string as_string() const {
        return std::format("{}.{}.{}.{}.{}",
            id, position.x, position.y, col_to_char(color), figure_type_to_char(type));
    }
    
    [[nodiscard]] constexpr Id get_id() const noexcept
        { return id; }
    
    [[nodiscard]] constexpr Pos get_pos() const noexcept
        { return position; }
    
    [[nodiscard]] constexpr Color get_col() const noexcept
        { return color; }
    
    [[nodiscard]] constexpr FigureType get_type() const noexcept
        { return type; }
    
    [[nodiscard]] constexpr bool is_col(const Color col) const noexcept
        { return color == col; }
    
    [[nodiscard]] constexpr bool is_col(const Figure* const fig) const noexcept
        { return color == fig->get_col(); }
    
    [[nodiscard]] constexpr bool empty() const noexcept
        { return id == ERR_ID; }
    
    [[nodiscard]] constexpr bool is(const Id id) const noexcept
        { return this->id == id; }
    
    [[nodiscard]] constexpr bool is(FigureType type) const noexcept
        { return this->type == type; }
    
    [[nodiscard]] constexpr bool at(Pos p) const noexcept
        { return position == p; }
};

[[nodiscard]] std::vector<Pos> to_pos_vector(const std::vector<Figure*>&) noexcept;
