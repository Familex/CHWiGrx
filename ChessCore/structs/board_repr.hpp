#pragma once

#include "figure.hpp"
#include "../stuff/stuff.hpp"
#include "move_message.hpp"
#include "figure_fabric.hpp"

#include <expected>

namespace board_repr {
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

        friend from_string<BoardRepr>;
        friend as_string<BoardRepr>;

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
        
        // implicit move constructor to easy use in std::expected
        [[nodiscard]] constexpr
            BoardRepr(BoardRepr&& br) noexcept
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

template <>
struct from_string<board_repr::BoardRepr> {
    enum class ParseErrorType {
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
    
    [[nodiscard]] inline auto
        operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseResult<board_repr::BoardRepr, ParseErrorType>
    {
        board_repr::BoardRepr result{};

        // FIXME inplement

        return result;
    }

    [[nodiscard]] inline auto
        operator()(const std::string_view sv) const noexcept
        -> ParseResult<board_repr::BoardRepr, ParseErrorType>
    {
        FromStringMeta meta{ };

        // FIXME implement
        meta.max_pos_length = 2;    

        return operator()(sv, meta);
    }
};

template <>
struct as_string<board_repr::BoardRepr> {
    [[nodiscard]] inline auto
        operator()(const board_repr::BoardRepr& br, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        // FIXME castlings
        using namespace std::literals::string_literals;

        std::string result{ ""s };
        // Header
        {
            result += std::format("{:0>2}H{}W{}C", meta.version, HEIGHT, WIDTH);
            for (const Id castle_id : br.can_castle) {
                result += std::format("{},", castle_id);
            }
            result += std::format("{}{}!", br.get_idw_char(), br.get_turn_char());
        }
        // Figures
        for (const auto& fig : br.figures) {
            result += as_string<const Figure*>{}(fig, meta);
        }
        // Prev moves
        result += "<"s;
        for (const auto& mm : br.past) {
            result += as_string<mvmsg::MoveMessage>{}(mm, meta) + "$"s;
        }
        // Future moves
        result += "><"s;
        for (const auto& mm : br.future) {
            result += as_string<mvmsg::MoveMessage>{}(mm, meta) + "$"s;
        }
        // Captured figures
        result += ">"s;
        for (const auto& fig : br.captured_figures) {
            result += as_string<const Figure*>{}(fig, meta);
        }
        return result;
    }

    [[nodiscard]] inline auto
        operator()(const board_repr::BoardRepr& br) const noexcept
        -> std::string
    {
        AsStringMeta meta{ };
        
        meta.version = 2;

        // calc min id in br.figures
        if (!br.figures.empty()) {
            meta.min_id = br.figures.front()->get_id();
            for (const auto& fig : br.figures) {
                if (fig->get_id() < meta.min_id) {
                    meta.min_id = fig->get_id();
                }
            }
        }
        
        meta.max_pos_length = std::to_string(HEIGHT * WIDTH - 1).length();
        
        return operator()(br, meta);
    }
};
