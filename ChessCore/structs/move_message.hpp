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
    enum class ParseErrorType : std::size_t {
        EmptyString = 0,
        CouldNotFindType = 1,
        InvalidType = 2,
        InvalidCastlingBreakId = 3,
        Max = InvalidCastlingBreakId,
    };

    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
       -> ParseEither<mvmsg::SideEvent, ParseErrorType>
    {
        if (sv.empty()) {
            return UNEXPECTED_PARSE(ParseErrorType, EmptyString, sv.size() );
        }
        if (sv.size() < 2) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindType, sv.size() );
        }
        if (sv.starts_with("CH"sv)) {
            return { { mvmsg::Check{}, 2ull } };
        }
        if (sv.starts_with("PR"sv)) {
            return { { mvmsg::Promotion{ }, 2ull } };
        }
        if (sv.starts_with("CB"sv)) {
            const auto id_sus = from_string<Id>{}(sv.substr(2));
            if (id_sus.has_value()) {
                return { { mvmsg::CastlingBreak{ id_sus.value().value }, id_sus.value().position } };
            }
            return UNEXPECTED_PARSE(ParseErrorType, InvalidCastlingBreakId, 2ull);
        }
        return UNEXPECTED_PARSE(ParseErrorType, InvalidType, 1ull);
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
                    + as_string<Id>{}(cb.whose, meta.min_id);
            },
        );
    }
};

template <>
struct from_string<mvmsg::MainEvent> {
    enum class ParseErrorType : std::size_t {
        CouldNotFindType,
        InvalidType,
        CouldNotFindCastlindSecondToMoveId,
        InvalidCastlingSecondToMoveId,
        CouldNotFindCastlingSecondInputFrom,
        InvalidCastlingSecondInputFrom,
        CouldNotFindCastlingSecondInputTo,
        InvalidCastlingSecondInputTo,
        CouldNotFindEnPassantEatenId,
        InvalidEnPassantEatenId,
        Max = InvalidEnPassantEatenId,
    };

    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseEither<mvmsg::MainEvent, ParseErrorType>
    {
        if (sv.empty()) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindType, 0ull);
        }
        if (sv.starts_with("E"sv)) {
            const auto id_sus = from_string<Id>{}(sv.substr(1));
            if (id_sus.has_value()) {
                return { { mvmsg::Eat{ id_sus.value().value }, id_sus.value().position + 1 } };
                //                                     skip full_stop at end of the id ^^^
            }
            return UNEXPECTED_PARSE(ParseErrorType, InvalidEnPassantEatenId, 1ull);
        }
        if (sv.starts_with("M"sv)) {
            return { { mvmsg::Move{ }, 1ull } };
        }
        if (sv.starts_with("L"sv)) {
            return { { mvmsg::LongMove{ }, 1ull } };
        }
        if (sv.starts_with("C"sv)) {
            const auto id_sus = from_string<Id>{}(sv.substr(1));
            if (id_sus.has_value()) {
                if (sv.size() < id_sus.value().position + 1) {
                    return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindCastlingSecondInputFrom, id_sus.value().position + 1);
                }
                const auto from_sus = from_string<Pos>{}(sv.substr(id_sus.value().position), meta);
                if (!from_sus) {
                    return UNEXPECTED_PARSE(ParseErrorType, InvalidCastlingSecondInputFrom, id_sus.value().position);
                }
                if (sv.size() < from_sus.value().position) {
                    return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindCastlingSecondInputTo, from_sus.value().position);
                }
                const auto to_sus = from_string<Pos>{}(sv.substr(from_sus.value().position), meta);
                if (!to_sus) {
                    return UNEXPECTED_PARSE(ParseErrorType, InvalidCastlingSecondInputTo, from_sus.value().position);
                }
                return { { 
                    mvmsg::Castling{
                        id_sus.value().value,
                        Input {
                            from_sus.value().value,
                            to_sus.value().value
                        }
                    },
                    to_sus.value().position
                } };
            }
            return UNEXPECTED_PARSE(ParseErrorType, InvalidCastlingSecondToMoveId, 1ull);
        }
        if (sv.starts_with("P"sv)) {
            return { { mvmsg::EnPassant{ }, 1ull } };
        }
        return UNEXPECTED_PARSE(ParseErrorType, InvalidType, 1ull);
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
                [&](const mvmsg::EnPassant&) constexpr {
                return "P"s;
            },
        );
    }
};  

template <>
struct from_string<mvmsg::MoveMessage> {
    enum class ParseErrorType : std::size_t {
        EmptyMap,
        CouldNotFindFrom,
        InvalidFrom,
        CouldNotFindTo,
        InvalidTo,
        CouldNotFindPromotionChoice,
        InvalidPromotionChoice,
        Max = InvalidPromotionChoice,
        // Figures parse errors (Same as from_string<Figure>::ParseErrorType)
        Figure_Base,
        Figure_UnexpectedEnd = Figure_Base,
        Figure_IdDelimeterMissing,
        Figure_InvalidId,
        Figure_InvalidPos,
        Figure_InvalidColor,
        Figure_InvalidType,
        Figure_Max = Figure_InvalidType,
        // Side events parse errors (Same as from_string<SideEvent>::ParseErrorType)
        SideEvent_Base,
        SideEvent_EmptyString = SideEvent_Base,
        SideEvent_CouldNotFindType,
        SideEvent_InvalidType,
        SideEvent_InvalidCastlingBreakId,
        SideEvent_Max = SideEvent_InvalidCastlingBreakId,
        // Main events parse errors (Same as from_string<MainEvent>::ParseErrorType)
        MainEvent_Base,
        MainEvent_CouldNotFindType = MainEvent_Base,
        MainEvent_InvalidType,
        MainEvent_CouldNotFindCastlindSecondToMoveId,
        MainEvent_InvalidCastlingSecondToMoveId,
        MainEvent_CouldNotFindCastlingSecondInputFrom,
        MainEvent_InvalidCastlingSecondInputFrom,
        MainEvent_CouldNotFindCastlingSecondInputTo,
        MainEvent_InvalidCastlingSecondInputTo,
        MainEvent_CouldNotFindEnPassantEatenId,
        MainEvent_InvalidEnPassantEatenId,
        MainEvent_Max = MainEvent_InvalidEnPassantEatenId,
    };

    static_assert(
        ((std::size_t)ParseErrorType::Figure_Max - (std::size_t)ParseErrorType::Figure_Base)
        == (std::size_t)from_string<Figure>::ParseErrorType::Max
    );

    static_assert(
        ((std::size_t)ParseErrorType::SideEvent_Max - (std::size_t)ParseErrorType::SideEvent_Base)
        == (std::size_t)from_string<mvmsg::SideEvent>::ParseErrorType::Max
    );

    static_assert(
        ((std::size_t)ParseErrorType::MainEvent_Max - (std::size_t)ParseErrorType::MainEvent_Base)
        == (std::size_t)from_string<mvmsg::MainEvent>::ParseErrorType::Max
    );

    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
       -> ParseEither<mvmsg::MoveMessage, ParseErrorType>
    {
        if (true || sv.empty()) {
            return UNEXPECTED_PARSE(ParseErrorType, EmptyMap, 0ull);
        }
        const char* parse_examples[] = {
              "23.52WP5236QL"
            , "13.09BP0925QL"
            , "23.36WP3628QM"
            , "17.13BP1329QL"
            , "23.28WP2821QP17"
            , "16.14BP1421QE23"
            , "30.61WB6125QE13"
            , "4.04BQ0431QCHM"
            , "22.53WP5345QM"
            , "11.06BK0623QM"
            , "21.54WP5445QE4"
            , "4.31BQ3145QCHE22"
        };

        const auto figure_sus = from_string<Figure>{}(sv, meta);
        if (!figure_sus) {
            return std::unexpected{ ParseError<ParseErrorType>{
                static_cast<ParseErrorType>(
                    static_cast<std::size_t>(ParseErrorType::Figure_Base)
                    + static_cast<std::size_t>(figure_sus.error().type)
                ),
                figure_sus.error().position
            } };
        }
        if (sv.size() < figure_sus.value().position) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindFrom, figure_sus.value().position);
        }
        const auto from_sus = from_string<Pos>{}(sv.substr(figure_sus.value().position), meta);
        if (!from_sus) {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidFrom, from_sus.error());
        }
        if (sv.size() < from_sus.value().position) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindTo, from_sus.value().position);
        }
        const auto to_sus = from_string<Pos>{}(sv.substr(from_sus.value().position), meta);
        if (!to_sus) {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidTo, to_sus.error());
        }
        if (sv.size() < to_sus.value().position) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindPromotionChoice, to_sus.value().position);
        }
        const auto promotion_choice_sus = from_string<FigureType>{}(sv.substr(to_sus.value().position));
        if (!promotion_choice_sus) {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidPromotionChoice, to_sus.value().position);
        }
        if (sv.size() < promotion_choice_sus.value().position) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindMainEvent, promotion_choice_sus.value().position);
        }
        const auto main_event_sus = from_string<mvmsg::MainEvent>{}(sv.substr(promotion_choice_sus.value().position), meta);
        if (!main_event_sus) {
            return std::unexpected{ ParseError<ParseErrorType>{
                static_cast<ParseErrorType>(
                    static_cast<std::size_t>(ParseErrorType::MainEvent_Base)
                    + static_cast<std::size_t>(main_event_sus.error().type)
                ),
                main_event_sus.error().position
            } };
        }
        std::size_t position = main_event_sus.value().position;
        std::vector<mvmsg::SideEvent> side_events;
        while (position < sv.size()) {
            const auto side_event_sus = from_string<mvmsg::SideEvent>{}(sv.substr(position), meta);
            if (!side_event_sus) {
                return std::unexpected{ ParseError<ParseErrorType>{
                    static_cast<ParseErrorType>(
                        static_cast<std::size_t>(ParseErrorType::SideEvent_Base)
                        + static_cast<std::size_t>(side_event_sus.error().type)
                    ),
                    side_event_sus.error().position
                    }
                };
            }
            side_events.push_back(side_event_sus.value().value);
            position += side_event_sus.value().position;
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
        }, position } };
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
