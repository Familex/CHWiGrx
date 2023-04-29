#include "bot.h"

Input bot::create_move(const bot::Type bot_type, const ChessGame& board, const Color turn) noexcept
{
    switch (bot_type) {
        case bot::Type::Random:
            return bot::impl::random_move(board, turn);

        case bot::Type::Minimax:
            return bot::impl::minimax_move(board, turn);

        case bot::Type::AlphaBeta:
            return bot::impl::alpha_beta_move(board, turn);

        case bot::Type::MonteCarlo:
            return bot::impl::monte_carlo_move(board, turn);

        case bot::Type::NeuralNetwork:
            return bot::impl::neural_network_move(board, turn);

        default:
            return bot::impl::random_move(board, turn);
    }
}

Input bot::impl::random_move(const ChessGame& board, const Color turn) noexcept
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
    return Input { fig->get_pos(), move.second };
}

Input bot::impl::minimax_move(const ChessGame& board, const Color turn) noexcept { return random_move(board, turn); }

Input bot::impl::alpha_beta_move(const ChessGame& board, const Color turn) noexcept { return random_move(board, turn); }

Input bot::impl::monte_carlo_move(const ChessGame& board, const Color turn) noexcept
{
    return random_move(board, turn);
}

Input bot::impl::neural_network_move(const ChessGame& board, const Color turn) noexcept
{
    return random_move(board, turn);
}
