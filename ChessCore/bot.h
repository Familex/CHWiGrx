#pragma once

#include "ChessGame.hpp"
#include "structs/figure.hpp"
#include "stuff/stuff.hpp"

namespace bot
{

enum class Type {
    None,          // bot is missing
    Unselected,    // bot is selected, but not yet chosen

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

[[nodiscard]] Input create_move(const Type, const ChessGame&, Color turn) noexcept;

namespace impl
{

[[nodiscard]] Input random_move(const ChessGame&, const Color) noexcept;
[[nodiscard]] Input minimax_move(const ChessGame&, const Color) noexcept;
[[nodiscard]] Input alpha_beta_move(const ChessGame&, const Color) noexcept;
[[nodiscard]] Input monte_carlo_move(const ChessGame&, const Color) noexcept;
[[nodiscard]] Input neural_network_move(const ChessGame&, const Color) noexcept;

}    // namespace impl

}    // namespace bot