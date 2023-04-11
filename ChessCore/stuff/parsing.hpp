#pragma once

#include "macro.h"

#include <cstddef>
#include <expected>

#pragma region Parse error types

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

/// Use this macro to return an parse error
#define UNEXPECTED_PARSE(type, pos) \
    std::unexpected{ ParseError<ParseErrorType>{ ParseErrorType :: type, pos } }

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
    // Figures parse errors
    Figure_Base,
    Figure_UnexpectedEnd = Figure_Base,
    Figure_IdDelimeterMissing,
    Figure_InvalidId,
    Figure_InvalidPos,
    Figure_InvalidColor,
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
