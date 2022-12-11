#include "move_rec.h"

// Преобразование в строку
std::string MoveRec::as_string() {
    std::string result{ "" };
    result += std::format("{}.{}.{}.{}.{}.{}.{}.{{",
        who_went.as_string(),
        input.from.x,
        input.from.y,
        input.target.x,
        input.target.y,
        to_string(ms.main_ev),
        promotion_choice
    );
    for (SideEvent& side_ev : ms.side_evs) {
        result += to_string(side_ev) + ",";
    }
    result += "}.{";
    for (Id& to_eat : ms.to_eat) {
        result += std::format("{},", to_eat);
    }
    result += "}.{";
    for (auto& [to_move, p] : ms.to_move) {
        result += std::format("{},{},{},{},{}",
            to_move, p.from.x, p.from.y, p.target.x, p.target.y
        );
    }
    result += "}.{";
    for (Id& cb : ms.what_castling_breaks) {
        result += std::format("{},", cb);
    }
    result += "}";

    return result;
}

// Коструктор из строки
MoveRec::MoveRec(std::string map) {
    if (map.empty()) throw std::invalid_argument("Empty map");
    auto data = split(map, ".");
    // Возможно нижнюю конструкцию стоит вставить в фабрику
    Id new_id = std::stoi(data[0]);
    Color new_col = char_to_col(data[3][0]);
    Pos new_pos = { std::stoi(data[1]), std::stoi(data[2]) };
    FigureType new_type = char_to_figure_type(data[4][0]);
    Figure* who_went_tmp = FigureFabric::instance()->create(
        new_pos, new_col, new_type, new_id
    );
    who_went = *who_went_tmp;
    delete who_went_tmp;
    turn = new_col;
    input.from.x = std::stoi(data[5]);
    input.from.y = std::stoi(data[6]);
    input.target.x = std::stoi(data[7]);
    input.target.y = std::stoi(data[8]);
    switch (data[9][0]) {
    case 'E':
        ms.main_ev = MainEvent::E;
        break;
    case 'T':
        ms.main_ev = MainEvent::EAT;
        break;
    case 'M':
        ms.main_ev = MainEvent::MOVE;
        break;
    case 'L':
        ms.main_ev = MainEvent::LMOVE;
        break;
    case 'C':
        ms.main_ev = MainEvent::CASTLING;
        break;
    case 'P':
        ms.main_ev = MainEvent::EN_PASSANT;
        break;
    default:
        ms.main_ev = MainEvent::E;
        break;
    }
    promotion_choice = data[10][0];
    for (const auto c : data[11]) {
        if (c != ',' && c != '{' && c != '}' && c != ' ')
            switch (c) {
            case 'E':
                ms.side_evs.push_back(SideEvent::E); break;
            case 'C':
                ms.side_evs.push_back(SideEvent::CHECK); break;
            case 'P':
                ms.side_evs.push_back(SideEvent::PROMOTION); break;
            case 'B':
                ms.side_evs.push_back(SideEvent::CASTLING_BREAK); break;
            default:
                ms.side_evs.push_back(SideEvent::E);
                break;
            }
    }
    if (data[12].length() >= 3) {
        for (const std::string& curr : split(data[12].substr(1, data[12].length() - 2), ",")) {
            if (!curr.empty() && curr != " ")
                ms.to_eat.push_back(std::stoi(curr));
        }
    }
    std::vector<int> tmp;
    if (data[13].length() >= 3) {
        for (const std::string& curr : split(data[13].substr(1, data[13].length() - 2), ",")) {
            if (!curr.empty() && curr != " ")
                tmp.push_back(std::stoi(curr));
        }
    }
    for (size_t i{}; i < tmp.size(); i += 5) {
        std::pair<Id, Input> to_move{ tmp[i], {{tmp[i + 1], tmp[i + 2]}, {tmp[i + 3], tmp[i + 4]}} };
        ms.to_move.push_back(to_move);
    }
    if (data[14].length() >= 5) { // 2 extra spaces from split...
        for (const std::string& curr : split(data[14].substr(1, data[14].length() - 2), ",")) {
            if (!curr.empty() && curr != " " && curr != "}" && curr != "} ")
                ms.what_castling_breaks.push_back(std::stoi(curr));
        }
    }
}
