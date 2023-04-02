#pragma once

#include "stuff.h"
#include "figure.hpp"
#include "figure_fabric.hpp"

#include <expected>

namespace moverec {

    enum class ParseError {
        EmptyMap,
    };

    struct MoveRec {
        CTOR MoveRec(const Figure* who_went,
            const Input& input,
            const Color turn,
            const MoveMessage& ms,
            const FigureType p) noexcept
            : who_went{ *who_went }
            , input{ input }
            , turn{ turn }
            , ms{ ms }
        , promotion_choice{ p } {};

        CTOR MoveRec() noexcept = default;

        // FIXME remove std::optional::value() calls
        FN static from_string(const std::string_view map) noexcept -> std::expected<MoveRec, ParseError>
        {
            if (map.empty())
                return std::unexpected{ ParseError::EmptyMap };

            MoveRec result;

            auto data = split(map, ".");

            // FIXME crap
            result.who_went =
                *(figfab::FigureFabric::instance()
                    .create(data[0])
                    .value()
                    .get());

            result.turn = result.who_went.get_col();
            result.input.from.x = svtoi(data[5]).value();
            result.input.from.y = svtoi(data[6]).value();
            result.input.target.x = svtoi(data[7]).value();
            result.input.target.y = svtoi(data[8]).value();
            switch (data[9][0]) {
            case 'E':
                result.ms.main_ev = MainEvent::E;
                break;
            case 'T':
                result.ms.main_ev = MainEvent::EAT;
                break;
            case 'M':
                result.ms.main_ev = MainEvent::MOVE;
                break;
            case 'L':
                result.ms.main_ev = MainEvent::LMOVE;
                break;
            case 'C':
                result.ms.main_ev = MainEvent::CASTLING;
                break;
            case 'P':
                result.ms.main_ev = MainEvent::EN_PASSANT;
                break;
            default:
                result.ms.main_ev = MainEvent::E;
                break;
            }
            result.promotion_choice = char_to_figure_type(data[10][0]);
            for (const auto c : data[11]) {
                if (c != ',' && c != '{' && c != '}' && c != ' ')
                    switch (c) {
                    case 'E':
                        result.ms.side_evs.push_back(SideEvent::E);
                        break;
                    case 'C':
                        result.ms.side_evs.push_back(SideEvent::CHECK);
                        break;
                    case 'P':
                        result.ms.side_evs.push_back(SideEvent::PROMOTION);
                        break;
                    case 'B':
                        result.ms.side_evs.push_back(SideEvent::CASTLING_BREAK);
                        break;
                    default:
                        result.ms.side_evs.push_back(SideEvent::E);
                        break;
                    }
            }
            if (data[12].length() >= 3) {
                for (const std::string_view curr : split(data[12].substr(1, data[12].length() - 2), ",")) {
                    if (!curr.empty() && curr != " ")
                        result.ms.to_eat.push_back(svtoi(curr).value());
                }
            }
            std::vector<int> tmp;
            if (data[13].length() >= 3) {
                for (const std::string_view curr : split(data[13].substr(1, data[13].length() - 2), ",")) {
                    if (!curr.empty() && curr != " ")
                        tmp.push_back(svtoi(curr).value());
                }
            }
            for (size_t i{}; i < tmp.size(); i += 5) {
                std::pair<Id, Input> to_move{ tmp[i], {{tmp[i + 1], tmp[i + 2]}, {tmp[i + 3], tmp[i + 4]}} };
                result.ms.to_move.push_back(to_move);
            }
            if (data[14].length() >= 5) { // 2 extra spaces from split...
                for (const std::string_view curr : split(data[14].substr(1, data[14].length() - 2), ",")) {
                    if (!curr.empty() && curr != " " && curr != "}" && curr != "} ")
                        result.ms.what_castling_breaks.push_back(svtoi(curr).value());
                }
            }

            return result;
        }

        FN as_string() const noexcept -> std::string
        {
            std::string result{ "" };
            result += std::format("{}.{}.{}.{}.{}.{}.{}.{{",
                who_went.as_string(),
                input.from.x,
                input.from.y,
                input.target.x,
                input.target.y,
                to_string(ms.main_ev),
                figure_type_to_char(promotion_choice)
            );
            for (const SideEvent& side_ev : ms.side_evs) {
                result += to_string(side_ev) + ",";
            }
            result += "}.{";
            for (const Id to_eat : ms.to_eat) {
                result += std::format("{},", to_eat);
            }
            result += "}.{";
            for (const auto& [to_move, p] : ms.to_move) {
                result += std::format("{},{},{},{},{}",
                    to_move, p.from.x, p.from.y, p.target.x, p.target.y
                );
            }
            result += "}.{";
            for (const Id cb : ms.what_castling_breaks) {
                result += std::format("{},", cb);
            }
            result += "}";

            return result;
        }

        Figure who_went{};
        Input input{};
        Color turn{ Color::None };
        MoveMessage ms{};
        FigureType promotion_choice{ FigureType::None };
    };

}   // namespace moverec