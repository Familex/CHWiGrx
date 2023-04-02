#pragma once

#include "stuff/stuff.h"
#include "stuff/figure.hpp"
#include "FigureBoard.h"

namespace bot {
    
    enum class Type {
        None,   // bot is missing
        Unselected, // bot is selected, but not yet chosen

        // bot behavior patterns
        Random,
        Minimax,
        AlphaBeta,
        MonteCarlo,
        NeuralNetwork
    };

    enum class Difficulty {
        D0 = 0,
        D1 = 1,
        D2 = 2,
        D3 = 3,
    };

    [[nodiscard]] Input create_move(const Type, const FigureBoard&, Color turn) noexcept;
    
    namespace impl {

        [[nodiscard]] Input random_move(const FigureBoard&, const Color) noexcept;
        [[nodiscard]] Input minimax_move(const FigureBoard&, const Color) noexcept;
        [[nodiscard]] Input alpha_beta_move(const FigureBoard&, const Color) noexcept;
        [[nodiscard]] Input monte_carlo_move(const FigureBoard&, const Color) noexcept;
        [[nodiscard]] Input neural_network_move(const FigureBoard&, const Color) noexcept;

    }
    
}  // namespace bot