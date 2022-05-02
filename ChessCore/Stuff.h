#pragma once
#include <map>
#include <list>
#include <tuple>
#include <format>
#include <vector>
#include <stdexcept>
#include <Windows.h>

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

class Figure {
public:
    Figure() : id{ ERR_ID }, position() {};
    Figure(Id id, pos position) :
        id(id), position(position), color(), type() {};
    void move_to(pos p) { position = p; }
    void move_to(int x, int y) { position.x = x; position.y = y; }
    Figure* submit_on(pos p) {
        Figure* tmp = { this };
        tmp->move_to(p);
        return tmp;
    }
    bool operator ==(const Figure& r) const { return this->id == r.id; }
    std::string as_string() {
        return std::format("{}.{}.{}.{}.{}",
            id, position.x, position.y, (char)color, (char)type);
    }
    Id get_id() const { return id; }
    pos get_pos() const { return position; }
    Color get_col() const { return color; }
    FigureType get_type() const { return type; }
    bool is_col(Color col) const { return color == col; }
    bool is_col(Figure* fig) const { return color == fig->get_col(); }
    bool empty() const { return id == ERR_ID; }
    bool is(Id id) const { return this->id == id; }
    bool at(pos p) const { return position == p; }
    virtual ~Figure() {};
protected:
    Id id;
    pos position{};
    Color color{};
    FigureType type{};
};

class Pawn : public Figure {
public:
    Pawn(Id id, pos position, Color color)
        : Figure(id, position) {
        this->color = color;
        this->type = EFigureType::Pawn;
    }
};

class Bishop : public Figure {
public:
    Bishop(Id id, pos position, Color color)
        : Figure(id, position) {
        this->color = color;
        this->type = EFigureType::Bishop;
    }
};

class King : public Figure {
public:
    King(Id id, pos position, Color color)
        : Figure(id, position) {
        this->color = color;
        this->type = EFigureType::King;
    }
};

class Queen : public Figure {
public:
    Queen(Id id, pos position, Color color)
        : Figure(id, position) {
        this->color = color;
        this->type = EFigureType::Queen;
    }
};

class Knight : public Figure {
public:
    Knight(Id id, pos position, Color color)
        : Figure(id, position) {
        this->color = color;
        this->type = EFigureType::Knight;
    }
};

class Rook : public Figure {
public:
    Rook(Id id, pos position, Color color)
        : Figure(id, position) {
        this->color = color;
        this->type = EFigureType::Rook;
    }
};

std::vector<pos> to_pos_vector(const std::vector<Figure*>&);

struct Input {
    pos from;
    pos target;
    Input(std::string);
    Input() : from({ -1, -1 }), target({ -1, -1 }) {};
    Input(pos from, pos target) : from(from), target(target) {};
};

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

struct MoveRec {
    MoveRec(Figure* who_went, Input input, Color turn, MoveMessage ms, char p)
        : who_went(who_went)
        , input(input)
        , turn(turn)
        , ms(ms)
        , promotion_choice(p) {};
    MoveRec()
        : who_went()
        , input()
        , turn()
        , ms()
        , promotion_choice('N') {};
    MoveRec(std::string);
    Figure* who_went{ nullptr };
    Input input;
    Color turn;
    MoveMessage ms;
    char promotion_choice;
    std::string as_string();
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
    std::vector<MoveRec> get_past() { return prev_moves; }
    std::vector<MoveRec> get_future() { return future_moves; }
    void   set_past(const std::vector<MoveRec>& past)   { prev_moves = past; }
    void set_future(const std::vector<MoveRec>& future) { future_moves = future; }
private:
    std::vector<MoveRec> prev_moves;
    std::vector<MoveRec> future_moves;
};

class BoardRepr {
public:
    BoardRepr(std::string);
    std::string as_string();
    char get_idw_char() const { return idw ? 'T' : 'F'; }
    bool get_idw() const { return idw; }
    char get_turn_char() const { return turn == EColor::White ? 'W' : 'B'; }
    Color get_turn() const { return turn; }
    bool empty() const { return figures.empty(); }
    void set_figures(std::list<Figure*>&& figs) { figures = figs; }
    void set_turn(Color t) { turn = t; }
    void set_idw(bool idw) { this->idw = idw; }
    std::list<Figure*> get_figures() const { return figures; }
    std::list<Figure*> get_figures() { return figures; }
    void set_past(const std::vector<MoveRec>& past) { this->past = past; }
    void set_future(const std::vector<MoveRec>& future) { this->future = future; }
    std::vector<MoveRec> get_past()   const { return past; }
    std::vector<MoveRec> get_future() const { return future; }
    std::list<Figure*> get_captured_figures() const { return captured_figures; }
private:
    std::list<Figure*> figures;
    Color turn{ EColor::White };
    bool idw{ true };
    std::vector<MoveRec> past;
    std::vector<MoveRec> future;
    std::list<Figure*> captured_figures;
};

class FigureFabric {
public:
    static FigureFabric* instance() {
        static FigureFabric INSTANCE;
        return &INSTANCE;
    }
    FigureFabric(FigureFabric const&) = delete;
    void operator=(FigureFabric const&) = delete;

    Figure* create(pos, Color, EFigureType);
    Figure* create(pos, Color, EFigureType, Id);
private:
    FigureFabric() {};
    Id id{ 1 };
};
