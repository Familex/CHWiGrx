#pragma once
#include <map>
#include <list>
#include <regex>
#include <tuple>
#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <Windows.h>

template <class T>
std::list<T> operator +(std::list<T> lst, T val) {
    lst.push_back(val);
    return lst;
}

template <class T>
std::list<T> operator +(std::list<T> l, std::list<T> r) {
    l.insert(l.end(), r.begin(), r.end());
    return l;
}

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
    bool in(std::list<pos> lst) { return std::find(lst.begin(), lst.end(), *this) != lst.end(); }
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

std::list<pos> to_pos_list(std::list<Figure>);

struct Input {
    pos from;
    pos target;
    Input(std::string);
    Input() : from({ -1, -1 }), target({ -1, -1 }) {};
    Input(pos from, pos target) : from(from), target(target) {};
};
