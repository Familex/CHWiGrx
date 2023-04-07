#pragma once

#include "../stuff/visit.h"
#include "../stuff/stuff.hpp"
#include "input.hpp"
#include "figure.hpp"

#include <expected>
#include <variant>
#include <vector>

namespace mvmsg /* move_message */ {

    enum class ParseError {
        EmptyMap,
    };

    // Side Event
    struct Check { };

    // Side Event
    struct Promotion { };

    // Side Event
    struct CastlingBreak {
        Id whose;
    };

    using SideEvent =
        std::variant<
            Check,
            Promotion,
            CastlingBreak
        >;

    // Main Event
    struct Eat {
        Id eaten;
    };

    // Main Event
    struct Move { };
    
    // Main Event
    struct LongMove { };
    
    // Main Event
    struct Castling {
        Id second_to_move;
        Input second_input;
    };
    
    // Main Event
    struct EnPassant {
        Id eaten;
    };

    using MainEvent =
        std::variant<
            Eat,
            Move,
            LongMove,
            Castling,
            EnPassant
        >;
    
    struct MoveMessage {
        Figure first;
        Input input;
        Color turn;
        FigureType promotion_choice;
        MainEvent main_event;
        std::vector<SideEvent> side_evs;

        CTOR MoveMessage(
                const Figure* first,
                const Input& input,
                const Color turn,
                const FigureType p,
                const MainEvent main_event,
                const std::vector<SideEvent>& side_evs) noexcept
            : first{ *first }
            , input{ input }
            , turn{ turn }
            , promotion_choice{ p }
            , main_event{ main_event }
            , side_evs{ side_evs }
        {};
    };

    FN as_string(const SideEvent& side_event) noexcept -> std::string {
        return VISIT(side_event,
            [](const Check&) {
                return "CH"s;
            },
            [](const Promotion&) {
                return "PR"s;
            },
            [](const CastlingBreak& cb) {
                return "CB"s
                    + as_string(cb.whose);
            },
        );
    }

    FN as_string(const MoveMessage& move_message) noexcept -> std::string {
        std::string result{
            USING_BEG(&)
                std::string result{
                      move_message.first.as_string()
                    + move_message.input.from.as_string()
                    + move_message.input.target.as_string()
                    + col_to_char(move_message.turn)
                    + as_string(move_message.promotion_choice)
                };
                for (const auto& side_event : move_message.side_evs) {
                    result += as_string(side_event);
                }
                return result;
            USING_END
        };
        
        result += VISIT(move_message.main_event,
            [&](const Eat& eat) constexpr {
                return "E"s
                    + as_string(eat.eaten);
            },
            [&](const Move& move) constexpr {
                return "M"s;
            },
            [&](const LongMove& long_move) constexpr {
                return "L"s;
            },
            [&](const Castling& castling) {
                return "C"s
                    + as_string(castling.second_to_move)
                    + castling.second_input.from.as_string()
                    + castling.second_input.target.as_string();
            },
            [&](const EnPassant& en_passant) constexpr {
                return "P"s
                    + as_string(en_passant.eaten);
            }
        );
        
        return result;
    }
    
    FN from_string(const std::string_view sv) noexcept -> std::expected<MoveMessage, ParseError> {
        return std::unexpected{ ParseError::EmptyMap };
    }
}   // namespace mvmsg
