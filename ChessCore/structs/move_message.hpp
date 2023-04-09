#pragma once

#include "../stuff/visit.h"
#include "../stuff/stuff.hpp"
#include "input.hpp"
#include "figure.hpp"

#include <expected>
#include <variant>
#include <vector>

namespace mvmsg /* move_message */ {
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

        friend from_string<MoveMessage>;
        friend as_string<MoveMessage>;

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
}   // namespace mvmsg

template <>
struct from_string<mvmsg::SideEvent> {
    // TODO?
};

template <>
struct as_string<mvmsg::SideEvent> {
    [[nodiscard]] inline auto
        operator()(const mvmsg::SideEvent& side_event, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        return VISIT(side_event,
            [&](const mvmsg::Check&) {
                return "CH"s;
            },
            [&](const mvmsg::Promotion&) {
                return "PR"s;
            },
            [&](const mvmsg::CastlingBreak& cb) {
                return "CB"s
                    + as_string<Id>{}(cb.whose, meta.min_id);
            },
        );
    }
};

template <>
struct from_string<mvmsg::MoveMessage> {
    enum class ParseErrorType {
        EmptyMap = 0,
        Max = 0,
    };

    [[nodiscard]] inline auto
        operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseResult<mvmsg::MoveMessage, ParseErrorType> {
        if (true || sv.empty()) {
            // return std::unexpected{ ParseError{ ParseErrorType::EmptyMap, 0ull } };
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

        auto fig = from_string<Figure>{}(sv.substr(0, 4), meta);
        return UNEXPECTED_PARSE(ParseErrorType, EmptyMap, 999ull);
    }
};

template <>
struct as_string<mvmsg::MoveMessage> {
    [[nodiscard]] inline auto
        operator()(const mvmsg::MoveMessage& move_message, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        std::string result{
            USING_BEG(&)
                std::string result{
                      as_string<Figure>{}(move_message.first, meta)
                    + as_string<Pos>{}(move_message.input.from, meta)
                    + as_string<Pos>{}(move_message.input.target, meta)
                    + as_string<FigureType>{}(move_message.promotion_choice)
                };
                for (const auto& side_event : move_message.side_evs) {
                    result += as_string<mvmsg::SideEvent>{}(side_event, meta);
                }
                return result;
            USING_END
        };

        result += VISIT(move_message.main_event,
            [&](const mvmsg::Eat& eat) constexpr {
                return "E"s
                    + as_string<Id>{}(eat.eaten, meta.min_id);
            },
            [&](const mvmsg::Move& move) constexpr {
                return "M"s;
            },
            [&](const mvmsg::LongMove& long_move) constexpr {
                return "L"s;
            },
            [&](const mvmsg::Castling& castling) {
                return "C"s
                    + as_string<Id>{}(castling.second_to_move, meta.min_id)
                    + as_string<Pos>{}(castling.second_input.from, meta)
                    + as_string<Pos>{}(castling.second_input.target, meta);
            },
            [&](const mvmsg::EnPassant& en_passant) constexpr {
                return "P"s
                    + as_string<Id>{}(en_passant.eaten, meta.min_id);
            }
        );

        return result;
    }
};
