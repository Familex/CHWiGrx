#pragma once
#include "_Stuff.h"

enum class MainEvent { E, EAT, MOVE, LMOVE, CASTLING, EN_PASSANT };
enum class SideEvent { E, CHECK, PROMOTION, CASTLING_BREAK };

struct MoveMessage {
    MainEvent main_ev{ MainEvent::E };
    std::list<SideEvent> side_evs;
    std::vector<std::list<Figure>::iterator> to_eat;
    std::list<std::pair<std::list<Figure>::iterator, pos>> to_move;
    std::list<Id> what_castling_breaks;
};

struct LastMove {
    MoveMessage ms;
    std::list<Figure>::iterator who_went;
};

class FigureBoard {
public:
    FigureBoard(bool = true, std::string = "");
    void reset(bool = true, std::string = "");
    std::list<Figure>::iterator get_fig(pos);
    bool cont_fig(pos);
    bool is_empty(pos);
    bool is_empty() { return figures.size() <= 1; }
    bool capture_figure(std::list<Figure>::iterator);
    std::list<Figure>::iterator find_king(Color);
    std::vector<Figure> get_figures_of(Color);
    std::list<std::pair<bool, pos>> expand_broom(Figure, const std::vector<pos>& = {}, const std::vector<pos>& = {}, const std::vector<pos>& = {});
    std::list<std::pair<bool, pos>> get_all_possible_moves(Figure, const std::vector<pos>& = {}, const std::vector<pos>& = {}, const std::vector<pos>& = {});
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
    LastMove get_last_move() { return lm; }
    void set_last_move(const LastMove& lm) { this->lm = lm; }
    void append_figures(const std::string&);
    void init_figures_moves(bool);
    bool game_end(Color);
    size_t cnt_of_figures() const { return figures.size() - 1; }
    bool insufficient_material();
private:
    bool idw;
    Id curr_id{};
    LastMove lm{};
    std::list<Figure> figures;
    std::list<Figure> captured_figures;
    std::map<Color, std::map<Id, bool>> castling;
    std::map<FigureType, std::vector<std::vector<pos>>> moves;
    std::map<FigureType, std::vector<std::vector<pos>>> eats;
    const HANDLE console{ GetStdHandle(STD_OUTPUT_HANDLE) };
    const int MAX_FIGURES_AMOUNT{ HEIGHT * WIDTH };
};