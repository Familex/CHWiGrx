#include "bot.h"
    
Input bot::create_move(const bot::Type bot_type,
                       const FigureBoard& board, 
                       const Color turn) noexcept {
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

Input bot::impl::random_move(const FigureBoard& board, const Color turn) noexcept {
    std::vector<std::pair<Figure*, std::pair<bool, Pos>>> possible_figures_and_moves;
    //                    ~~~~~~~            ~~~~  ~~~
    //                      in_hand           is_eat target
    for (const auto& fig : board.get_figures_of(turn)) {
        for (const auto& move : board.get_all_possible_moves(fig))
        {
            auto moved_fig = FigureFabric::instance()->submit_on(fig, move.second);
            const std::vector<Pos> to_ignore = 
                move.first 
                ? std::vector<Pos>{ move.second, fig->get_pos() } 
                : std::vector<Pos>{ fig->get_pos() };
            Pos to_defend =
                fig->is(FigureType::King)
                ? move.second
                : Pos{};
            if (!board.check_for_when(turn, to_ignore, to_defend, { moved_fig.get() }))
            {
                possible_figures_and_moves.push_back({fig, move});
            }
        }
    }
    if (possible_figures_and_moves.empty()) {
        return Input();
    }
    const auto& [fig, move] = possible_figures_and_moves[rand() % possible_figures_and_moves.size()];
    return { fig->get_pos(), move.second };
}

Input bot::impl::minimax_move(const FigureBoard& board, const Color turn) noexcept {
    return random_move(board, turn);
}

Input bot::impl::alpha_beta_move(const FigureBoard& board, const Color turn) noexcept {
    return random_move(board, turn);
}

Input bot::impl::monte_carlo_move(const FigureBoard& board, const Color turn) noexcept {
    return random_move(board, turn);
}

Input bot::impl::neural_network_move(const FigureBoard& board, const Color turn) noexcept {
    return random_move(board, turn);
}