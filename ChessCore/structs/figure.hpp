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
    Id id_;
    Pos position_;
    Color color_;
    FigureType type_;
    
public:
    friend FromString<Figure>;
    friend AsString<Figure>;

    CTOR Figure(const Id id, const Pos& position, const Color color, const FigureType type) noexcept
        : id_{ id }
        , position_{ position }
        , color_{ color }
        , type_{ type } {};
        
    FN move_to(const Pos& p) noexcept {
        position_ = p;
    }
    
    FN move_to(const int x, const int y) noexcept {
        position_.x = x; position_.y = y;
    }
    
    FN operator <=>(const Figure& r) const noexcept {
        return this->id_ <=> r.id_;
    }
    
    FN get_id() const noexcept -> Id {
        return id_;
    }
    
    FN get_pos() const noexcept -> Pos {
        return position_;
    }
    
    FN get_col() const noexcept -> Color {
        return color_;
    }
    
    FN get_type() const noexcept -> FigureType {
        return type_;
    }
    
    FN is_col(const Color col) const noexcept -> bool {
        return color_ == col;
    }
    
    FN is_col(const Figure* const fig) const noexcept -> bool {
        return color_ == fig->get_col();
    }

    FN is(const Id id) const noexcept -> bool {
        return this->id_ == id;
    }
    
    FN is(const FigureType type) const noexcept -> bool {
        return this->type_ == type;
    }
    
    FN at(const Pos p) const noexcept -> bool {
        return position_ == p;
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
struct FromString<Figure> {
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
struct AsString<Figure> {
    FN operator()(const Figure& fig, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        return AsString<Id>{}(fig.id_, meta.min_id)
               + "."s
               + AsString<Pos>{}(fig.position_, meta)
               + AsString<Color>{}(fig.color_)
               + AsString<FigureType>{}(fig.type_);
    }
};

template <>
struct AsString<const Figure*> {
    FN operator()(const Figure* fig, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        if (!fig) {
            return "nullptr";
        }
        return AsString<Figure>{}(*fig, meta);
    }
};
