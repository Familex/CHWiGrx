#pragma once

#include "stuff.h"
#include "figure.h"
#include "move_rec.h"

/* Data Transfer Structure */
struct BoardRepr {
    /* ---- Fields ---- */
    std::vector<Figure*> figures;
    Color turn{ Color::White };
    bool idw{ true };
    std::vector<MoveRec> past;
    std::vector<MoveRec> future;
    std::vector<Figure*> captured_figures;
    std::vector<Id> can_castle;
    
    /* ---- Methods ---- */
    explicit BoardRepr(const BoardRepr&) noexcept;    
    explicit BoardRepr(std::string) noexcept;
    explicit BoardRepr(
                std::vector<Figure*>&& figures, 
                Color turn, 
                bool idw, 
                const std::vector<Id>& can_castle, 
                const std::vector<MoveRec>& past = {},
                const std::vector<MoveRec>& future = {},
                const std::vector<Figure*>& captured_figures = {}) noexcept
        : figures(figures)
        , turn(turn)
        , idw(idw)
        , past(past)
        , future(future)
        , captured_figures(captured_figures)
        , can_castle(can_castle)
    {
    };
    
    /* Without castling (automatically set all to true) */
    explicit BoardRepr(
                std::vector<Figure*>&& figures, 
                Color turn, 
                bool idw,
                const std::vector<MoveRec>& past = {},
                const std::vector<MoveRec>& future = {}, 
                const std::vector<Figure*>& captured_figures = {}) noexcept
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
    BoardRepr(BoardRepr&&) noexcept;
    BoardRepr* operator =(const BoardRepr& other) noexcept;
    void clear() noexcept;
    std::string as_string() noexcept;
    char get_idw_char() const noexcept { return idw ? 'T' : 'F'; }
    char get_turn_char() const noexcept;
    bool empty() const noexcept { return figures.empty(); }
};
