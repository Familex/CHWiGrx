#pragma once

#include "stuff/board_repr.h"
#include "stuff/move_logger.hpp"
#include <variant>

class FigureBoard {
    /*  x-axis from top to bottom (↓)  **
    **  y-axis from left to right (→)  */
    using shift_broom = std::vector<std::vector<Pos>>;

    bool idw{ true };
    Id curr_id{ };
    MoveLogger move_logger{ };
    std::map<Pos, Figure*> figures;
    std::vector<Figure*> captured_figures;
    std::map<Id, bool> castling;
    std::map<FigureType, shift_broom> moves;
    std::map<FigureType, shift_broom> eats;
    
public:
    [[nodiscard]] FigureBoard(BoardRepr&&) noexcept;
    void reset(BoardRepr&&) noexcept;
    void operator =(BoardRepr&&) noexcept;
    void operator =(const BoardRepr&) = delete;
    void apply_map(BoardRepr&&) noexcept;
    FigureBoard(const FigureBoard&) = delete;
    [[nodiscard]] Figure* const get_fig(const Pos) const noexcept;
    [[nodiscard]] Figure* const get_fig(const Id) const noexcept;
    [[nodiscard]] bool cont_fig(const Pos) const noexcept;
    [[nodiscard]] bool is_empty(const Pos) const noexcept;
    
    [[nodiscard]] bool is_empty() const noexcept 
        { return figures.size() <= 1ull; }
    
    bool capture_figure(Figure* const);
    bool capture_figure(const Id);
    void uncapture_figure(const Id);
    void delete_fig(const Pos);
    void place_fig(Figure* const);
    [[nodiscard]] const Figure* const find_king(const Color) const noexcept;
    [[nodiscard]] std::vector<Figure*> get_figures_of(const Color) const noexcept;
    [[nodiscard]] std::vector<std::pair<bool, Pos>> expand_broom(const Figure*, 
                                                                        const std::vector<Pos> & = {},
                                                                        const std::vector<Pos> & = {}, 
                                                                        const std::vector<Pos> & = {}) const noexcept;
    [[nodiscard]] std::vector<std::pair<bool, Pos>> get_all_moves(const Figure*,
                                                                         const std::vector<Pos> & = {},
                                                                         const std::vector<Pos> & = {},
                                                                         const std::vector<Pos> & = {}) const noexcept;
    [[nodiscard]] std::vector<std::pair<bool, Pos>> get_all_possible_moves(const Figure*,
                                                                                  const std::vector<Pos> & = {},
                                                                                  const std::vector<Pos> & = {},
                                                                                  const std::vector<Pos> & = {}) const noexcept;
    [[nodiscard]] bool checkmate_for(const Color,
                                     const std::vector<Pos>& = {}, 
                                     const Pos = {}) const noexcept;
    [[nodiscard]] bool stalemate_for(const Color,
                                     const std::vector<Pos>& = {}, 
                                     Pos = {}) const noexcept;
    [[nodiscard]] bool check_for_when(const Color,
                                      const std::vector<Pos>& = {},
                                      const Pos = {}, 
                                      const std::vector<Figure*>& = {}, 
                                      const std::vector<Figure*>& = {}) const noexcept;
    [[nodiscard]] std::variant<ErrorEvent, MoveMessage> move_check(const Figure* const,
                                                                          const Input&) const noexcept;
    [[nodiscard]] std::tuple<bool, MoveMessage, const Figure*, const Figure*> castling_check(MoveMessage,
                                                                                                    const Figure*, 
                                                                                                    const Input&, 
                                                                                                    const int,  
                                                                                                    const int) const noexcept;
    void reset_castling(const bool=true) noexcept;
    void reset_castling(const BoardRepr&) noexcept;
    
    [[nodiscard]] Figure* get_default_fig() const noexcept
        { return FigureFabric::instance()->get_default_fig(); }
    
    [[nodiscard]] inline bool get_idw() const noexcept
        { return idw; }
    
    [[nodiscard]] inline void set_idw(const bool new_idw) noexcept {
        idw = new_idw;
        init_figures_moves();
    }
    
    [[nodiscard]] std::vector<Figure*> get_all_figures() const {
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
    
    [[nodiscard]] bool has_castling(Id id) const noexcept {
        if (castling.contains(id))
            return castling.at(id); 
        return false;
    }
    
    void off_castling(Id id) noexcept
        { castling[id] = false; }
    
    void on_castling(Id id) noexcept
        { castling[id] = true; }
    
    [[nodiscard]] const std::vector<MoveRec>& get_last_moves() const noexcept
        { return move_logger.get_past(); }
    
    [[nodiscard]] const std::vector<MoveRec>& get_future_moves() const noexcept
        { return move_logger.get_future(); }
    
    [[nodiscard]] const MoveRec& get_last_move() const noexcept
        { return move_logger.get_last_move(); }
    
    void set_last_move(const MoveRec& move_rec) noexcept
        { this->move_logger.add(move_rec); }
    
    template <typename Func> 
        requires std::is_invocable_v<Func>&& std::is_same_v<std::invoke_result_t<Func>, FigureType>
    [[nodiscard]] std::pair<bool, MoveRec> provide_move(Figure*, const Input&, Color turn, const Func&);
    
    bool provide_move(const MoveRec&);
    
    bool undo_move();
    bool restore_move();

    void place_figure(Figure* const fig) noexcept
        { figures[fig->get_pos()] = fig; }
    
    void init_figures_moves() noexcept;
    [[nodiscard]] GameEndType game_end_check(const Color) const noexcept;
    void promotion_fig(Figure* , const FigureType);
    
    [[nodiscard]] size_t cnt_of_figures() const noexcept
        { return figures.size(); }
    
    [[nodiscard]] bool insufficient_material() const noexcept;
    [[nodiscard]] const BoardRepr& get_repr(const Color, const bool) const noexcept;
    
    ~FigureBoard() noexcept {
        for (auto& [_, fig] : figures) {
            if (not fig->empty())
                delete fig;
        }
        for (auto& fig : captured_figures) {
            if (not fig->empty())
                delete fig;
        }
    }
    
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

[[nodiscard]] constexpr bool is_valid_coords(const Pos position) noexcept
{
    const int x = position.x;   // use of constexpr?
    const int y = position.y;
    return ((x >= 0) && (x < HEIGHT) &&
        (y >= 0) && (y < WIDTH));
}
