#pragma once
#include <map>
#include <list>
#include <regex>
#include <tuple>
#include <format>
#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <Windows.h>
#include <functional>

template <class T>
std::vector<T> operator +(std::vector<T> lst, T val) {
    lst.push_back(val);
    return lst;
}

template <class T>
std::vector<T> operator +(std::vector<T> l, std::vector<T> r) {
    l.insert(l.end(), r.begin(), r.end());
    return l;
}

constexpr int EN_PASSANT_INDENT = 4;
enum class EFigureType { Pawn, Knight, Rook, Bishop, Queen, King, None };
enum class EColor { Black, White, None };
inline const std::string ALL_FIGURES{ "PHRBQK" };
inline const std::string ALL_PROMOTION_FIGURES{ "HRBQ" };
inline const std::string NOT_FIGURES{ "E" };
inline const std::string COLOR_CHARS{ "NBW" };

typedef int Id;
const Id ERR_ID{ -1 };
const int HEIGHT{ 8 };
const int WIDTH{ 8 };
struct pos {
    int x;
    int y;
    pos() : x(-1), y(-1) {};
    pos(int x, int y) : x(x), y(y) {};
    friend bool operator==(const pos& left, const pos& right);
    friend bool operator!=(const pos& left, const pos& right);
    friend bool operator<(const pos& left, const pos& right);
    friend pos operator-(const pos& left, const pos& right);
    friend pos operator+(const pos& left, const pos& right);
    pos& operator +=(const pos& r) { this->x += r.x; this->y += r.y; return *this; }
    pos mul_x(int mx) { return { x * mx, y }; }
    void loop_add(pos add, int max_x, int max_y) {
        this->x += add.x; this->y += add.y;
        if (this->x >= max_x) this->x = 0;
        if (this->x < 0) this->x = max_x - 1;
        if (this->y >= max_y) this->y = 0;
        if (this->y < 0) this->y = max_y - 1;
    }
    bool in(std::vector<pos> lst) { return std::find(lst.begin(), lst.end(), *this) != lst.end(); }
};

class Color {
public:
    Color(char ch = 'N');
    Color(EColor data) : data(data) {};
    operator EColor() const { return data; }
    operator char();
    Color to_next();
    Color what_next() const;
    friend bool operator!=(const Color& left, const Color& right);
    friend bool operator==(const Color& left, const Color& right);
    friend bool operator==(const Color& left, const EColor& right);
    friend bool operator==(const EColor& left, const Color& right);
private:
    EColor data;
};

class FigureType {
public:
    FigureType(char ch = 'N');
    FigureType(EFigureType data) : data(data) {};
    operator EFigureType() const { return data; }
    operator char();
    EFigureType get_data() const { return data; }
    bool operator==(EFigureType l) { return data == l; }
private:
    EFigureType data;
};

struct Figure {
    Figure() : id{ ERR_ID }, position(), color(), type() {};
    Figure(Id id, pos position, Color color, FigureType type) :
        id(id), position(position), color(color), type(type) {};
    void move_to(pos p) { position = p; }
    void move_to(int x, int y) { position.x = x; position.y = y; }
    Figure submit_on(pos p) const {
        Figure tmp{ *this };
        tmp.move_to(p);
        return tmp;
    }
    bool operator ==(const Figure& r) const { return this->id == r.id; }
    Id id;
    pos position;
    Color color;
    FigureType type;
};

std::vector<pos> to_pos_vector(const std::vector<Figure>&);

struct Input {
    pos from;
    pos target;
    Input(std::string);
    Input() : from({ -1, -1 }), target({ -1, -1 }) {};
    Input(pos from, pos target) : from(from), target(target) {};
};

enum class MainEvent { E, EAT, MOVE, LMOVE, CASTLING, EN_PASSANT };
enum class SideEvent { E, CHECK, PROMOTION, CASTLING_BREAK };

struct MoveMessage {
    MainEvent main_ev{ MainEvent::E };
    std::list<SideEvent> side_evs;
    std::vector<Figure> to_eat;
    std::list<std::pair<Figure, Input>> to_move;
    std::list<Id> what_castling_breaks;
};

struct MoveRec {
    Figure who_went;
    Input input;
    Color turn;
    MoveMessage ms;
    char promotion_choice;
};

class MoveLogger {
public:
    MoveRec get_last_move();
    void add(const MoveRec& move_rec);
    void add_without_reset(const MoveRec& move_rec) { prev_moves.push_back(move_rec); }
    void reset();
    MoveRec pop_future_move();
    MoveRec move_last_to_future();
    bool prev_empty() const { return prev_moves.empty(); }
    bool future_empty() const { return future_moves.empty(); }
private:
    std::vector<MoveRec> prev_moves;
    std::vector<MoveRec> future_moves;
};
