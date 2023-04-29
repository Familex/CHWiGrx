#pragma once

#include "../chess_game.hpp"

namespace bot
{

/**
 * \brief bot behavior patterns
 */
enum class Type {
    Random,
    Minimax,
    AlphaBeta,
    MonteCarlo,
    NeuralNetwork
};

/**
 * \brief bot difficulty 
 */
enum class Difficulty {
    D0 = 0,
    D1 = 1,
    D2 = 2,
    D3 = 3,
};

[[nodiscard]] Input create_move(Type, Difficulty, const ChessGame&, Color) noexcept;

}    // namespace bot