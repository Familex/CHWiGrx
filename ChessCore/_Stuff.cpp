// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "_Stuff.h"

FigureType::FigureType(char ch) {
    switch (ch)
    {
    case 'K': case 'k': data = EFigureType::King; break;
    case 'H': case 'h': data = EFigureType::Knight; break;
    case 'P': case 'p': data = EFigureType::Pawn; break;
    case 'B': case 'b': data = EFigureType::Bishop; break;
    case 'Q': case 'q': data = EFigureType::Queen; break;
    case 'R': case 'r': data = EFigureType::Rook; break;
    case 'N': case 'n': data = EFigureType::None; break;
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
    if (meta_start == npos || meta_end == npos) {
        figures = board_repr;
        return;
    }
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
    // TODO
    board_repr.erase(board_repr.begin() + past_start, board_repr.begin() + past_end + 1);
    size_t future_start = board_repr.find('<');
    size_t future_end = board_repr.find('>');
    // TODO
    board_repr.erase(board_repr.begin() + future_start, board_repr.begin() + future_end + 1);
    figures = board_repr;
    //TODO
}

std::string MoveRec::as_string() {
    std::string result{ "" };
    /*
    who_went.id;
    who_went.color;
    who_went.position.x;
    who_went.position.y;
    who_went.color;
    who_went.type;
    
    input.from.x;
    input.from.y;
    input.target.x;
    input.target.y;

    turn; // same as who_went.color;

    ms.main_ev;
    ms.side_evs; // list -> side_event
    ms.to_eat; // vector -> figure(!)
    ms.to_move; // list -> pair -> figure, input
    ms.what_castling_breaks; // list -> id

    promotion_choice;
    */
    return result;
}

std::string BoardRepr::as_string() {
    std::string result{ "" };
    result += std::format("{}[{}{}]<", figures, get_idw_char(), get_turn_char());
    for (auto& mr : past) {
        result += mr.as_string() + ", ";
    }
    result += "><";
    for (auto& mr : future) {
        result += mr.as_string() + ", ";
    }
    return result + ">";
}