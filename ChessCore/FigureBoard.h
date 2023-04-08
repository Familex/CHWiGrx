#pragma once

#include "stuff/stuff.hpp"
#include "structs/board_repr.hpp"
#include "structs/move_logger.hpp"

#include <variant>
#include <map>

namespace figboard {

    constexpr int EN_PASSANT_INDENT = 4;

    class FigureBoard {
        /*  x-axis from top to bottom (↓)  **
        **  y-axis from left to right (→)  */
        using shift_broom = std::vector<std::vector<Pos>>;

        bool idw{ true };
        Id curr_id{ };
        MoveLogger move_logger{ };
        std::map<Pos, Figure*> figures;
        std::vector<Figure*> captured_figures;
        std::map<Id, bool> castling;
        std::map<FigureType, shift_broom> moves;
        std::map<FigureType, shift_broom> eats;

    public:
        [[nodiscard]] FigureBoard(board_repr::BoardRepr&&) noexcept;

        void reset(board_repr::BoardRepr&&) noexcept;

        void operator =(board_repr::BoardRepr&&) noexcept;

        void operator =(const board_repr::BoardRepr&) = delete;

        void apply_map(board_repr::BoardRepr&&) noexcept;

        FigureBoard(const FigureBoard&) = delete;

        [[nodiscard]] auto
            get_fig(Pos position) const noexcept -> std::optional<Figure*>
        {
            if (figures.find(position) != figures.end()) {
                return figures.at(position);
            }
            else {
                return std::nullopt;
            }
        }

        [[nodiscard]] auto
            get_fig(Id id) const noexcept -> std::optional<Figure*>
        {
            for (const auto& [_, fig] : figures) {
                if (fig->is(id)) {
                    return fig;
                }
            }
            return std::nullopt;
        }

        [[nodiscard]] Figure* get_fig_unsafe(const Pos) const noexcept;

        [[nodiscard]] Figure* get_fig_unsafe(const Id) const noexcept;

        [[nodiscard]] bool cont_fig(const Pos) const noexcept;

        [[nodiscard]] bool is_empty(const Pos) const noexcept;

        [[nodiscard]] bool is_empty() const noexcept
        {
            return figures.size() <= 1ull;
        }

        bool capture_figure(Figure* const);

        bool capture_figure(const Id);

        void uncapture_figure(const Id);

        void delete_fig(const Pos);

        void place_fig(Figure* const);

        [[nodiscard]] std::optional<const Figure*> find_king(const Color) const noexcept;

        [[nodiscard]] std::vector<Figure*> get_figures_of(const Color) const noexcept;

        [[nodiscard]] auto
            expand_broom(const Figure*, const std::vector<Pos> & = {},
                const std::vector<Pos> & = {},
                const std::vector<Pos> & = {}) const noexcept -> std::vector<std::pair<bool, Pos>>;

        [[nodiscard]] auto
            get_all_moves(const Figure*, const std::vector<Pos> & = {},
                const std::vector<Pos> & = {},
                const std::vector<Pos> & = {}) const noexcept -> std::vector<std::pair<bool, Pos>>;

        [[nodiscard]] auto
            get_all_possible_moves(const Figure*, const std::vector<Pos> & = {},
                const std::vector<Pos> & = {},
                const std::vector<Pos> & = {}) const noexcept -> std::vector<std::pair<bool, Pos>>;

        [[nodiscard]] auto
            checkmate_for(const Color, const std::vector<Pos> & = {}, const Pos = Pos{}) const noexcept -> bool;

        [[nodiscard]] auto
            stalemate_for(const Color, const std::vector<Pos> & = {}, Pos = Pos{}) const noexcept -> bool;

        [[nodiscard]] auto
            check_for_when(const Color, const std::vector<Pos> & = {},
                const Pos = Pos{}, const std::vector<Figure*> & = {},
                const std::vector<Figure*> & = {}) const noexcept -> bool;

        [[nodiscard]] auto
            move_check(const Figure* const, const Input&, const FigureType) const noexcept
            -> std::expected<mvmsg::MoveMessage, ErrorEvent>;

        /// <summary>
        /// Helper type for FigureBoard::castling_check
        /// </summary>
        struct CastlingCheckResult {
            const Figure* rook;
            const Figure* king;
            Input second_figure_to_move;    // for rook or king
        };

        [[nodiscard]] auto
            castling_check(const Figure*, const Input&, const int, const int) const noexcept
            -> std::optional<CastlingCheckResult>;

        auto
            reset_castling(const bool = true) noexcept -> void;

        auto
            reset_castling(const board_repr::BoardRepr&) noexcept -> void;

        FN get_idw() const noexcept -> bool {
            return idw;
        }

        inline auto
            set_idw(const bool new_idw) noexcept -> void
        {
            idw = new_idw;
            init_figures_moves();
        }

        [[nodiscard]] auto
            get_all_figures() const -> std::vector<Figure*>
        {
            std::vector<Figure*> tmp;
            for (const auto& [_, fig] : figures) {
                tmp.push_back(fig);
            }
            return tmp;
        }

        auto
            move_fig(Figure* fig, Pos to, bool capture = true) -> void
        {
            if (fig->get_pos() == to) {
                return;
            }
            if (auto maybe_eat = get_fig(to);
                maybe_eat.has_value()) {
                if (capture) {
                    capture_figure(maybe_eat.value());
                }
                else {
                    figures.erase(maybe_eat.value()->get_pos());
                    delete maybe_eat.value();
                }
            }
            figures.erase(fig->get_pos());
            fig->move_to(to);
            figures[fig->get_pos()] = fig;
        }

        auto inline
            move_fig(Input input, bool capture = true) -> void
        {
            auto fig = get_fig(input.from);
            if (fig.has_value()) {
                move_fig(fig.value(), input.target, capture);
            }
        }

        /// <summary>
        /// Swap figures on board
        /// </summary>
        auto inline
            swap_fig(Figure* fig1, Figure* fig2) -> void
        {
            figures.erase(fig1->get_pos());
            figures.erase(fig2->get_pos());
            
            Pos fig1_prev_pos = fig1->get_pos();
            fig1->move_to(fig2->get_pos());
            fig2->move_to(fig1_prev_pos);

            figures[fig1->get_pos()] = fig1;
            figures[fig2->get_pos()] = fig2;
        }

        [[nodiscard]] auto
            has_castling(Id id) const noexcept -> bool
        {
            if (castling.contains(id))
                return castling.at(id);
            return false;
        }

        void off_castling(Id id) noexcept {
            castling[id] = false;
        }

        void on_castling(Id id) noexcept {
            castling[id] = true;
        }

        FN get_last_moves() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
            return move_logger.get_past();
        }

        FN get_future_moves() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
            return move_logger.get_future();
        }

        FN get_last_move() const noexcept -> std::optional<mvmsg::MoveMessage> {
            return move_logger.get_last_move();
        }

        FN set_last_move(const mvmsg::MoveMessage& move_rec) noexcept -> void {
            this->move_logger.add(move_rec);
        }

        template <typename Func>
            requires requires(Func func) {
                { func() } -> std::same_as<FigureType>;
        }
        FN provide_move(Figure* in_hand,
            const Input& input,
            Color turn,
            Func&& get_choice) noexcept
            -> std::optional<mvmsg::MoveMessage>
        {
            const auto move_message_sus = move_check(in_hand, input, get_choice());
            
            if (!move_message_sus.has_value()) {
                return std::nullopt;
            }

            const auto& move_message = move_message_sus.value();

            if (!provide_move(move_message)) {
                return std::nullopt;
            }

            return move_message;
        }

        bool provide_move(const mvmsg::MoveMessage&);

        bool undo_move();

        bool restore_move();

        void place_figure(Figure* const fig) noexcept {
            figures[fig->get_pos()] = fig;
        }

        void init_figures_moves() noexcept;

        [[nodiscard]] GameEndType game_end_check(const Color) const noexcept;

        void promotion_fig(Figure*, const FigureType);

        [[nodiscard]] size_t cnt_of_figures() const noexcept {
            return figures.size();
        }

        [[nodiscard]] auto insufficient_material() const noexcept -> bool;

        [[nodiscard]] auto get_repr(const Color, const bool) const noexcept -> board_repr::BoardRepr;

        ~FigureBoard() noexcept {
            for (auto& [_, fig] : figures) {
                delete fig;
            }
            for (auto& fig : captured_figures) {
                delete fig;
            }
        }
    };

    FN is_valid_coords(const Pos position) noexcept -> bool
    {
        const int x = position.x;
        const int y = position.y;
        return ((x >= 0) && (x < HEIGHT) &&
            (y >= 0) && (y < WIDTH));
    }

}   // namespace figboard

using figboard::FigureBoard, figboard::is_valid_coords;
