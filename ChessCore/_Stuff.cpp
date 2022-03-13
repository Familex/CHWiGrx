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