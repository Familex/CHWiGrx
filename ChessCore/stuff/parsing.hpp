#pragma once

#include "macro.h"

#include <cstddef>
#include <expected>

#pragma region Parse error types

#define UNEXPECTED_PARSE(type, pos) \
    std::unexpected{ ParseError<ParseErrorType>{ ParseErrorType :: type, pos } }

template <typename ErrorType>
struct ParseError {
    ErrorType type;
    std::size_t position;
};

template <typename ResultType>
struct ParseResult {
    ResultType value;
    std::size_t position;
};

template <typename ResultType, typename ErrorType>
using ParseEither = std::expected<ParseResult<ResultType>, ParseError<ErrorType>>;

#pragma endregion   // Parse error types

#pragma region from_string, as_string declarations
/// implement operator() for this type with your type as template parameter
template <typename ResultType>
struct from_string;

/// implement operator() for this type with your type as template parameter
template <typename ResultType>
struct as_string;

#pragma endregion   // from_string, as_string declarations

enum class ParseErrorType {
    // General parse errors
    General_Base,
    General_EmptyString = General_Base,
    General_Max = General_EmptyString,
    // Meta parse errors
    Meta_Base,
    Meta_CouldNotFindMeta = Meta_Base,
    Meta_InvalidVersion,
    Meta_UnsupportedVersion,
    Meta_CouldNotFindHeight,
    Meta_InvalidHeight,
    Meta_CouldNotFindWidth,
    Meta_InvalidWidth,
    Meta_CouldNotFindIDW,
    Meta_CouldNotFindCurrentTurn,
    Meta_InvalidCurrentTurn,
    Meta_CouldNotFindCastlings,
    Meta_InvalidCastling,
    Meta_Max = Meta_InvalidCastling,
    // Id parse errors
    Id_Base,
    Id_Invalid = Id_Base,
    Id_Max = Id_Invalid,
    // Pos parse errors
    Pos_Base,
    Pos_Invalid = Pos_Base,
    Pos_Max = Pos_Invalid,
    // Color parse errors
    Color_Base,
    Color_CouldNotFound = Color_Base,
    Color_Invalid,
    Color_Max = Color_Invalid,
    // FigureType parse errors
    FigureType_Base,
    FigureType_CouldNotFound = FigureType_Base,
    FigureType_Invalid,
    FigureType_Max = FigureType_Invalid,
    // Figures parse errors
    Figure_Base,
    Figure_CouldNotFindId = Figure_Base,
    Figure_InvalidId,
    Figure_CouldNotFindPos,
    Figure_InvalidPos,
    Figure_CouldNotFindColor,
    Figure_InvalidColor,
    Figure_CouldNotFindType,
    Figure_InvalidType,
    Figure_Max = Figure_InvalidType,
    // MoveMessage parse error
    MoveMessage_Base,
    MoveMessage_EmptyMap = MoveMessage_Base,
    MoveMessage_CouldNotFindTo,
    MoveMessage_InvalidTo,
    MoveMessage_CouldNotFindPromotionChoice,
    MoveMessage_InvalidPromotionChoice,
    MoveMessage_InvalidEnPassantToEatId,
    MoveMessage_CouldNotFindMainEvent,
    MoveMessage_Max = MoveMessage_CouldNotFindMainEvent,
    // Side events parse errors
    SideEvent_Base,
    SideEvent_EmptyString = SideEvent_Base,
    SideEvent_CouldNotFindType,
    SideEvent_InvalidType,
    SideEvent_InvalidCastlingBreakId,
    SideEvent_Max = SideEvent_InvalidCastlingBreakId,
    // Main events parse errors
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

#pragma region Parse step types

#define PARSE_UNEXPECTED_END_GUARD_BASE(sv, parse_error_type, error_type, position) \
    if (sv.size() < curr_pos) { \
        return std::unexpected{ \
            ParseError<parse_error_type>{ \
                error_type, \
                position \
            } \
        }; \
    }

#define PARSE_UNEXPECTED_END_GUARD(sv, error_value, position) \
    PARSE_UNEXPECTED_END_GUARD_BASE(sv, ParseErrorType, ParseErrorType :: error_value, position)

#define PARSE_STEP_MAKE_UNEXPECTED(parse_error_type, error_type, position) \
    std::unexpected{ \
        ParseError<parse_error_type>{ \
            error_type, \
            position \
        } \
    }

#define PARSE_STEP_PACK(...) \
    __VA_ARGS__

#define PARSE_STEP_BASE(from_string_arg, error_type, error_position, value_name, type_name, curr_pos, parse_error_type, extra_pos) \
    const auto value_name ## _sus = from_string<type_name>{}(from_string_arg); \
    if (value_name ## _sus) { \
        curr_pos += value_name ## _sus->position + extra_pos; \
    } \
    else { \
        return PARSE_STEP_MAKE_UNEXPECTED( parse_error_type, error_type, error_position ); \
    } \
    const auto value_name = value_name ## _sus->value;

#define PARSE_STEP_FORWARD_WITH_META_EX(sv, value_name, type_name, curr_pos, meta, parse_error_type, extra_pos) \
    PARSE_STEP_BASE( \
          PARSE_STEP_PACK( sv.substr(curr_pos), meta ), \
          value_name ## _sus.error().type, curr_pos + value_name ## _sus.error().position, \
          value_name, type_name, curr_pos, parse_error_type, extra_pos )

#define PARSE_STEP_FORWARD_WITH_META(sv, value_name, type_name, curr_pos, meta, parse_error_type) \
    PARSE_STEP_FORWARD_WITH_META_EX(sv, value_name, type_name, curr_pos, meta, parse_error_type, 0)

#define PARSE_STEP_FORWARD_EX(sv, value_name, type_name, curr_pos, parse_error_type, extra_pos) \
    PARSE_STEP_BASE( \
          PARSE_STEP_PACK( sv.substr(curr_pos) ), \
          value_name ## _sus.error().type, curr_pos + value_name ## _sus.error().position, \
          value_name, type_name, curr_pos, parse_error_type, extra_pos )

#define PARSE_STEP_FORWARD(sv, value_name, type_name, curr_pos, parse_error_type) \
    PARSE_STEP_FORWARD_EX(sv, value_name, type_name, curr_pos, parse_error_type, 0)

#define PARSE_STEP_WITHOUT_SUBSTR_WITH_META_EX(sv, value_name, type_name, curr_pos, meta, parse_error_type, error_value, extra_pos) \
    PARSE_STEP_BASE( \
          PARSE_STEP_PACK( sv, meta ), \
          parse_error_type :: error_value, curr_pos + value_name ## _sus.error().position, \
          value_name, type_name, curr_pos, parse_error_type, extra_pos )

#define PARSE_STEP_WITHOUT_SUBSTR_WITH_META(sv, value_name, type_name, curr_pos, meta, parse_error_type, error_value) \
    PARSE_STEP_WITHOUT_SUBSTR_WITH_META_EX(sv, value_name, type_name, curr_pos, meta, parse_error_type, error_value, 0)

#define PARSE_STEP_EX(sv, value_name, type_name, curr_pos, parse_error_type, error_value, extra_pos) \
    PARSE_STEP_BASE( \
          PARSE_STEP_PACK( sv.substr(curr_pos) ), \
          parse_error_type :: error_value, curr_pos + value_name ## _sus.error().position, \
          value_name, type_name, curr_pos, parse_error_type, extra_pos )

#define PARSE_STEP(sv, value_name, type_name, curr_pos, parse_error_type, error_value) \
    PARSE_STEP_EX(sv, value_name, type_name, curr_pos, parse_error_type, error_value, 0)