#pragma once

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

#pragma endregion   // Parse error types

#pragma region from_string, as_string declarations
/// implement operator() for this type_ with your type_ as template parameter
template <typename ResultType>
struct FromString;

/// implement operator() for this type_ with your type_ as template parameter
template <typename ResultType>
struct AsString;

#pragma endregion   // FromString, AsString declarations

enum class ParseErrorType {
    // General parse errors
    General_EmptyString,
    // Meta parse errors
    Meta_CouldNotFindMeta,
    Meta_InvalidVersion,
    Meta_UnsupportedVersion,
    Meta_CouldNotFindHeight,
    Meta_InvalidHeight,
    Meta_CouldNotFindWidth,
    Meta_InvalidWidth,
    Meta_CouldNotFindIdw,
    Meta_CouldNotFindCurrentTurn,
    Meta_InvalidCurrentTurn,
    Meta_CouldNotFindCastlings,
    Meta_InvalidCastling,
    // Id parse errors
    Id_Invalid,
    // Pos parse errors
    Pos_Invalid,
    // Color parse errors
    Color_CouldNotFound,
    Color_Invalid,
    // FigureType parse errors
    FigureType_CouldNotFound,
    FigureType_Invalid,
    // Figures parse errors
    Figure_CouldNotFindId,
    Figure_InvalidId,
    Figure_CouldNotFindPos,
    Figure_InvalidPos,
    Figure_CouldNotFindColor,
    Figure_InvalidColor,
    Figure_CouldNotFindType,
    Figure_InvalidType,
    // MoveMessage parse error
    MoveMessage_EmptyMap,
    MoveMessage_CouldNotFindTo,
    MoveMessage_InvalidTo,
    MoveMessage_CouldNotFindPromotionChoice,
    MoveMessage_InvalidPromotionChoice,
    MoveMessage_InvalidEnPassantToEatId,
    MoveMessage_CouldNotFindMainEvent,
    // Side events parse errors
    SideEvent_EmptyString,
    SideEvent_CouldNotFindType,
    SideEvent_InvalidType,
    SideEvent_CouldNotFindCastlingBreakId,
    SideEvent_InvalidCastlingBreakId,
    // Main events parse errors
    MainEvent_CouldNotFindType,
    MainEvent_InvalidType,
    MainEvent_CouldNotFindCastlingSecondToMoveId,
    MainEvent_InvalidCastlingSecondToMoveId,
    MainEvent_CouldNotFindCastlingSecondInputFrom,
    MainEvent_InvalidCastlingSecondInputFrom,
    MainEvent_CouldNotFindCastlingSecondInputTo,
    MainEvent_InvalidCastlingSecondInputTo,
    MainEvent_CouldNotFindEnPassantEatenId,
    MainEvent_InvalidEnPassantEatenId,
};

#define PARSE_ERROR_TYPE_AS_T_STRING_IMPL(name, liter) \
    inline auto name(ParseErrorType error_type) { \
        switch (error_type) \
        { \
            case ParseErrorType::General_EmptyString: return liter( "General: Empty string" ); \
            case ParseErrorType::Meta_CouldNotFindMeta:  return liter( "Meta: Could not find meta" ); \
            case ParseErrorType::Meta_InvalidVersion: return liter( "Meta: Invalid version" ); \
            case ParseErrorType::Meta_UnsupportedVersion: return liter( "Meta: Unsupported version" ); \
            case ParseErrorType::Meta_CouldNotFindHeight: return liter( "Meta: Could not find height" ); \
            case ParseErrorType::Meta_InvalidHeight: return liter( "Meta: Invalid height" ); \
            case ParseErrorType::Meta_CouldNotFindWidth: return liter( "Meta: Could not find width" ); \
            case ParseErrorType::Meta_InvalidWidth: return liter( "Meta: Invalid width" ); \
            case ParseErrorType::Meta_CouldNotFindIdw: return liter( "Meta: Could not find idw" ); \
            case ParseErrorType::Meta_CouldNotFindCurrentTurn: return liter( "Meta: Could not find current turn" ); \
            case ParseErrorType::Meta_InvalidCurrentTurn: return liter( "Meta: Invalid current turn" ); \
            case ParseErrorType::Meta_CouldNotFindCastlings: return liter( "Meta: Could not find castlings" ); \
            case ParseErrorType::Meta_InvalidCastling: return liter( "Meta: Invalid castling" ); \
            case ParseErrorType::Figure_CouldNotFindId: return liter( "Figure: Could not find id" ); \
            case ParseErrorType::Figure_InvalidId: return liter( "Figure: Invalid id" ); \
            case ParseErrorType::Figure_InvalidPos: return liter( "Figure: Invalid pos" ); \
            case ParseErrorType::Figure_InvalidColor: return liter( "Figure: Invalid color" ); \
            case ParseErrorType::Figure_InvalidType: return liter( "Figure: Invalid type" ); \
            case ParseErrorType::MoveMessage_EmptyMap: return liter( "MoveMessage: Empty map" ); \
            case ParseErrorType::MoveMessage_CouldNotFindTo: return liter( "MoveMessage: Could not find to" ); \
            case ParseErrorType::MoveMessage_InvalidTo: return liter( "MoveMessage: Invalid to" ); \
            case ParseErrorType::MoveMessage_CouldNotFindPromotionChoice: return liter( "MoveMessage: Could not find promotion choice" ); \
            case ParseErrorType::MoveMessage_InvalidEnPassantToEatId: return liter( "MoveMessage: Invalid en passant to eat id" ); \
            case ParseErrorType::MoveMessage_InvalidPromotionChoice: return liter( "MoveMessage: Invalid promitoin choice" ); \
            case ParseErrorType::MoveMessage_CouldNotFindMainEvent: return liter( "MoveMessage: Could not find main event" ); \
            case ParseErrorType::SideEvent_EmptyString: return liter( "SideEvent: Empty string" ); \
            case ParseErrorType::SideEvent_CouldNotFindType: return liter( "SideEvent: Could not find type" ); \
            case ParseErrorType::SideEvent_InvalidType: return liter( "SideEvent: Invalid type" ); \
            case ParseErrorType::SideEvent_CouldNotFindCastlingBreakId: return liter( "SideEvent: Could not find castling break id" ); \
            case ParseErrorType::SideEvent_InvalidCastlingBreakId: return liter( "SideEvent: Invalid castling break id" ); \
            case ParseErrorType::MainEvent_CouldNotFindType: return liter( "MainEvent: Could not find type" ); \
            case ParseErrorType::MainEvent_InvalidType: return liter( "MainEvent: Invalid type" ); \
            case ParseErrorType::MainEvent_CouldNotFindCastlingSecondToMoveId: return liter( "MainEvent: Could not find castling second to move id" ); \
            case ParseErrorType::MainEvent_InvalidCastlingSecondToMoveId: return liter( "MainEvent: Invalid castling second to move id" ); \
            case ParseErrorType::MainEvent_CouldNotFindCastlingSecondInputFrom: return liter( "MainEvent: Could not find castling second input from" ); \
            case ParseErrorType::MainEvent_InvalidCastlingSecondInputFrom: return liter( "MainEvent: Invalid castling second input from" ); \
            case ParseErrorType::MainEvent_CouldNotFindCastlingSecondInputTo: return liter( "MainEvent: Could not find castling second input to" ); \
            case ParseErrorType::MainEvent_InvalidCastlingSecondInputTo: return liter( "MainEvent: Invalind casling second input to" ); \
            case ParseErrorType::MainEvent_CouldNotFindEnPassantEatenId: return liter( "MainEvent: Could not find en passant eaten id" ); \
            case ParseErrorType::MainEvent_InvalidEnPassantEatenId: return liter( "MainEvent: Invalid en passant eaten id" ); \
            default: return liter( "Unknown error" ); \
        } \
    }

#define ADD_L(str) L ## str
#define ADD_NO(str) str

PARSE_ERROR_TYPE_AS_T_STRING_IMPL(parse_error_type_as_wstring, ADD_L)
PARSE_ERROR_TYPE_AS_T_STRING_IMPL(parse_error_type_as_string, ADD_NO)

#undef ADD_L
#undef ADD_NO
#undef PARSE_ERROR_TYPE_AS_T_STRING_IMPL

/// For external usage
#define PARSE_STEP_UNEXPECTED(error_type, error_value, position) \
    std::unexpected{ \
        ParseError<error_type>{ \
            error_type :: error_value, \
            position \
        } \
    }
