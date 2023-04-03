#pragma once

#include "stuff.hpp"
#include "figure.hpp"
#include "move_rec.hpp"

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
        CTOR BoardRepr(
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
            const auto _2 =
                "H8W8CTW!"
                "1626WH1529BH1349WK1454BK"
                "<1609WH.09.26.M.Q.{}.{}.{}.{}$"
                    "1514BH.14.29.T.Q.{}.{17,}.{}.{}$>"
                "<1349WK.49.42.M.Q.{B,}.{}.{}.{}$>"
                    "1729WR";
            
            BoardRepr result{};



            return std::expected<BoardRepr, ParseError>{ result };
        }

        FN as_string() const noexcept -> std::string
        {
            using namespace std::literals::string_literals;

            std::string result{ ""s };
            // Header
            {
                result += std::format("H{}W{}C", HEIGHT, WIDTH);
                for (const Id castle_id : can_castle) {
                    result += std::format("{},", castle_id);
                }
                result += std::format("{}{}!", get_idw_char(), get_turn_char());
            }
            // Figures
            for (const auto& fig : figures) {
                auto pos = fig->get_pos();
                result += fig->as_string();
            }
            // Prev moves
            result += "<"s;
            for (const auto& mr : past) {
                result += mr.as_string() + "$"s;
            }
            // Future moves
            result += "><"s;
            for (const auto& mr : future) {
                result += mr.as_string() + "$"s;
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