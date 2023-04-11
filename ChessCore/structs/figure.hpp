#pragma once

#include "pos.hpp"
#include "../stuff/enums.hpp"
#include "../stuff/macro.h"
#include "../stuff/stuff.hpp"
#include "../stuff/parsing.hpp"
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
    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseEither<Figure, ParseErrorType>
    {
        std::size_t curr_pos{ };
        
        if (sv.empty()) {
            return UNEXPECTED_PARSE(Figure_CouldNotFindId, curr_pos);
        }
        const auto id_sus = from_string<Id>{}(sv.substr(curr_pos));
        if (id_sus) {
            curr_pos += id_sus.value().position + 1;
            //         cause full_stop after id ^^^
        }
        else {
            return UNEXPECTED_PARSE(Figure_InvalidId, curr_pos + id_sus.error());
        }
        if (sv.size() < curr_pos) {
            return UNEXPECTED_PARSE(Figure_CouldNotFindPos, curr_pos);
        }
        const auto pos_sus = from_string<Pos>{}(sv.substr(curr_pos, meta.max_pos_length), meta);
        if (pos_sus) {
            curr_pos += pos_sus.value().position;
        }
        else {
            return UNEXPECTED_PARSE(Figure_InvalidPos, curr_pos + pos_sus.error());
        }
        if (sv.size() < curr_pos) {
            return UNEXPECTED_PARSE(Figure_CouldNotFindColor, curr_pos);
        }
        const auto col_sus = from_string<Color>{}(sv.substr(curr_pos));
        if (col_sus) {
            curr_pos += col_sus.value().position;
        }
        else {
            return UNEXPECTED_PARSE(Figure_InvalidColor, curr_pos);
        }
        if (sv.size() < curr_pos) {
            return UNEXPECTED_PARSE(Figure_CouldNotFindType, curr_pos);
        }
        const auto type_sus = from_string<FigureType>{}(sv.substr(curr_pos));
        if (type_sus) {
            curr_pos += type_sus.value().position;
        }
        else {
            return UNEXPECTED_PARSE(Figure_InvalidType, curr_pos);
        }
        
        return { { Figure { 
                id_sus.value().value,
                pos_sus.value().value,
                col_sus.value().value,
                type_sus.value().value
            }, curr_pos
        } };
    }
};

template <>
struct as_string<Figure> {
    FN operator()(const Figure& fig, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        return as_string<Id>{}(fig.id, meta.min_id)
               + "."s
               + as_string<Pos>{}(fig.position, meta)
               + as_string<Color>{}(fig.color)
               + as_string<FigureType>{}(fig.type);
    }
};

template <>
struct as_string<const Figure*> {
    FN operator()(const Figure* fig, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        if (!fig) {
            return "nullptr";
        }
        return as_string<Figure>{}(*fig, meta);
    }
};
