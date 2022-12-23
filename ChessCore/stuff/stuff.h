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
#include <stdexcept>
#include <functional>

template <class T>
std::vector<T> operator +(std::vector<T> vec, T val) {
    vec.push_back(val);
    return vec;
}

template <class T>
std::vector<T> operator +(std::vector<T> l, std::vector<T> r) {
    l.insert(l.end(), r.begin(), r.end());
    return l;
}

std::vector<std::string> split(std::string, const std::string&&);

void remove_first_occurrence(std::string& str, char c);

constexpr int EN_PASSANT_INDENT = 4;

enum class GameEndType {Checkmate, FiftyRule, Stalemate, InsufficientMaterial, MoveRepeat, NotGameEnd};

typedef int Id;
const Id ERR_ID{ -1 };
const int HEIGHT{ 8 };  // this definitely shouldn't be here (todo)
const int WIDTH{ 8 };

struct Pos {
    int x;
    int y;
    Pos() : x(-1), y(-1) {};
    Pos(int x, int y) : x(x), y(y) {};
    auto operator <=> (const Pos& other) const = default;
    Pos operator-(const Pos& right) const { return { (x - right.x), (y - right.y) }; }
    Pos operator+(const Pos& right) const { return { (x + right.x), (y + right.y) }; }
    Pos& operator +=(const Pos& r) { this->x += r.x; this->y += r.y; return *this; }
    Pos mul_x(int mx) { return { x * mx, y }; }
    void loop_add(Pos add, int max_x, int max_y) {
        this->x += add.x; this->y += add.y;
        if (this->x >= max_x) this->x = 0;
        if (this->x < 0) this->x = max_x - 1;
        if (this->y >= max_y) this->y = 0;
        if (this->y < 0) this->y = max_y - 1;
    }
    Pos change_axes() { std::swap(x, y); return *this; }
    bool in(std::vector<Pos> lst) { return std::find(lst.begin(), lst.end(), *this) != lst.end(); }
};

inline Pos change_axes(const Pos& val) {
    return Pos(val.y, val.x);
}

enum class Color { None, Black, White };
Color char_to_col(char);
char col_to_char(Color);
Color what_next(Color);

inline const std::set<Color> PLAYABLE_COLORS = { {Color::White, Color::Black} };

enum class FigureType { None, Pawn, Knight, Rook, Bishop, Queen, King };
FigureType char_to_figure_type(char ch = 'N');
char figure_type_to_char(FigureType);

inline const std::set<FigureType> PLAYABLE_FIGURES{
    {FigureType::Pawn, FigureType::Knight, FigureType::Rook, FigureType::Bishop, FigureType::Queen, FigureType::King}
};

inline const std::set<FigureType> PROMOTION_FIGURES{
    {FigureType::Knight, FigureType::Rook, FigureType::Bishop, FigureType::Queen}
};

struct Input {
    Pos from;
    Pos target;
    Input(std::string);
    Input() : from({ -1, -1 }), target({ -1, -1 }) {};
    Input(Pos from, Pos target) : from(from), target(target) {};
};

enum class ErrorEvent { INVALID_MOVE, UNDER_CHECK, CHECK_IN_THAT_TILE, UNFORESEEN};
enum class MainEvent { E, EAT, MOVE, LMOVE, CASTLING, EN_PASSANT };
enum class SideEvent { E, CHECK, PROMOTION, CASTLING_BREAK };

std::string to_string(SideEvent side_event);
std::string to_string(MainEvent main_event);

struct MoveMessage {
    MainEvent main_ev{ MainEvent::E };
    std::list<SideEvent> side_evs;
    std::vector<Id> to_eat;
    std::list<std::pair<Id, Input>> to_move;
    std::list<Id> what_castling_breaks;
};
