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
            BoardRepr result{};
#if 0
            const size_t npos = std::string::npos;
            size_t meta_start = board_repr.find('[');
            size_t meta_end = board_repr.find(']');
            std::string_view meta = board_repr.substr(meta_start + 1, meta_end - meta_start - 1);

            if (meta.find('t') != npos || meta.find('T') != npos) {
                result.idw = true;
            }
            else if (meta.find('f') != npos || meta.find('F') != npos) {
                result.idw = false;
            }
            else {
                return std::unexpected{ ParseError::CouldNotFindIdw };
            }

            if (meta.find('w') != npos || meta.find('W') != npos) {
                result.turn = Color::White;
            }
            else if (meta.find('b') != npos || meta.find('B') != npos) {
                result.turn = Color::Black;
            }
            else {
                return std::unexpected{ ParseError::CouldNotFindTurn };
            }

            for (const char& c : { 't', 'T', 'f', 'F', 'w', 'W', 'b', 'B' }) {
                remove_first_occurrence(meta, c);
            }
            // здесь значение meta содержит только информацию о возможных рокировках
            for (auto& castle_id : split(meta, ";")) {
                if (!castle_id.empty())
                    result.can_castle.push_back(std::stoi(castle_id));
            }
            board_repr.erase(board_repr.begin() + meta_start, board_repr.begin() + meta_end + 1);
            size_t past_start = board_repr.find('<');
            size_t past_end = board_repr.find('>');
            if (past_end - past_start > 2) {
                std::string past_to_parse = board_repr.substr(past_start + 1, past_end - past_start - 1);
                for (const std::string& mr_to_parse : split(past_to_parse, "$")) {
                    if (!mr_to_parse.empty() && mr_to_parse != " ")
                        result.past.push_back(mr_to_parse);
                }
            }
            board_repr.erase(board_repr.begin() + past_start, board_repr.begin() + past_end + 1);
            size_t future_start = board_repr.find('<');
            size_t future_end = board_repr.find('>');
            if (future_end - future_start > 2) {
                std::string future_to_parse = board_repr.substr(future_start + 1, future_end - future_start - 1);
                for (const std::string& mr_to_parse : split(future_to_parse, "$")) {
                    if (!mr_to_parse.empty() && mr_to_parse != " ")
                        result.future.push_back(mr_to_parse);
                }
            }
            board_repr.erase(board_repr.begin() + future_start, board_repr.begin() + future_end + 1);
            size_t deleted_start = board_repr.find('~');
            std::vector<std::string> tmp;
            for (const std::string& fig_piece : split(board_repr.substr(deleted_start + 1), ",")) {
                if (!fig_piece.empty() && fig_piece != " ")
                    tmp.push_back(fig_piece);
            }
            for (size_t i{}; i < tmp.size(); i += 5) {
                Id new_id = std::stoi(tmp[i]);
                Pos new_pos = { std::stoi(tmp[i + 1]), std::stoi(tmp[i + 2]) };
                Color new_col = char_to_col(tmp[i + 3][0]);
                FigureType new_type = char_to_figure_type(tmp[i + 4][0]);
                Figure* new_fig = FigureFabric::instance()->create(
                    new_pos, new_col, new_type, new_id
                );
                result.captured_figures.push_back(new_fig);
            }
            board_repr.erase(board_repr.begin() + deleted_start, board_repr.end());

            tmp.clear();
            for (const std::string& fig_piece : split(board_repr, ";")) {
                if (!fig_piece.empty() && fig_piece != " ")
                    tmp.push_back(fig_piece);
            }
            for (size_t i{}; i < tmp.size(); i += 5) {
                Id new_id = std::stoi(tmp[i]);
                Pos new_pos = { std::stoi(tmp[i + 1]), std::stoi(tmp[i + 2]) };
                Color new_col = char_to_col(tmp[i + 3][0]);
                FigureType new_type = char_to_figure_type(tmp[i + 4][0]);
                Figure* new_fig = FigureFabric::instance()->create(
                    new_pos, new_col, new_type, new_id
                );
                result.figures.push_back(new_fig);
            }
#endif
            return std::expected<BoardRepr, ParseError>{ result };
        }

        FN as_string() const noexcept -> std::string
        {
            std::string result{ "" };
            for (const auto& fig : figures) {
                auto pos = fig->get_pos();
                result += std::format("{};{};{};{};{};",
                    fig->get_id(), pos.x, pos.y, col_to_char(fig->get_col()), figure_type_to_char(fig->get_type())
                );
            }
            result += std::format("[{}{}", get_idw_char(), get_turn_char());
            for (const Id castle_id : can_castle) {
                result += std::format("{};", castle_id);
            }
            result += "]<";
            for (const auto& mr : past) {
                result += mr.as_string() + "$";
            }
            result += "><";
            for (const auto& mr : future) {
                result += mr.as_string() + "$";
            }
            result += ">~";
            for (const auto& fig : captured_figures) {
                result += std::format("{},{},{},{},{},",
                    fig->get_id(),
                    fig->get_pos().x,
                    fig->get_pos().y,
                    col_to_char(fig->get_col()),
                    figure_type_to_char(fig->get_type())
                );
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