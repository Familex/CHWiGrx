#include "figure.h"

// Оставить от вектора фигур только вектор их позиций
std::vector<Pos> to_pos_vector(const std::vector<Figure*>& lst) {
    std::vector<Pos> acc{};
    for (const auto& fig : lst) {
        acc.emplace_back(fig->get_pos());
    }
    return acc;
}
