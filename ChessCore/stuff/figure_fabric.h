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
        [[nodiscard]] static FigureFabric& instance() noexcept {
            static FigureFabric INSTANCE;
            return INSTANCE;
        }

        [[nodiscard]] std::expected<std::unique_ptr<Figure>, ParseError>
            create(const std::string_view) noexcept;

        [[nodiscard]] std::unique_ptr<Figure>
            create(const Pos, const Color, const FigureType) noexcept;

        [[nodiscard]] std::unique_ptr<Figure>
            create(const Pos, const Color, const FigureType, const Id) noexcept;

        void
            create_in_place(const Pos, const Color, const FigureType, const Id, Figure* const) noexcept;

        [[nodiscard]] std::unique_ptr<Figure>
            create(const Figure* const, const bool = true) noexcept;

        [[nodiscard]] std::unique_ptr<Figure> submit_on(const Figure* who, const Pos on) {
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