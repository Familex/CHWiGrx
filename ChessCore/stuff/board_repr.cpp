#include "board_repr.h"

std::string BoardRepr::as_string() {
    std::string result{ "" };
    for (const auto& fig : figures) {
        auto pos = fig->get_pos();
        result += std::format("{};{};{};{};{};",
            fig->get_id(), pos.x, pos.y, col_to_char(fig->get_col()), figure_type_to_char(fig->get_type())
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
            fig->get_id(), fig->get_pos().x, fig->get_pos().y, col_to_char(fig->get_col()), figure_type_to_char(fig->get_type())
        );
    }
    return result;
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
        turn = Color::White;
    }
    else if (meta.find('b') != npos || meta.find('B') != npos) {
        turn = Color::Black;
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
        Id new_id = std::stoi(tmp[i]);
        Pos new_pos = { std::stoi(tmp[i + 1]), std::stoi(tmp[i + 2]) };
        Color new_col = char_to_col(tmp[i + 3][0]);
        FigureType new_type = char_to_figure_type(tmp[i + 4][0]);
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
        Pos new_pos = { std::stoi(tmp[i + 1]), std::stoi(tmp[i + 2]) };
        Color new_col = char_to_col(tmp[i + 3][0]);
        FigureType new_type = char_to_figure_type(tmp[i + 4][0]);
        Figure* new_fig = FigureFabric::instance()->create(
            new_pos, new_col, new_type, new_id
        );
        figures.push_back(new_fig);
    }
}

void BoardRepr::set_figures(std::vector<Figure*>&& figs) {
    for (auto prev_fig : figures)
    {
        delete prev_fig;
    }
    figures = figs;
}

void BoardRepr::clear()
{
    for (auto fig : figures)
    {
        delete fig;
    }
    figures.clear();
    for (auto fig : captured_figures)
    {
        delete fig;
    }
    captured_figures.clear();
    can_castle.clear();
    past.clear();
    future.clear();
}

BoardRepr* BoardRepr::operator =(const BoardRepr& other) {
    this->clear();
    figures = other.figures;
    turn = other.turn;
    idw = other.idw;
    past = other.past;
    future = other.future;
    captured_figures = other.captured_figures;
    can_castle = other.can_castle;
    return this;
}
