#include "bot.h"

namespace
{

[[nodiscard]] Input random_move(bot::Difficulty, const ChessGame& board, const Color turn) noexcept
{
    std::vector<std::pair<Figure*, std::pair<bool, Pos>>> possible_figure_moves;
    //                    ~~~~~~~            ~~~~  ~~~
    //                      in_hand           is_eat target
    for (const auto& fig : board.get_figures_of(turn)) {
        for (const auto& move : board.get_all_possible_moves(fig)) {
            possible_figure_moves.emplace_back(fig, move);
        }
    }
    if (possible_figure_moves.empty()) {
        return Input();
    }
    const auto& [fig, move] = possible_figure_moves[rand() % possible_figure_moves.size()];
    return Input{ fig->get_pos(), move.second };
}

[[nodiscard]] Input minimax_move(const bot::Difficulty diff, const ChessGame& board, const Color turn) noexcept
{
    return random_move(diff, board, turn);
}

[[nodiscard]] Input alpha_beta_move(const bot::Difficulty diff, const ChessGame& board, const Color turn) noexcept
{
    return random_move(diff, board, turn);
}

[[nodiscard]] Input monte_carlo_move(const bot::Difficulty diff, const ChessGame& board, const Color turn) noexcept
{
    return random_move(diff, board, turn);
}

[[nodiscard]] Input neural_network_move(const bot::Difficulty diff, const ChessGame& board, const Color turn) noexcept
{
    return random_move(diff, board, turn);
}

}    // namespace

/**
 * \brief create bot move based on bot type and difficulty
 * \param bot_type bot behavior pattern
 * \param diff bot difficulty
 * \param board game board
 * \param turn bot color
 * \return bot move
 */
Input bot::create_move(const Type bot_type, const Difficulty diff, const ChessGame& board, const Color turn) noexcept
{
    switch (bot_type) {
        case bot::Type::Random:
            return random_move(diff, board, turn);

        case bot::Type::Minimax:
            return minimax_move(diff, board, turn);

        case bot::Type::AlphaBeta:
            return alpha_beta_move(diff, board, turn);

        case bot::Type::MonteCarlo:
            return monte_carlo_move(diff, board, turn);

        case bot::Type::NeuralNetwork:
            return neural_network_move(diff, board, turn);

        default:
            return random_move(diff, board, turn);
    }
}
