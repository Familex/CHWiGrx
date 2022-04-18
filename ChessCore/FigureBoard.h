#pragma once
#include "Stuff.h"

class FigureBoard {
public:
    FigureBoard(BoardRepr);
    void reset(const BoardRepr&);
    std::list<Figure>::iterator get_fig(pos);
    std::list<Figure>::iterator get_fig(Id);
    bool cont_fig(pos);
    bool is_empty(pos);
    bool is_empty() { return figures.size() <= 1; }
    bool capture_figure(std::list<Figure>::iterator);
    bool capture_figure(const Id&);
    void uncapture_figure(const Id&);
    std::list<Figure>::iterator find_king(Color);
    std::vector<Figure> get_figures_of(Color);
    std::vector<std::pair<bool, pos>> expand_broom(const Figure&, const std::vector<pos>& = {}, const std::vector<pos>& = {}, const std::vector<pos>& = {});
    std::vector<std::pair<bool, pos>> get_all_possible_moves(const Figure&, const std::vector<pos>& = {}, const std::vector<pos>& = {}, const std::vector<pos>& = {});
    bool checkmate_for(Color, const std::vector<pos>& = {}, pos = {});
    bool stalemate_for(Color, const std::vector<pos>& = {}, pos = {});
    bool check_for_when(Color, const std::vector<pos>& = {}, pos = {}, const std::vector<Figure>& = {}, const std::vector<Figure>& = {});
    MoveMessage move_check(std::list<Figure>::iterator, Input);
    std::tuple<bool, MoveMessage, std::list<Figure>::iterator, std::list<Figure>::iterator> castling_check(MoveMessage, std::list<Figure>::iterator, const Input&, int, int);
    void reset_castling();
    std::list<Figure>::iterator get_default_fig() { return figures.begin(); }
    std::list<Figure> all_figures() {
        std::list<Figure> tmp{ ++figures.begin(), figures.end() };
        return tmp;
    }
    bool has_castling(Color col, Id id) { return castling[col][id]; }
    void off_castling(Color col, Id id) { castling[col][id] = false; }
    void on_castling(Color col, Id id)  { castling[col][id] = true; }
    MoveRec get_last_move() { return move_logger.get_last_move(); }
    void set_last_move(const MoveRec& move_rec) { this->move_logger.add(move_rec); }
    template <typename Func> std::pair<bool, MoveRec> provide_move(std::list<Figure>::iterator, const Input&, Color turn, const Func&);
    bool provide_move(const MoveRec&);
    bool undo_move();
    bool restore_move();
    void apply_map(const BoardRepr&);
    void place_figure(const Figure& fig) { figures.push_back(fig); }
    void init_figures_moves();
    bool game_end(Color);
    size_t cnt_of_figures() const { return figures.size() - 1; }
    bool insufficient_material();
    BoardRepr get_repr(bool);
private:
    bool idw{true};
    Id curr_id{};
    MoveLogger move_logger{};
    std::list<Figure> figures;
    std::list<Figure> captured_figures;
    std::map<Color, std::map<Id, bool>> castling;
    std::map<FigureType, std::vector<std::vector<pos>>> moves;
    std::map<FigureType, std::vector<std::vector<pos>>> eats;
    const HANDLE console{ GetStdHandle(STD_OUTPUT_HANDLE) };
    const int MAX_FIGURES_AMOUNT{ HEIGHT * WIDTH };
};

/// <summary>
/// Производит ход
/// </summary>
/// <typeparam name="Func">Тип функции, возвращающей тип фигуры</typeparam>
/// <param name="in_hand">Текущая фигура</param>
/// <param name="input">Ввод</param>
/// <param name="turn">Текущий ход</param>
/// <param name="get_choise">Функция, возвращающая тип фигуры для превращения</param>
/// <returns></returns>
template <typename Func>
std::pair<bool, MoveRec> FigureBoard::provide_move(std::list<Figure>::iterator in_hand, const Input& input, Color turn, const Func& get_choise) {
    char choice = get_choise();
    MoveMessage ms{};
    try {
        ms = move_check(in_hand, input);
    }
    catch (std::invalid_argument&) {
        return { false, {} };
    }

    MoveRec curr_move{ *in_hand, input, turn, ms, choice };

    if (!provide_move(curr_move)) {
        return { false, {} };
    }

    return { true, curr_move };
}