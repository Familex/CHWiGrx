#include "Stuff.h"

/// <summary>
/// Делит строку на подстроки
/// по разделителю
/// </summary>
/// <param name="str">Строка</param>
/// <param name="delimiter">Разделитель</param>
/// <returns>Вектор подстрок</returns>
std::vector<std::string> split(std::string str, const std::string&& delimiter) {
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

void remove_first_occurrence(std::string& str, char c) {
    auto p = str.find(c);
    if (p != std::string::npos) {
        str.erase(p, p + 1);
    }
}

/// <summary>
/// Конструктор типа фигуры из буквы
/// </summary>
/// <exception cref="std::invatid_argument">Непредвиденный символ</exception>
/// <param name="ch">Тип фигуры</param>
FigureType::FigureType(char ch) {
    switch (ch)
    {
    case 'K': case 'k': data = FigureType::Type::King;   break;
    case 'H': case 'h': data = FigureType::Type::Knight; break;
    case 'P': case 'p': data = FigureType::Type::Pawn;   break;
    case 'B': case 'b': data = FigureType::Type::Bishop; break;
    case 'Q': case 'q': data = FigureType::Type::Queen;  break;
    case 'R': case 'r': data = FigureType::Type::Rook;   break;
    case 'N': case 'n': data = FigureType::Type::None;   break;
    default:
        throw std::invalid_argument("Can't parse '" + std::string(ch, 1) + "' - figure type");
    }
}

/// <summary>
/// Коструктор цвета из буквы
/// </summary>
/// <exception cref="std::invatid_argument">Непредвиденный символ</exception>
/// <param name="ch">Цвет</param>
Color::Color(char ch) {
    switch (ch) {
    case 'w': case 'W': data = Color::Type::White; break;
    case 'B': case 'b': data = Color::Type::Black; break;
    case 'N': case 'n': data = Color::Type::None; break;
    default:
        throw std::invalid_argument("Can't parse '" + std::string(ch, 1) + "' - color");
    }
}

// Возвращает цвет игрока на следующем ходу
Color Color::what_next() const {
    switch (data) {
    case Color::Type::Black:
        return Color::Type::White;
    case Color::Type::White:
        return Color::Type::Black;
    default:
        throw std::invalid_argument("unaccepted color");
    }
}

// Меняет цвет на следующий
Color Color::to_next() {
    data = what_next();
    return *this;
}

// Перевод в символ
FigureType::operator char() {
    switch (data) {
    case FigureType::Type::Pawn: return 'P';
    case FigureType::Type::Rook: return 'R';
    case FigureType::Type::Knight: return 'H';
    case FigureType::Type::Bishop: return 'B';
    case FigureType::Type::Queen: return 'Q';
    case FigureType::Type::King: return 'K';
    case FigureType::Type::None: return 'N';
    default:
        return 'e';
    }
}

// Перевод в символ
Color::operator char() {
    switch (data) {
    case Color::Type::Black: return 'B';
    case Color::Type::White: return 'W';
    case Color::Type::None: return 'N';
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
bool operator== (const Color& left, const Color::Type& right) { return left.data == right; }
bool operator== (const Color::Type& left, const Color& right) { return left == right.data; }
bool operator<  (const pos& left, const pos& right) {
    if (left.x != right.x) {
        return (left.x < right.x);
    }
    else {
        return (left.y < right.y);
    }
}

// Оставить от вектора фигур только вектор их позиций
std::vector<pos> to_pos_vector(const std::vector<Figure*>& lst) {
    std::vector<pos> acc{};
    for (const auto& fig : lst) {
        acc.push_back(fig->get_pos());
    }
    return acc;
}

// Конструктор ввода
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

// Возвращает последний сделанный ход
MoveRec MoveLogger::get_last_move() {
    if (prev_moves.empty())
        return {};
    return prev_moves.back();
}

// Добавляет в лог ход
void MoveLogger::add(const MoveRec& move_rec) {
    prev_moves.push_back(move_rec);
    future_moves.clear();
}

// Обнуляет лог
void MoveLogger::reset() {
    prev_moves.clear();
    future_moves.clear();
}

// Достает из будущих ходов следующий
MoveRec MoveLogger::pop_future_move() {
    if (future_moves.empty()) return {};
    auto future = future_moves.back();
    future_moves.pop_back();
    return future;
}

// Добавляет последний ход в следующие и возвращает его
MoveRec MoveLogger::move_last_to_future() {
    if (prev_moves.empty()) return {};
    auto last = get_last_move();
    prev_moves.pop_back();
    future_moves.push_back(last);
    return last;
}

bool MoveLogger::is_fifty_move_rule_was_triggered() {
    size_t without_eat_and_pawnmoves = 0;
    for (auto move{ prev_moves.rbegin() }; move != prev_moves.rend(); ++move) {
        if (move->get_who_went()->get_type() == FigureType::Type::Pawn || move->ms.main_ev == MainEvent::EAT) {
            break;
        }
        else {
            ++without_eat_and_pawnmoves;
        }
    }
    return without_eat_and_pawnmoves >= 50;
}

bool MoveLogger::is_moves_repeat_rule_was_triggered() {
    return false;
}

// Сведения о рокировке привязаны к id соответствующих ладей (заключены в квадратные скобки)
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
        turn = Color::Type::White;
    }
    else if (meta.find('b') != npos || meta.find('B') != npos) {
        turn = Color::Type::Black;
    }
    for (const char& c : { 't', 'T', 'f', 'F', 'w', 'W', 'b', 'B' }) {
        remove_first_occurrence(meta, c);
    }
    // здесь значение meta содержит только информацию о возможных рокировках
    for (auto& castle_id : split(meta, ";")) {
        if (!castle_id.empty())
            can_castle.push_back(stoi(castle_id));
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
        Id new_id =             std::stoi(tmp[i]);
        pos new_pos =         { std::stoi(tmp[i + 1]), std::stoi(tmp[i + 2]) };
        Color new_col =       { tmp[i + 3][0] };
        FigureType new_type = { tmp[i + 4][0] };
        Figure* new_fig = FigureFabric::instance()->create(
            new_pos, new_col, new_type, new_id
        );
        captured_figures.push_back(new_fig);
    }
    board_repr.erase(board_repr.begin() + deleted_start, board_repr.end());

    tmp.clear();
    for (const std::string& fig_piece : split(board_repr, ";")) {
        if (!fig_piece.empty() && fig_piece != " ")
            tmp.push_back(fig_piece);
    }
    for (size_t i{}; i < tmp.size(); i += 5) {
        Id new_id = std::stoi(tmp[i]);
        pos new_pos = { std::stoi(tmp[i + 1]), std::stoi(tmp[i + 2]) };
        Color new_col = { tmp[i + 3][0] };
        FigureType new_type = { tmp[i + 4][0] };
        Figure* new_fig = FigureFabric::instance()->create(
            new_pos, new_col, new_type, new_id
        );
        figures.push_back(new_fig);
    }
}

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
    Color new_col = data[3][0];
    pos new_pos = { std::stoi(data[1]), std::stoi(data[2]) };
    FigureType new_type = data[4][0];
    Figure* who_went_tmp = FigureFabric::instance()->create(
        new_pos, new_col, new_type, new_id
    );
    who_went = *who_went_tmp;
    delete who_went_tmp;
    turn = new_col;
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

// Преобразование в строку
std::string BoardRepr::as_string() {
    std::string result{ "" };
    for (auto& fig : figures) {
        result += std::format("{};{};{};{};{};",
            fig->get_id(), fig->get_pos().x, fig->get_pos().y, (char)fig->get_col(), (char)fig->get_type()
        );
    }
    result += std::format("[{}{}", get_idw_char(), get_turn_char());
    for (Id& castle_id : can_castle) {
        result += std::format("{};", castle_id);
    }
    result += "]<";
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
            fig->get_id(), fig->get_pos().x, fig->get_pos().y, (char)fig->get_col(), (char)fig->get_type()
        );
    }
    return result;
}

// Преобразование в строку
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

// Преобразование в строку
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

Figure* FigureFabric::create(pos position, Color color, FigureType::Type type, Id id, Figure* placement) {
    switch (type) {
        case FigureType::Type::Pawn:
            return placement
                   ? new (placement) Figure(id, position, color, FigureType::Type::Pawn)
                   : new Figure(id, position, color, FigureType::Type::Pawn);
        case FigureType::Type::Knight:
            return placement
                   ? new (placement) Figure(id, position, color, FigureType::Type::Knight)
                   : new Figure(id, position, color, FigureType::Type::Knight);
        case FigureType::Type::Rook:
            return placement
                   ? new (placement) Figure(id, position, color, FigureType::Type::Rook)
                   : new Figure(id, position, color, FigureType::Type::Rook);
        case FigureType::Type::Bishop:
            return placement
                   ? new (placement) Figure(id, position, color, FigureType::Type::Bishop)
                   : new Figure(id, position, color, FigureType::Type::Bishop);
        case FigureType::Type::Queen:
            return placement
                   ? new (placement) Figure(id, position, color, FigureType::Type::Queen)
                   : new Figure(id, position, color, FigureType::Type::Queen);
        case FigureType::Type::King:
            return placement
                   ? new (placement) Figure(id, position, color, FigureType::Type::King)
                   : new Figure(id, position, color, FigureType::Type::King);
        case FigureType::Type::None:
            return new Figure(id, position, Color::Type::None, FigureType::Type::None);
        default:
            return get_default_fig();
        }
}

Figure* FigureFabric::create(pos position, Color color, FigureType::Type type) {
    switch (type) {
    case FigureType::Type::Pawn:
        return new Figure(this->id++, position, color, FigureType::Type::Pawn);
    case FigureType::Type::Knight:
        return new Figure(this->id++, position, color, FigureType::Type::Knight);
    case FigureType::Type::Rook:
        return new Figure(this->id++, position, color, FigureType::Type::Rook);
    case FigureType::Type::Bishop:
        return new Figure(this->id++, position, color, FigureType::Type::Bishop);
    case FigureType::Type::Queen:
        return new Figure(this->id++, position, color, FigureType::Type::Queen);
    case FigureType::Type::King:
        return new Figure(this->id++, position, color, FigureType::Type::King);
    case FigureType::Type::None:
        return new Figure(this->id++, position, Color::Type::None, FigureType::Type::None);
    default:
        return get_default_fig();
    }
}

Figure* FigureFabric::get_default_fig() {
    if (not DEFAULT->empty()) throw std::logic_error("default figure was deleted *.*"); // на время разработки - очень часто вылезает эта ошибка
    return DEFAULT;
}

Figure* FigureFabric::create(Figure* to_copy) {
    if (to_copy->empty()) return DEFAULT;
    return create(
        to_copy->get_pos(),
        to_copy->get_col(),
        to_copy->get_type(),
        to_copy->get_id()
    );
}