#pragma once

#include "macro.h"

#include <cassert>
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
    None,
    Black,
    White
};

FN char_to_col(const char ch) noexcept -> Color
{
    switch (ch) {
    case 'W': case 'w': return Color::White;
    case 'B': case 'b': return Color::Black;
    case 'N': case 'n': return Color::None;
    }
    assert(!"char_to_col: invalid character");
    return Color::None;
}

FN col_to_char(const Color col) noexcept -> char
{
    switch (col) {
    case Color::Black: return 'B';
    case Color::White: return 'W';
    case Color::None: return 'N';
    }
    assert(!"col_to_char: invalid color");
    return 'E';
}

FN what_next(const Color col) noexcept -> Color
{
    switch (col) {
    case Color::Black:
        return Color::White;
    case Color::White:
        return Color::Black;
    }
    assert(!"what_next: invalid color");
    return Color::None; // Perhaps there is a better way
}

inline const std::set<Color> PLAYABLE_COLORS{
    Color::White,
    Color::Black,
};

#pragma endregion

#pragma region FigureType

enum class FigureType {
    None,
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
        case 'K': case 'k': return FigureType::King;
        case 'H': case 'h': return FigureType::Knight;
        case 'P': case 'p': return FigureType::Pawn;
        case 'B': case 'b': return FigureType::Bishop;
        case 'Q': case 'q': return FigureType::Queen;
        case 'R': case 'r': return FigureType::Rook;
        case 'N': case 'n': return FigureType::None;
        default:
            assert(!"char_to_figure_type: invalid character");
            return FigureType::None;
    }
}

FN figure_type_to_char(const FigureType ft) noexcept -> char
{
    switch (ft) {
        case FigureType::Pawn: return 'P';
        case FigureType::Rook: return 'R';
        case FigureType::Knight: return 'H';
        case FigureType::Bishop: return 'B';
        case FigureType::Queen: return 'Q';
        case FigureType::King: return 'K';
        case FigureType::None: return 'N';
        default:
            assert(!"figure_type_to_char: invalid figure type");
            return 'E';
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
        case FigureType::None:
            return "None";
        default:
            return "Error";
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
        case FigureType::None:
            return L"None";
        default:
            return L"Error";
    }
}

#pragma endregion

#pragma region Events

enum class ErrorEvent { INVALID_MOVE, UNDER_CHECK, CHECK_IN_THAT_TILE, UNFORESEEN };
enum class MainEvent { E, EAT, MOVE, LMOVE, CASTLING, EN_PASSANT };
enum class SideEvent { E, CHECK, PROMOTION, CASTLING_BREAK };

FN as_string(SideEvent side_event) noexcept -> std::string {
    switch (side_event) {
        case SideEvent::E:
            return "E";
        case SideEvent::CHECK:
            return "C";
        case SideEvent::PROMOTION:
            return "P";
        case SideEvent::CASTLING_BREAK:
            return "B";
        default:
            return "N";
    }
}

FN as_wstring(SideEvent side_event) noexcept -> std::wstring {
    switch (side_event) {
        case SideEvent::E:
            return L"E";
        case SideEvent::CHECK:
            return L"C";
        case SideEvent::PROMOTION:
            return L"P";
        case SideEvent::CASTLING_BREAK:
            return L"B";
        default:
            return L"N";
    }
}

FN as_string(MainEvent main_event) noexcept -> std::string {
    switch (main_event) {
        case MainEvent::E:
            return "E";
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
            return "N";
    }
}

FN as_wstring(MainEvent main_event) noexcept -> std::wstring {
    switch (main_event) {
        case MainEvent::E:
            return L"E";
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
            return L"N";
    }
}

#pragma endregion

