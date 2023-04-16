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

        friend FromString<BoardRepr>;
        friend AsString<BoardRepr>;

        /* ---- Methods ---- */
        CTOR BoardRepr(const BoardRepr& other) noexcept
            : turn(other.turn)
            , idw(other.idw)
            , past(other.past)
            , future(other.future)
            , can_castle(other.can_castle)
        {
            for (const auto fig : other.figures) {
                figures.push_back(figfab::FigureFabric::instance().create(fig, true).release());
            }
            for (const auto fig : other.captured_figures) {
                captured_figures.push_back(figfab::FigureFabric::instance().create(fig, true).release());
            }
        }
        
        CTOR BoardRepr(
                 std::vector<Figure*>&& figures,
                 const Color turn,
                 const bool idw,
                 std::vector<mvmsg::MoveMessage> past = {},
                 std::vector<mvmsg::MoveMessage> future = {},
                 std::vector<Figure*> captured_figures = {},
                 const std::optional<std::vector<Id>>& can_castle = std::nullopt
        ) noexcept
            : figures(figures)
            , turn(turn)
            , idw(idw)
            , past(std::move(past))
            , future(std::move(future))
            , captured_figures(std::move(captured_figures))
        {
            if (can_castle) {
                this->can_castle = *can_castle;
            } else {
                for (const auto fig : figures) {
                    if (fig->is(FigureType::Rook)) {
                        this->can_castle.push_back(fig->get_id());
                    }
                }
            }
        }

        CTOR BoardRepr() noexcept = default;
        // FIXME is destructor will be look like this?
        // I tried to use clear() method and all was broken
        ~BoardRepr() noexcept = default;
        
        // implicit move constructor to easy use in std::expected
        [[nodiscard]] constexpr
            BoardRepr(BoardRepr&& br) noexcept
        {
            clear();
            *this = br;
        }

        // FIXME not tested yet
        FN operator= (BoardRepr&& br) noexcept -> BoardRepr&
        {
            clear();
            figures = std::move(br.figures);
            turn = br.turn;
            idw = br.idw;
            past = std::move(br.past);
            future = std::move(br.future);
            captured_figures = std::move(br.captured_figures);
            can_castle = std::move(br.can_castle);
            return *this;
        }

        constexpr BoardRepr&
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
            return *this;
        }

        FN clear() noexcept -> void
        {
            for (const auto fig : figures) {
                delete fig;
            }
            figures.clear();
            for (const auto fig : captured_figures) {
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
struct FromString<board_repr::BoardRepr> {

    FN parse_v2(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> ParseEither<board_repr::BoardRepr, ParseErrorType>
    {
        board_repr::BoardRepr result{};

        result.can_castle = meta.castlings;

        auto process_figures =
            [&meta](const std::string_view proc_sv, std::vector<Figure*>& out, const std::size_t start_pos)
            -> std::optional<ParseError<ParseErrorType>>
        {
            std::size_t fig_curr_pos{ };

            while (fig_curr_pos < proc_sv.size()) {
                if (const auto& figure_sus = FromString<Figure>{}(proc_sv.substr(fig_curr_pos), meta)) {
                    out.push_back(
                        figfab::FigureFabric::instance().create(&figure_sus.value().value, true).release()
                    );
                    fig_curr_pos += figure_sus.value().position;
                }
                else {
                    return ParseError<ParseErrorType>{
                        figure_sus.error().type,
                        figure_sus.error().position
                            + fig_curr_pos
                            + start_pos
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
                if (const auto& move_message_sus = FromString<mvmsg::MoveMessage>{}(move_message_sv, meta)) {
                    out.push_back(move_message_sus.value().value);
                    move_message_curr_pos += move_message_sus.value().position + 1;
                    //                        + 1 because of the '$' delimiter ^^^
                }
                else {
                    return ParseError<ParseErrorType>{
                        move_message_sus.error().type,
                        move_message_sus.error().position
                            + move_message_curr_pos
                            + start_pos
                    };
                }
            }
            return std::nullopt;
        };

        
        /* Figures */ {
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
        /* Past */ {
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
        /* Future */ {
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
        /* Captured */ {
            const auto start_pos = sv.find('>', sv.find('>') + 1) + 1;
            const auto captured = sv.substr(
                start_pos,
                sv.size() - sv.find('>', sv.find('>') + 1)
            );

            if (const auto& error_sus =
                    process_figures(
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

    FN operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
       -> ParseEither<board_repr::BoardRepr, ParseErrorType>
    {
        switch (meta.version) {
            case 2: return parse_v2(sv, meta);
            default: return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_UnsupportedVersion, 0ull);
        }
    }
    
    FN operator()(const std::string_view sv) const noexcept
       -> ParseEither<board_repr::BoardRepr, ParseErrorType>
    {
        FromStringMeta meta{ };
        std::size_t pos{ };
        
        if (sv.empty()) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, General_EmptyString, pos);
        }
        const auto meta_end = sv.find('!');
        if (meta_end == std::string_view::npos) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_CouldNotFindMeta, pos);
        }
        const auto sv_meta = sv.substr(0, meta_end);

        if (const auto version = svtoi(sv_meta.substr(pos, 2))) {
            meta.version = version.value().value;
            pos += 2;
        } else {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_InvalidVersion, version.error());
        }
        if (sv_meta[pos++] != 'H') {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_CouldNotFindHeight, pos - 1);
        }
        const auto w_pos = sv_meta.find('W', pos);
        if (w_pos == std::string_view::npos) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_CouldNotFindWidth, pos - 1);
        }
        if (auto height = svtoi(sv_meta.substr(pos, w_pos - pos))) {
            meta.height = height.value().value;
            pos = w_pos + 1;
            //          ^^^ because of the 'W' delimiter
        } else {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_InvalidHeight, height.error());
        }
        auto idw_pos = sv_meta.find('T', pos);
        if (idw_pos == std::string_view::npos) {
            idw_pos = sv_meta.find('F', pos);
            if (idw_pos == std::string_view::npos) {
                return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_CouldNotFindIdw, pos - 1);
            }
            else {
                meta.idw = false;
            }
        }
        else {
            meta.idw = true;
        }
        if (const auto width = svtoi(sv_meta.substr(pos, idw_pos - pos))) {
            meta.width = width->value;
            pos = idw_pos + 1;
            //            ^^^ because of the 'T' or 'F' delimiter
        }
        else {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_InvalidWidth, pos - 1);
        }
        if (sv_meta.size() == pos) {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_CouldNotFindCurrentTurn, pos - 1);
        }
        if (sv_meta[pos] != 'B' && sv_meta[pos] != 'W') {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_InvalidCurrentTurn, pos);
        }
        meta.turn =
            sv_meta[pos] == 'B'
            ? Color::Black
            : Color::White;
        pos += 1;
        if (sv_meta.size() != pos) {
            if (sv_meta[pos] != 'C') {
                return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_CouldNotFindCastlings, pos);
            }
            for (const auto& castling
                    : split(sv_meta.substr(pos + 1, sv_meta.size() - pos - 1), ","sv))
            {
                if (const auto id = svtoi(castling)) {
                    meta.castlings.emplace_back(static_cast<IdType>(id.value().value));
                }
                else {
                    return PARSE_STEP_UNEXPECTED(ParseErrorType, Meta_InvalidCastling, id.error());
                }
            }
        }
        
        meta.max_pos_length = std::to_string(meta.width * meta.height - 1).size();

        return operator()(sv, meta);
    }
};

template <>
struct AsString<board_repr::BoardRepr> {
    FN operator()(const board_repr::BoardRepr& br,
                  const AsStringMeta& meta) const noexcept
       -> std::string
    {
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
                result += AsString<Id>{}(castle_id, meta.min_id) + ',';
            }
            result.back() = '!';
        }
        // Figures
        for (const auto& fig : br.figures) {
            result += AsString<const Figure*>{}(fig, meta);
        }
        // Prev moves
        result += '<';
        for (const auto& mm : br.past) {
            result += AsString<mvmsg::MoveMessage>{}(mm, meta) + '$';
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
            result += AsString<mvmsg::MoveMessage>{}(mm, meta) + '$';
        }
        // Captured figures
        if (br.future.empty()) {
            result += '>';
        }
        else {
            result.back() = '>';
        }
        for (const auto& fig : br.captured_figures) {
            result += AsString<const Figure*>{}(fig, meta);
        }
        return result;
    }

    FN operator()(const board_repr::BoardRepr& br) const noexcept
       -> std::string
    {
        AsStringMeta meta{ };
        
        meta.version = 2;

        // calc min id_ in br.figures
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
