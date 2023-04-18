#include "FigureBoard.h"

#include "structs/pos.hpp"
#include "structs/input.hpp"
#include "structs/move_message.hpp"
#include "structs/board_repr.hpp"

FigureBoard::FigureBoard(board_repr::BoardRepr&& board_repr) noexcept {
    reset(std::move(board_repr));
}

void FigureBoard::reset(board_repr::BoardRepr&& map) noexcept {
    move_logger_.reset();
    for (const auto& fig : figures_ | std::views::values) {
        delete fig;
    }
    figures_.clear();
    for (const auto& fig : captured_figures_) {
        delete fig;
    }
    captured_figures_.clear();
    reset_castling(map);
    apply_map(std::move(map));
    init_figures_moves();
}

void FigureBoard::init_figures_moves() noexcept {
    std::vector<Pos>temp_left_up;
    std::vector<Pos>temp_left_down;
    std::vector<Pos>temp_right_up;
    std::vector<Pos>temp_right_down;
    std::vector<Pos>temp_left;
    std::vector<Pos>temp_right;
    std::vector<Pos>temp_up;
    std::vector<Pos>temp_down;
    
    for (int ij = 1; ij < std::min(WIDTH, HEIGHT); ++ij) {
        temp_left_up.emplace_back(-ij, -ij);
        temp_left_down.emplace_back(ij, -ij);
        temp_right_up.emplace_back(-ij, ij);
        temp_right_down.emplace_back(ij, ij);
        temp_left.emplace_back(0, -ij);
        temp_right.emplace_back(0, ij);
        temp_up.emplace_back(-ij, 0);
        temp_down.emplace_back(ij, 0);
    }

    // Задаётся относительно белых через idw
    moves_[FigureType::Pawn] = { { Pos{ (idw_ ? -1 : 1), 0}} };
    eats_[FigureType::Pawn] = { { Pos{ (idw_ ? -1 : 1), -1}}, { Pos{ (idw_ ? -1 : 1), 1}} };

    moves_[FigureType::Knight] = { { Pos{-1, 2} }, { Pos{-1, -2} },
        { Pos{1, -2} }, { Pos{1, 2} }, { Pos{2, -1} }, { Pos{-2, -1} },
        { Pos{2, 1} }, { Pos{-2, 1} }, 
    };
    eats_[FigureType::Knight] = moves_[FigureType::Knight];

    moves_[FigureType::King] = { { Pos{-1, -1} }, { Pos{-1, 1} }, { Pos{-1, 0} },
        { Pos{0, -1} }, { Pos{0, 1} }, { Pos{1, 1} }, { Pos{1, -1} },
        { Pos{1, 0} }
    };
    eats_[FigureType::King] = moves_[FigureType::King];

    moves_[FigureType::Bishop] = { temp_left_up, temp_left_down, temp_right_up, temp_right_down };
    eats_[FigureType::Bishop] = moves_[FigureType::Bishop];

    moves_[FigureType::Rook] = { temp_left, temp_right, temp_up, temp_down };
    eats_[FigureType::Rook] = moves_[FigureType::Rook];

    moves_[FigureType::Queen] = { temp_left_up, temp_left_down, temp_right_up, temp_right_down, temp_left, temp_right, temp_up, temp_down };
    eats_[FigureType::Queen] = moves_[FigureType::Queen];

}

FigureBoard& FigureBoard::operator= (board_repr::BoardRepr&& board_repr) noexcept
{
    reset(std::move(board_repr));
    return *this;
}

void FigureBoard::apply_map(board_repr::BoardRepr&& board_repr) noexcept {
    idw_ = board_repr.idw;
    move_logger_.set_past(board_repr.past);
    move_logger_.set_future(board_repr.future);
    for (const auto& cap_fig : board_repr.captured_figures) {
        captured_figures_.push_back( cap_fig );  /* move ownership */
    }
    board_repr.captured_figures.clear();    /* delete ownership */
    for (const auto& fig : board_repr.figures) {
        figures_[fig->get_pos()] = fig;  /* move ownership */
    }
    board_repr.figures.clear();  /* delete ownership */
}

auto FigureBoard::
    get_repr(const Color turn, const bool save_all_moves) const noexcept
    -> board_repr::BoardRepr
{
    std::vector<Figure*> fig_vec;
    for (const auto& fig : figures_ | std::views::values)
        fig_vec.push_back(fig);
    const auto& prev_moves = move_logger_.get_past();
    const auto& has_castlings =
        castling_
        | std::views::filter([](const auto& pair) { return pair.second; })
        | std::views::keys
        | std::ranges::to<std::vector<Id>>();
    return board_repr::BoardRepr{
        std::move(fig_vec),
        turn,
        idw_,
        save_all_moves
            ? prev_moves
            : prev_moves.empty()
                ? std::vector<mvmsg::MoveMessage>{}
                : std::vector{ prev_moves.back() },
        move_logger_.get_future(),
        captured_figures_,
        has_castlings 
    };
}

void FigureBoard::
    reset_castling(const bool castle_state) noexcept
{
    castling_.clear();
    for (const Color& col : { Color::Black, Color::White }) {
        for (const auto& aspt_to_rook : get_figures_of(col)) {
            if (aspt_to_rook->is(FigureType::Rook)) {
                castling_[aspt_to_rook->get_id()] = castle_state;
            }
        }
    }
}

void FigureBoard::
    reset_castling(const board_repr::BoardRepr& board_repr) noexcept
{
    reset_castling(false);
    for (Id castle_id : board_repr.can_castle) {
        castling_[castle_id] = true;
    }
}

auto FigureBoard::
    get_fig_unsafe(const Pos position) const noexcept -> Figure*
{
    const auto out = get_fig(position);
    return out.has_value() ? out.value() : nullptr;
}

auto FigureBoard::
    get_fig_unsafe(const Id id) const noexcept -> Figure*
{
    const auto out = get_fig(id);
    return out.has_value() ? out.value() : nullptr;
}

/// <summary>
/// Показывает содержит ли клетка фигуру
/// </summary>
bool FigureBoard::cont_fig(const Pos position) const noexcept {
    return figures_.contains(position);
}

/// <summary>
/// Показывает не содержит ли клетка фигуру
/// </summary>
bool FigureBoard::is_empty(const Pos position) const noexcept {
    return !figures_.contains(position);
}

/// <summary>
/// Убирает фигуру с доски и добавляет её к съеденным
/// </summary>
/// <param name="it">Итератор на фигуру</param>
/// <returns>Получилось ли съесть</returns>
bool FigureBoard::capture_figure(Figure* const it) {
    captured_figures_.push_back(it);
    figures_.erase(it->get_pos());
    return true;
}

/// <summary>
/// Убирает фигуру с доски и добавляет её к съеденным
/// </summary>
/// <param name="id">Идентификатор фигуры</param>
/// <returns>Получилось ли съесть</returns>
bool FigureBoard::capture_figure(const Id id) {
    if (const auto fig = get_fig(id)) {
        captured_figures_.push_back(fig.value());
        figures_.erase(fig.value()->get_pos());
        return true;
    }
    else {
        return false;
    }
}

void FigureBoard::delete_fig(const Pos pos) {
    delete figures_[pos];
    figures_.erase(pos);
}

void FigureBoard::place_fig(Figure* const fig) {
    if (cont_fig(fig->get_pos())) delete_fig(fig->get_pos());
    figures_[fig->get_pos()] = fig;
}

/// <param name="col">Цвет фигур</param>
/// <returns>Все фигуры определённого цвета</returns>
auto FigureBoard::
    get_figures_of(Color col) const noexcept -> std::vector<Figure*>
{
    std::vector<Figure*> acc{};
    for (const auto& fig : figures_ | std::views::values) {
        if (fig->is_col(col)) {
            acc.push_back(fig);
        }
    }
    return acc;
}

auto FigureBoard::
    find_king(Color col) const noexcept -> std::optional<const Figure*>
{
    if (const auto map_ptr =
        std::ranges::find_if(
            figures_, 
      [col](const auto& it) {
                return it.second->is_col(col) && it.second->get_type() == FigureType::King;
            }
        ); map_ptr == figures_.end()) {
        return std::nullopt;
    }
    else {
        return map_ptr->second;
    }
}

/// <summary>
/// Используя швабру ходов искомой фигуры, возвращает
/// доступные ей ходы
/// </summary>
/// <param name="in_hand">Фигура, которая ходит</param>
/// <param name="to_ignore">Фигуры, которые нужно игнорировать</param>
/// <param name="ours">Фигуры, в которые врезаемся, но не можем съесть</param>
/// <param name="enemies">Фигуры, в которые врезаемся и можем съесть</param>
/// <returns>Серия из пар ест ли фигура и куда попадает</returns>
auto FigureBoard::
    expand_broom(const Figure* in_hand, const std::vector<Pos>& to_ignore, const std::vector<Pos>& ours, 
                       const std::vector<Pos>& enemies) const noexcept -> std::vector<std::pair<bool, Pos>>
{
    std::vector<std::pair<bool, Pos>> possible_moves{}; // list { pair{ is_eat, targ }, ... }
    const auto in_hand_pos = in_hand->get_pos();
    for (const auto in_hand_type = in_hand->get_type();
        const std::vector<Pos>& eat_beams : eats_.at(in_hand_type))
    {
        for (Pos eat : eat_beams) {
            Pos curr{ in_hand_pos + (in_hand->is_col(Color::White) ? eat : eat.mul_x(-1)) };
            if (!is_valid_coords(curr)) continue;
            if (curr.in(ours)) {
                break; // Врезались в союзника
            }
            else if (curr.in(enemies)) {
                possible_moves.emplace_back(true, curr); // Съели противника
                break; // И врезались...
            }
            else if (cont_fig(curr) && !curr.in(to_ignore)) {
                if (!get_fig(curr).value()->is_col(in_hand->get_col())) {
                    possible_moves.emplace_back(true, curr);
                    break; // Врезались
                }
                else {
                    break; // Врезались
                }
            }
        }
    }
    for (const auto& move_beams : moves_.at(in_hand->get_type())) {
        for (Pos move : move_beams) {
            Pos curr{ in_hand_pos + (in_hand->is_col(Color::White) ? move : move.mul_x(-1)) };
            if (not is_valid_coords(curr)) continue;
            if (not ((not curr.in(to_ignore) && cont_fig(curr)) || curr.in(ours) || curr.in(enemies))) {
                possible_moves.emplace_back(false, curr);
            }
            else {
                break; // Врезались
            }
        }
    }

    return possible_moves;
}

/// <summary>
/// Возвращает доступные фигуре ходы, подключая к
/// её швабре ещё и особые случаи
/// </summary>
/// <param name="in_hand">Фигура, которая ходит</param>
/// <param name="to_ignore">Фигуры, которые нужно игнорировать</param>
/// <param name="ours">Фигуры, в которые врезаемся, но не можем съесть</param>
/// <param name="enemies">Фигуры, в которые врезаемся и можем съесть</param>
/// <returns>Серия из пар ест ли фигура и куда попадает</returns>
auto FigureBoard::
    get_all_moves(const Figure* in_hand, const std::vector<Pos>& to_ignore, 
                    const std::vector<Pos>& ours,
                      const std::vector<Pos>&enemies) const noexcept -> std::vector<std::pair<bool, Pos>>
{
    std::vector<std::pair<bool, Pos>> all_moves{ expand_broom(in_hand, to_ignore, ours, enemies) };
    const Pos in_hand_pos = in_hand->get_pos();
    if (in_hand->get_type() == FigureType::Pawn) {
        // Ход пешки на 2 (смотрю свои фигуры на 2 линии)
        if (in_hand->is_col(Color::White)) {
            if (in_hand_pos.x == (HEIGHT - 2) && idw_ && is_empty(in_hand_pos + Pos(-1, 0)) &&
                is_empty(in_hand_pos + Pos(-2, 0))) {
                all_moves.emplace_back(false, in_hand_pos + Pos(-2, 0));
            }
            if (in_hand_pos.x == 1 && not idw_ && is_empty(in_hand_pos + Pos(1, 0)) &&
                is_empty(in_hand_pos + Pos(2, 0))) {
                all_moves.emplace_back(false, in_hand_pos + Pos(2, 0));
            }
        }
        if (in_hand->is_col(Color::Black)) {
            if (in_hand_pos.x == 1 && idw_ && is_empty(in_hand_pos + Pos(1, 0)) &&
                is_empty(in_hand_pos + Pos(2, 0))) {
                all_moves.emplace_back(false, in_hand_pos + Pos(2, 0));
            }
            if (in_hand_pos.x == (HEIGHT - 2) && not idw_ && is_empty(in_hand_pos + Pos(-1, 0)) &&
                is_empty(in_hand_pos + Pos(-2, 0))) {
                all_moves.emplace_back(false, in_hand_pos + Pos(-2, 0));
            }
        }

        // Взятие на проходе
        if (const auto& last_move_sus = move_logger_.get_last_move(); last_move_sus.has_value()) {
            const auto& last_move = last_move_sus.value();
            if (const Pos& who_went_at_last_move_pos = last_move.first.get_pos();
                std::holds_alternative<mvmsg::LongMove>(last_move.main_event) && std::abs(who_went_at_last_move_pos.y - in_hand_pos.y) == 1)
            {
                const int shift_y = who_went_at_last_move_pos.y - in_hand_pos.y;
                if (in_hand->is_col(Color::White)) {
                    if (in_hand_pos.x == (EN_PASSANT_INDENT - 1) && idw_ && cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(-1, shift_y))) {
                        all_moves.emplace_back(true, in_hand_pos + Pos(-1, shift_y));
                    }
                    if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && not idw_ && cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(1, shift_y))) {
                        all_moves.emplace_back(true, in_hand_pos + Pos(1, shift_y));
                    }
                }
                if (in_hand->is_col(Color::Black)) {
                    if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && idw_ && cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(1, shift_y))) {
                        all_moves.emplace_back(true, in_hand_pos + Pos(1, shift_y));
                    }
                    if (in_hand_pos.x == (EN_PASSANT_INDENT - 1) && not idw_ && cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(-1, shift_y))) {
                        all_moves.emplace_back(true, in_hand_pos + Pos(-1, shift_y));
                    }
                }
            }
        }
    }
    // FIXME use Figure::is
    if (in_hand->get_type() == FigureType::King) {
        for (auto [king_end_col, rook_end_col] : { std::pair(6, 5), std::pair(2, 3) } ) {
            if (const auto check_result_sus =
                castling_check(in_hand, Input{ in_hand_pos, Pos{in_hand_pos.x, king_end_col} }, king_end_col, rook_end_col))
            {
                if (const auto& [rook, king, second_figure_to_move] = check_result_sus.value(); has_castling(rook->get_id())) {
                    all_moves.push_back({ false, Pos{in_hand_pos.x, king_end_col} });
                }
            }
        }
    }
    if (in_hand->get_type() == FigureType::Rook) {
        for (auto [king_end_col, rook_end_col] : { std::pair(6, 5), std::pair(2, 3) }) {
            if (const auto check_result_sus =
                    castling_check(in_hand, Input{ in_hand_pos, Pos{in_hand_pos.x, rook_end_col} }, king_end_col, rook_end_col))
            {
                if (const auto& [rook, king, second_figure_to_move] = check_result_sus.value(); has_castling(rook->get_id())) {
                    all_moves.push_back({ false, Pos{in_hand_pos.x, rook_end_col} });
                }
            }
        }
    }

    return all_moves;
}

// get_all_moves with check for check
// FIXME to_ignore is not used
auto FigureBoard::
    get_all_possible_moves(const Figure* in_hand, const std::vector<Pos>& to_ignore,
                             const std::vector<Pos>& ours, const std::vector<Pos>& enemies) const noexcept
                               -> std::vector<std::pair<bool, Pos>>
{
    std::vector<std::pair<bool, Pos>> all_possible_moves;

    for (const auto& move : get_all_moves(in_hand))
    {
        auto moved_fig = figfab::FigureFabric::instance().submit_on(in_hand, move.second);
        const std::vector<Pos> ignore =
            move.first
            ? std::vector{ move.second, in_hand->get_pos() }
            : std::vector{ in_hand->get_pos() };
        if (const Pos to_defend =
            in_hand->is(FigureType::King)
                ? move.second
        //                                            extra argument when checking the king ↓
                : Pos{}; !check_for_when(in_hand->get_col(), ignore, to_defend, { moved_fig.get() }))
        {
            all_possible_moves.push_back( move );
        }
    }

    return all_possible_moves;
}

/// <summary>
/// Проверка доски на мат
/// </summary>
/// <param name="col">Цвет, для которого мат проверяется</param>
/// <param name="to_ignore">Фигуры, которые нужно игнорировать</param>
/// <param name="to_defend">Позиция короля или фигуры, которой можно поставить шах (мат)</param>
/// <returns>Наличие мата</returns>
auto FigureBoard::
    checkmate_for(const Color col, const std::vector<Pos>& to_ignore,
                    Pos to_defend /*тут, возможно, не король*/) const noexcept -> bool
{
    const auto king = find_king(col);
    if (to_defend == Pos()) {
        if (king.has_value()) {
            to_defend = king.value()->get_pos();
        }
        else {
            return false; // Нечего защищать
        }
    }
    for (const auto& figure : get_figures_of(col)) {
        for (const auto& [is_eat, curr] : expand_broom(figure, to_ignore, { to_defend })) {
            if (figure->get_type() == FigureType::King) {
                if (is_eat
                    ? !check_for_when(col, to_ignore + figure->get_pos() + curr + to_defend, curr)
                    : !check_for_when(col, to_ignore + figure->get_pos() + to_defend, curr)
                    ) {
                    return false;
                }
            }
            else {
                // FIXME move !check from end for readability
                auto tmp = figfab::FigureFabric::instance().submit_on(figure, curr);
                if (const bool check = 
                    is_eat
                        ? check_for_when(col, to_ignore + figure->get_pos() + curr, to_defend, { tmp.get() })
                        : check_for_when(col, to_ignore + figure->get_pos(), to_defend, { tmp.get() }); !check) {
                    return false;
                }
            }
        }
    }
    return true;
}

/// <summary>
/// Проверка доски на пат
/// </summary>
/// <param name="col">Цвет, для которого пат проверяется</param>
/// <param name="to_ignore">Фигуры, которые нужно игнорировать</param>
/// <param name="to_defend">Позиция короля или фигуры, которой можно поставить шах</param>
/// <returns>Наличие пата</returns>
auto FigureBoard::
    stalemate_for(const Color col, const std::vector<Pos>& to_ignore,
                    Pos to_defend) const noexcept -> bool
{
    const auto king_sus = find_king(col);
    if (!king_sus.has_value()) {
        return false; // Нет короля
    }
    const auto king = king_sus.value();
    if (to_defend == Pos()) to_defend = king->get_pos();
    for (const auto& aspt : get_figures_of(col)) {
        for (const auto& [is_eat, curr] : expand_broom(aspt)) {
            if (aspt->get_type() == FigureType::King) {
                if (is_eat
                    ? not check_for_when(col, to_ignore + curr + to_defend, curr)
                    : not check_for_when(col, to_ignore + to_defend, curr)
                    ) {
                    return false;
                }
            }
            else {
                auto tmp = figfab::FigureFabric::instance().submit_on(aspt, curr);
                if (const bool check =
                    is_eat
                        ? check_for_when(col, to_ignore + aspt->get_pos() + curr, to_defend, { tmp.get()}, {})
                        : check_for_when(col, to_ignore + aspt->get_pos(), to_defend, { tmp.get()}, {}); not check) {
                    return false;
                }
            }
        }
    }
    return true;
}

/// <summary>
/// Проверяет доску на шах
/// </summary>
/// <param name="col">Цвет, для которого шах проверяется</param>
/// <param name="to_ignore">Фигуры, которые нужно игнорировать</param>
/// <param name="to_defend">Позиция короля или фигуры, для которой проверяется шах</param>
/// <param name="ours">Фигуры, которые предотвращают шах</param>
/// <param name="enemies">Фигуры, которые шах могут поставить</param>
/// <returns>Наличие шаха</returns>
auto FigureBoard::
    check_for_when(const Color col, const std::vector<Pos>& to_ignore, Pos to_defend,
                     const std::vector<Figure*>& ours, const std::vector<Figure*>& enemies) const noexcept
                       -> bool
{
    const auto king_sus = find_king(col);
    if (to_defend == Pos()) {
        if (king_sus.has_value()) {
            to_defend = king_sus.value()->get_pos();
        }
        else {
            return false; // Нечего защищать
        }
    }
    for (const auto& enemy : get_figures_of(what_next(col)) + enemies) {
        if (std::ranges::find(to_ignore, enemy->get_pos()) != to_ignore.end()) {
            if (std::ranges::find(enemies, enemy) == enemies.end()) // Пока не нужно, но должно быть тут на всякий
                continue;
        }
        for (const auto& [is_eat, curr] : expand_broom(enemy, to_ignore, to_pos_vector(enemies), to_pos_vector(ours) + to_defend)) {
            if (is_eat && curr == to_defend) {
                return true;
            }
        }
    }
    return false; // Никто не атакует
}

/// <summary>
/// <para>Проверка валидности рокировки</para>
/// <para>Рокировка как в 960 (ширина обязательно 8)</para>
/// <para>Условия:</para>
/// <para>1. Король и рокируемая ладья не ходили ранее</para>
/// <para>2. Поля между начальной и конечной позицией короля и ладьи соответственно пусты</para>
/// <para>3. Король не проходит через битое поле, не находится под шахом и не встаёт под него</para>
/// <para><i>Ладья может быть под шахом</i></para>
/// <para><i>Король мог быть под шахом до этого</i></para>
/// <para><i>Рокировка сбрасывает все рокировки</i></para>
/// </summary>
/// <param name="in_hand">Фигура, которой собираются сходить</param>
/// <param name="input">Ввод для проверки</param>
/// <param name="king_end_col">Целевой столбец для короля</param>
/// <param name="rook_end_col">Целевой столбец для ладьи</param>
/// <returns>Если рокировка валидна, возвращает фигуру короля и ладьи</returns>
auto FigureBoard::
    castling_check(const Figure* in_hand, const Input& input,  
                     const int king_end_col, const int rook_end_col) const noexcept
                       -> std::optional<CastlingCheckResult>
{
    // Рокировка на g-фланг
    const auto king_sus = in_hand->get_type() == FigureType::King ? in_hand : find_king(in_hand->get_col());
    if (!king_sus.has_value()) {
        return std::nullopt;
    }
    const auto king = king_sus.value();
    if (Pos king_pos = king->get_pos(); (in_hand->is(FigureType::King) && input.target.y == king_end_col && in_hand->get_pos().y != king_end_col) ||
        (king_pos.y == king_end_col && in_hand->is(FigureType::Rook) && input.target.y == rook_end_col && in_hand->get_pos().y != rook_end_col))
    {
        bool castling_can_be_done = true;
        // input правильный (король уже мог быть на g вертикали, так что доступна возможность рокировки от ладьи)
        // Нужно проверить, что все промежуточные позиции (где идёт король) не под шахом
        const Figure* rook = (*figures_.begin()).second;
        int step{ king_end_col - king_pos.y > 0 ? 1 : -1 };
        for (Pos rook_aspt_pos{ king_pos };
            is_valid_coords(rook_aspt_pos); rook_aspt_pos.y += step) {
            if (auto rook_sus = get_fig(rook_aspt_pos);
                    rook_sus.has_value() && rook_sus.value()->is_col(in_hand->get_col()) && rook_sus.value()->get_type() == FigureType::Rook)
            {
                rook = rook_sus.value();
                break;
            }
        }
        if ((rook->get_type() != FigureType::Rook)
            || (king_pos.x != input.target.x && rook->get_pos().x != input.target.x) 
            ) {
            return std::nullopt;
        }
        
        Pos rook_pos = rook->get_pos();
        const Pos target{ king_pos.x, king_end_col };
        for (step = king_pos.y < target.y ? 1 : -1; king_pos.y != target.y; king_pos.y += step) {
            castling_can_be_done &= not check_for_when(in_hand->get_col(), { king_pos, rook_pos }, king_pos);
        }
        for (step = king_pos.y < rook_pos.y ? 1 : -1; king_pos.y != rook_pos.y; king_pos.y += step) {
            auto cell_sus = get_fig(king_pos);
            castling_can_be_done &= 
                !cell_sus.has_value() 
                || cell_sus.value()->get_id() == king->get_id() 
                || cell_sus.value()->get_id() == rook->get_id();
        }
        // FIXME can this be done in loop body?
        castling_can_be_done &= 
            !check_for_when(in_hand->get_col(), { king_pos, rook_pos }, Pos{ king_pos.x, king_end_col });
        const auto cell_sus = get_fig(Pos{ king_pos.x, rook_end_col });
        castling_can_be_done &= 
            !cell_sus.has_value()
            || cell_sus.value()->get_id() == king->get_id() 
            || cell_sus.value()->get_id() == rook->get_id();
        if (castling_can_be_done) {
            return CastlingCheckResult{
                rook,
                king,
                in_hand->is(FigureType::King)
                    ? Input{ rook_pos, Pos{rook_pos.x, rook_end_col} }
                    : Input{ king_pos, Pos{king_pos.x, king_end_col} }
            };
        }
    }
    return std::nullopt;
}

/// <summary>
/// <para>Проверка на недостаточный материал для мата</para>
/// <para>Варианты:</para>
/// <para>Король против короля</para>
/// <para>Король против короля и коня</para>
/// <para>Король против короля и слона</para>
/// <para>Король и слон против короля и слонов, где все слоны на одном цвете</para>
/// </summary>
/// <returns>Не хватает ли материала для мата</returns>
auto FigureBoard::
    insufficient_material() const noexcept -> bool
{
    const size_t size = cnt_of_figures();
    if (size <= 2) return true;
    if (size == 3 &&
            std::ranges::find_if(
                figures_,
                [](const auto& it) {
                    return it.second->get_type() == FigureType::Knight || it.second->get_type() == FigureType::Bishop;
                }
            ) != figures_.end()
        ) return true;
    size_t b_cell_bishops_cnt{};
    size_t w_cell_bishops_cnt{};
    for (const auto& fig : get_all_figures()) {
        if (fig->get_type() == FigureType::Bishop)
            if ((fig->get_pos().x + fig->get_pos().y) % 2)
                ++b_cell_bishops_cnt;
            else
                ++w_cell_bishops_cnt;
        else if (fig->get_type() != FigureType::King)
            return false; // при модификации функции не забыть тут поставить хотя бы goto
    }
    return not (b_cell_bishops_cnt && w_cell_bishops_cnt);
}

auto FigureBoard::
    game_end_check(const Color col) const noexcept -> GameEndType
{
    if (checkmate_for(col)) return GameEndType::Checkmate;
    if (stalemate_for(col)) return GameEndType::Stalemate;
    if (insufficient_material()) return GameEndType::InsufficientMaterial;
    if (move_logger_.is_fifty_move_rule_was_triggered()) return GameEndType::FiftyRule;
    if (move_logger_.is_moves_repeat_rule_was_triggered()) return GameEndType::MoveRepeat;
    return GameEndType::NotGameEnd;
}

auto FigureBoard::
    determine_move(const Figure* in_hand, const Input& input, const FigureType promotion_choice) const noexcept
    -> std::optional<mvmsg::MoveMessage>
{
    std::vector<mvmsg::SideEvent> side_events;

    // Castling breaks
    if (in_hand->get_type() == FigureType::Rook && has_castling(in_hand->get_id())) {
        side_events.emplace_back( mvmsg::CastlingBreak{ in_hand->get_id() } );
    }
    else if (in_hand->is(FigureType::King)) {
        for (const auto& [id, has_castling] : castling_) {
            if (has_castling && (*get_fig(id))->is_col(in_hand)) {
                side_events.emplace_back(mvmsg::CastlingBreak{id});
            }
        }
    }

    // Castling
    if ((in_hand->is(FigureType::King) || in_hand->is(FigureType::Rook)) && WIDTH == 8) {
        using PairInt = std::pair<int, int>;
        
        for (auto [king_end_col, rook_end_col] : { PairInt{6, 5}, PairInt{2, 3} }) {
            if (const auto check_result_sus = castling_check(in_hand, input, king_end_col, rook_end_col)) {
                if (const auto& [rook, king, second_figure_to_move] = check_result_sus.value(); has_castling(rook->get_id())) {
                    const auto king_tmp = figfab::FigureFabric::instance().submit_on(king, Pos{ in_hand->get_pos().x, king_end_col });
                    const auto rook_tmp = figfab::FigureFabric::instance().submit_on(rook, Pos{ rook->get_pos().x, rook_end_col });
                    // FIXME debug this branch (move_message.push and check after success castling)
                    if (check_for_when(
                            what_next(in_hand->get_col()),
                            { input.from, king->get_pos(), rook->get_pos() }, 
                            Pos{}, 
                            {},
                            { king_tmp.get(), rook_tmp.get() })
                        ) {
                        side_events.emplace_back(mvmsg::Check{ });
                    }
                    return mvmsg::MoveMessage{ 
                        *in_hand,
                        input,
                        promotion_choice,
                        mvmsg::Castling{
                            (in_hand->is(FigureType::King) 
                                ? rook 
                                : king)->get_id(),
                            second_figure_to_move
                        }, 
                        side_events};
                }
            }
        }
    }

    // Pawn
    if (in_hand->is(FigureType::Pawn)) {
        // Promotion
        if (in_hand->is_col(Color::White) &&
                (idw_ && input.target.x == 0 || !idw_ && input.target.x == (HEIGHT - 1)) ||
                in_hand->get_col() == Color::Black &&
                (!idw_ && input.target.x == 0 || idw_ && input.target.x == (HEIGHT - 1))
                ) {
            side_events.emplace_back(mvmsg::Promotion{ });
        }

        // Long move
        const Pos shift{ input.target - input.from };
        // Ход пешки на 2 (смотрю свои фигуры на 2 линии)
        if (shift.y == 0 && is_empty(input.target) && (
            in_hand->get_col() == Color::White && (
                input.from.x == (HEIGHT - 2) && idw_ && shift.x == -2 && is_empty(input.from + Pos(-1, 0)) ||
                input.from.x == 1 && !idw_ && shift.x == 2 && is_empty(input.from + Pos(1, 0))
                ) ||
            in_hand->is_col(Color::Black) && (
                input.from.x == 1 && idw_ && shift.x == 2 && is_empty(input.from + Pos(1, 0)) ||
                input.from.x == (HEIGHT - 2) && !idw_ && shift.x == -2 && is_empty(input.from + Pos(-1, 0))
                )
            )) {
            const auto in_hand_in_targ_tmp = figfab::FigureFabric::instance().submit_on(in_hand, input.target);
            if (check_for_when(what_next(in_hand->get_col()), { input.from }, Pos{}, {}, { in_hand_in_targ_tmp.get()})) {
                side_events.emplace_back(mvmsg::Check{ });
            }
            return mvmsg::MoveMessage{
                *in_hand,
                input,
                promotion_choice,
                mvmsg::LongMove{ },
                side_events
            };
        }

        // Взятие на проходе (смотрю чужие фигуры на 4 линии)
        // А ещё проверяю прошлый ход
        if (const auto& last_move_sus = move_logger_.get_last_move(); last_move_sus.has_value()) {
            const auto& last_move = last_move_sus.value();
            if (const Pos& who_went_at_last_move_pos = last_move.first.get_pos();
                   std::abs(shift.y) == 1 && is_empty(input.target)
                && std::holds_alternative<mvmsg::LongMove>(last_move.main_event)
                && who_went_at_last_move_pos.y == input.target.y &&
                (
                    // maybe can be simplified into one condition
                    in_hand->is_col(Color::White) && (
                        input.from.x == (EN_PASSANT_INDENT - 1) && idw_ && shift.x == -1 && cont_fig(input.from + Pos(0, shift.y)) ||
                        input.from.x == (HEIGHT - EN_PASSANT_INDENT) && !idw_ && shift.x == 1 && cont_fig(input.from + Pos(0, shift.y))
                        ) ||
                    in_hand->is_col(Color::Black) && (
                        input.from.x == (HEIGHT - EN_PASSANT_INDENT) && idw_ && shift.x == 1 && cont_fig(input.from + Pos(0, shift.y)) ||
                        input.from.x == (EN_PASSANT_INDENT - 1) && !idw_ && shift.x == -1 && cont_fig(input.from + Pos(0, shift.y))
                        )
                    )) {
                const auto in_hand_in_targ_tmp = figfab::FigureFabric::instance().submit_on(in_hand, input.target);
                if (check_for_when(what_next(in_hand->get_col()), { input.from, Pos{input.from.x, input.target.y}, input.target }, Pos{}, {}, { in_hand_in_targ_tmp.get() })) {
                    side_events.emplace_back(mvmsg::Check{ });
                }
                auto const& to_eat_sus = get_fig(Pos{ input.from.x, input.target.y });
                return mvmsg::MoveMessage{
                    *in_hand,
                    input,
                    promotion_choice,
                    mvmsg::EnPassant{
                        to_eat_sus.value()->get_id()
                    },
                    side_events
                };
            }
        }
    }

    const auto targ_sus = get_fig(input.target);
    for (const auto& [is_eat, curr] : expand_broom(in_hand)) {
        if (curr != input.target) continue;
        if (is_eat && targ_sus.has_value()) {
            const auto in_hand_in_curr_tmp = figfab::FigureFabric::instance().submit_on(in_hand, curr);
            // Фигура на которой сейчас стоим всё ещё учитывается!
            if (check_for_when(what_next(in_hand->get_col()), {input.from, input.target}, Pos{}, {}, { in_hand_in_curr_tmp.get() })) {
                side_events.emplace_back(mvmsg::Check{ });
            }
            return mvmsg::MoveMessage{
                *in_hand,
                input,
                promotion_choice,
                mvmsg::Eat{
                    targ_sus.value()->get_id()
                },
                side_events
            };
        }
        else if (!is_eat && !targ_sus.has_value()) {
            const auto in_hand_in_curr_tmp = figfab::FigureFabric::instance().submit_on(in_hand, curr);
            // FIXME {input.from} without input.target?? see prev check_for_when
            if (check_for_when(what_next(in_hand->get_col()), {input.from}, Pos{}, {}, { in_hand_in_curr_tmp.get() })) {
                side_events.emplace_back(mvmsg::Check{ });
            }
            return mvmsg::MoveMessage{
                *in_hand,
                input,
                promotion_choice,
                mvmsg::Move{ },
                side_events
            };
        }
    }
    return std::nullopt;
}

auto FigureBoard::  // FIXME move check and under check checks to separate function
    move_check(const Figure* const in_hand, const Input& input, const FigureType promotion_choice) const noexcept
        -> std::expected<mvmsg::MoveMessage, ErrorEvent>
{
    if (!(is_valid_coords(input.from) && is_valid_coords(input.target)) || !cont_fig(input.from))
    {
        return std::unexpected{ ErrorEvent::InvalidMove };
    }

    auto move_sus = determine_move(in_hand, input, promotion_choice);

    if (!move_sus.has_value()) {
        return std::unexpected{ ErrorEvent::InvalidMove };
    }

    FigureBoard next_board{ *this };

    next_board.provide_move(*move_sus);

    if (next_board.check_for_when(in_hand->get_col())) {
        return std::unexpected{ ErrorEvent::UnderCheck };
    }
    if (next_board.check_for_when(what_next(in_hand->get_col()))) {
        move_sus->side_evs.emplace_back(mvmsg::Check{ });
    }

    return *move_sus;
}

auto FigureBoard::
    undo_move() -> bool
{
    if (move_logger_.prev_empty()) return false;
    const mvmsg::MoveMessage move_message = move_logger_.move_last_to_future().value();
    const auto in_hand_sus = get_fig(move_message.first.get_id());
    if (!in_hand_sus.has_value()) return false;
    const auto in_hand = in_hand_sus.value();
    
    VISIT(move_message.main_event,
        [&](const mvmsg::Move&) {
            move_fig(in_hand, move_message.input.from);
        },
        [&](const mvmsg::LongMove&) {
            move_fig(in_hand, move_message.input.from);
        },
        [&](const mvmsg::Eat& eat) {
            move_fig(in_hand, move_message.input.from);
            recapture_figure(eat.eaten);
        },
        [&](const mvmsg::EnPassant& en_passant) {
            // FIXME debug on rook with castling
            move_fig(in_hand, move_message.input.from);
            recapture_figure(en_passant.eaten);
        },
        [&](const mvmsg::Castling& castling) {
            move_fig(in_hand, move_message.input.from);
            const auto who = get_fig_unsafe(castling.second_to_move);
            move_fig(who, castling.second_input.from);
        }
    );
    for (const auto& side_event : move_message.side_evs) {
        VISIT(side_event,
            [&](const mvmsg::Promotion& promotion) {
                promotion_fig(in_hand, FigureType::Pawn);
            },
            [&](const mvmsg::Check&) {
                // do nothing
            },
            /* !ms.what_castling_breaks.empty() && !has_castling(ms.what_castling_breaks.back())
            * This check was in the original code, but it seems to be redundant
            * FIXME check it */
            [&](const mvmsg::CastlingBreak& castling_break) {
                on_castling(castling_break.whose);
            }
        );
    }
    return true;
}

auto FigureBoard::
    provide_move(const mvmsg::MoveMessage& move_message) -> bool
{
    const auto in_hand_sus = get_fig(move_message.first.get_id());
    if (!in_hand_sus.has_value()) return false;
    const auto in_hand = in_hand_sus.value();
    
    VISIT(move_message.main_event,
        [&](const mvmsg::Move&) {
            move_fig(in_hand, move_message.input.target);
        },
        [&](const mvmsg::LongMove&) {
            move_fig(in_hand, move_message.input.target);
        },
        /* (auto to_capture_sus = get_fig(it);
        * to_capture_sus.has_value() && to_capture_sus.value()->get_pos() != input.target)
        * Was in Eat and EnPassant events */
        [&](const mvmsg::Eat& eat) {
            move_fig(in_hand, move_message.input.target);
        },
        [&](const mvmsg::EnPassant& en_passant) {
            move_fig(in_hand, move_message.input.target);
            capture_figure(en_passant.eaten);
        },
        /* (has_castling(ms.to_move.back().first)) */
        [&](const mvmsg::Castling& castling) {
            const auto who = get_fig_unsafe(castling.second_to_move);
            if (who->at(move_message.input.target)) {
                swap_fig(in_hand, who);
            }
            move_fig(in_hand, move_message.input.target);
            move_fig(who, castling.second_input.target);
        }
    );
    for (const auto& side_event : move_message.side_evs) {
        VISIT(side_event,
            [&](const mvmsg::Promotion& promotion) {
                promotion_fig(in_hand, move_message.promotion_choice);
            },
            [&](const mvmsg::Check&) {
                // do nothing
            },
            /* (!ms.what_castling_breaks.empty() && has_castling(ms.what_castling_breaks.back()) */
            [&](const mvmsg::CastlingBreak& castling_break) {
                off_castling(castling_break.whose);
            }
        );
    }

    return true;
}

auto FigureBoard::
    restore_move() -> bool
{
    if (const auto future_sus = move_logger_.pop_future_move()) {
        provide_move(future_sus.value());
        move_logger_.add_without_reset(future_sus.value());
        return true;
    }
    return false;
}

void FigureBoard::
    recapture_figure(const Id id)
{
    const auto to_resurrect_id = std::ranges::find_if(
        captured_figures_,
        [&id](auto&& val) { return id == val->get_id(); }
    );
    if (to_resurrect_id == captured_figures_.end()) {
        assert(!"This figure can't be resurrected");
    }
    Figure* to_resurrect = *to_resurrect_id;
    place_figure(to_resurrect);
    captured_figures_.erase(to_resurrect_id);
}

void FigureBoard::
    promotion_fig(Figure* to_promote, const FigureType new_type)
{
    const Id id = to_promote->get_id();
    const Color color = to_promote->get_col();
    const Pos position = to_promote->get_pos();
    figfab::FigureFabric::instance().create_in_place(position, color, new_type, id, to_promote);
}

