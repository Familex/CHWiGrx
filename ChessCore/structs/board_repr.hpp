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
    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
       -> ParseEither<board_repr::BoardRepr, ParseErrorType>
    {
        board_repr::BoardRepr result{};

        auto process_figures =
            [&meta](const std::string_view sv, std::vector<Figure*>& out, std::size_t start_pos)
            -> std::optional<ParseError<ParseErrorType>>
        {
            std::size_t fig_curr_pos{ };

            while (fig_curr_pos < sv.size()) {
                if (const auto& figure_sus = from_string<Figure>{}(sv.substr(fig_curr_pos), meta)) {
                    out.push_back(
                        figfab::FigureFabric::instance().create(&figure_sus.value().value, true).release()
                    );
                    fig_curr_pos += figure_sus.value().position;
                }
                else {
                    return ParseError<ParseErrorType>{
                        static_cast<ParseErrorType>(
                            static_cast<std::size_t>(ParseErrorType::Figure_Base)
                            + static_cast<std::size_t>(figure_sus.error().type)
                        ),
                        figure_sus.error().position + fig_curr_pos + start_pos
                    };
                }
            }
            return std::nullopt;
        };

        auto process_move_message =
            [&meta](const std::string_view sv, std::vector<mvmsg::MoveMessage>& out, std::size_t start_pos)
            -> std::optional<ParseError<ParseErrorType>>
        {
            std::size_t move_message_curr_pos{ };
            
            for (const auto& move_message_sv : split(sv, "$"sv)) {
                if (const auto& move_message_sus = from_string<mvmsg::MoveMessage>{}(move_message_sv, meta)) {
                    out.push_back(move_message_sus.value().value);
                    move_message_curr_pos += move_message_sus.value().position + 1;
                    //                        + 1 because of the '$' delimiter ^^^
                }
                else {
                    return ParseError<ParseErrorType>{
                        static_cast<ParseErrorType>(
                            static_cast<std::size_t>(ParseErrorType::MoveMessage_Base)
                            + static_cast<std::size_t>(move_message_sus.error().type)
                        ),
                        move_message_sus.error().position + move_message_curr_pos + start_pos
                    };
                }
            }
            return std::nullopt;
        };

        {
            const auto start_pos = sv.find('!') + 1;
            const auto figures = sv.substr(
                start_pos,
                sv.find('<') - sv.find('!') - 1
            );

            if (const auto& error_sus =
                    process_figures(
                        figures,
                        result.figures,
                        start_pos
                    )
                )
            {
                return std::unexpected{ *error_sus };
            }
        }
        {
            const auto start_pos = sv.find('<') + 1;
            const auto past = sv.substr(
                start_pos,
                sv.find('>') - sv.find('<') - 1
            );
            
            if (const auto& error_sus =
                    process_move_message(
                        past,
                        result.past,
                        start_pos
                    )
                )
            {
                return std::unexpected{ *error_sus };
            }
        }
        {
            const auto start_pos = sv.find('<', sv.find('<') + 1) + 1;
            const auto future = sv.substr(
                start_pos,
                sv.find('>', sv.find('>') + 1) - sv.find('<', sv.find('<') + 1) - 1
            );
            
            if (const auto& error_sus =
                    process_move_message(
                        future,
                        result.future,
                        start_pos
                    )
                )
            {
                return std::unexpected{ *error_sus };
            }
        }
        {
            const auto start_pos = sv.find('>', sv.find('>') + 1) + 1;
            const auto captured = sv.substr(
                start_pos,
                sv.size() - sv.find('>', sv.find('>') + 1)
            );

            if (const auto& error_sus = process_figures(
                    captured, 
                    result.captured_figures,
                    start_pos
                    )
                )
            {
                return std::unexpected{ *error_sus };
            }
        }
        
        return { { std::move(result), sv.size() } };
        //              seems useless ^^^^^^^^^
    }
    
    FN operator()(const std::string_view sv) const noexcept
       -> ParseEither<board_repr::BoardRepr, ParseErrorType>
    {
        FromStringMeta meta{ };
        std::size_t pos{ };
        
        const auto meta_end = sv.find('!');
        if (meta_end == std::string_view::npos) {
            return UNEXPECTED_PARSE(Meta_CouldNotFindMeta, pos);
        }
        const auto sv_meta = sv.substr(0, meta_end);

        if (const auto version = svtoi(sv_meta.substr(pos, 2))) {
            meta.version = version.value().value;
            pos += 2;
        } else {
            return UNEXPECTED_PARSE(Meta_InvalidVersion, version.error());
        }
        if (sv_meta[pos++] != 'H') {
            return UNEXPECTED_PARSE(Meta_CouldNotFindHeight, pos - 1);
        }
        auto w_pos = sv_meta.find('W', pos);
        if (w_pos == std::string_view::npos) {
            return UNEXPECTED_PARSE(Meta_CouldNotFindWidth, pos - 1);
        }
        if (auto height = svtoi(sv_meta.substr(pos, w_pos - pos))) {
            meta.height = height.value().value;
            pos = w_pos + 1;
            //          ^^^ because of the 'W' delimiter
        } else {
            return UNEXPECTED_PARSE(Meta_InvalidHeight, height.error());
        }
        auto idw_pos = sv_meta.find('T', pos);
        if (idw_pos == std::string_view::npos) {
            idw_pos = sv_meta.find('F', pos);
            if (idw_pos == std::string_view::npos) {
                return UNEXPECTED_PARSE(Meta_CouldNotFindIDW, pos - 1);
            }
            else {
                meta.idw = false;
            }
        }
        else {
            meta.idw = true;
        }
        if (auto width = svtoi(sv_meta.substr(pos, idw_pos - pos))) {
            meta.width = width.value().value;
            pos = idw_pos + 1;
            //            ^^^ because of the 'T' or 'F' delimiter
        }
        else {
            return UNEXPECTED_PARSE(Meta_InvalidWidth, pos - 1);
        }
        if (sv_meta.size() == pos) {
            return UNEXPECTED_PARSE(Meta_CouldNotFindCurrentTurn, pos - 1);
        }
        if (sv_meta[pos] != 'B' && sv_meta[pos] != 'W') {
            return UNEXPECTED_PARSE(Meta_InvalidCurrentTurn, pos);
        }
        meta.turn =
            sv_meta[pos] == 'B'
            ? Color::Black
            : Color::White;
        pos += 1;
        if (sv_meta.size() != pos) {
            if (sv_meta[pos] != 'C') {
                return UNEXPECTED_PARSE(Meta_CouldNotFindCastlings, pos);
            }
            for (const auto& castling
                    : split(sv_meta.substr(pos + 1, sv_meta.size() - pos - 1), ","sv))
            {
                if (const auto id = svtoi(castling)) {
                    meta.castlings.push_back(Id{ static_cast<Id_type>(id.value().value) });
                }
                else {
                    return UNEXPECTED_PARSE(Meta_InvalidCastling, id.error());
                }
            }
        }
        
        meta.max_pos_length = std::to_string(meta.width * meta.height - 1).size();

        return operator()(sv, meta);
    }
};

template <>
struct as_string<board_repr::BoardRepr> {
    FN operator()(const board_repr::BoardRepr& br,
                  const AsStringMeta& meta) const noexcept
       -> std::string
    {
        // FIXME castlings
        using namespace std::literals::string_literals;

        std::string result{ };
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
                result += as_string<Id>{}(castle_id, meta.min_id) + ',';
            }
            result.back() = '!';
        }
        // Figures
        for (const auto& fig : br.figures) {
            result += as_string<const Figure*>{}(fig, meta);
        }
        // Prev moves
        result += '<';
        for (const auto& mm : br.past) {
            result += as_string<mvmsg::MoveMessage>{}(mm, meta) + '$';
        }
        // Future moves
        if (br.past.empty()) {
            result += "><"s;
        }
        else {
            result.back() = '>';
            result += '<';
        }
        for (const auto& mm : br.future) {
            result += as_string<mvmsg::MoveMessage>{}(mm, meta) + '$';
        }
        // Captured figures
        if (br.future.empty()) {
            result += '>';
        }
        else {
            result.back() = '>';
        }
        for (const auto& fig : br.captured_figures) {
            result += as_string<const Figure*>{}(fig, meta);
        }
        return result;
    }

    FN operator()(const board_repr::BoardRepr& br) const noexcept
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
