#pragma once

#include <new>
#include <map>
#include <set>
#include <list>
#include <tuple>
#include <format>
#include <vector>
#include <compare>
#include <cassert>
#include <functional>

template <class T>
[[nodiscard]] std::vector<T> operator +(std::vector<T> vec, T val) {
    vec.push_back(val);
    return vec;
}

template <class T>
[[nodiscard]] std::vector<T> operator +(std::vector<T> l, std::vector<T> r) {
    l.insert(l.end(), r.begin(), r.end());
    return l;
}

std::vector<std::string> split(std::string, const std::string&&);

void remove_first_occurrence(std::string& str, const char c);

constexpr int EN_PASSANT_INDENT = 4;

enum class GameEndType {
    Checkmate, 
    FiftyRule, 
    Stalemate, 
    InsufficientMaterial, 
    MoveRepeat,
    NotGameEnd
};

typedef int Id;
const Id ERR_ID{ -1 };
const int HEIGHT{ 8 };  // this definitely shouldn't be here (todo)
const int WIDTH{ 8 };

struct Pos {
    int x{ -1 };
    int y{ -1 };
    
    constexpr Pos() noexcept
        {};
    
    constexpr Pos(int x, int y) noexcept
        : x(x)
        , y(y) {};
    
    constexpr auto operator <=> (const Pos& other) const noexcept = default;
    
    [[nodiscard]] constexpr Pos operator-(const Pos& right) const noexcept
        { return { (x - right.x), (y - right.y) }; }
    
    [[nodiscard]] constexpr Pos operator+(const Pos& right) const noexcept
        { return { (x + right.x), (y + right.y) }; }
    
    constexpr Pos& operator +=(const Pos& r) noexcept
        { this->x += r.x; this->y += r.y; return *this; }
    
    [[nodiscard]] constexpr Pos mul_x(int mx) const noexcept
        { return { x * mx, y }; }
    
    constexpr void loop_add(Pos add, int max_x, int max_y) noexcept {
        this->x += add.x; this->y += add.y;
        if (this->x >= max_x) this->x = 0;
        if (this->x < 0) this->x = max_x - 1;
        if (this->y >= max_y) this->y = 0;
        if (this->y < 0) this->y = max_y - 1;
    }
    
    constexpr Pos& change_axes() noexcept
        { std::swap(x, y); return *this; }
    
    constexpr bool in(const std::vector<Pos>& lst) const noexcept 
        { return std::find(lst.cbegin(), lst.cend(), *this) != lst.cend(); }
    
};

[[nodiscard]] constexpr inline Pos change_axes(const Pos& val) noexcept {
    return { val.y, val.x };
}

enum class Color { 
    None, 
    Black, 
    White 
};

[[nodiscard]] constexpr Color char_to_col(const char ch) noexcept 
{
    switch (ch) {
        case 'W': case 'w': return Color::White;
        case 'B': case 'b': return Color::Black;
        case 'N': case 'n': return Color::None;
    }
    assert(!"char_to_col: invalid character");
    return Color::None;
}

[[nodiscard]] constexpr char col_to_char(const Color col) noexcept 
{
    switch (col) {
        case Color::Black: return 'B';
        case Color::White: return 'W';
        case Color::None: return 'N';
    }
    assert(!"col_to_char: invalid color");
    return 'E';
}

[[nodiscard]] constexpr Color what_next(const Color col) noexcept
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

inline const std::set<Color> PLAYABLE_COLORS { 
    Color::White, 
    Color::Black,
};

enum class FigureType { 
    None, 
    Pawn, 
    Knight, 
    Rook, 
    Bishop, 
    Queen, 
    King,
};

[[nodiscard]] constexpr FigureType char_to_figure_type(const char ch) noexcept
{
    switch (ch)
    {
        case 'K': case 'k': return FigureType::King;
        case 'H': case 'h': return FigureType::Knight;
        case 'P': case 'p': return FigureType::Pawn;
        case 'B': case 'b': return FigureType::Bishop;
        case 'Q': case 'q': return FigureType::Queen;
        case 'R': case 'r': return FigureType::Rook;
        case 'N': case 'n': return FigureType::None;
    }
    assert(!"char_to_figure_type: invalid character");
    return FigureType::None;
}

[[nodiscard]] constexpr char figure_type_to_char(const FigureType ft) noexcept
{
    switch (ft) {
        case FigureType::Pawn: return 'P';
        case FigureType::Rook: return 'R';
        case FigureType::Knight: return 'H';
        case FigureType::Bishop: return 'B';
        case FigureType::Queen: return 'Q';
        case FigureType::King: return 'K';
        case FigureType::None: return 'N';
    }
    assert(!"figure_type_to_char: invalid figure type");
    return 'E';
}

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

struct Input {
    Pos from{ };
    Pos target{ };
    
    [[nodiscard]] Input(std::string);
    [[nodiscard]] constexpr Input() noexcept = default;
        
    [[nodiscard]] constexpr Input(const Pos& from, const Pos& target) noexcept
        : from{ from }
        , target{ target } {}
};

enum class ErrorEvent { INVALID_MOVE, UNDER_CHECK, CHECK_IN_THAT_TILE, UNFORESEEN};
enum class MainEvent { E, EAT, MOVE, LMOVE, CASTLING, EN_PASSANT };
enum class SideEvent { E, CHECK, PROMOTION, CASTLING_BREAK };

[[nodiscard]] std::string to_string(SideEvent side_event) noexcept;
[[nodiscard]] std::wstring to_wstring(SideEvent side_event) noexcept;
[[nodiscard]] std::string to_string(MainEvent main_event) noexcept;
[[nodiscard]] std::wstring to_wstring(MainEvent main_event) noexcept;
[[nodiscard]] std::string to_string(FigureType) noexcept;
[[nodiscard]] std::wstring to_wstring(FigureType) noexcept;

struct MoveMessage {
    MainEvent main_ev{ MainEvent::E };
    std::vector<SideEvent> side_evs;
    std::vector<Id> to_eat;
    std::vector<std::pair<Id, Input>> to_move;
    std::vector<Id> what_castling_breaks;
};
