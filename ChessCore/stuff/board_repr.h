#pragma once

#include "stuff.h"
#include "figure.hpp"
#include "move_rec.h"

#include <expected>

namespace board_repr {

    enum class ParseError {
        InvalidFormat,
        InvalidFigure,
        InvalidMove,
        InvalidCastling,
        InvalidCapturedFigure,
        CouldNotFindTurn,
        CouldNotFindIdw,
        InvalidPast,
        InvalidFuture,
        InvalidCanCastle,
        InvalidCapturedFigures,
        InvalidBoardRepr
    };

    /* Data Transfer Structure */
    struct BoardRepr {
        /* ---- Fields ---- */
        std::vector<Figure*> figures;
        Color turn{ Color::White };
        bool idw{ true };
        std::vector<moverec::MoveRec> past;
        std::vector<moverec::MoveRec> future;
        std::vector<Figure*> captured_figures;
        std::vector<Id> can_castle;

        /* ---- Methods ---- */
        [[nodiscard]] explicit BoardRepr(const BoardRepr&) noexcept;
        [[nodiscard]] explicit BoardRepr(
            std::vector<Figure*>&& figures,
            Color turn,
            bool idw,
            const std::vector<Id>& can_castle,
            const std::vector<moverec::MoveRec>& past = {},
            const std::vector<moverec::MoveRec>& future = {},
            const std::vector<Figure*>& captured_figures = {}) noexcept
            : figures(figures)
            , turn(turn)
            , idw(idw)
            , past(past)
            , future(future)
            , captured_figures(captured_figures)
            , can_castle(can_castle)
        {
        };

        /* Without castling (automatically set all to true) */
        [[nodiscard]] explicit BoardRepr(
            std::vector<Figure*>&& figures,
            Color turn,
            bool idw,
            const std::vector<moverec::MoveRec>& past = {},
            const std::vector<moverec::MoveRec>& future = {},
            const std::vector<Figure*>& captured_figures = {}) noexcept
            : figures(figures)
            , turn(turn)
            , idw(idw)
            , past(past)
            , future(future)
            , captured_figures(captured_figures) {
            // all can castle by default
            for (auto fig : figures) {
                if (fig->is(FigureType::Rook)) {
                    can_castle.push_back(fig->get_id());
                }
            }
        };

        [[nodiscard]] BoardRepr(BoardRepr&&) noexcept;

        BoardRepr* operator =(const BoardRepr& other) noexcept;

        void clear() noexcept;

        [[nodiscard]] static auto
            from_string(const std::string_view board_repr) noexcept -> std::expected<BoardRepr, ParseError>;

        [[nodiscard]] std::string as_string() const noexcept;

        [[nodiscard]] constexpr char get_idw_char() const noexcept
        {
            return idw ? 'T' : 'F';
        }

        [[nodiscard]] constexpr char get_turn_char() const noexcept;

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            return figures.empty();
        }

    private:
        [[nodiscard]] constexpr BoardRepr() noexcept = default;
    };

}   // namespace board_repr