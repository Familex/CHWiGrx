#pragma once

#include "macro.h"

#include <set>
#include <string>

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

FN char_to_col(const char ch) noexcept -> Color
{
    switch (ch) {
        case 'W': case 'w':
            return Color::White;
        case 'B': case 'b':
            return Color::Black;
        default:
            std::unreachable();
    }
}

FN col_to_char(const Color col) noexcept -> char
{
    switch (col) {
        case Color::Black:
            return 'B';
        case Color::White:
            return 'W';
        default:
            std::unreachable();
    }
}

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

FN char_to_figure_type(const char ch) noexcept -> FigureType
{
    switch (ch) {
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
            std::unreachable();
    }
}

FN figure_type_to_char(const FigureType ft) noexcept -> char
{
    switch (ft) {
        case FigureType::Pawn:
            return 'P';
        case FigureType::Rook:
            return 'R';
        case FigureType::Knight:
            return 'H';
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

FN as_string(FigureType figure_type) noexcept -> std::string {
    switch (figure_type) {
        case FigureType::Pawn:
            return "Pawn";
        case FigureType::Rook:
            return "Rook";
        case FigureType::Knight:
            return "Knight";
        case FigureType::Bishop:
            return "Bishop";
        case FigureType::Queen:
            return "Queen";
        case FigureType::King:
            return "King";
        default:
            std::unreachable();
    }
}

FN as_wstring(FigureType figure_type) noexcept -> std::wstring {
    switch (figure_type) {
        case FigureType::Pawn:
            return L"Pawn";
        case FigureType::Rook:
            return L"Rook";
        case FigureType::Knight:
            return L"Knight";
        case FigureType::Bishop:
            return L"Bishop";
        case FigureType::Queen:
            return L"Queen";
        case FigureType::King:
            return L"King";
        default:
            std::unreachable();
    }
}

#pragma endregion   // FigureType

#pragma region Events

enum class ErrorEvent {
    INVALID_MOVE,
    UNDER_CHECK,
    CHECK_IN_THAT_TILE,
    UNFORESEEN
};

enum class MainEvent {
    EAT,
    MOVE,
    LMOVE,
    CASTLING,
    EN_PASSANT
};

enum class SideEvent {
    CHECK,
    PROMOTION,
    CASTLING_BREAK
};

FN as_string(SideEvent side_event) noexcept -> std::string {
    switch (side_event) {
        case SideEvent::CHECK:
            return "C";
        case SideEvent::PROMOTION:
            return "P";
        case SideEvent::CASTLING_BREAK:
            return "B";
        default:
            std::unreachable();
    }
}

FN as_wstring(SideEvent side_event) noexcept -> std::wstring {
    switch (side_event) {
        case SideEvent::CHECK:
            return L"C";
        case SideEvent::PROMOTION:
            return L"P";
        case SideEvent::CASTLING_BREAK:
            return L"B";
        default:
            std::unreachable();
    }
}

FN as_string(MainEvent main_event) noexcept -> std::string {
    switch (main_event) {
        case MainEvent::EAT:
            return "T";
        case MainEvent::MOVE:
            return "M";
        case MainEvent::LMOVE:
            return "L";
        case MainEvent::CASTLING:
            return "C";
        case MainEvent::EN_PASSANT:
            return "P";
        default:
            std::unreachable();
    }
}

FN as_wstring(MainEvent main_event) noexcept -> std::wstring {
    switch (main_event) {
        case MainEvent::EAT:
            return L"T";
        case MainEvent::MOVE:
            return L"M";
        case MainEvent::LMOVE:
            return L"L";
        case MainEvent::CASTLING:
            return L"C";
        case MainEvent::EN_PASSANT:
            return L"P";
        default:
            std::unreachable();
    }
}

#pragma endregion   // Events

