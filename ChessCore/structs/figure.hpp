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

    enum class ParseErrorType : std::size_t {
        UnexpectedEnd = 0,
        IdDelimeterMissing = 1,
        InvalidId = 2,
        InvalidPos = 3,
        InvalidColor = 4,
        InvalidType = 5,
        Max = InvalidType,
    };
    
    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseResult<Figure, ParseErrorType>
    {
        Id id{ }; Pos pos{ }; Color col{ }; FigureType type{ };
        std::size_t curr_pos{ };
        const auto full_stop_pos = sv.find('.');
        if (full_stop_pos == sv.npos) {
            return UNEXPECTED_PARSE(ParseErrorType, IdDelimeterMissing, sv.size());
        }
        if (sv.size() - full_stop_pos - 1 < meta.max_pos_length + 1 + 1) {
        //  expected size of pos, col, type ^^^^^^^^^^^^^^^^^^^^^^^^^^^
        //  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ size of the rest of the string after the full stop
            return UNEXPECTED_PARSE(ParseErrorType, UnexpectedEnd, sv.size());
        }
        if (const auto id_sus = from_string<Id>{}(sv.substr(0, full_stop_pos))) {
            id = id_sus.value();
            curr_pos = full_stop_pos + 1;
        }
        else {
            UNEXPECTED_PARSE(ParseErrorType, InvalidId, id_sus.error());
        }
        if (const auto pos_sus = from_string<Pos>{}(sv.substr(curr_pos, meta.max_pos_length), meta)) {
            pos = pos_sus.value();
            curr_pos += meta.max_pos_length;
        }
        else {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidPos, curr_pos + pos_sus.error());
        }
        if (const auto col_sus = from_string<Color>{}(sv.substr(curr_pos, 1))) {
            col = col_sus.value();
            curr_pos += 1;
        }
        else {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidColor, curr_pos);
        }
        if (const auto type_sus = from_string<FigureType>{}(sv.substr(curr_pos, 1))) {
            type = type_sus.value();
        }
        else {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidType, curr_pos);
        }
        
        return Figure{ id, pos, col, type };
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
