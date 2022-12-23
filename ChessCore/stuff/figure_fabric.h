#pragma once

#include "figure.h"

class FigureFabric {
public:
    static FigureFabric* instance() {
        static FigureFabric INSTANCE;
        return &INSTANCE;
    }
    FigureFabric(FigureFabric const&) = delete;
    void operator=(FigureFabric const&) = delete;

    Figure* create(Pos, Color, FigureType);
    Figure* create(Pos, Color, FigureType, Id, Figure* = nullptr);
    Figure* create(Figure*, bool = true);
    Figure* get_default_fig() const;
    std::unique_ptr<Figure> submit_on(Figure* who, Pos on) {
        std::unique_ptr<Figure> tmp(FigureFabric::instance()->create(who, true));
        tmp->move_to(on);
        return tmp;
    }
    ~FigureFabric() {
        delete DEFAULT;
    }
private:
    FigureFabric() = default;
    Figure* DEFAULT = new Figure();
    Id id{ 1 };
};
