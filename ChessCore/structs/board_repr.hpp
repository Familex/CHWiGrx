#pragma once

#include "figure.hpp"
#include "../stuff/stuff.hpp"
#include "move_message.hpp"
#include "figure_fabric.hpp"

#include <expected>

namespace board_repr {

    constexpr auto TO_STRING_VERSION = "02";

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
        std::vector<mvmsg::MoveMessage> past;
        std::vector<mvmsg::MoveMessage> future;
        std::vector<Figure*> captured_figures;
        std::vector<Id> can_castle;

        /* ---- Methods ---- */
        CTOR BoardRepr(const BoardRepr& other) noexcept
            : turn(other.turn)
            , idw(other.idw)
            , past(other.past)
            , future(other.future)
            , can_castle(other.can_castle)
        {
            for (auto fig : other.figures) {
                figures.push_back(figfab::FigureFabric::instance().create(fig, true).release());
            }
            for (auto fig : other.captured_figures) {
                captured_figures.push_back(figfab::FigureFabric::instance().create(fig, true).release());
            }
        }
        
        CTOR BoardRepr(
                 std::vector<Figure*>&& figures,
                 Color turn,
                 bool idw,
                 const std::vector<Id>& can_castle,
                 const std::vector<mvmsg::MoveMessage>& past = {},
                 const std::vector<mvmsg::MoveMessage>& future = {},
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
        CTOR BoardRepr(
                  std::vector<Figure*>&& figures,
                  Color turn,
                  bool idw,
                  const std::vector<mvmsg::MoveMessage>& past = {},
                  const std::vector<mvmsg::MoveMessage>& future = {},
                  const std::vector<Figure*>& captured_figures = {}) noexcept
            : figures(figures)
            , turn(turn)
            , idw(idw)
            , past(past)
            , future(future)
            , captured_figures(captured_figures)
        {
            // all can castle by default
            for (auto fig : figures) {
                if (fig->is(FigureType::Rook)) {
                    can_castle.push_back(fig->get_id());
                }
            }
        };
        
        CTOR BoardRepr() noexcept = default;
        
        CTOR BoardRepr(BoardRepr&& br) noexcept
        {
            clear();
            *this = std::move(br);
        }

        constexpr BoardRepr*
            operator =(const BoardRepr& other) noexcept
        {
            this->clear();
            for (const auto& fig : other.figures) {
                this->figures.push_back(figfab::FigureFabric::instance().create(fig, true).release());
            }
            for (const auto& fig : other.captured_figures) {
                this->captured_figures.push_back(figfab::FigureFabric::instance().create(fig, true).release());
            }
            future = other.future;
            turn = other.turn;
            idw = other.idw;
            past = other.past;
            can_castle = other.can_castle;
            return this;
        }

        FN clear() noexcept -> void
        {
            for (auto fig : figures) {
                delete fig;
            }
            figures.clear();
            for (auto fig : captured_figures) {
                delete fig;
            }
            captured_figures.clear();
            can_castle.clear();
            past.clear();
            future.clear();
        }

        FN static
            from_string(const std::string_view board_repr) noexcept -> std::expected<BoardRepr, ParseError>
        {
            BoardRepr result{};

            // FIXME inplement

            return std::expected<BoardRepr, ParseError>{ result };
        }

        FN as_string() const noexcept -> std::string
        {
            // FIXME castlings
            using namespace std::literals::string_literals;

            std::string result{ ""s };
            // Header
            {
                result += std::format("{}H{}W{}C", TO_STRING_VERSION, HEIGHT, WIDTH);
                for (const Id castle_id : can_castle) {
                    result += std::format("{},", castle_id);
                }
                result += std::format("{}{}!", get_idw_char(), get_turn_char());
            }
            // Figures
            for (const auto& fig : figures) {
                result += fig->as_string();
            }
            // Prev moves
            result += "<"s;
            for (const auto& mm : past) {
                result += mvmsg::as_string(mm) + "$"s;
            }
            // Future moves
            result += "><"s;
            for (const auto& mm : future) {
                result += mvmsg::as_string(mm) + "$"s;
            }
            // Captured figures
            result += ">"s;
            for (const auto& fig : captured_figures) {
                result += fig->as_string();
            }
            return result;
        }

        FN get_idw_char() const noexcept -> char
        {
            return idw ? 'T' : 'F';
        }

        FN get_turn_char() const noexcept -> char
        {
            return
                turn == Color::White ? 'W'
                : turn == Color::Black ? 'B'
                : 'E';
        }

        FN empty() const noexcept -> bool
        {
            return figures.empty();
        }
    };

}   // namespace board_repr