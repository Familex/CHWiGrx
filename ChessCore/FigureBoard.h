#pragma once
#include "Stuff.h"

class FigureBoard {
public:
    FigureBoard(BoardRepr);
    void reset(const BoardRepr&);
    Figure* get_fig(pos);
    Figure* get_fig(Id);
    bool cont_fig(pos);
    bool is_empty(pos);
    bool is_empty() { return figures.size() <= 1; }
    bool capture_figure(Figure*);
    bool capture_figure(const Id&);
    void uncapture_figure(const Id&);
    Figure* find_king(Color);
    std::vector<Figure*> get_figures_of(Color);
    std::vector<std::pair<bool, pos>> expand_broom(const Figure*, const std::vector<pos>& = {}, const std::vector<pos>& = {}, const std::vector<pos>& = {});
    std::vector<std::pair<bool, pos>> get_all_possible_moves(const Figure*, const std::vector<pos>& = {}, const std::vector<pos>& = {}, const std::vector<pos>& = {});
    bool checkmate_for(Color, const std::vector<pos>& = {}, pos = {});
    bool stalemate_for(Color, const std::vector<pos>& = {}, pos = {});
    bool check_for_when(Color, const std::vector<pos>& = {}, pos = {}, const std::vector<Figure*>& = {}, const std::vector<Figure*>& = {});
    MoveMessage move_check(Figure*, Input);
    std::tuple<bool, MoveMessage, Figure*, Figure*> castling_check(MoveMessage, Figure*, const Input&, int, int);
    void reset_castling();
    Figure* get_default_fig() { return FigureFabric::instance()->get_default_fig(); }
    std::list<Figure*> all_figures() {
        std::list<Figure*> tmp;
        for (const auto [_, fig] : figures) {
            tmp.push_back(fig);
        }
        return tmp;
    }
    void move_fig(Figure* fig, pos to) {
        Figure* maybe_eat = get_fig(to);
        if (not maybe_eat->empty()) {
            capture_figure(maybe_eat);
        }
        figures.erase(fig->get_pos());
        fig->move_to(to);
        figures[fig->get_pos()] = fig;
    }
    bool has_castling(Color col, Id id) { return castling[col][id]; }
    void off_castling(Color col, Id id) { castling[col][id] = false; }
    void on_castling(Color col, Id id)  { castling[col][id] = true; }
    MoveRec get_last_move() { return move_logger.get_last_move(); }
    void set_last_move(const MoveRec& move_rec) { this->move_logger.add(move_rec); }
    template <typename Func> std::pair<bool, MoveRec> provide_move(Figure*, const Input&, Color turn, const Func&);
    bool provide_move(const MoveRec&);
    bool undo_move();
    bool restore_move();
    void apply_map(const BoardRepr&);
    void place_figure(Figure*& fig) { figures[fig->get_pos()] = fig; }
    void init_figures_moves();
    bool game_end(Color);
    void promotion_fig(Figure*, FigureType);
    size_t cnt_of_figures() const { return figures.size() - 1; }
    bool insufficient_material();
    BoardRepr get_repr(bool);
private:
    bool idw{true};
    Id curr_id{};
    MoveLogger move_logger{};
    std::map<pos, Figure*> figures;
    std::list<Figure*> captured_figures;
    std::map<Color, std::map<Id, bool>> castling;
    std::map<FigureType, std::vector<std::vector<pos>>> moves;
    std::map<FigureType, std::vector<std::vector<pos>>> eats;
    const HANDLE console{ GetStdHandle(STD_OUTPUT_HANDLE) };
    const int MAX_FIGURES_AMOUNT{ HEIGHT * WIDTH };
};

/// <summary>
/// ���������� ���
/// </summary>
/// <typeparam name="Func">��� �������, ������������ ��� ������</typeparam>
/// <param name="in_hand">������� ������</param>
/// <param name="input">����</param>
/// <param name="turn">������� ���</param>
/// <param name="get_choise">�������, ������������ ��� ������ ��� �����������</param>
/// <returns></returns>
template <typename Func>
std::pair<bool, MoveRec> FigureBoard::provide_move(Figure* in_hand, const Input& input, Color turn, const Func& get_choise) {
    char choice = get_choise();
    MoveMessage ms{};
    try {
        ms = move_check(in_hand, input);
    }
    catch (std::invalid_argument&) {
        return { false, {} };
    }

    MoveRec curr_move{ in_hand, input, turn, ms, choice };

    if (!provide_move(curr_move)) {
        return { false, {} };
    }

    return { true, curr_move };
}