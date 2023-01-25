#pragma once

#include "stuff/board_repr.h"
#include "stuff/move_logger.h"
#include <variant>

class FigureBoard {
    /*  x-axis from top to bottom (↓)  **
    **  y-axis from left to right (→)  */
public:
    FigureBoard(BoardRepr&&);
    void reset(BoardRepr&&);
    Figure* get_fig(Pos) const;
    Figure* get_fig(Id) const;
    bool cont_fig(Pos) const;
    bool is_empty(Pos) const;
    bool is_empty() const { return figures.size() <= 1; }
    bool capture_figure(Figure*);
    bool capture_figure(const Id);
    void uncapture_figure(const Id);
    void delete_fig(Pos);
    void place_fig(Figure*);
    Figure* find_king(Color) const;
    std::vector<Figure*> get_figures_of(Color) const;
    std::vector<std::pair<bool, Pos>> expand_broom(const Figure*, 
                                                   const std::vector<Pos> & = {},
                                                   const std::vector<Pos> & = {}, 
                                                   const std::vector<Pos> & = {}) const;
    std::vector<std::pair<bool, Pos>> get_all_moves(const Figure*, 
                                                    const std::vector<Pos> & = {},
                                                    const std::vector<Pos> & = {},
                                                    const std::vector<Pos> & = {}) const;
    std::vector<std::pair<bool, Pos>> get_all_possible_moves(const Figure*,
                                                             const std::vector<Pos> & = {},
                                                             const std::vector<Pos> & = {},
                                                             const std::vector<Pos> & = {}) const;
    bool checkmate_for(Color, 
                       const std::vector<Pos>& = {}, 
                       Pos = {}) const;
    bool stalemate_for(Color,
                       const std::vector<Pos>& = {}, 
                       Pos = {}) const;
    bool check_for_when(Color, 
                        const std::vector<Pos>& = {},
                        Pos = {}, 
                        const std::vector<Figure*>& = {}, 
                        const std::vector<Figure*>& = {}) const;
    std::variant<ErrorEvent, MoveMessage> move_check(Figure*, 
                                                     Input) const;
    std::tuple<bool, MoveMessage, Figure*, Figure*> castling_check(MoveMessage, 
                                                                   Figure*, 
                                                                   const Input&, 
                                                                   int,  
                                                                   int) const;
    void reset_castling(bool=true);
    void reset_castling(const BoardRepr&);
    Figure* get_default_fig() const { return FigureFabric::instance()->get_default_fig(); }
    inline bool get_idw() const { return idw; }
    inline void set_idw(bool new_idw) {
        idw = new_idw;
        init_figures_moves();
    }
    std::vector<Figure*> get_all_figures() const {
        std::vector<Figure*> tmp;
        for (const auto& [_, fig] : figures) {
            tmp.push_back(fig);
        }
        return tmp;
    }
    void move_fig(Figure* fig, Pos to, bool capture=true) {
        Figure* maybe_eat = get_fig(to);
        if (not maybe_eat->empty()) {
            if (capture) {
                capture_figure(maybe_eat);
            }
            else {
                figures.erase(maybe_eat->get_pos());
                delete maybe_eat;
            }
        }
        figures.erase(fig->get_pos());
        fig->move_to(to);
        figures[fig->get_pos()] = fig;
    }
    void move_fig(Input input, bool capture=true) {
        move_fig(get_fig(input.from), input.target, capture);
    }
    bool has_castling(Id id) const { 
        if (castling.contains(id))
            return castling.at(id); 
        return false;
    }
    void off_castling(Id id) { castling[id] = false; }
    void on_castling(Id id)  { castling[id] = true; }
    std::vector<MoveRec> get_last_moves() const { return move_logger.get_past(); }
    std::vector<MoveRec> get_future_moves() const { return move_logger.get_future(); }
    MoveRec get_last_move() const { return move_logger.get_last_move(); }
    void set_last_move(const MoveRec& move_rec) { this->move_logger.add(move_rec); }
    template <typename Func> 
        requires std::is_invocable_v<Func>&& std::is_same_v<std::invoke_result_t<Func>, FigureType>
    std::pair<bool, MoveRec> provide_move(Figure*, const Input&, Color turn, const Func&);
    bool provide_move(const MoveRec&);
    bool undo_move();
    bool restore_move();
    void apply_map(BoardRepr&&);
    void place_figure(Figure* fig) { figures[fig->get_pos()] = fig; }
    void init_figures_moves();
    GameEndType game_end_check(Color) const;
    void promotion_fig(Figure*, FigureType);
    size_t cnt_of_figures() const { return figures.size(); }
    bool insufficient_material() const;
    BoardRepr get_repr(Color, bool) const;
    ~FigureBoard() {
        for (auto& [_, fig] : figures) {
            if (not fig->empty())
                delete fig;
        }
        for (auto& fig : captured_figures) {
            if (not fig->empty())
                delete fig;
        }
    }
private:
    using shift_broom = std::vector<std::vector<Pos>>;

    bool idw{true};

    Id curr_id{};
    MoveLogger move_logger{};
    std::map<Pos, Figure*> figures;
    std::vector<Figure*> captured_figures;
    std::map<Id, bool> castling;
    std::map<FigureType, shift_broom> moves;
    std::map<FigureType, shift_broom> eats;

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
    requires std::is_invocable_v<Func>&& std::is_same_v<std::invoke_result_t<Func>, FigureType>
std::pair<bool, MoveRec> FigureBoard::provide_move(Figure* in_hand, const Input& input, Color turn, const Func& get_choise) {
    auto choice = get_choise();
    auto ms = move_check(in_hand, input);
    if (std::holds_alternative<ErrorEvent>(ms)) {
        return { false, {} };
    }

    MoveRec curr_move{ in_hand, input, turn, std::get<1>(ms), choice };

    if (!provide_move(curr_move)) {
        return { false, {} };
    }

    return { true, curr_move };
}

bool is_valid_coords(Pos);
