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

FigureType char_to_figure_type(char ch) {
    switch (ch)
    {
    case 'K': case 'k': return FigureType::King;  
    case 'H': case 'h': return FigureType::Knight;
    case 'P': case 'p': return FigureType::Pawn;  
    case 'B': case 'b': return FigureType::Bishop;
    case 'Q': case 'q': return FigureType::Queen; 
    case 'R': case 'r': return FigureType::Rook;  
    case 'N': case 'n': return FigureType::None;  
    default:
        throw std::invalid_argument("Can't parse '" + std::string(ch, 1) + "' - figure type");
    }
}

Color char_to_col(char ch) {
    switch (ch) {
    case 'w': case 'W': return Color::White;
    case 'B': case 'b': return Color::Black;
    case 'N': case 'n': return Color::None;
    default:
        throw std::invalid_argument("Can't parse '" + std::string(ch, 1) + "' - color");
    }
}

Color what_next(Color col) {
    switch (col) {
    case Color::Black:
        return Color::White;
    case Color::White:
        return Color::Black;
    default:
        throw std::invalid_argument("unaccepted color");
    }
}

char figure_type_to_char(FigureType type) {
    switch (type) {
    case FigureType::Pawn: return 'P';
    case FigureType::Rook: return 'R';
    case FigureType::Knight: return 'H';
    case FigureType::Bishop: return 'B';
    case FigureType::Queen: return 'Q';
    case FigureType::King: return 'K';
    case FigureType::None: return 'N';
    default:
        return 'e';
    }
}

char col_to_char(Color col) {
    switch (col) {
    case Color::Black: return 'B';
    case Color::White: return 'W';
    case Color::None: return 'N';
    default:
        return 'e';
    }
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

// Преобразование в строку (только один символ)
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

// Преобразование в строку (только один символ)
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

