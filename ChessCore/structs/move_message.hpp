#pragma once

#include "../stuff/visit.h"
#include "../stuff/stuff.hpp"
#include "input.hpp"
#include "figure.hpp"

#include <expected>
#include <variant>
#include <vector>

namespace mvmsg /* move_message */ {
    
    enum class ParseErrorType {
        EmptyMap,
    };

    struct ParseError {
        ParseErrorType type;
        std::size_t position;
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
        FigureType promotion_choice;
        MainEvent main_event;
        std::vector<SideEvent> side_evs;

        CTOR MoveMessage(
                const Figure* first,
                const Input& input,
                const FigureType p,
                const MainEvent main_event,
                const std::vector<SideEvent>& side_evs) noexcept
            : first{ *first }
            , input{ input }
            , promotion_choice{ p }
            , main_event{ main_event }
            , side_evs{ side_evs }
        {};
    };

    FN as_string(const SideEvent& side_event, const AsStringMeta& meta) noexcept -> std::string {
        return VISIT(side_event,
            [&](const Check&) {
                return "CH"s;
            },
            [&](const Promotion&) {
                return "PR"s;
            },
            [&](const CastlingBreak& cb) {
                return "CB"s
                    + as_string(cb.whose, meta);
            },
        );
    }

    FN as_string(const MoveMessage& move_message, const AsStringMeta& meta) noexcept -> std::string {
        std::string result{
            USING_BEG(&)
                std::string result{
                      move_message.first.as_string(meta)
                    + move_message.input.from.as_string()
                    + move_message.input.target.as_string()
                    + figure_type_to_char(move_message.promotion_choice)
                };
                for (const auto& side_event : move_message.side_evs) {
                    result += as_string(side_event, meta);
                }
                return result;
            USING_END
        };
        
        result += VISIT(move_message.main_event,
            [&](const Eat& eat) constexpr {
                return "E"s
                    + as_string(eat.eaten, meta);
            },
            [&](const Move& move) constexpr {
                return "M"s;
            },
            [&](const LongMove& long_move) constexpr {
                return "L"s;
            },
            [&](const Castling& castling) {
                return "C"s
                    + as_string(castling.second_to_move, meta)
                    + castling.second_input.from.as_string()
                    + castling.second_input.target.as_string();
            },
            [&](const EnPassant& en_passant) constexpr {
                return "P"s
                    + as_string(en_passant.eaten, meta);
            }
        );
        
        return result;
    }
    
    FN from_string(const std::string_view sv, const FromStringMeta& meta) noexcept -> std::expected<MoveMessage, ParseError> {
        if (true || sv.empty()) {
            return std::unexpected{ ParseError{ ParseErrorType::EmptyMap, 0ull } };
        }
        auto example0 =
            "02H8W8C17,TW!"
            "1542BQ" "1448BK" "1355WK" "1763WR"
            "<1828WB2835QM$"
                "1540BQ4041QM$"
                "1835WB3542QM$"
                "1541BQ4142QE18$>"
            "<1355WK5554QCB17M$>"
            "1842WB";
        auto example =
            "02H8W8CTW!"
            "1400WH"
                "1503BH"
                "1852WQ"
                "1656BK"
                "1763WK"
            "<1855WQ5552QM$"
                "1948BQ4800QCHM$"
                "1410WH1000QE19$"
                "1520BH2003QM$>"
            "<1656BK5657QM$"
                "1852WQ5228QCHM$>"
            "1900BQ";
        
        auto fig = Stringify<Figure>::from_string(sv.substr(0, 4), meta);
        
    }
}   // namespace mvmsg
