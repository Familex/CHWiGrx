#pragma once

#include "figure.h"

class FigureFabric {
public:
    [[nodiscard]] static FigureFabric* instance() noexcept {
        static FigureFabric INSTANCE;
        return &INSTANCE;
    }

    [[nodiscard]] Figure* create(const Pos, const Color, const FigureType);
    Figure* create(const Pos, const Color, const FigureType, const Id, Figure* = nullptr);
    [[nodiscard]] Figure* create(const Figure*, const bool = true);
    [[nodiscard]] Figure* get_default_fig() const;
    
    [[nodiscard]] std::unique_ptr<Figure> submit_on(const Figure* who, const Pos on) {
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
