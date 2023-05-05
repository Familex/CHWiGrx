#pragma once

#include "structs/board_repr.hpp"
#include "structs/figure_board.hpp"
#include "structs/move_logger.hpp"
#include "stuff/enums.hpp"
#include "stuff/stuff.hpp"

#include <map>
#include <ranges>

namespace chess_game
{

constexpr int EN_PASSANT_INDENT = 4;

FN is_valid_coords(const Pos& position) noexcept -> bool
{
    const int x = position.x;
    const int y = position.y;
    return ((x >= 0) && (x < HEIGHT) && (y >= 0) && (y < WIDTH));
}

FN is_valid_coords(const Input& input) noexcept -> bool
{
    return is_valid_coords(input.from) && is_valid_coords(input.target);
}

/// FIXME rearrange methods
class ChessGame
{
    /*  x-axis from top to bottom (↓)  **
    **  y-axis from left to right (→)  */
    using shift_broom = std::vector<std::vector<Pos>>;

    bool idw_{ true };
    std::map<Id, bool> castling_;
    std::map<FigureType, shift_broom> moves_;
    std::map<FigureType, shift_broom> eats_;

    MoveLogger move_logger_{};
    FigureBoard board_{};

public:
    [[nodiscard]] explicit ChessGame(board_repr::BoardRepr&& board_repr) noexcept { reset(std::move(board_repr)); }

    [[nodiscard]] explicit ChessGame() noexcept = default;

    [[nodiscard]] explicit ChessGame(const ChessGame& other) noexcept
    {
        idw_ = other.idw_;
        moves_ = other.moves_;
        eats_ = other.eats_;
        move_logger_ = other.move_logger_;
        castling_ = other.castling_;
        for (const auto& [pos, fig] : other.board_.figures) {
            board_.figures.insert({ pos, figfab::FigureFabric::instance().create(fig, true).release() });
        }
        for (const auto& fig : other.board_.captured_figures) {
            board_.captured_figures.push_back(figfab::FigureFabric::instance().create(fig, true).release());
        }
    }

    ChessGame(ChessGame&&) noexcept = delete;
    ChessGame& operator=(ChessGame&&) noexcept = delete;
    ChessGame& operator=(ChessGame const&) noexcept = delete;

    ~ChessGame() noexcept = default;

    FN get_column_by_name(char name) const noexcept
    {
        if ('A' <= name && name <= 'Z') {
            name -= 'A' - 'a';
        }
        return idw_ ? name - 'a' : WIDTH - (name - 'a') - 1;
    }

    void reset(board_repr::BoardRepr&& map) noexcept
    {
        move_logger_.reset();
        board_.clear();
        reset_castling(map);
        apply_map(std::move(map));
        init_figures_moves();
    }

    void reset_move_logger() noexcept { move_logger_.reset(); }

    ChessGame& operator=(board_repr::BoardRepr&& board_repr) noexcept
    {
        reset(std::move(board_repr));
        return *this;
    }

    void operator=(const board_repr::BoardRepr&) noexcept = delete;

    void apply_map(board_repr::BoardRepr&& board_repr) noexcept
    {
        idw_ = board_repr.idw;
        move_logger_.set_past(board_repr.past);
        move_logger_.set_future(board_repr.future);
        for (const auto& cap_fig : board_repr.captured_figures) {
            board_.captured_figures.push_back(cap_fig); /* move ownership */
        }
        board_repr.captured_figures.clear(); /* delete ownership */
        for (const auto& fig : board_repr.figures) {
            board_.figures[fig->get_pos()] = fig; /* move ownership */
        }
        board_repr.figures.clear(); /* delete ownership */
    }

    template <typename Self, typename T>
    FN get_fig(this Self&& self, T&& value) noexcept
    {
        return std::forward<Self>(self).board_.get_fig(std::forward<T>(value));
    }

    template <typename Self, typename T>
    FN get_fig_unsafe(this Self&& self, T&& value) noexcept
    {
        return std::forward<Self>(self).board_.get_fig_unsafe(std::forward<T>(value));
    }

    [[nodiscard]] bool cont_fig(const Pos& pos) const noexcept { return board_.cont_fig(pos); }

    [[nodiscard]] bool is_empty(const Pos& pos) const noexcept { return !cont_fig(pos); }

    [[nodiscard]] bool is_empty() const noexcept { return board_.figures.empty(); }

    template <typename T>
    bool capture_figure(T&& value)
    {
        return board_.capture_figure(std::forward<T>(value));
    }

    template <typename T>
    void recapture_figure(T&& value)
    {
        board_.recapture_figure(std::forward<T>(value));
    }

    template <typename T>
    void delete_fig(T&& value)
    {
        board_.delete_fig(std::forward<T>(value));
    }

    void place_fig(Figure* const fig) { board_.place_fig(fig); }

    [[nodiscard]] auto find_king(const Color col) const noexcept -> std::optional<const Figure*>
    {
        return board_.find_king(col);
    }

    [[nodiscard]] auto get_figures_of(const Color col) const noexcept -> std::vector<Figure*>
    {
        return board_.get_figures_of(col);
    }

    /**
     * \brief Get all moves of figure using broom of figure type
     * \param in_hand target figure
     * \param to_ignore figures to ignore
     * \param ours figures, what can't be eaten
     * \param enemies figures, what can be eaten
     * \return vector of (is_eat, target cell)
     */
    [[nodiscard]] auto expand_broom(
        const Figure* in_hand,
        const std::vector<Pos>& to_ignore = {},
        const std::vector<Pos>& ours = {},
        const std::vector<Pos>& enemies = {}
    ) const noexcept -> std::vector<std::pair<bool, Pos>>
    {
        std::vector<std::pair<bool, Pos>> possible_moves{};    // list { pair{ is_eat, targ }, ... }
        const auto in_hand_pos = in_hand->get_pos();
        for (const auto in_hand_type = in_hand->get_type(); const std::vector<Pos>& eat_beams : eats_.at(in_hand_type))
        {
            for (Pos eat : eat_beams) {
                Pos curr{ in_hand_pos + (in_hand->is_col(Color::White) ? eat : eat.mul_x(-1)) };
                if (!is_valid_coords(curr))
                    continue;
                if (curr.in(ours)) {
                    break;    // Bumped into an ally
                }
                else if (curr.in(enemies)) {
                    possible_moves.emplace_back(true, curr);    // Ate the enemy
                    break;                                      // And bumped into...
                }
                else if (cont_fig(curr) && !curr.in(to_ignore)) {
                    if (!get_fig(curr).value()->is_col(in_hand->get_col())) {
                        possible_moves.emplace_back(true, curr);
                        break;    // Bumped
                    }
                    else {
                        break;    // Bumped
                    }
                }
            }
        }
        for (const auto& move_beams : moves_.at(in_hand->get_type())) {
            for (Pos move : move_beams) {
                Pos curr{ in_hand_pos + (in_hand->is_col(Color::White) ? move : move.mul_x(-1)) };
                if (not is_valid_coords(curr))
                    continue;
                if (not((not curr.in(to_ignore) && cont_fig(curr)) || curr.in(ours) || curr.in(enemies))) {
                    possible_moves.emplace_back(false, curr);
                }
                else {
                    break;    // Bumped
                }
            }
        }

        return possible_moves;
    }

    /**
     * \brief Get all moves of figure including special cases
     * \param in_hand target figure
     * \param to_ignore figures to ignore
     * \param ours figures, what can't be eaten
     * \param enemies figures, what can be eaten
     * \return vector of (is_eat, target cell)
     */
    [[nodiscard]] auto get_all_moves(
        const Figure* in_hand,
        const std::vector<Pos>& to_ignore = {},
        const std::vector<Pos>& ours = {},
        const std::vector<Pos>& enemies = {}
    ) const noexcept -> std::vector<std::pair<bool, Pos>>
    {
        std::vector<std::pair<bool, Pos>> all_moves{ expand_broom(in_hand, to_ignore, ours, enemies) };
        const Pos in_hand_pos = in_hand->get_pos();
        if (in_hand->is(FigureType::Pawn)) {
            // Pawn moves 2 (check my figures on 2 line)
            if (in_hand->is_col(Color::White)) {
                if (in_hand_pos.x == (HEIGHT - 2) && idw_ && is_empty(in_hand_pos + Pos(-1, 0)) &&
                    is_empty(in_hand_pos + Pos(-2, 0)))
                {
                    all_moves.emplace_back(false, in_hand_pos + Pos(-2, 0));
                }
                if (in_hand_pos.x == 1 && not idw_ && is_empty(in_hand_pos + Pos(1, 0)) &&
                    is_empty(in_hand_pos + Pos(2, 0)))
                {
                    all_moves.emplace_back(false, in_hand_pos + Pos(2, 0));
                }
            }
            if (in_hand->is_col(Color::Black)) {
                if (in_hand_pos.x == 1 && idw_ && is_empty(in_hand_pos + Pos(1, 0)) &&
                    is_empty(in_hand_pos + Pos(2, 0)))
                {
                    all_moves.emplace_back(false, in_hand_pos + Pos(2, 0));
                }
                if (in_hand_pos.x == (HEIGHT - 2) && not idw_ && is_empty(in_hand_pos + Pos(-1, 0)) &&
                    is_empty(in_hand_pos + Pos(-2, 0)))
                {
                    all_moves.emplace_back(false, in_hand_pos + Pos(-2, 0));
                }
            }

            // En passant
            if (const auto& last_move_sus = move_logger_.get_last_move(); last_move_sus.has_value()) {
                const auto& last_move = last_move_sus.value();
                if (const Pos& who_went_at_last_move_pos = last_move.first.get_pos();
                    std::holds_alternative<mvmsg::LongMove>(last_move.main_event) &&
                    std::abs(who_went_at_last_move_pos.y - in_hand_pos.y) == 1)
                {
                    const int shift_y = who_went_at_last_move_pos.y - in_hand_pos.y;
                    if (in_hand->is_col(Color::White)) {
                        if (in_hand_pos.x == (EN_PASSANT_INDENT - 1) && idw_ &&
                            cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(-1, shift_y)))
                        {
                            all_moves.emplace_back(true, in_hand_pos + Pos(-1, shift_y));
                        }
                        if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && not idw_ &&
                            cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(1, shift_y)))
                        {
                            all_moves.emplace_back(true, in_hand_pos + Pos(1, shift_y));
                        }
                    }
                    if (in_hand->is_col(Color::Black)) {
                        if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && idw_ &&
                            cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(1, shift_y)))
                        {
                            all_moves.emplace_back(true, in_hand_pos + Pos(1, shift_y));
                        }
                        if (in_hand_pos.x == (EN_PASSANT_INDENT - 1) && not idw_ &&
                            cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(-1, shift_y)))
                        {
                            all_moves.emplace_back(true, in_hand_pos + Pos(-1, shift_y));
                        }
                    }
                }
            }
        }

        using TripleInt = std::tuple<int, int, int>;
        using SomePair = std::pair<FigureType, std::initializer_list<TripleInt>>;
        for (const auto& [type, cols] :
             { SomePair{ FigureType::King,
                         { TripleInt{ get_column_by_name('g'), get_column_by_name('g'), get_column_by_name('f') },
                           TripleInt{ get_column_by_name('c'), get_column_by_name('c'), get_column_by_name('d') } } },
               SomePair{ FigureType::Rook,
                         { TripleInt{ get_column_by_name('f'), get_column_by_name('g'), get_column_by_name('f') },
                           TripleInt{ get_column_by_name('d'), get_column_by_name('c'), get_column_by_name('d') } } } })
        {
            if (in_hand->is(FigureType::King)) {
                for (const auto& [this_end_col, king_end_col, rook_end_col] : cols) {
                    if (const auto check_result_sus = castling_check(
                            in_hand,
                            Input{ in_hand_pos, Pos{ in_hand_pos.x, this_end_col } },
                            king_end_col,
                            rook_end_col
                        ))
                    {
                        if (const auto& [rook, king, second_figure_to_move] = check_result_sus.value();
                            has_castling(rook->get_id()))
                        {
                            all_moves.push_back({ false, Pos{ in_hand_pos.x, this_end_col } });
                        }
                    }
                }
            }
        }

        return all_moves;
    }

    /**
     * \brief Get all moves of figure with check check
     * \param in_hand target figure
     * \param to_ignore figures to ignore
     * \param ours figures, what can't be eaten
     * \param enemies figures, what can be eaten
     * \return vector of (is_eat, target cell)
     */
    [[nodiscard]] auto get_all_possible_moves(
        const Figure* in_hand,
        const std::vector<Pos>& to_ignore = {},
        const std::vector<Pos>& ours = {},
        const std::vector<Pos>& enemies = {}
    ) const noexcept -> std::vector<std::pair<bool, Pos>>
    {
        // FIXME to_ignore is not used
        std::vector<std::pair<bool, Pos>> all_possible_moves;

        for (const auto& move : get_all_moves(in_hand)) {
            auto moved_fig = figfab::FigureFabric::instance().submit_on(in_hand, move.second);
            const std::vector<Pos> ignore =
                move.first ? std::vector{ move.second, in_hand->get_pos() } : std::vector{ in_hand->get_pos() };
            if (const Pos to_defend = in_hand->is(FigureType::King) ? move.second : Pos{};
                //                                            extra argument when checking the king ↓
                !check_for_when(in_hand->get_col(), ignore, to_defend, { moved_fig.get() }))
            {
                all_possible_moves.push_back(move);
            }
        }

        return all_possible_moves;
    }

    /**
     * \brief Check board for checkmate
     * \param col target color
     * \param to_ignore figures to ignore
     * \param to_defend position of figure to defend (may not be king)
     * \return is checkmate
     */
    [[nodiscard]] auto
    checkmate_for(const Color col, const std::vector<Pos>& to_ignore = {}, Pos to_defend = Pos{}) const noexcept -> bool
    {
        const auto king = find_king(col);
        if (to_defend == Pos()) {
            if (king.has_value()) {
                to_defend = (*king)->get_pos();
            }
            else {
                return false;    // Nothing to defend
            }
        }
        for (const auto& figure : get_figures_of(col)) {
            for (const auto& [is_eat, curr] : expand_broom(figure, to_ignore, { to_defend })) {
                if (figure->is(FigureType::King)) {
                    if (is_eat ? !check_for_when(col, to_ignore + figure->get_pos() + curr + to_defend, curr)
                               : !check_for_when(col, to_ignore + figure->get_pos() + to_defend, curr))
                    {
                        return false;
                    }
                }
                else {
                    // FIXME move !check from end for readability
                    auto tmp = figfab::FigureFabric::instance().submit_on(figure, curr);
                    if (const bool check =
                            is_eat ? check_for_when(col, to_ignore + figure->get_pos() + curr, to_defend, { tmp.get() })
                                   : check_for_when(col, to_ignore + figure->get_pos(), to_defend, { tmp.get() });
                        !check)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    /**
     * \brief Check board for stalemate
     * \param col target color
     * \param to_ignore figures to ignore
     * \param to_defend position of figure to defend (FIXME may not be king?)
     * \return is stalemate
     */
    [[nodiscard]] auto
    stalemate_for(const Color col, const std::vector<Pos>& to_ignore = {}, Pos to_defend = Pos{}) const noexcept -> bool
    {
        const auto king_sus = find_king(col);
        if (!king_sus.has_value()) {
            return false;    // No king
        }
        const auto king = *king_sus;
        if (to_defend == Pos())
            to_defend = king->get_pos();
        for (const auto& aspt : get_figures_of(col)) {
            for (const auto& [is_eat, curr] : expand_broom(aspt)) {
                if (aspt->is(FigureType::King)) {
                    if (is_eat ? not check_for_when(col, to_ignore + curr + to_defend, curr)
                               : not check_for_when(col, to_ignore + to_defend, curr))
                    {
                        return false;
                    }
                }
                else {
                    auto tmp = figfab::FigureFabric::instance().submit_on(aspt, curr);
                    if (const bool check =
                            is_eat
                                ? check_for_when(col, to_ignore + aspt->get_pos() + curr, to_defend, { tmp.get() }, {})
                                : check_for_when(col, to_ignore + aspt->get_pos(), to_defend, { tmp.get() }, {});
                        not check)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    /**
     * \brief Check board for check
     * \param col target color
     * \param to_ignore figures to ignore
     * \param to_defend position of figure to defend (FIXME may not be king?)
     * \param ours figures, what blocks check
     * \param enemies figures, what checks.
     * \return is check
     */
    [[nodiscard]] auto check_for_when(
        const Color col,
        const std::vector<Pos>& to_ignore = {},
        Pos to_defend = Pos{},
        const std::vector<Figure*>& ours = {},
        const std::vector<Figure*>& enemies = {}
    ) const noexcept -> bool
    {
        const auto king_sus = find_king(col);
        if (to_defend == Pos()) {
            if (king_sus.has_value()) {
                to_defend = (*king_sus)->get_pos();
            }
            else {
                return false;    // Nothing to defend
            }
        }
        for (const auto& enemy : get_figures_of(what_next(col)) + enemies) {
            if (std::ranges::find(to_ignore, enemy->get_pos()) != to_ignore.end()) {
                if (std::ranges::find(enemies, enemy) ==
                    enemies.end())    // While not needed, but should be here just in case.
                    continue;
            }
            for (const auto& [is_eat, curr] :
                 expand_broom(enemy, to_ignore, to_pos_vector(enemies), to_pos_vector(ours) + to_defend))
            {
                if (is_eat && curr == to_defend) {
                    return true;
                }
            }
        }
        return false;    // No one attacks.
    }

    /**
     * \brief Determines move and check for checkmate
     * \param in_hand Figure to move
     * \param input User input
     * \param promotion_choice Type of figure to promote pawns
     */
    [[nodiscard]] auto
    move_check(const Figure* const in_hand, const Input& input, const FigureType promotion_choice) const noexcept
        -> std::expected<mvmsg::MoveMessage, ErrorEvent>
    {
        if (!(is_valid_coords(input.from) && is_valid_coords(input.target)) || !cont_fig(input.from)) {
            return std::unexpected{ ErrorEvent::InvalidMove };
        }

        auto move_sus = determine_move(in_hand, input, promotion_choice);

        if (!move_sus.has_value()) {
            return std::unexpected{ ErrorEvent::InvalidMove };
        }

        /// Crutch... rook castlings must be in Figure field.
        if (!std::holds_alternative<mvmsg::Castling>(move_sus->main_event)) {
            if (const auto rook_sus = get_fig(move_sus->input.target);
                rook_sus && rook_sus.value()->is(FigureType::Rook))
            {
                move_sus->side_evs.emplace_back(mvmsg::CastlingBreak{ rook_sus.value()->get_id() });
            }
        }

        ChessGame next_board{ *this };

        next_board.provide_move(*move_sus);

        if (next_board.check_for_when(in_hand->get_col())) {
            return std::unexpected{ ErrorEvent::UnderCheck };
        }
        if (next_board.check_for_when(what_next(in_hand->get_col()))) {
            move_sus->side_evs.emplace_back(mvmsg::Check{});
        }

        return *move_sus;
    }

    /**
     * \brief Helper type_ for ChessGame::castling_check
     */
    struct CastlingCheckResult
    {
        const Figure* rook;
        const Figure* king;
        Input second_figure_to_move;    // for rook or king
    };

    /**
     * \brief Castling validity check
     * \details Castling like in 960 (width must be 8).
     * Rules:
     * 0. Target rook and king cols is f-g or c-d
     * 1. King and rook must not be moved before
     * 2. Fields between start and end positions of king and rook are empty
     * 3. King does not pass through a broken field, is not under check and does not stand under it
     * 4. Rook can be under check
     * 5. King may have been under check before
     * 6. Castling resets all castlings for target player color
     * \param in_hand Figure, which is going to move (may be rook or king)
     * \param input Input for check
     * \param king_end_col Target column for king
     * \param rook_end_col Target column for rook
     * \return If castling is valid, returns king and rook figures, and input for second figure to move
     */
    [[nodiscard]] auto
    castling_check(const Figure* in_hand, const Input& input, const int king_end_col, const int rook_end_col)
        const noexcept -> std::optional<CastlingCheckResult>
    {
        if (board_.empty()) {
            return std::nullopt;
        }
        const auto king_sus = in_hand->is(FigureType::King) ? in_hand : find_king(in_hand->get_col());
        if (!king_sus.has_value()) {
            return std::nullopt;
        }
        const auto king = *king_sus;
        const auto king_start_pos = king->get_pos();
        // Is input correct
        if ((in_hand->is(FigureType::King) && input.target.y == king_end_col) ||
            (in_hand->is(FigureType::Rook) && input.target.y == rook_end_col))
        {
            const auto rook_sus = [&]() -> std::optional<const Figure*> {
                if (in_hand->is(FigureType::Rook)) {
                    return in_hand;
                }
                else {
                    const auto bound{ king_end_col > king_start_pos.y ? WIDTH : 0 };

                    for (const auto rook_aspt_y : between(king_start_pos.y, bound)) {
                        const auto rook_aspt_pos = Pos{ king_start_pos.x, rook_aspt_y };
                        if (auto rook_sus = (*this).get_fig(rook_aspt_pos);
                            rook_sus && rook_sus.value()->is_col(in_hand->get_col()) &&
                            rook_sus.value()->is(FigureType::Rook))
                        {
                            return *rook_sus;
                        }
                    }
                }
                return std::nullopt;
            }();

            if (!rook_sus) {
                return std::nullopt;
            }

            const auto rook = *rook_sus;

            // king and rook xs must be in input's xs
            if (king_start_pos.x != input.target.x && rook->get_pos().x != input.target.x) {
                return std::nullopt;
            }

            // valid rook and king relative positions
            if ((rook->get_pos().y > king->get_pos().y) == (rook_end_col > king_end_col)) {
                return std::nullopt;
            }

            const auto rook_start_pos = rook->get_pos();
            const auto rook_end_pos = Pos{ rook_start_pos.x, rook_end_col };
            const auto king_end_pos = Pos{ king_start_pos.x, king_end_col };

            for (const auto king_aspt_y : between(king_start_pos.y, king_end_pos.y)) {
                const auto king_aspt_pos = Pos{ king_start_pos.x, king_aspt_y };
                if (check_for_when(in_hand->get_col(), { king_start_pos, rook_start_pos }, king_aspt_pos)) {
                    return std::nullopt;
                }
            }

            for (const auto& left_right =
                     std::minmax({ king_start_pos.y, king_end_pos.y, rook_start_pos.y, rook_end_pos.y });
                 const auto cell_aspt_y : between(left_right.first, left_right.second))
            // COMPILER BUG using variables from structured binding in init statement in range statement does not
            // compile (VS 17.5.4). e.g: for (cnst auto& [l, r] = minmax(...); cnst auto a : between(l, r)) {...}
            {
                auto cell_sus = get_fig(Pos{ king_start_pos.x, cell_aspt_y });
                if (cell_sus && !cell_sus.value()->is(king->get_id()) && !cell_sus.value()->is(rook->get_id())) {
                    return std::nullopt;
                }
            }

            return CastlingCheckResult{ rook,
                                        king,
                                        in_hand->is(FigureType::King)
                                            ? Input{ rook_start_pos, Pos{ rook_start_pos.x, rook_end_col } }
                                            : Input{ king_start_pos, Pos{ king_start_pos.x, king_end_col } } };
        }
        return std::nullopt;
    }

    void reset_castling(const bool castle_state = true) noexcept
    {
        castling_.clear();
        for (const Color& col : { Color::Black, Color::White }) {
            for (const auto& aspt_to_rook : get_figures_of(col)) {
                if (aspt_to_rook->is(FigureType::Rook)) {
                    castling_[aspt_to_rook->get_id()] = castle_state;
                }
            }
        }
    }

    void reset_castling(const board_repr::BoardRepr& board_repr) noexcept
    {
        reset_castling(false);
        for (Id castle_id : board_repr.can_castle) {
            castling_[castle_id] = true;
        }
    }

    FN get_idw() const noexcept -> bool { return idw_; }

    auto set_idw(const bool new_idw) noexcept -> void
    {
        idw_ = new_idw;
        init_figures_moves();
    }

    [[nodiscard]] auto get_all_figures() const -> std::vector<Figure*> { return board_.get_figures(); }

    void move_fig(Figure* fig, const Pos& to, const bool capture = true) { board_.move_fig(fig, to, capture); }

    auto move_fig(const Input& input, const bool capture = true) -> void
    {
        if (const auto fig = get_fig(input.from)) {
            move_fig(fig.value(), input.target, capture);
        }
    }

    /**
     * \brief Swap figures on board
     * \param fig1 figure 1
     * \param fig2 figure 2
     */
    void swap_fig(Figure* fig1, Figure* fig2) { board_.swap_fig(fig1, fig2); }

    [[nodiscard]] auto has_castling(const Id& id) const noexcept -> bool
    {
        if (castling_.contains(id))
            return castling_.at(id);
        return false;
    }

    void off_castling(const Id& id) noexcept { castling_[id] = false; }

    void on_castling(const Id& id) noexcept { castling_[id] = true; }

    FN get_last_moves() const noexcept -> const std::vector<mvmsg::MoveMessage>&
    {
        return move_logger_.get_past();
    }

    FN get_future_moves() const noexcept -> const std::vector<mvmsg::MoveMessage>&
    {
        return move_logger_.get_future();
    }

    FN get_last_move() const noexcept -> std::optional<mvmsg::MoveMessage>
    {
        return move_logger_.get_last_move();
    }

    template <typename Func>
        requires requires(Func func) {
            /// FIXME clang-format. put on one line: { func() } -> std::same_as<FigureType>;
            {
                func()
            } -> std::same_as<FigureType>;
        }
    FN
    provide_move(Figure* in_hand, const Input& input, Color turn, Func&& get_choice) noexcept
        -> std::optional<mvmsg::MoveMessage>
    {
        const auto move_message_sus = move_check(in_hand, input, get_choice());

        if (move_message_sus && provide_move(*move_message_sus)) {
            return *move_message_sus;
        }

        return std::nullopt;
    }

    /**
     * \brief Apply move message to board and set to move log
     * \param move_message Move message
     * \param set_to_log If false, move message will not be set to log
     */
    auto provide_move(const mvmsg::MoveMessage& move_message, const bool set_to_log = true) noexcept -> bool
    {
        const auto in_hand_sus = get_fig(move_message.first.get_id());
        if (!in_hand_sus.has_value())
            return false;
        const auto in_hand = in_hand_sus.value();

        visit_one(
            move_message.main_event,
            [&](const mvmsg::Move&) { move_fig(in_hand, move_message.input.target); },
            [&](const mvmsg::LongMove&) { move_fig(in_hand, move_message.input.target); },
            /* (auto to_capture_sus = get_fig(it);
             * to_capture_sus.has_value() && to_capture_sus.value()->get_pos() != input.target)
             * Was in Eat and EnPassant events */
            [&](const mvmsg::Eat& eat) { move_fig(in_hand, move_message.input.target); },
            [&](const mvmsg::EnPassant& en_passant) {
                move_fig(in_hand, move_message.input.target);
                capture_figure(en_passant.eaten);
            },
            /* (has_castling(ms.to_move.back().first)) */
            [&](const mvmsg::Castling& castling) {
                using Test = decltype(this);
                const auto who = (*this).get_fig_unsafe(castling.second_to_move);
                //               ^^^^^^^ "no matching overloaded function found" without this
                if (who->at(move_message.input.target)) {
                    swap_fig(in_hand, who);
                }
                move_fig(in_hand, move_message.input.target);
                move_fig(who, castling.second_input.target);
            }
        );

        for (const auto& side_event : move_message.side_evs) {
            visit_one(
                side_event,
                [&](const mvmsg::Promotion& promotion) { promotion_fig(in_hand, move_message.promotion_choice); },
                [&](const mvmsg::Check&) {
                    // do nothing
                },
                /* (!ms.what_castling_breaks.empty() && has_castling(ms.what_castling_breaks.back()) */
                [&](const mvmsg::CastlingBreak& castling_break) { off_castling(castling_break.whose); }
            );
        }

        if (set_to_log) {
            move_logger_.add(move_message);
        }

        return true;
    }

    auto undo_move() noexcept -> bool
    {
        if (move_logger_.prev_empty())
            return false;
        const mvmsg::MoveMessage move_message = move_logger_.move_last_to_future().value();
        const auto in_hand_sus = get_fig(move_message.first.get_id());
        if (!in_hand_sus.has_value())
            return false;
        const auto in_hand = in_hand_sus.value();

        visit_one(
            move_message.main_event,
            [&](const mvmsg::Move&) { move_fig(in_hand, move_message.input.from); },
            [&](const mvmsg::LongMove&) { move_fig(in_hand, move_message.input.from); },
            [&](const mvmsg::Eat& eat) {
                move_fig(in_hand, move_message.input.from);
                recapture_figure(eat.eaten);
            },
            [&](const mvmsg::EnPassant& en_passant) {
                // FIXME debug on rook with castling
                move_fig(in_hand, move_message.input.from);
                recapture_figure(en_passant.eaten);
            },
            [&](const mvmsg::Castling& castling) {
                const auto who = (*this).get_fig_unsafe(castling.second_to_move);
                if (who->at(move_message.input.from)) {
                    swap_fig(in_hand, who);
                }
                move_fig(in_hand, move_message.input.from);
                move_fig(who, castling.second_input.from);
            }
        );

        for (const auto& side_event : move_message.side_evs) {
            visit_one(
                side_event,
                [&](const mvmsg::Promotion& promotion) { promotion_fig(in_hand, FigureType::Pawn); },
                [&](const mvmsg::Check&) {
                    // do nothing
                },
                /* !ms.what_castling_breaks.empty() && !has_castling(ms.what_castling_breaks.back())
                 * This check was in the original code, but it seems to be redundant
                 * FIXME check it */
                [&](const mvmsg::CastlingBreak& castling_break) { on_castling(castling_break.whose); }
            );
        }
        return true;
    }

    auto restore_move() noexcept -> bool
    {
        if (const auto future_sus = move_logger_.pop_future_move()) {
            provide_move(future_sus.value(), false);
            move_logger_.add_without_reset(future_sus.value());
            return true;
        }
        return false;
    }

    void init_figures_moves() noexcept
    {
        std::vector<Pos> temp_left_up;
        std::vector<Pos> temp_left_down;
        std::vector<Pos> temp_right_up;
        std::vector<Pos> temp_right_down;
        std::vector<Pos> temp_left;
        std::vector<Pos> temp_right;
        std::vector<Pos> temp_up;
        std::vector<Pos> temp_down;

        for (int ij = 1; ij < std::min(WIDTH, HEIGHT); ++ij) {
            temp_left_up.emplace_back(-ij, -ij);
            temp_left_down.emplace_back(ij, -ij);
            temp_right_up.emplace_back(-ij, ij);
            temp_right_down.emplace_back(ij, ij);
            temp_left.emplace_back(0, -ij);
            temp_right.emplace_back(0, ij);
            temp_up.emplace_back(-ij, 0);
            temp_down.emplace_back(ij, 0);
        }

        // Set relative to white via idw
        moves_[FigureType::Pawn] = { { Pos{ (idw_ ? -1 : 1), 0 } } };
        eats_[FigureType::Pawn] = { { Pos{ (idw_ ? -1 : 1), -1 } }, { Pos{ (idw_ ? -1 : 1), 1 } } };

        moves_[FigureType::Knight] = {
            { Pos{ -1, 2 } }, { Pos{ -1, -2 } }, { Pos{ 1, -2 } }, { Pos{ 1, 2 } },
            { Pos{ 2, -1 } }, { Pos{ -2, -1 } }, { Pos{ 2, 1 } },  { Pos{ -2, 1 } },
        };
        eats_[FigureType::Knight] = moves_[FigureType::Knight];

        moves_[FigureType::King] = { { Pos{ -1, -1 } }, { Pos{ -1, 1 } }, { Pos{ -1, 0 } }, { Pos{ 0, -1 } },
                                     { Pos{ 0, 1 } },   { Pos{ 1, 1 } },  { Pos{ 1, -1 } }, { Pos{ 1, 0 } } };
        eats_[FigureType::King] = moves_[FigureType::King];

        moves_[FigureType::Bishop] = { temp_left_up, temp_left_down, temp_right_up, temp_right_down };
        eats_[FigureType::Bishop] = moves_[FigureType::Bishop];

        moves_[FigureType::Rook] = { temp_left, temp_right, temp_up, temp_down };
        eats_[FigureType::Rook] = moves_[FigureType::Rook];

        moves_[FigureType::Queen] = { temp_left_up, temp_left_down, temp_right_up, temp_right_down,
                                      temp_left,    temp_right,     temp_up,       temp_down };
        eats_[FigureType::Queen] = moves_[FigureType::Queen];
    }

    [[nodiscard]] auto game_end_check(const Color col) const noexcept -> GameEndType
    {
        if (checkmate_for(col))
            return GameEndType::Checkmate;
        if (stalemate_for(col))
            return GameEndType::Stalemate;
        if (insufficient_material())
            return GameEndType::InsufficientMaterial;
        if (move_logger_.is_fifty_move_rule_was_triggered())
            return GameEndType::FiftyRule;
        if (move_logger_.is_moves_repeat_rule_was_triggered())
            return GameEndType::MoveRepeat;
        return GameEndType::NotGameEnd;
    }

    static void promotion_fig(Figure* to_promote, const FigureType new_type) noexcept
    {
        const Id id = to_promote->get_id();
        const Color color = to_promote->get_col();
        const Pos position = to_promote->get_pos();
        figfab::FigureFabric::instance().create_in_place(position, color, new_type, id, to_promote);
    }

    [[nodiscard]] size_t cnt_of_figures() const noexcept { return board_.figures.size(); }

    /**
     * \brief Insufficient material check
     * \details Cases:
     * 1. King vs king
     * 2. King vs king and knight
     * 3. King vs king and bishop
     * 4. King and bishop vs king and bishops of same cell color
     * \return Is material insufficient for checkmate
     */
    [[nodiscard]] bool insufficient_material() const noexcept
    {
        const size_t size = cnt_of_figures();
        if (size <= 2)
            return true;
        if (size == 3 && std::ranges::find_if(board_.figures, [](const auto& it) {
                             return it.second->is(FigureType::Knight) || it.second->is(FigureType::Bishop);
                         }) != board_.figures.end())
            return true;
        size_t b_cell_bishops_cnt{};
        size_t w_cell_bishops_cnt{};
        for (const auto& fig : get_all_figures()) {
            if (fig->is(FigureType::Bishop))
                if ((fig->get_pos().x + fig->get_pos().y) % 2) {
                    ++b_cell_bishops_cnt;
                }
                else {
                    ++w_cell_bishops_cnt;
                }
            else if (!fig->is(FigureType::King)) {
                return false;    // Careful. Result based on assumption of default chess rules.
            }
        }
        return not(b_cell_bishops_cnt && w_cell_bishops_cnt);
    }

    [[nodiscard]] auto get_repr(const Color turn, const bool save_all_moves) const noexcept -> board_repr::BoardRepr
    {
        const auto& prev_moves = move_logger_.get_past();
        const auto& has_castlings = castling_ | std::views::filter([](const auto& pair) { return pair.second; }) |
                                    std::views::keys | std::ranges::to<std::vector<Id>>();
        return board_repr::BoardRepr{ board_.get_figures(),
                                      turn,
                                      idw_,
                                      save_all_moves       ? prev_moves
                                      : prev_moves.empty() ? std::vector<mvmsg::MoveMessage>{}
                                                           : std::vector{ prev_moves.back() },
                                      move_logger_.get_future(),
                                      board_.captured_figures,
                                      has_castlings };
    }

    auto determine_move(const Figure* in_hand, const Input& input, const FigureType promotion_choice) const noexcept
        -> std::optional<mvmsg::MoveMessage>
    {
        std::vector<mvmsg::SideEvent> side_events;

        // Castling breaks
        if (in_hand->is(FigureType::Rook) && has_castling(in_hand->get_id())) {
            side_events.emplace_back(mvmsg::CastlingBreak{ in_hand->get_id() });
        }
        else if (in_hand->is(FigureType::King)) {
            for (const auto& [id, has_castling] : castling_) {
                if (has_castling && (*get_fig(id))->is_col(in_hand)) {
                    side_events.emplace_back(mvmsg::CastlingBreak{ id });
                }
            }
        }

        // Pawn
        if (in_hand->is(FigureType::Pawn)) {
            // Promotion
            if (in_hand->is_col(Color::White) &&
                    (idw_ && input.target.x == 0 || !idw_ && input.target.x == (HEIGHT - 1)) ||
                in_hand->is_col(Color::Black) &&
                    (!idw_ && input.target.x == 0 || idw_ && input.target.x == (HEIGHT - 1)))
            {
                side_events.emplace_back(mvmsg::Promotion{});
            }

            // Long move
            const Pos shift{ input.target - input.from };
            // Pawn move by 2 (check my figures on 2nd line)
            if (shift.y == 0 && is_empty(input.target) &&
                (in_hand->get_col() == Color::White &&
                     (input.from.x == (HEIGHT - 2) && idw_ && shift.x == -2 && is_empty(input.from + Pos(-1, 0)) ||
                      input.from.x == 1 && !idw_ && shift.x == 2 && is_empty(input.from + Pos(1, 0))) ||
                 in_hand->is_col(Color::Black) &&
                     (input.from.x == 1 && idw_ && shift.x == 2 && is_empty(input.from + Pos(1, 0)) ||
                      input.from.x == (HEIGHT - 2) && !idw_ && shift.x == -2 && is_empty(input.from + Pos(-1, 0)))))
            {
                const auto in_hand_in_targ_tmp = figfab::FigureFabric::instance().submit_on(in_hand, input.target);
                if (check_for_when(
                        what_next(in_hand->get_col()), { input.from }, Pos{}, {}, { in_hand_in_targ_tmp.get() }
                    ))
                {
                    side_events.emplace_back(mvmsg::Check{});
                }
                return mvmsg::MoveMessage{ *in_hand, input, promotion_choice, mvmsg::LongMove{}, side_events };
            }

            // En passant move (check enemy figures on 4th line and prev. move)
            if (const auto& last_move_sus = move_logger_.get_last_move(); last_move_sus.has_value()) {
                const auto& last_move = last_move_sus.value();
                if (const Pos& who_went_at_last_move_pos = last_move.first.get_pos();
                    std::abs(shift.y) == 1 && is_empty(input.target) &&
                    std::holds_alternative<mvmsg::LongMove>(last_move.main_event) &&
                    who_went_at_last_move_pos.y == input.target.y &&
                    (
                        // maybe can be simplified into one condition
                        in_hand->is_col(Color::White) && (input.from.x == (EN_PASSANT_INDENT - 1) && idw_ &&
                                                              shift.x == -1 && cont_fig(input.from + Pos(0, shift.y)) ||
                                                          input.from.x == (HEIGHT - EN_PASSANT_INDENT) && !idw_ &&
                                                              shift.x == 1 && cont_fig(input.from + Pos(0, shift.y))) ||
                        in_hand->is_col(Color::Black) && (input.from.x == (HEIGHT - EN_PASSANT_INDENT) && idw_ &&
                                                              shift.x == 1 && cont_fig(input.from + Pos(0, shift.y)) ||
                                                          input.from.x == (EN_PASSANT_INDENT - 1) && !idw_ &&
                                                              shift.x == -1 && cont_fig(input.from + Pos(0, shift.y)))
                    ))
                {
                    const auto in_hand_in_targ_tmp = figfab::FigureFabric::instance().submit_on(in_hand, input.target);
                    if (check_for_when(
                            what_next(in_hand->get_col()),
                            { input.from, Pos{ input.from.x, input.target.y }, input.target },
                            Pos{},
                            {},
                            { in_hand_in_targ_tmp.get() }
                        ))
                    {
                        side_events.emplace_back(mvmsg::Check{});
                    }
                    auto const& to_eat_sus = get_fig(Pos{ input.from.x, input.target.y });
                    return mvmsg::MoveMessage{
                        *in_hand, input, promotion_choice, mvmsg::EnPassant{ to_eat_sus.value()->get_id() }, side_events
                    };
                }
            }
        }

        const auto targ_sus = get_fig(input.target);
        for (const auto& [is_eat, curr] : expand_broom(in_hand)) {
            if (curr != input.target)
                continue;
            if (is_eat && targ_sus.has_value()) {
                const auto in_hand_in_curr_tmp = figfab::FigureFabric::instance().submit_on(in_hand, curr);
                // Figure what we are standing on is still counted!
                if (check_for_when(
                        what_next(in_hand->get_col()),
                        { input.from, input.target },
                        Pos{},
                        {},
                        { in_hand_in_curr_tmp.get() }
                    ))
                {
                    side_events.emplace_back(mvmsg::Check{});
                }
                return mvmsg::MoveMessage{
                    *in_hand, input, promotion_choice, mvmsg::Eat{ targ_sus.value()->get_id() }, side_events
                };
            }
            else if (!is_eat && !targ_sus.has_value()) {
                const auto in_hand_in_curr_tmp = figfab::FigureFabric::instance().submit_on(in_hand, curr);
                // FIXME {input.from} without input.target?? see prev check_for_when
                // FIXME double check for when? see move_check
                if (check_for_when(
                        what_next(in_hand->get_col()), { input.from }, Pos{}, {}, { in_hand_in_curr_tmp.get() }
                    ))
                {
                    side_events.emplace_back(mvmsg::Check{});
                }
                return mvmsg::MoveMessage{ *in_hand, input, promotion_choice, mvmsg::Move{}, side_events };
            }
        }

        // Castling
        // Checks after brooms moves
        if ((in_hand->is(FigureType::King) || in_hand->is(FigureType::Rook)) && WIDTH == 8) {
            using PairInt = std::pair<int, int>;

            for (auto [king_end_col, rook_end_col] : { PairInt{ get_column_by_name('g'), get_column_by_name('f') },
                                                       PairInt{ get_column_by_name('c'), get_column_by_name('d') } })
            {
                if (const auto check_result_sus = castling_check(in_hand, input, king_end_col, rook_end_col)) {
                    if (const auto& [rook, king, second_figure_to_move] = check_result_sus.value();
                        has_castling(rook->get_id()))
                    {
                        const auto king_tmp =
                            figfab::FigureFabric::instance().submit_on(king, Pos{ in_hand->get_pos().x, king_end_col });
                        const auto rook_tmp =
                            figfab::FigureFabric::instance().submit_on(rook, Pos{ rook->get_pos().x, rook_end_col });
                        // FIXME debug this branch (move_message.push and check after success castling)
                        if (check_for_when(
                                what_next(in_hand->get_col()),
                                { input.from, king->get_pos(), rook->get_pos() },
                                Pos{},
                                {},
                                { king_tmp.get(), rook_tmp.get() }
                            ))
                        {
                            side_events.emplace_back(mvmsg::Check{});
                        }
                        return mvmsg::MoveMessage{ *in_hand,
                                                   input,
                                                   promotion_choice,
                                                   mvmsg::Castling{
                                                       (in_hand->is(FigureType::King) ? rook : king)->get_id(),
                                                       second_figure_to_move },
                                                   side_events };
                    }
                }
            }
        }

        return std::nullopt;
    }
};

}    // namespace chess_game

using chess_game::ChessGame, chess_game::is_valid_coords;
