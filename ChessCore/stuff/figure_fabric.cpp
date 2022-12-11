#include "figure_fabric.h"

Figure* FigureFabric::create(Pos position, Color color, FigureType type, Id id, Figure* placement) {
    this->id = std::max(id, this->id);  // this->id должен быть актуальным
    switch (type) {
    case FigureType::Pawn:
    case FigureType::Knight:
    case FigureType::Rook:
    case FigureType::Bishop:
    case FigureType::Queen:
    case FigureType::King:
        return placement
            ? new (placement) Figure(id++, position, color, type)
            : new Figure(id++, position, color, type);
    case FigureType::None:
        return new Figure(id++, position, Color::None, FigureType::None);
    default:
        return get_default_fig();
    }
}

Figure* FigureFabric::create(Pos position, Color color, FigureType type) {
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

Figure* FigureFabric::get_default_fig() {
    if (not DEFAULT->empty()) throw std::logic_error("default figure was deleted *.*"); // на время разработки - очень часто вылезает эта ошибка
    return DEFAULT;
}

Figure* FigureFabric::create(Figure* to_copy, bool is_id_will_be_copied) {
    if (to_copy->empty()) return DEFAULT;
    return create(
        to_copy->get_pos(),
        to_copy->get_col(),
        to_copy->get_type(),
        is_id_will_be_copied ? to_copy->get_id() : this->id++
    );
}