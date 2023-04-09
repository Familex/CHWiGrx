#pragma once

#include "../stuff/macro.h"
#include "../stuff/stuff.hpp"
#include "../stuff/parse_typedefs.hpp"

#include <format>

class Figure {
    Id id;
    Pos position;
    Color color;
    FigureType type;
    
public:
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
        as_string(const AsStringMeta& meta) const noexcept -> std::string
    {
        return std::format("{}.{}{}{}",
            ::as_string(id, meta), position.as_string(),
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

STRINGIFY_DECLARE_BEGIN(Figure)

    STRINGIFY_DECLARE_ERROR_TYPE{
        UnexpectedEnd,
        IdDelimeterMissing,
        InvalidId,
        InvalidPos,
        InvalidColor,
        InvalidType,
    };

    STRINGIFY_DECLARE_FROM_STRING {
        // if length less than Pos(constant) + Color(1 ch) + Type(1 ch) + Id(1+ ch) + IdDelimeter (1 ch)
        if (sv.find('.') == sv.npos) {
            return UNEXPECTED_PARSE(ParseErrorType::IdDelimeterMissing, sv.size());
        }
        if (sv.size() < meta.max_pos_length + 4) {
            return UNEXPECTED_PARSE(ParseErrorType::UnexpectedEnd, sv.size());
        }
    }

    STRINGIFY_DECLARE_AS_STRING{
        return " ";
    }
        
STRINGIFY_DECLARE_END
