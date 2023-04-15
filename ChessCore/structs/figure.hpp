#pragma once

#include "pos.hpp"
#include "../stuff/enums.hpp"
#include "../stuff/macro.h"
#include "../stuff/stuff.hpp"
#include "../stuff/parsing.hpp"
#include "../stuff/parse_meta.hpp"
#include "../stuff/parse_step.hpp"

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
    template <typename StepResult>
    using StepB = parse_step::ParseStepBuilder<StepResult>;

    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseEither<Figure, ParseErrorType>
    {
        using parse_step::execute_sequence, parse_step::ParseStepBuilder, parse_step::gen_constructor;
        using enum ParseErrorType;

        return execute_sequence(
            0ull, sv, meta,
            gen_constructor<Figure>()

            , StepB<Id>{}.error(Figure_InvalidId).on_abrupt_halt(Figure_CouldNotFindId).extra(1)
            , StepB<Pos>{}.error(Figure_InvalidPos).on_abrupt_halt(Figure_CouldNotFindPos).max_length(meta.max_pos_length)
            , StepB<Color>{}.error(Figure_InvalidColor).on_abrupt_halt(Figure_CouldNotFindColor)
            , StepB<FigureType>{}.error(Figure_InvalidType).on_abrupt_halt(Figure_CouldNotFindType)
        );
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
