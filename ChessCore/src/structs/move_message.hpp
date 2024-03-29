#pragma once

#include "../stuff/stuff.hpp"
#include "../stuff/visit.hpp"
#include "figure.hpp"
#include "figure_fabric.hpp"
#include "input.hpp"

#include <expected>
#include <variant>
#include <vector>

namespace mvmsg /* move_message */
{
// Side Event
struct Check
{ };

// Side Event
struct Promotion
{ };

// Side Event
struct CastlingBreak
{
    Id whose;
};

using SideEvent = std::variant<Check, Promotion, CastlingBreak>;

// Main Event
struct Eat
{
    Id eaten;
};

// Main Event
struct Move
{ };

// Main Event
struct LongMove
{ };

// Main Event
struct Castling
{
    Id second_to_move;
    Input second_input;
};

// Main Event
struct EnPassant
{
    Id eaten;
};

using MainEvent = std::variant<Eat, Move, LongMove, Castling, EnPassant>;

struct MoveMessage
{
    Figure first;
    Input input;
    FigureType promotion_choice;
    MainEvent main_event;
    std::vector<SideEvent> side_evs;

    friend FromString<MoveMessage>;
    friend AsString<MoveMessage>;

    CTOR MoveMessage(
        const Figure& first,
        const Input& input,
        const FigureType p,
        const MainEvent& main_event,
        std::vector<SideEvent> side_evs
    ) noexcept
      : first{ first }
      , input{ input }
      , promotion_choice{ p }
      , main_event{ main_event }
      , side_evs{ std::move(side_evs) }
    { }
};
}    // namespace mvmsg

template <>
struct FromString<mvmsg::SideEvent>
{
    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseEither<mvmsg::SideEvent, ParseErrorType>
    {
        if (sv.empty()) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, SideEvent_EmptyString, sv.size());
        }
        if (sv.size() < 2) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, SideEvent_CouldNotFindType, sv.size());
        }
        if (sv.starts_with("CH"sv)) {
            return { { mvmsg::Check{}, 2ull } };
        }
        if (sv.starts_with("PR"sv)) {
            return { { mvmsg::Promotion{}, 2ull } };
        }
        if (sv.starts_with("CB"sv)) {
            using parse_step::execute_sequence, parse_step::ParseStepBuilder;
            using enum ParseErrorType;

            constexpr std::size_t curr_pos{ 2 };
            return execute_sequence(
                2ull,
                sv,
                meta,
                [](const Id id) { return mvmsg::SideEvent{ mvmsg::CastlingBreak{ id } }; },
                ParseStepBuilder<Id>{}
                    .extra(1)
                    .error(SideEvent_InvalidCastlingBreakId)
                    .on_abrupt_halt(SideEvent_CouldNotFindCastlingBreakId)
            );
        }
        return PARSE_STEP_UNEXPECTED(ParseErrorType, SideEvent_InvalidType, 1ull);
    }
};

template <>
struct AsString<mvmsg::SideEvent>
{
    FN
    operator()(const mvmsg::SideEvent& side_event, const AsStringMeta& meta) const noexcept -> std::string
    {
        return visit_one(
            side_event,
            [&](const mvmsg::Check&) { return "CH"s; },
            [&](const mvmsg::Promotion&) { return "PR"s; },
            [&](const mvmsg::CastlingBreak& cb) { return "CB"s + AsString<Id>{}(cb.whose, meta.min_id) + "."s; }
        );
    }
};

template <>
struct FromString<mvmsg::MainEvent>
{

    template <typename StepResult>
    using StepB = parse_step::ParseStepBuilder<StepResult>;

    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseEither<mvmsg::MainEvent, ParseErrorType>
    {
        if (sv.empty()) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, MainEvent_CouldNotFindType, sv.size());
        }

        std::size_t curr_pos{ 0 };

        if (sv.starts_with("E"sv)) {
            curr_pos += 1;
            if (const auto id_sus = FromString<Id>{}(sv.substr(curr_pos)); id_sus.has_value()) {
                return { { mvmsg::Eat{ id_sus->value }, curr_pos + id_sus->position + 1 } };
                //                                                skip full_stop at end of the id_ ^^^
            }
            return PARSE_STEP_UNEXPECTED(ParseErrorType, MainEvent_InvalidEnPassantEatenId, curr_pos);
        }
        if (sv.starts_with("M"sv)) {
            curr_pos += 1;
            return { { mvmsg::Move{}, curr_pos } };
        }
        if (sv.starts_with("L"sv)) {
            curr_pos += 1;
            return { { mvmsg::LongMove{}, curr_pos } };
        }
        if (sv.starts_with("C"sv)) {
            using parse_step::execute_sequence, parse_step::ParseStepBuilder;
            using enum ParseErrorType;

            return execute_sequence(
                ++curr_pos,
                sv,
                meta,
                [](const Id id, const Pos from, const Pos to) {
                    return mvmsg::MainEvent{ mvmsg::Castling{ id, Input{ from, to } } };
                },
                StepB<Id>{}
                    .error(MainEvent_InvalidCastlingSecondToMoveId)
                    .on_abrupt_halt(MainEvent_CouldNotFindCastlingSecondToMoveId)
                    .extra(1),
                StepB<Pos>{}
                    .error(MainEvent_CouldNotFindCastlingSecondInputFrom)
                    .on_abrupt_halt(MainEvent_InvalidCastlingSecondInputFrom)
                    .max_length(meta.max_pos_length),
                StepB<Pos>{}
                    .error(MainEvent_CouldNotFindCastlingSecondInputTo)
                    .on_abrupt_halt(MainEvent_InvalidCastlingSecondInputTo)
                    .max_length(meta.max_pos_length)
            );
        }
        if (sv.starts_with("P"sv)) {
            curr_pos += 1;
            if (const auto id_sus = FromString<Id>{}(sv.substr(curr_pos)); id_sus.has_value()) {
                return { { mvmsg::EnPassant{ id_sus->value }, curr_pos + id_sus->position + 1 } };
                //                                                      skip full_stop at end of the id_ ^^^
            }
            return PARSE_STEP_UNEXPECTED(ParseErrorType, MoveMessage_InvalidEnPassantToEatId, curr_pos);
        }
        return PARSE_STEP_UNEXPECTED(ParseErrorType, MainEvent_InvalidType, 1ull);
    }
};

template <>
struct AsString<mvmsg::MainEvent>
{
    FN
    operator()(const mvmsg::MainEvent& main_event, const AsStringMeta& meta) const noexcept -> std::string
    {
        return visit_one(
            main_event,
            [&](const mvmsg::Eat& eat) constexpr { return "E"s + AsString<Id>{}(eat.eaten, meta.min_id) + '.'; },
            [&](const mvmsg::Move&) constexpr { return "M"s; },
            [&](const mvmsg::LongMove&) constexpr { return "L"s; },
            [&](const mvmsg::Castling& castling) {
                return "C"s + AsString<Id>{}(castling.second_to_move, meta.min_id) + '.' +
                       AsString<Pos>{}(castling.second_input.from, meta) +
                       AsString<Pos>{}(castling.second_input.target, meta);
            },
            [&](const mvmsg::EnPassant& en_passant) constexpr {
                return "P"s + AsString<Id>{}(en_passant.eaten, meta.min_id) + "."s;
            }
        );
    }
};

template <>
struct FromString<mvmsg::MoveMessage>
{
    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseEither<mvmsg::MoveMessage, ParseErrorType>
    {
        if (sv.empty()) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, MoveMessage_EmptyMap, sv.size());
        }
        std::size_t curr_pos{};

        const auto figure_sus = FromString<Figure>{}(sv, meta);
        if (!figure_sus) {
            return std::unexpected{ ParseError<ParseErrorType>{ figure_sus.error().type,
                                                                figure_sus.error().position } };
        }
        else {
            curr_pos += figure_sus.value().position;
        }
        if (sv.size() < curr_pos) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, MoveMessage_CouldNotFindTo, curr_pos);
        }
        const auto to_sus = FromString<Pos>{}(sv.substr(curr_pos, meta.max_pos_length), meta);
        if (!to_sus) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, MoveMessage_InvalidTo, curr_pos + to_sus.error().position);
        }
        else {
            curr_pos += to_sus.value().position;
        }
        if (sv.size() < curr_pos) {
            return PARSE_STEP_UNEXPECTED(
                ParseErrorType, MoveMessage_CouldNotFindPromotionChoice, curr_pos + to_sus.value().position
            );
        }
        const auto promotion_choice_sus = FromString<FigureType>{}(sv.substr(curr_pos));
        if (!promotion_choice_sus) {
            return PARSE_STEP_UNEXPECTED(
                ParseErrorType, MoveMessage_InvalidPromotionChoice, curr_pos + to_sus.value().position
            );
        }
        else {
            curr_pos += promotion_choice_sus.value().position;
        }
        if (sv.size() < curr_pos) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, MoveMessage_CouldNotFindMainEvent, curr_pos);
        }
        const auto main_event_sus = FromString<mvmsg::MainEvent>{}(sv.substr(curr_pos), meta);
        if (!main_event_sus) {
            return std::unexpected{ ParseError<ParseErrorType>{ main_event_sus.error().type,
                                                                curr_pos + main_event_sus.error().position } };
        }
        else {
            curr_pos += main_event_sus.value().position;
        }
        std::vector<mvmsg::SideEvent> side_events;
        while (curr_pos < sv.size()) {
            const auto side_event_sus = FromString<mvmsg::SideEvent>{}(sv.substr(curr_pos), meta);
            if (!side_event_sus) {
                return std::unexpected{ ParseError<ParseErrorType>{ side_event_sus.error().type,
                                                                    curr_pos + side_event_sus.error().position } };
            }
            side_events.push_back(side_event_sus.value().value);
            curr_pos += side_event_sus.value().position;
        }
        return { { mvmsg::MoveMessage{
                       figure_sus.value().value,
                       Input{ figure_sus.value().value.get_pos(), to_sus.value().value },
                       promotion_choice_sus.value().value,
                       main_event_sus.value().value,
                       side_events,
                   },
                   curr_pos } };
        //          ^^^^^^^^ looks useless cause of while loop
    }
};

template <>
struct AsString<mvmsg::MoveMessage>
{
    FN
    operator()(const mvmsg::MoveMessage& move_message, const AsStringMeta& meta) const noexcept -> std::string
    {
        std::string result{ AsString<Figure>{}(move_message.first, meta) +
                            AsString<Pos>{}(move_message.input.target, meta) +
                            AsString<FigureType>{}(move_message.promotion_choice) +
                            AsString<mvmsg::MainEvent>{}(move_message.main_event, meta) };

        for (const auto& side_event : move_message.side_evs) {
            result += AsString<mvmsg::SideEvent>{}(side_event, meta);
        }

        return result;
    }
};
