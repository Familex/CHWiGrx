#include "Stuff.h"

/// <summary>
/// ����� ������ �� ���������
/// �� �����������
/// </summary>
/// <param name="str">������</param>
/// <param name="delimiter">�����������</param>
/// <returns>������ ��������</returns>
auto split(std::string str, const std::string&& delimiter) {
    size_t token_end{};
    std::vector<std::string> tokens{};
    while ((token_end = str.find(delimiter)) != str.npos) {
        tokens.emplace_back(str.substr(0, token_end));
        str.erase(0, token_end + delimiter.length());
    }
    for (char c : str) {
        if (c == ' ' || delimiter.find(c) != std::string::npos) {
            return tokens;
        }
    }
    if (!str.empty())
        tokens.push_back(str);
    return tokens;
}

FigureType::FigureType(char ch) {
    switch (ch)
    {
    case 'K': case 'k': data = EFigureType::King;   break;
    case 'H': case 'h': data = EFigureType::Knight; break;
    case 'P': case 'p': data = EFigureType::Pawn;   break;
    case 'B': case 'b': data = EFigureType::Bishop; break;
    case 'Q': case 'q': data = EFigureType::Queen;  break;
    case 'R': case 'r': data = EFigureType::Rook;   break;
    case 'N': case 'n': data = EFigureType::None;   break;
    default:
        throw std::invalid_argument("Can't parse '" + std::string(ch, 1) + "' - figure type");
    }
}

Color::Color(char ch) {
    switch (ch) {
    case 'w': case 'W': data = EColor::White; break;
    case 'B': case 'b': data = EColor::Black; break;
    case 'N': case 'n': data = EColor::None; break;
    default:
        throw std::invalid_argument("Can't parse '" + std::string(ch, 1) + "' - color");
    }
}

Color Color::what_next() const {
    switch (data) {
    case EColor::Black:
        return EColor::White;
    case EColor::White:
        return EColor::Black;;
    default:
        throw std::invalid_argument("unaccepted color");
    }
}

Color Color::to_next() {
    data = what_next();
    return *this;
}

FigureType::operator char() {
    switch (data) {
    case EFigureType::Pawn: return 'P';
    case EFigureType::Rook: return 'R';
    case EFigureType::Knight: return 'H';
    case EFigureType::Bishop: return 'B';
    case EFigureType::Queen: return 'Q';
    case EFigureType::King: return 'K';
    case EFigureType::None: return 'N';
    default:
        return 'e';
    }
}

Color::operator char() {
    switch (data) {
    case EColor::Black: return 'B';
    case EColor::White: return 'W';
    case EColor::None: return 'N';
    default:
        return 'e';
    }
}

bool operator== (const pos& left, const pos& right) { return (left.x == right.x) and (left.y == right.y); }
bool operator!= (const pos& left, const pos& right) { return (left.x != right.x) or (left.y != right.y); }
pos operator-  (const pos& left, const pos& right) { return { (left.x - right.x), (left.y - right.y) }; }
pos operator+  (const pos& left, const pos& right) { return { (left.x + right.x), (left.y + right.y) }; }
bool operator!= (const Color& left, const Color& right) { return left.data != right.data; }
bool operator== (const Color& left, const Color& right) { return left.data == right.data; }
bool operator== (const Color& left, const EColor& right) { return left.data == right; }
bool operator== (const EColor& left, const Color& right) { return left == right.data; }
bool operator<  (const pos& left, const pos& right) {
    if (left.x != right.x) {
        return (left.x < right.x);
    }
    else {
        return (left.y < right.y);
    }
}

std::vector<pos> to_pos_vector(const std::vector<Figure>& lst) {
    std::vector<pos> acc{};
    for (const auto& fig : lst) {
        acc.push_back(fig.position);
    }
    return acc;
}

Input::Input(std::string str) {
    /* "from.first from.second target.first target.second" */
    str += " ";
    char delimiter = ' ';
    size_t delimiter_len = 1;
    size_t pos = 0;
    std::string token;
    std::vector<int> acc{};
    for (int i{}; i < 4; ++i) {
        if ((pos = str.find(delimiter)) != std::string::npos) {
            token = str.substr(0, pos);
            acc.push_back(std::stoi(token));
            str.erase(0, pos + delimiter_len);
        }
        else { throw std::invalid_argument("unexpected end of line"); }
    }
    from.x = acc[0];
    from.y = acc[1];
    target.x = acc[2];
    target.y = acc[3];
}

MoveRec MoveLogger::get_last_move() {
    if (prev_moves.empty())
        return {};
    return prev_moves.back();
}

void MoveLogger::add(const MoveRec& move_rec) {
    prev_moves.push_back(move_rec);
    future_moves.clear();
}

void MoveLogger::reset() {
    prev_moves.clear();
    future_moves.clear();
}

MoveRec MoveLogger::pop_future_move() {
    if (future_moves.empty()) return {};
    auto future = future_moves.back();
    future_moves.pop_back();
    return future;
}

MoveRec MoveLogger::move_last_to_future() {
    if (prev_moves.empty()) return {};
    auto last = get_last_move();
    prev_moves.pop_back();
    future_moves.push_back(last);
    return last;
}

BoardRepr::BoardRepr(std::string board_repr) {
    const size_t npos = std::string::npos;
    size_t meta_start = board_repr.find('[');
    size_t meta_end = board_repr.find(']');
    std::string meta = board_repr.substr(meta_start + 1, meta_end - meta_start - 1);
    if (meta.find('t') != npos || meta.find('T') != npos) {
        idw = true;
    }
    else if (meta.find('f') != npos || meta.find('F') != npos) {
        idw = false;
    }
    if (meta.find('w') != npos || meta.find('W') != npos) {
        turn = EColor::White;
    }
    else if (meta.find('b') != npos || meta.find('B') != npos) {
        turn = EColor::Black;
    }
    board_repr.erase(board_repr.begin() + meta_start, board_repr.begin() + meta_end + 1);
    size_t past_start = board_repr.find('<');
    size_t past_end = board_repr.find('>');
    if (past_end - past_start > 2) {
        std::string past_to_parse = board_repr.substr(past_start + 1, past_end - past_start - 1);
        for (const std::string& mr_to_parse : split(past_to_parse, "$")) {
            if (!mr_to_parse.empty() && mr_to_parse != " ")
                past.push_back(mr_to_parse);
        }
    }
    board_repr.erase(board_repr.begin() + past_start, board_repr.begin() + past_end + 1);
    size_t future_start = board_repr.find('<');
    size_t future_end = board_repr.find('>');
    if (future_end - future_start > 2) {
        std::string future_to_parse = board_repr.substr(future_start + 1, future_end - future_start - 1);
        for (const std::string& mr_to_parse : split(future_to_parse, "$")) {
            if (!mr_to_parse.empty() && mr_to_parse != " ")
                future.push_back(mr_to_parse);
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
        captured_figures.push_back({ std::stoi(tmp[i]),
                                    {std::stoi(tmp[i + 1]), std::stoi(tmp[i + 2])},
                                    tmp[i + 3][0],
                                    tmp[i + 4][0]
            });
    }
    board_repr.erase(board_repr.begin() + deleted_start, board_repr.end());

    tmp.clear();
    for (const std::string& fig_piece : split(board_repr, ";")) {
        if (!fig_piece.empty() && fig_piece != " ")
            tmp.push_back(fig_piece);
    }
    for (size_t i{}; i < tmp.size(); i += 5) {
        figures.push_back({ std::stoi(tmp[i]),
                                    {std::stoi(tmp[i + 1]), std::stoi(tmp[i + 2])},
                                    tmp[i + 3][0],
                                    tmp[i + 4][0]
            });
    }
}

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

MoveRec::MoveRec(std::string map) {
    if (map.empty()) throw std::invalid_argument("Empty map");
    std::string data[16]; // last always empty
    int i{ 0 };
    for (const std::string& curr : split(map, ".")) {
        data[i++] = curr;
    }

    who_went.id = std::stoi(data[0]);
    who_went.position.x = std::stoi(data[1]);
    who_went.position.y = std::stoi(data[2]);
    who_went.color = data[3][0];
    turn = who_went.color;
    who_went.type = data[4][0];
    input.from.x   = std::stoi(data[5]);
    input.from.y   = std::stoi(data[6]);
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

std::string BoardRepr::as_string() {
    std::string result{ "" };
    for (auto& fig : figures) {
        result += std::format("{};{};{};{};{};",
            fig.id, fig.position.x, fig.position.y, (char)fig.color, (char)fig.type
        );
    }
    result += std::format("[{}{}]<", get_idw_char(), get_turn_char());
    for (auto& mr : past) {
        result += mr.as_string() + "$";
    }
    result += "><";
    for (auto& mr : future) {
        result += mr.as_string() + "$";
    }
    result += ">~";
    for (auto& fig : captured_figures) {
        result += std::format("{},{},{},{},{},",
            fig.id, fig.position.x, fig.position.y, (char)fig.color, (char)fig.type
        );
    }
    return result;
}

std::string to_string(SideEvent side_event) {
    switch (side_event)
    {
    case SideEvent::E:
        return "E";
    case SideEvent::CHECK:
        return "C";
    case SideEvent::PROMOTION:
        return "P";
    case SideEvent::CASTLING_BREAK:
        return "B";
    default:
        return "N";
    }
}

std::string to_string(MainEvent main_event) {
    switch (main_event)
    {
    case MainEvent::E:
        return "E";
    case MainEvent::EAT:
        return "T";
    case MainEvent::MOVE:
        return "M";
    case MainEvent::LMOVE:
        return "L";
    case MainEvent::CASTLING:
        return "C";
    case MainEvent::EN_PASSANT:
        return "P";
    default:
        return "N";
    }
}