#pragma once

#include "stuff/stuff.hpp"
#include "structs/input.hpp"
#include "structs/figure.hpp"

#include <expected>
#include <variant>
#include <vector>

namespace mvmsg /* move_message */ {

    enum class ParseError {
        EmptyMap,
    };

    struct Check {};
    struct Promotion {};
    struct CastlingBreak {
        Id whose;
    };

    using SideEvent =
        std::variant<
            Check,
            Promotion,
            CastlingBreak
        >;

    struct MainEventWithFigureMove {
        Figure first;
        Input input;
        Color turn;
        FigureType promotion_choice;
        std::vector<SideEvent> side_evs;

        CTOR MainEventWithFigureMove(
                   const Figure* first,
                   const Input& input,
                   const Color turn,
                   const FigureType p,
                   const std::vector<SideEvent>& side_evs) noexcept
            : first{ *first }
            , input{ input }
            , turn{ turn }
            , promotion_choice{ p }
            , side_evs{ side_evs }
        {};
    };

    struct Eat : MainEventWithFigureMove {
        Id eaten;

        CTOR Eat(const Figure* first,
                 const Input& input,
                 const Color turn,
                 const FigureType p,
                 const std::vector<SideEvent>& side_evs,
                 const Id eaten) noexcept
            : MainEventWithFigureMove{ first, input, turn, p, side_evs }
            , eaten{ eaten }
        {};
    };
    struct Move : MainEventWithFigureMove {
        using MainEventWithFigureMove::MainEventWithFigureMove;
    };
    struct LongMove : MainEventWithFigureMove {
        using MainEventWithFigureMove::MainEventWithFigureMove;
    };
    struct Castling : MainEventWithFigureMove {
        Id second_to_move;
        Input second_input;

        CTOR Castling(const Figure* first,
                      const Input& input,
                      const Color turn,
                      const FigureType p,
                      const std::vector<SideEvent>& side_evs,
                      const Id second_to_move,
                      const Input& second_input) noexcept
            : MainEventWithFigureMove{ first, input, turn, p, side_evs }
            , second_to_move{ second_to_move }
            , second_input{ second_input }
        {};
    };
    struct EnPassant : MainEventWithFigureMove {
        Id eaten;

        CTOR EnPassant(const Figure* first,
                       const Input& input,
                       const Color turn,
                       const FigureType p,
                       const std::vector<SideEvent>& side_evs,
                       const Id eaten) noexcept
            : MainEventWithFigureMove{ first, input, turn, p, side_evs }
            , eaten{ eaten }
        {};
    };
    
    using MoveMessage =
        std::variant<
            Eat,
            Move,
            LongMove,
            Castling,
            EnPassant
        >;

    FN as_string(const MoveMessage& move_message) noexcept -> std::string {
        return "";
    }

    FN from_string(const std::string_view sv) noexcept -> std::expected<MoveMessage, ParseError> {
        return std::unexpected{ ParseError::EmptyMap };
    }
}   // namespace mvmsg
