#pragma once

#include "stuff/stuff.h"
#include "stuff/figure.h"
#include "FigureBoard.h"

namespace bot {
    
    enum class Type {
        None,   // bot is missing

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

    Input create_move(const Type, const FigureBoard&, Color turn) noexcept;
    
    namespace impl {

        Input random_move(const FigureBoard&, const Color) noexcept;
        Input minimax_move(const FigureBoard&, const Color) noexcept;
        Input alpha_beta_move(const FigureBoard&, const Color) noexcept;
        Input monte_carlo_move(const FigureBoard&, const Color) noexcept;
        Input neural_network_move(const FigureBoard&, const Color) noexcept;

    }
    
}  // namespace bot