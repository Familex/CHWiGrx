#include "figure_fabric.h"

Figure* FigureFabric::create(const Pos position, const Color color, const FigureType type, const Id id, Figure* placement) const {
    // ignore this->id
    switch (type) {
    case FigureType::Pawn:
    case FigureType::Knight:
    case FigureType::Rook:
    case FigureType::Bishop:
    case FigureType::Queen:
    case FigureType::King:
        return placement
            ? new (placement) Figure(id, position, color, type)
            : new Figure(id, position, color, type);
    case FigureType::None:
        return new Figure(id, position, Color::None, FigureType::None);
    default:
        return get_default_fig();
    }
}

Figure* FigureFabric::create(const Pos position, const Color color, const FigureType type) {
    switch (type) {
    case FigureType::Pawn:
    case FigureType::Knight:
    case FigureType::Rook:
    case FigureType::Bishop:
    case FigureType::Queen:
    case FigureType::King:
        return new Figure(this->id++, position, color, type);
    case FigureType::None:
        return new Figure(this->id++, position, Color::None, FigureType::None);
    default:
        return get_default_fig();
    }
}

Figure* FigureFabric::get_default_fig() const {
    #ifdef _DEBUG
        if (not DEFAULT->empty()) throw std::logic_error("default figure was deleted *.*"); // на время разработки - очень часто вылезает эта ошибка
    #endif // _DEBUG
    return DEFAULT;
}

Figure* FigureFabric::create(const Figure* to_copy, const bool is_id_will_be_copied) {
    if (to_copy->empty()) return DEFAULT;
    return create(
        to_copy->get_pos(),
        to_copy->get_col(),
        to_copy->get_type(),
        is_id_will_be_copied ? to_copy->get_id() : this->id++
    );
}
