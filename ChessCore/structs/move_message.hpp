#pragma once

#include "../stuff/visit.h"
#include "../stuff/stuff.hpp"
#include "input.hpp"
#include "figure.hpp"
#include "figure_fabric.hpp"

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
                const Figure first,
                const Input& input,
                const FigureType p,
                const MainEvent main_event,
                const std::vector<SideEvent>& side_evs) noexcept
            : first{ first }
            , input{ input }
            , promotion_choice{ p }
            , main_event{ main_event }
            , side_evs{ side_evs }
        {};
    };
}   // namespace mvmsg

template <>
struct from_string<mvmsg::SideEvent> {
    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
       -> ParseEither<mvmsg::SideEvent, ParseErrorType>
    {
        if (sv.empty()) {
            return UNEXPECTED_PARSE(SideEvent_EmptyString, sv.size() );
        }
        if (sv.size() < 2) {
            return UNEXPECTED_PARSE(SideEvent_CouldNotFindType, sv.size() );
        }
        if (sv.starts_with("CH"sv)) {
            return { { mvmsg::Check{}, 2ull } };
        }
        if (sv.starts_with("PR"sv)) {
            return { { mvmsg::Promotion{ }, 2ull } };
        }
        if (sv.starts_with("CB"sv)) {
            std::size_t curr_pos{ 2 };
            const auto id_sus = from_string<Id>{}(sv.substr(curr_pos));
            if (id_sus.has_value()) {
                return { { mvmsg::CastlingBreak{ id_sus.value().value }, curr_pos + id_sus.value().position + 1 } };
                //                                                          skip full_stop at end of the id ^^^
            }
            return UNEXPECTED_PARSE(SideEvent_InvalidCastlingBreakId, curr_pos);
        }
        return UNEXPECTED_PARSE(SideEvent_InvalidType, 1ull);
    }
};

template <>
struct as_string<mvmsg::SideEvent> {
    FN operator()(const mvmsg::SideEvent& side_event, const AsStringMeta& meta) const noexcept
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
                    + as_string<Id>{}(cb.whose, meta.min_id)
                    + "."s;
            },
        );
    }
};

template <>
struct from_string<mvmsg::MainEvent> {
    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseEither<mvmsg::MainEvent, ParseErrorType>
    {
        if (sv.empty()) {
            return UNEXPECTED_PARSE(MainEvent_CouldNotFindType, 0ull);
        }
        if (sv.starts_with("E"sv)) {
            std::size_t curr_pos{ 1 };
            const auto id_sus = from_string<Id>{}(sv.substr(curr_pos));
            if (id_sus.has_value()) {
                return { { mvmsg::Eat{ id_sus.value().value }, curr_pos + id_sus.value().position + 1 } };
                //                                                skip full_stop at end of the id ^^^
            }
            return UNEXPECTED_PARSE(MainEvent_InvalidEnPassantEatenId, 1ull);
        }
        if (sv.starts_with("M"sv)) {
            return { { mvmsg::Move{ }, 1ull } };
        }
        if (sv.starts_with("L"sv)) {
            return { { mvmsg::LongMove{ }, 1ull } };
        }
        if (sv.starts_with("C"sv)) {
            std::size_t curr_pos{ 1 };
            const auto id_sus = from_string<Id>{}(sv.substr(1));
            if (id_sus.has_value()) {
                curr_pos += id_sus.value().position;
                if (sv.size() < curr_pos) {
                    return UNEXPECTED_PARSE(MainEvent_CouldNotFindCastlingSecondInputFrom, curr_pos);
                }
                const auto from_sus = from_string<Pos>{}(sv.substr(curr_pos, meta.max_pos_length), meta);
                if (!from_sus) {
                    return UNEXPECTED_PARSE(MainEvent_InvalidCastlingSecondInputFrom, curr_pos + from_sus.error());
                }
                else {
                    curr_pos += from_sus.value().position;
                }
                if (sv.size() < curr_pos) {
                    return UNEXPECTED_PARSE(MainEvent_CouldNotFindCastlingSecondInputTo, curr_pos);
                }
                const auto to_sus = from_string<Pos>{}(sv.substr(curr_pos, meta.max_pos_length), meta);
                if (!to_sus) {
                    return UNEXPECTED_PARSE(MainEvent_InvalidCastlingSecondInputTo, curr_pos + to_sus.error());
                }
                else {
                    curr_pos += to_sus.value().position;
                }
                return { { 
                    mvmsg::Castling{
                        id_sus.value().value,
                        Input {
                            from_sus.value().value,
                            to_sus.value().value
                        }
                    },
                    curr_pos
                } };
            }
            return UNEXPECTED_PARSE(MainEvent_InvalidCastlingSecondToMoveId, 1ull);
        }
        if (sv.starts_with("P"sv)) {
            std::size_t curr_pos{ 1 };
            const auto id_sus = from_string<Id>{}(sv.substr(curr_pos));
            if (id_sus.has_value()) {
                return { { mvmsg::EnPassant{ id_sus.value().value }, curr_pos + id_sus.value().position + 1 } };
                //                                                      skip full_stop at end of the id ^^^
            }
            return UNEXPECTED_PARSE(MoveMessage_InvalidEnPassantToEatId, curr_pos);
        }
        return UNEXPECTED_PARSE(MainEvent_InvalidType, 1ull);
    }
};

template <>
struct as_string<mvmsg::MainEvent> {
    FN operator()(const mvmsg::MainEvent& main_event, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        return VISIT(main_event,
            [&](const mvmsg::Eat& eat) constexpr {
                return "E"s
                    + as_string<Id>{}(eat.eaten, meta.min_id)
                    + '.';
            },
            [&](const mvmsg::Move&) constexpr {
                return "M"s;
            },
                [&](const mvmsg::LongMove&) constexpr {
                return "L"s;
            },
                [&](const mvmsg::Castling& castling) {
                return "C"s
                    + as_string<Id>{}(castling.second_to_move, meta.min_id)
                    + '.'
                    + as_string<Pos>{}(castling.second_input.from, meta)
                    + as_string<Pos>{}(castling.second_input.target, meta);
            },
                [&](const mvmsg::EnPassant& en_passant) constexpr {
                return "P"s
                    + as_string<Id>{}(en_passant.eaten, meta.min_id)
                    + "."s;
            },
        );
    }
};  

template <>
struct from_string<mvmsg::MoveMessage> {
    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
       -> ParseEither<mvmsg::MoveMessage, ParseErrorType>
    {
        std::size_t curr_pos{ };

        if (sv.empty()) {
            return UNEXPECTED_PARSE(MoveMessage_EmptyMap, 0ull);
        }
        const auto figure_sus = from_string<Figure>{}(sv, meta);
        if (!figure_sus) {
            return std::unexpected{ ParseError<ParseErrorType>{ figure_sus.error().type, figure_sus.error().position } };
        }
        else {
            curr_pos += figure_sus.value().position;
        }
        if (sv.size() < curr_pos) {
            return UNEXPECTED_PARSE(MoveMessage_CouldNotFindFrom, curr_pos);
        }
        const auto from_sus = from_string<Pos>{}(sv.substr(curr_pos, meta.max_pos_length), meta);
        if (!from_sus) {
            return UNEXPECTED_PARSE(MoveMessage_InvalidFrom, curr_pos + from_sus.error());
        }
        else {
            curr_pos += from_sus.value().position;
        }
        if (sv.size() < curr_pos) {
            return UNEXPECTED_PARSE(MoveMessage_CouldNotFindTo, curr_pos);
        }
        const auto to_sus = from_string<Pos>{}(sv.substr(curr_pos, meta.max_pos_length), meta);
        if (!to_sus) {
            return UNEXPECTED_PARSE(MoveMessage_InvalidTo, curr_pos + to_sus.error());
        }
        else {
            curr_pos += to_sus.value().position;
        }
        if (sv.size() < curr_pos) {
            return UNEXPECTED_PARSE(MoveMessage_CouldNotFindPromotionChoice, curr_pos + to_sus.value().position);
        }
        const auto promotion_choice_sus = from_string<FigureType>{}(sv.substr(curr_pos));
        if (!promotion_choice_sus) {
            return UNEXPECTED_PARSE(MoveMessage_InvalidPromotionChoice, curr_pos + to_sus.value().position);
        }
        else {
            curr_pos += promotion_choice_sus.value().position;
        }
        if (sv.size() < curr_pos) {
            return UNEXPECTED_PARSE(MoveMessage_CouldNotFindMainEvent, curr_pos);
        }
        const auto main_event_sus = from_string<mvmsg::MainEvent>{}(sv.substr(curr_pos), meta);
        if (!main_event_sus) {
            return std::unexpected{ ParseError<ParseErrorType>{
                main_event_sus.error().type,
                curr_pos
                    + main_event_sus.error().position
            } };
        }
        else {
            curr_pos += main_event_sus.value().position;
        }
        std::vector<mvmsg::SideEvent> side_events;
        while (curr_pos < sv.size()) {
            const auto side_event_sus = from_string<mvmsg::SideEvent>{}(sv.substr(curr_pos), meta);
            if (!side_event_sus) {
                return std::unexpected{ ParseError<ParseErrorType>{
                    side_event_sus.error().type,
                    curr_pos
                        + side_event_sus.error().position
                } };
            }
            side_events.push_back(side_event_sus.value().value);
            curr_pos += side_event_sus.value().position;
        }
        return { { mvmsg::MoveMessage {
            figure_sus.value().value,
            Input {
                from_sus.value().value,
                to_sus.value().value
            },
            promotion_choice_sus.value().value,
            main_event_sus.value().value,
            side_events,
        }, curr_pos } };
        // ^^^^^^^^ looks useless cause of while loop
    }
};

template <>
struct as_string<mvmsg::MoveMessage> {
    FN operator()(const mvmsg::MoveMessage& move_message, const AsStringMeta& meta) const noexcept
       -> std::string
    {
        std::string result{
              as_string<Figure>{}(move_message.first, meta)
            + as_string<Pos>{}(move_message.input.from, meta)
            + as_string<Pos>{}(move_message.input.target, meta)
            + as_string<FigureType>{}(move_message.promotion_choice)
            + as_string<mvmsg::MainEvent>{}(move_message.main_event, meta)
        };

        for (const auto& side_event : move_message.side_evs) {
            result += as_string<mvmsg::SideEvent>{}(side_event, meta);
        }

        return result;
    }
};
