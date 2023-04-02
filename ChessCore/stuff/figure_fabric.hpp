#pragma once

#include "figure.hpp"

#include <expected>

namespace figfab {

    enum class ParseError {
        WrongId,
        WrongColor,
        WrongPosX,
        WrongPosY,
        WrongFigureType,
    };

    class FigureFabric {
    public:
        #if __cpp_constexpr >= 202211L
            FN static
        #else   // Permitting static constexpr variables in constexpr functions
            [[nodiscard]] static auto
        #endif 
            instance() noexcept -> FigureFabric& 
        {
            static FigureFabric INSTANCE;
            return INSTANCE;
        }

        FN create(const std::string_view str) noexcept -> std::expected<std::unique_ptr<Figure>, ParseError> {
            std::vector<std::string_view> data = split(str, ".");
            Id new_id = svtoi(data[0]).value();
            Color new_col = char_to_col(data[3][0]);
            Pos new_pos = { svtoi(data[1]).value(), svtoi(data[2]).value() };
            FigureType new_type = char_to_figure_type(data[4][0]);
            return instance().create(new_pos, new_col, new_type, new_id);
        }

        FN create(const Pos position, const Color color, const FigureType type,
                const Id new_id) noexcept -> std::unique_ptr<Figure>
        {
            this->id = std::max(this->id, new_id + 1);
            return std::make_unique<Figure>(new_id, position, color, type);
        }

        FN create(const Pos position, const Color color, const FigureType type) noexcept -> std::unique_ptr<Figure>
        {
            return std::make_unique<Figure>(this->id++, position, color, type);
        }

        FN create_in_place(const Pos position, const Color color, const FigureType type,
            const Id new_id, Figure* const placement) noexcept -> void
        {
            this->id = std::max(this->id, new_id + 1);
            new (placement) Figure(new_id, position, color, type);
        }

        FN create(const Figure* const to_copy, const bool is_id_will_be_copied) noexcept -> std::unique_ptr<Figure>
        {
            return create(
                to_copy->get_pos(),
                to_copy->get_col(),
                to_copy->get_type(),
                is_id_will_be_copied ? to_copy->get_id() : this->id++
            );
        }

        FN submit_on(const Figure* who, const Pos on) noexcept -> std::unique_ptr<Figure> {
            std::unique_ptr<Figure> tmp(FigureFabric::instance().create(who, true));
            tmp->move_to(on);
            return tmp;
        }
        
    private:
        constexpr FigureFabric() noexcept = default;
        FigureFabric(FigureFabric const&) = delete;
        void operator=(FigureFabric const&) = delete;

        Id id{ 1 };
    };

}   // namespace figfab