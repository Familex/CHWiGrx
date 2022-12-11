#include "figure.h"

// �������� �� ������� ����� ������ ������ �� �������
std::vector<Pos> to_pos_vector(const std::vector<Figure*>& lst) {
    std::vector<Pos> acc{};
    for (const auto& fig : lst) {
        acc.emplace_back(fig->get_pos());
    }
    return acc;
}
