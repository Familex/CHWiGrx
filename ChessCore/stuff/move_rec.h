#pragma once

#include "stuff.h"
#include "figure.h"
#include "figure_fabric.h"

struct MoveRec {
    [[nodiscard]] MoveRec(const Figure* who_went, 
                          const Input& input,
                          const Color turn, 
                          const MoveMessage& ms, 
                          const FigureType p) noexcept
        : who_went{ *who_went }
        , input{ input }
        , turn{ turn }
        , ms{ ms }
        , promotion_choice{ p } {};
        
    [[nodiscard]] MoveRec() noexcept = default;
        
    [[nodiscard]] MoveRec(const std::string&);
    
    [[nodiscard]] std::string as_string() const;
    
    Figure who_went{};
    Input input{};
    Color turn{ Color::None };
    MoveMessage ms{};
    FigureType promotion_choice{ FigureType::None };
};
