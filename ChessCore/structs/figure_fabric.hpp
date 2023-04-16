#pragma once

#include "figure.hpp"

#include <expected>
#include <memory>

namespace figfab {

    class FigureFabric {
    public:
        #if __cpp_constexpr >= 202211L
            FN static
        #else   // Permitting static constexpr variables in constexpr functions
            [[nodiscard]] inline static auto
        #endif 
            instance() noexcept -> FigureFabric& 
        {
            static FigureFabric instance;
            return instance;
        }

        FN create(const Pos position, const Color color, const FigureType type,
                const Id new_id) noexcept -> std::unique_ptr<Figure>
        {
            this->id_ = std::max(this->id_, new_id + 1_id);
            return std::make_unique<Figure>(new_id, position, color, type);
        }

        FN create(const Pos position, const Color color, const FigureType type) noexcept -> std::unique_ptr<Figure>
        {
            return std::make_unique<Figure>(this->id_++, position, color, type);
        }

        void create_in_place(const Pos position, const Color color, const FigureType type,
                const Id new_id, Figure* const placement) noexcept
        {
            this->id_ = std::max(this->id_, new_id + 1_id);
            new (placement) Figure(new_id, position, color, type);
        }

        FN create(const Figure* const to_copy, const bool is_id_will_be_copied) noexcept -> std::unique_ptr<Figure>
        {
            return create(
                to_copy->get_pos(),
                to_copy->get_col(),
                to_copy->get_type(),
                is_id_will_be_copied ? to_copy->get_id() : this->id_++
            );
        }

        [[nodiscard]] auto static submit_on(const Figure* who, const Pos on) noexcept -> std::unique_ptr<Figure> {
            std::unique_ptr<Figure> tmp(FigureFabric::instance().create(who, true));
            tmp->move_to(on);
            return tmp;
        }
        
        FigureFabric(FigureFabric const&) noexcept = delete;
        FigureFabric& operator=(FigureFabric const&) noexcept = delete;
        FigureFabric(FigureFabric&&) noexcept = delete;
        FigureFabric& operator=(FigureFabric&&) noexcept = delete;

        ~FigureFabric() noexcept = default;
    private:
        constexpr FigureFabric() noexcept = default;

        Id id_{ 1_id };
    };

}   // namespace figfab