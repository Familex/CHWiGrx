#pragma once

#include "macro.h"
#include "parse_typedefs.hpp"

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
struct from_string<Color> {
    [[nodiscard]] inline auto
        operator()(const std::string_view sv) const noexcept
        -> std::optional<Color>
    {
        if (sv.empty()) {
            return std::nullopt;
        }
        switch (sv[0]) {
            case 'W': case 'w':
                return Color::White;
            case 'B': case 'b':
                return Color::Black;
            default:
                return std::nullopt;
        }
    }
};

template <>
struct as_string<Color> {
    [[nodiscard]] inline auto
        operator()(const Color color) const noexcept -> std::string
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
struct from_string<FigureType> {
    [[nodiscard]] inline auto
        operator()(const std::string_view sv) const noexcept
        -> std::optional<FigureType>
    {
        if (sv.empty()) {
            return std::nullopt;
        }
        switch (sv[0]) {
            case 'K': case 'k':
                return FigureType::King;
            case 'H': case 'h':
                return FigureType::Knight;
            case 'P': case 'p':
                return FigureType::Pawn;
            case 'B': case 'b':
                return FigureType::Bishop;
            case 'Q': case 'q':
                return FigureType::Queen;
            case 'R': case 'r':
                return FigureType::Rook;
            default:
                return std::nullopt;
        }
    }
};

template <>
struct as_string<FigureType> {
    [[nodiscard]] inline auto
        operator()(const FigureType figure_type) const noexcept -> std::string
    {
        switch (figure_type) {
            case FigureType::Pawn:
                return "P";
            case FigureType::Rook:
                return "R";
            case FigureType::Knight:
                return "K";
            case FigureType::Bishop:
                return "B";
            case FigureType::Queen:
                return "Q";
            case FigureType::King:
                return "K";
            default:
                std::unreachable();
        }
    }
};

inline const std::set<FigureType> PLAYABLE_FIGURES{
    FigureType::Pawn,
    FigureType::Knight,
    FigureType::Rook,
    FigureType::Bishop,
    FigureType::Queen,
    FigureType::King,
};

inline const std::set<FigureType> PROMOTION_FIGURES{
    FigureType::Knight,
    FigureType::Rook,
    FigureType::Bishop,
    FigureType::Queen,
};

#pragma endregion   // FigureType

#pragma region Events

enum class ErrorEvent {
    INVALID_MOVE,
    UNDER_CHECK,
    CHECK_IN_THAT_TILE,
    UNFORESEEN
};

#pragma endregion   // Events
