#pragma once

#include "pos.hpp"
#include "../stuff/enums.hpp"
#include "../stuff/macro.h"
#include "../stuff/stuff.hpp"
#include "../stuff/parse_typedefs.hpp"
#include "../stuff/parse_meta.hpp"

#include <format>

class Figure {
    Id id;
    Pos position;
    Color color;
    FigureType type;
    
public:
    friend from_string<Figure>;
    friend as_string<Figure>;

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

template <>
struct from_string<Figure> {

    enum class ParseErrorType {
        UnexpectedEnd,
        IdDelimeterMissing,
        InvalidId,
        InvalidPos,
        InvalidColor,
        InvalidType,
    };
    
    [[nodiscard]] inline auto
        operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseResult<Figure, ParseErrorType>
    {
        // if length less than Pos(constant) + Color(1 ch) + Type(1 ch) + Id(1+ ch) + IdDelimeter (1 ch)
        if (sv.find('.') == sv.npos) {
            // return make_unexpected_parse<ParseError<ParseErrorType>>(ParseErrorType::IdDelimeterMissing, sv.size());
        }
        if (true || sv.size() < meta.max_pos_length + 4) {
            // return UNEXPECTED_PARSE(ParseErrorType::UnexpectedEnd, sv.size());
        }
        // FIXME todo
    }
};

template <>
struct as_string<Figure> {
    [[nodiscard]] inline auto
        operator()(const Figure& fig, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        return std::format("{}.{}{}{}",
            as_string<Id>{}(fig.id, meta.min_id), as_string<Pos>{}(fig.position, meta),
            as_string<Color>{}(fig.color), as_string<FigureType>{}(fig.type)
        );
    }
};

template <>
struct as_string<const Figure*> {
    [[nodiscard]] inline auto
        operator()(const Figure* fig, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        if (!fig) {
            return "nullptr";
        }
        return as_string<Figure>{}(*fig, meta);
    }
};
