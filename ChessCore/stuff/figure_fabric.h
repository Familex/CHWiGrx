#pragma once

#include "figure.h"

class FigureFabric {
public:
    static FigureFabric* instance() {
        static FigureFabric INSTANCE;
        return &INSTANCE;
    }

    Figure* create(const Pos, const Color, const FigureType);
    Figure* create(const Pos, const Color, const FigureType, const Id, Figure* = nullptr) const;
    Figure* create(const Figure*, const bool = true);
    Figure* get_default_fig() const;
    std::unique_ptr<Figure> submit_on(const Figure* who, const Pos on) {
        std::unique_ptr<Figure> tmp(FigureFabric::instance()->create(who, true));
        tmp->move_to(on);
        return tmp;
    }
    ~FigureFabric() {
        delete DEFAULT;
    }
private:
    FigureFabric() = default;
    FigureFabric(FigureFabric const&) = delete;
    void operator=(FigureFabric const&) = delete;
    
    Figure* DEFAULT = new Figure();
    Id id{ 1 };
};
