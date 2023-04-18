#pragma once

#include "stuff/stuff.hpp"
#include "stuff/enums.hpp"
#include "structs/board_repr.hpp"
#include "structs/move_logger.hpp"

#include <variant>
#include <map>
#include <ranges>

namespace figboard {

    constexpr int EN_PASSANT_INDENT = 4;

    // FIXME add constructors, assign operators, etc.
    class FigureBoard {
        /*  x-axis from top to bottom (↓)  **
        **  y-axis from left to right (→)  */
        using shift_broom = std::vector<std::vector<Pos>>;

        bool idw_{ true };
        MoveLogger move_logger_{ };
        std::map<Pos, Figure*> figures_;
        std::vector<Figure*> captured_figures_;
        std::map<Id, bool> castling_;
        std::map<FigureType, shift_broom> moves_;
        std::map<FigureType, shift_broom> eats_;

    public:
        [[nodiscard]] explicit FigureBoard(board_repr::BoardRepr&&) noexcept;

        CTOR FigureBoard() noexcept = default;

        void reset(board_repr::BoardRepr&&) noexcept;

        FigureBoard& operator =(board_repr::BoardRepr&&) noexcept;

        void operator =(const board_repr::BoardRepr&) noexcept = delete;

        void apply_map(board_repr::BoardRepr&&) noexcept;

        [[nodiscard]] explicit FigureBoard(const FigureBoard& other) noexcept {
            idw_ = other.idw_;
            moves_ = other.moves_;
            eats_ = other.eats_;
            move_logger_ = other.move_logger_;
            castling_ = other.castling_;
            for (const auto& [pos, fig] : other.figures_) {
                figures_.insert({ pos, figfab::FigureFabric::instance().create(fig, false).release() });
            }
            for (const auto& fig : other.captured_figures_) {
                captured_figures_.push_back(figfab::FigureFabric::instance().create(fig, false).release());
            }
        } 

        [[nodiscard]] auto
            get_fig(const Pos position) const noexcept -> std::optional<Figure*>
        {
            if (figures_.contains(position)) {
                return figures_.at(position);
            }
            else {
                return std::nullopt;
            }
        }

        [[nodiscard]] auto
            get_fig(const Id id) const noexcept -> std::optional<Figure*>
        {
            for (const auto& fig : figures_ | std::views::values) {
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
            return figures_.size() <= 1ull;
        }

        bool capture_figure(Figure* const);

        bool capture_figure(const Id);

        void recapture_figure(const Id);

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
        /// Helper type_ for FigureBoard::castling_check
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
            return idw_;
        }

        inline auto
            set_idw(const bool new_idw) noexcept -> void
        {
            idw_ = new_idw;
            init_figures_moves();
        }

        [[nodiscard]] auto
            get_all_figures() const -> std::vector<Figure*>
        {
            std::vector<Figure*> tmp;
            for (const auto& fig : figures_ | std::views::values) {
                tmp.push_back(fig);
            }
            return tmp;
        }

        auto
            move_fig(Figure* fig, const Pos to, const bool capture = true) -> void
        {
            if (fig->get_pos() == to) {
                return;
            }
            if (const auto maybe_eat = get_fig(to);
                maybe_eat.has_value()) {
                if (capture) {
                    capture_figure(maybe_eat.value());
                }
                else {
                    figures_.erase(maybe_eat.value()->get_pos());
                    delete maybe_eat.value();
                }
            }
            figures_.erase(fig->get_pos());
            fig->move_to(to);
            figures_[fig->get_pos()] = fig;
        }

        auto inline
            move_fig(const Input input, const bool capture = true) -> void
        {
            if (const auto fig = get_fig(input.from)) {
                move_fig(fig.value(), input.target, capture);
            }
        }

        /// <summary>
        /// Swap figures on board
        /// </summary>
        auto inline
            swap_fig(Figure* fig1, Figure* fig2) -> void
        {
            figures_.erase(fig1->get_pos());
            figures_.erase(fig2->get_pos());

            const Pos fig1_prev_pos = fig1->get_pos();
            fig1->move_to(fig2->get_pos());
            fig2->move_to(fig1_prev_pos);

            figures_[fig1->get_pos()] = fig1;
            figures_[fig2->get_pos()] = fig2;
        }

        [[nodiscard]] auto
            has_castling(const Id id) const noexcept -> bool
        {
            if (castling_.contains(id))
                return castling_.at(id);
            return false;
        }

        void off_castling(const Id id) noexcept {
            castling_[id] = false;
        }

        void on_castling(const Id id) noexcept {
            castling_[id] = true;
        }

        FN get_last_moves() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
            return move_logger_.get_past();
        }

        FN get_future_moves() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
            return move_logger_.get_future();
        }

        FN get_last_move() const noexcept -> std::optional<mvmsg::MoveMessage> {
            return move_logger_.get_last_move();
        }

        FN set_last_move(const mvmsg::MoveMessage& move_rec) noexcept -> void {
            this->move_logger_.add(move_rec);
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
            figures_[fig->get_pos()] = fig;
        }

        void init_figures_moves() noexcept;

        [[nodiscard]] GameEndType game_end_check(const Color) const noexcept;

        static void promotion_fig(Figure*, const FigureType);

        [[nodiscard]] size_t cnt_of_figures() const noexcept {
            return figures_.size();
        }

        [[nodiscard]] auto insufficient_material() const noexcept -> bool;

        [[nodiscard]] auto get_repr(const Color, const bool) const noexcept -> board_repr::BoardRepr;

        ~FigureBoard() noexcept {
            for (const auto& fig : figures_ | std::views::values) {
                delete fig;
            }
            for (const auto& fig : captured_figures_) {
                delete fig;
            }
        }

    private /* ---- Helper methods ---- */:
        auto determine_move( const Figure*, const Input&, const FigureType ) const noexcept
            -> std::optional<mvmsg::MoveMessage>;
    };

    FN is_valid_coords(const Pos position) noexcept -> bool {
        const int x = position.x;
        const int y = position.y;
        return ((x >= 0) && (x < HEIGHT) &&
            (y >= 0) && (y < WIDTH));
    }

    FN is_valid_coords(const Input input) noexcept -> bool {
        return is_valid_coords(input.from) && is_valid_coords(input.target);
    }

}   // namespace figboard

using figboard::FigureBoard, figboard::is_valid_coords;

