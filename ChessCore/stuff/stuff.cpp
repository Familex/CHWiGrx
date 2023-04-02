#include "Stuff.h"

#include <string>

// Конструктор ввода    /// @todo fix this
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
        else { 
            assert(!"unexpected end of line"); 
        }
    }
    from.x = acc[0];
    from.y = acc[1];
    target.x = acc[2];
    target.y = acc[3];
}

// Преобразование в строку (только один символ)
std::string to_string(SideEvent side_event) noexcept {
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

std::wstring to_wstring(SideEvent side_event) noexcept {
    switch (side_event)
    {
        case SideEvent::E:
            return L"E";
        case SideEvent::CHECK:
            return L"C";
        case SideEvent::PROMOTION:
            return L"P";
        case SideEvent::CASTLING_BREAK:
            return L"B";
        default:
            return L"N";
    }
}

// Преобразование в строку (только один символ)
std::string to_string(MainEvent main_event) noexcept {
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

std::wstring to_wstring(MainEvent main_event) noexcept {
    switch (main_event)
    {
        case MainEvent::E:
            return L"E";
        case MainEvent::EAT:
            return L"T";
        case MainEvent::MOVE:
            return L"M";
        case MainEvent::LMOVE:
            return L"L";
        case MainEvent::CASTLING:
            return L"C";
        case MainEvent::EN_PASSANT:
            return L"P";
        default:
            return L"N";
    }
}

std::string to_string(FigureType figure_type) noexcept
{
    switch (figure_type)
    {
        case FigureType::Pawn:
            return "Pawn";
        case FigureType::Rook:
            return "Rook";
        case FigureType::Knight:
            return "Knight";
        case FigureType::Bishop:
            return "Bishop";
        case FigureType::Queen:
            return "Queen";
        case FigureType::King:
            return "King";
        case FigureType::None:
            return "None";
        default:
            return "Error";
    }
}

std::wstring to_wstring(FigureType figure_type) noexcept
{
    switch (figure_type)
    {
        case FigureType::Pawn:
            return L"Pawn";
        case FigureType::Rook:
            return L"Rook";
        case FigureType::Knight:
            return L"Knight";
        case FigureType::Bishop:
            return L"Bishop";
        case FigureType::Queen:
            return L"Queen";
        case FigureType::King:
            return L"King";
        case FigureType::None:
            return L"None";
        default:
            return L"Error";
    }
}
