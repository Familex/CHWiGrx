#pragma once

#include "macro.h"
#include "parsing.hpp"

#include <set>
#include <string>
#include <optional>

enum class GameEndType {
    Checkmate,
    FiftyRule,
    Stalemate,
    InsufficientMaterial,
    MoveRepeat,
    NotGameEnd
};

#pragma region Color

enum class Color {
    Black,
    White
};

FN col_to_char(const Color color) noexcept -> char {
    switch (color) {
        case Color::Black: return 'B';
        case Color::White: return 'W';
        default: std::unreachable();
    }
}

template <>
struct FromString<Color> {
    FN operator()(const std::string_view sv) const noexcept
        -> ParseEither<Color, ParseErrorType>
    {
        if (sv.empty()) {
            return std::unexpected{ ParseError<ParseErrorType>{ ParseErrorType::Color_CouldNotFound, 0ull } };
        }
        switch (sv[0]) {
            case 'W': case 'w':
                return { { Color::White, 1ull } };
            case 'B': case 'b':
                return { { Color::Black, 1ull } };
            default:
                return std::unexpected{ ParseError<ParseErrorType>{ ParseErrorType::Color_Invalid, 1ull } };
        }
    }
};

template <>
struct AsString<Color> {
    FN operator()(const Color color) const noexcept -> std::string
    {
        switch (color) {
            case Color::Black:
                return "B";
            case Color::White:
                return "W";
            default:
                std::unreachable();
        }
    }
};

FN what_next(const Color col) noexcept -> Color
{
    switch (col) {
        case Color::Black:
            return Color::White;
        case Color::White:
            return Color::Black;
        default:
            std::unreachable();
    }
}

inline const std::set<Color> PLAYABLE_COLORS{
    Color::White,
    Color::Black,
};

#pragma endregion   // Color

#pragma region FigureType

enum class FigureType {
    Pawn,
    Knight,
    Rook,
    Bishop,
    Queen,
    King,
};

FN figure_type_to_char(const FigureType figure_type) noexcept -> char
{
    switch (figure_type) {
        case FigureType::Pawn:
            return 'P';
        case FigureType::Knight:
            return 'H';
        case FigureType::Rook:
            return 'R';
        case FigureType::Bishop:
            return 'B';
        case FigureType::Queen:
            return 'Q';
        case FigureType::King:
            return 'K';
        default:
            std::unreachable();
    }
}

template <>
struct FromString<FigureType> {
    FN operator()(const std::string_view sv) const noexcept
        -> ParseEither<FigureType, ParseErrorType>
    {
        if (sv.empty()) {
            return std::unexpected { ParseError<ParseErrorType>{ ParseErrorType::FigureType_CouldNotFound, 0ull } };
        }
        switch (sv[0]) {
            case 'K': case 'k':
                return { { FigureType::King, 1ull } };
            case 'H': case 'h':
                return { { FigureType::Knight, 1ull } };
            case 'P': case 'p':
                return { { FigureType::Pawn, 1ull } };
            case 'B': case 'b':
                return { { FigureType::Bishop, 1ull } };
            case 'Q': case 'q':
                return { { FigureType::Queen, 1ull } };
            case 'R': case 'r':
                return { { FigureType::Rook, 1ull } };
            default:
                return std::unexpected{ ParseError<ParseErrorType>{ ParseErrorType::FigureType_Invalid, 1ull } };
        }
    }
};

template <>
struct AsString<FigureType> {
    FN operator()(const FigureType figure_type) const noexcept -> std::string
    {
        switch (figure_type) {
            case FigureType::Pawn:
                return "P";
            case FigureType::Rook:
                return "R";
            case FigureType::Knight:
                return "H";
            case FigureType::Bishop:
                return "B";
            case FigureType::Queen:
                return "Q";
            case FigureType::King:
                return "K";
        }
        return "?";
    }
};

inline const std::set<FigureType> PLAYABLE_FIGURES {
    FigureType::Pawn,
    FigureType::Knight,
    FigureType::Rook,
    FigureType::Bishop,
    FigureType::Queen,
    FigureType::King,
};

inline const std::set<FigureType> PROMOTION_FIGURES {
    FigureType::Knight,
    FigureType::Rook,
    FigureType::Bishop,
    FigureType::Queen,
};

#pragma endregion   // FigureType

#pragma region Events

enum class ErrorEvent {
    InvalidMove,
    UnderCheck,
    Unforeseen
};

#pragma endregion   // Events
