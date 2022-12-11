#pragma once

#include "stuff.h"
#include "figure.h"
#include "figure_fabric.h"

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
    Pos get_who_went_pos() const {
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
