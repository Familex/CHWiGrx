#pragma once

#include "stuff.h"
#include "figure.h"
#include "move_rec.h"

class BoardRepr {
public:
    BoardRepr(const BoardRepr&);
    BoardRepr(std::string);
    BoardRepr(const std::vector<Figure*>& figures, 
              Color turn, 
              bool idw, 
              const std::vector<Id>& can_castle, 
              const std::vector<MoveRec>& past = {},
              const std::vector<MoveRec>& future = {},
              const std::vector<Figure*>& captured_figures = {}) 
        : figures(figures)
        , turn(turn)
        , idw(idw)
        , past(past)
        , future(future)
        , captured_figures(captured_figures)
        , can_castle(can_castle)
    {
    };
    BoardRepr(const std::vector<Figure*>& figures, 
              Color turn, 
              bool idw,
              const std::vector<MoveRec>& past = {},
              const std::vector<MoveRec>& future = {}, 
              const std::vector<Figure*>& captured_figures = {}) 
        : figures(figures)
        , turn(turn)
        , idw(idw)
        , past(past)
        , future(future)
        , captured_figures(captured_figures) {
        // all can castle by default
        for (auto fig : figures) {
            if (fig->is(FigureType::Rook)) {
                can_castle.push_back(fig->get_id());
            }
        }
    };
    BoardRepr* operator =(const BoardRepr& other);
    void clear();
    std::string as_string();
    char get_idw_char() const { return idw ? 'T' : 'F'; }
    bool get_idw() const { return idw; }
    char get_turn_char() const { return turn == Color::White ? 'W' : 'B'; }
    std::vector<Id> get_who_can_castle() const { return can_castle; }
    bool empty() const { return figures.empty(); }
    void set_figures(std::vector<Figure*>&& figs);
    void set_turn(Color t) { turn = t; }
    void set_idw(bool idw) { this->idw = idw; }
    void set_past(const std::vector<MoveRec>& past) { this->past = past; }
    void set_future(const std::vector<MoveRec>& future) { this->future = future; }
    Color get_turn() const { return turn; }
    std::vector<Figure*> get_figures() const { return figures; }
    std::vector<MoveRec> get_past() const { return past; }
    std::vector<MoveRec> get_future() const { return future; }
    std::vector<Figure*> get_captured_figures() const { return captured_figures; }
private:
    std::vector<Figure*> figures;
    Color turn{ Color::White };
    bool idw{ true };
    std::vector<MoveRec> past;
    std::vector<MoveRec> future;
    std::vector<Figure*> captured_figures;
    std::vector<Id> can_castle;
};
