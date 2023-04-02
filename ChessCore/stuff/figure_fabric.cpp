#include "figure_fabric.h"

namespace figfab {

    [[nodiscard]] std::expected<std::unique_ptr<Figure>, ParseError> FigureFabric::
        create(const std::string_view str) noexcept {
        std::vector<std::string_view> data = split(str, ".");
        Id new_id = svtoi(data[0]).value();
        Color new_col = char_to_col(data[3][0]);
        Pos new_pos = { svtoi(data[1]).value(), svtoi(data[2]).value() };
        FigureType new_type = char_to_figure_type(data[4][0]);
        return instance().create( new_pos, new_col, new_type, new_id );
    }

    std::unique_ptr<Figure> FigureFabric::
        create(const Pos position, const Color color, const FigureType type,
            const Id new_id) noexcept {
        this->id = std::max(this->id, new_id + 1);
        return std::make_unique<Figure>(new_id, position, color, type);
    }

    void FigureFabric::
        create_in_place(const Pos position, const Color color, const FigureType type,
            const Id new_id, Figure* const placement) noexcept {
        this->id = std::max(this->id, new_id + 1);
        new (placement) Figure(new_id, position, color, type);
    }
    
    std::unique_ptr<Figure> FigureFabric::
        create(const Pos position, const Color color, const FigureType type) noexcept {
        return std::make_unique<Figure>(this->id++, position, color, type);
    }

    std::unique_ptr<Figure> FigureFabric::
        create(const Figure *const to_copy, const bool is_id_will_be_copied) noexcept {
        return create(
            to_copy->get_pos(),
            to_copy->get_col(),
            to_copy->get_type(),
            is_id_will_be_copied ? to_copy->get_id() : this->id++
        );
    }

}   // namespace figfab