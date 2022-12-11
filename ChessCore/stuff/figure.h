#pragma once

#include "stuff.h"

class Figure {
public:
    Figure() : id{ ERR_ID }, position() {};
    Figure(Id id, Pos position, Color color, FigureType type) :
        id(id), position(position), color(color), type(type) {};
    void move_to(Pos p) { position = p; }
    void move_to(int x, int y) { position.x = x; position.y = y; }
    bool operator ==(const Figure& r) const { return this->id == r.id; }
    std::string as_string() {
        return std::format("{}.{}.{}.{}.{}",
            id, position.x, position.y, col_to_char(color), figure_type_to_char(type));
    }
    Id get_id() const { return id; }
    Pos get_pos() const { return position; }
    Color get_col() const { return color; }
    FigureType get_type() const { return type; }
    bool is_col(Color col) const { return color == col; }
    bool is_col(Figure* fig) const { return color == fig->get_col(); }
    bool empty() const { return id == ERR_ID; }
    bool is(Id id) const { return this->id == id; }
    bool is(FigureType type) const { return this->type == type; }
    bool at(Pos p) const { return position == p; }

private:
    Id id{ ERR_ID };
    Pos position{};
    Color color{ Color::None };
    FigureType type{ FigureType::None };
};

std::vector<Pos> to_pos_vector(const std::vector<Figure*>&);
