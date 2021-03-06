#pragma once
#include <new>
#include <map>
#include <list>
#include <tuple>
#include <format>
#include <vector>
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
inline const std::string ALL_FIGURES{ "PHRBQK" };
inline const std::string ALL_PROMOTION_FIGURES{ "HRBQ" };
inline const std::string NOT_FIGURES{ "E" };
inline const std::string COLOR_CHARS{ "NBW" };

enum class GameEndType {Checkmate, FiftyRule, Stalemate, InsufficientMaterial, MoveRepeat, NotGameEnd};

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
    enum Type { Black, White, None };
    Color(char ch = 'N');
    Color(Type data) : data(data) {};
    operator Type() const { return data; }
    operator char() const;
    Color to_next();
    Color what_next() const;
    friend bool operator!=(const Color& left, const Color& right);
    friend bool operator==(const Color& left, const Color& right);
    friend bool operator==(const Color& left, const Color::Type& right);
    friend bool operator==(const Color::Type& left, const Color& right);
private:
    Type data;
};

class FigureType {
public:
    enum Type { Pawn, Knight, Rook, Bishop, Queen, King, None };
    FigureType(char ch = 'N');
    FigureType(Type data) : data(data) {};
    operator Type() const { return data; }
    operator char() const;
    Type get_data() const { return data; }
    bool operator==(Type l) { return data == l; }
private:
    Type data;
};

class Figure {
public:
    Figure() : id{ ERR_ID }, position() {};
    Figure(Id id, pos position, Color color, FigureType type) :
        id(id), position(position), color(color), type(type) {};
    void move_to(pos p) { position = p; }
    void move_to(int x, int y) { position.x = x; position.y = y; }
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
private:
    Id id;
    pos position{};
    Color color{};
    FigureType type{};
};

std::vector<pos> to_pos_vector(const std::vector<Figure*>&);

struct Input {
    pos from;
    pos target;
    Input(std::string);
    Input() : from({ -1, -1 }), target({ -1, -1 }) {};
    Input(pos from, pos target) : from(from), target(target) {};
};

enum class ErrorEvent { INVALID_MOVE, UNDER_CHECK, CHECK_IN_THAT_TILE, UNFORESEEN};
enum class MainEvent { E, EAT, MOVE, LMOVE, CASTLING, EN_PASSANT };
enum class SideEvent { E, CHECK, PROMOTION, CASTLING_BREAK };

std::string to_string(SideEvent side_event);
std::string to_string(MainEvent main_event);


class FigureFabric {
public:
    static FigureFabric* instance() {
        static FigureFabric INSTANCE;
        return &INSTANCE;
    }
    FigureFabric(FigureFabric const&) = delete;
    void operator=(FigureFabric const&) = delete;

    Figure* create(pos, Color, FigureType::Type);
    Figure* create(pos, Color, FigureType::Type, Id, Figure* =nullptr);
    Figure* create(Figure*);
    Figure* get_default_fig();
    // ???? ???????????????? ?????????????? ?????????????????? ????????????
    Figure* submit_on(Figure* who, pos on) {
        Figure* tmp = FigureFabric::instance()->create(who);
        tmp->move_to(on);
        return tmp;
    }
    ~FigureFabric() {
        delete DEFAULT;
    }
private:
    FigureFabric() {};
    Figure* DEFAULT = new Figure();
    Id id{ 1 };
};


struct MoveMessage {
    MainEvent main_ev{ MainEvent::E };
    std::list<SideEvent> side_evs;
    std::vector<Id> to_eat;
    std::list<std::pair<Id, Input>> to_move;
    std::list<Id> what_castling_breaks;
};

struct MoveRec {
    MoveRec(Figure* who_went, Input input, Color turn, MoveMessage ms, char p)
        : who_went(*who_went)
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
    Figure* get_who_went() const {
        if (who_went.empty()) return FigureFabric::instance()->get_default_fig();
        return FigureFabric::instance()->create(
            who_went.get_pos(),
            who_went.get_col(),
            who_went.get_type(),
            who_went.get_id()
        );
    }
    pos get_who_went_pos() const {
        if (who_went.empty()) return { -1, -1 };
        return who_went.get_pos();
    }
    MoveRec(std::string);
    Figure who_went{};
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
    bool is_fifty_move_rule_was_triggered();
    bool is_moves_repeat_rule_was_triggered();
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
    char get_turn_char() const { return turn == Color::Type::White ? 'W' : 'B'; }
    std::vector<Id> get_who_can_castle() const { return can_castle; }
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
    Color turn{ Color::Type::White };
    bool idw{ true };
    std::vector<MoveRec> past;
    std::vector<MoveRec> future;
    std::list<Figure*> captured_figures;
    std::vector<Id> can_castle;
};
