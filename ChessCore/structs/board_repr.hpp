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
        CouldNotFindMeta,
        InvalidVersion,
        CouldNotFindHeight,
        InvalidHeight,
        CouldNotFindWidth,
        InvalidWidth,
        CouldNotFindIDW,
        CouldNotFindCurrentTurn,
        InvalidCurrentTurn,
        CouldNotFindCastlings,
        InvalidCastling,
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
        std::size_t pos{ };

        auto parse_example =
            // meta
            "02H8W8TBC22,21,45,44!"
            // figures
            "6.00BR" "10.01BK" "7.02BB" "3.03BK"
            "8.05BB" "11.06BK" "5.07BR" "12.08BP"
            "14.10BP" "19.11BP" "18.12BP" "15.15BP"
            "16.21BP" "30.25WB" "4.31BQ" "22.45WP"
            "27.48WP" "26.49WP" "25.50WP" "24.51WP"
            "21.54WP" "20.55WP" "29.56WR" "34.57WK"
            "31.58WB" "1.59WK" "0.60WQ" "33.62WK"
            "28.63WR"
            // prev
            "<23.52WP5236QL$"
                "13.09BP0925QL$"
                "23.36WP3628QM$"
                "17.13BP1329QL$"
                "23.28WP2821QP17$"
                "16.14BP1421QE23$"
                "30.61WB6125QE13$"
                "4.04BQ0431QCHM$"
                "22.53WP5345QM$>"
            // future
            "<11.06BK0623QM$"
                "21.54WP5445QE4$"
                "4.31BQ3145QCHE22$>"
            // captured
            "17.29BP" "23.21WP" "13.25BP";

        const auto meta_end = sv.find('!');
        if (meta_end == std::string_view::npos) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindMeta, pos);
        }
        const auto sv_meta = sv.substr(0, meta_end - 1);

        if (const auto version = svtoi(sv_meta.substr(pos, 2))) {
            meta.version = *version;
            pos += 2;
        } else {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidVersion, version.error());
        }
        if (sv_meta[pos++] != 'H') {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindHeight, pos - 1);
        }
        auto w_pos = sv_meta.find('W', pos);
        if (w_pos == std::string_view::npos) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindWidth, pos - 1);
        }
        if (auto height = svtoi(sv_meta.substr(pos, w_pos - pos))) {
            meta.height = *height;
            pos = w_pos + 1;
        } else {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidHeight, height.error());
        }
        auto idw_pos = sv_meta.find('T', pos);
        if (idw_pos == std::string_view::npos) {
            idw_pos = sv_meta.find('F', pos);
            if (idw_pos == std::string_view::npos) {
                return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindIDW, pos - 1);
            }
            else {
                meta.idw = false;
            }
        }
        else {
            meta.idw = true;
        }
        if (auto width = svtoi(sv_meta.substr(pos, idw_pos - pos))) {
            meta.width = *width;
            pos = idw_pos + 1;
        }
        else {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidWidth, pos - 1);
        }
        if (sv_meta.size() == pos) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindCurrentTurn, pos - 1);
        }
        if (sv_meta[pos] != 'B' && sv_meta[pos] != 'W') {
            return UNEXPECTED_PARSE(ParseErrorType, InvalidCurrentTurn, pos);
        }
        meta.turn =
            sv_meta[pos] == 'B'
            ? Color::Black
            : Color::White;
        pos += 1;
        if (sv_meta.size() == pos) {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindCastlings, pos);
        }
        if (sv_meta[pos] != 'C') {
            return UNEXPECTED_PARSE(ParseErrorType, CouldNotFindCastlings, pos);
        }
        std::vector<Id> castlings{ };
        for (const auto& castling : split(sv_meta.substr(pos + 1, sv_meta.size() - pos - 1), ",")) {
            if (const auto id = svtoi(castling)) {
                castlings.push_back(Id{ static_cast<Id_type>(*id) });
            }
            else {
                return UNEXPECTED_PARSE(ParseErrorType, InvalidCastling, id.error());
            }
        }
        meta.castlings = castlings;
        
        meta.max_pos_length = std::to_string(meta.width * meta.height - 1).size();

        return operator()(sv, meta);
    }
};

#include "../stuff/cexau.h"

CEXAU meta = "02H8W8TBC22,21,45,44!"sv;
CEXAU w_pos = meta.find('W', 0);
CEXAU test = meta.substr(meta.find('C') + 1, meta.size() - meta.find('C') - 1);

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
            result += std::format("{:0>2}H{}W{}{}{}C",
                meta.version,
                HEIGHT,
                WIDTH,
                br.get_idw_char(),
                br.get_turn_char()
            );
            for (const Id castle_id : br.can_castle) {
                result += as_string<Id>{}(castle_id, meta.min_id) + ","s;
            }
            result.back() = '!';
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