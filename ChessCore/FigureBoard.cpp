#include "FigureBoard.h"

FigureBoard::FigureBoard(BoardRepr board_repr) {
    reset(board_repr);
}

void FigureBoard::init_figures_moves() {
    std::vector<Pos>temp_left_up;
    std::vector<Pos>temp_left_down;
    std::vector<Pos>temp_right_up;
    std::vector<Pos>temp_right_down;
    std::vector<Pos>temp_left;
    std::vector<Pos>temp_right;
    std::vector<Pos>temp_up;
    std::vector<Pos>temp_down;
    
    for (int ij = 1; ij < std::min(WIDTH, HEIGHT); ++ij) {
        temp_left_up.push_back({ -ij, -ij });
        temp_left_down.push_back({ ij, -ij });
        temp_right_up.push_back({ -ij, ij });
        temp_right_down.push_back({ ij, ij });
        temp_left.push_back({ 0, -ij });
        temp_right.push_back({ 0, ij });
        temp_up.push_back({ -ij, 0 });
        temp_down.push_back({ ij, 0 });
    }

    // Задаётся относительно белых через idw
    moves[FigureType::Pawn] = { {{ (idw ? -1 : 1), 0}} };
    eats[FigureType::Pawn] = { {{ (idw ? -1 : 1), -1}}, {{ (idw ? -1 : 1), 1}} };

    moves[FigureType::Knight] = { {{-1, 2}}, {{-1, -2}},
        {{1, -2}}, {{1, 2}}, {{2, -1}}, {{-2, -1}},
        {{2, 1}}, {{-2, 1}}, };
    eats[FigureType::Knight] = moves[FigureType::Knight];

    moves[FigureType::King] = { {{-1, -1}}, {{-1, 1}}, {{-1, 0}},
        {{0, -1}}, {{0, 1}}, {{1, 1}}, {{1, -1}},
        {{1, 0}} };
    eats[FigureType::King] = moves[FigureType::King];

    moves[FigureType::Bishop] = { temp_left_up, temp_left_down, temp_right_up, temp_right_down };
    eats[FigureType::Bishop] = moves[FigureType::Bishop];

    moves[FigureType::Rook] = { temp_left, temp_right, temp_up, temp_down };
    eats[FigureType::Rook] = moves[FigureType::Rook];

    moves[FigureType::Queen] = { temp_left_up, temp_left_down, temp_right_up, temp_right_down, temp_left, temp_right, temp_up, temp_down };
    eats[FigureType::Queen] = moves[FigureType::Queen];

}

void FigureBoard::reset(const BoardRepr& map) {
    move_logger.reset();
    curr_id = 0;
    for (auto& [_, fig] : figures) {
        if (not fig->empty())
            delete fig;
    }
    figures.clear();
    for (auto& fig : captured_figures) {
        if (not fig->empty())
            delete fig;
    }
    captured_figures.clear();
    apply_map(map);
    init_figures_moves();
    reset_castling(map);
}

void FigureBoard::apply_map(const BoardRepr& board_repr) {
    idw = board_repr.get_idw();
    move_logger.set_past(board_repr.get_past());
    move_logger.set_future(board_repr.get_future());
    for (const auto& cap_fig : board_repr.get_captured_figures()) {
        captured_figures.push_back( FigureFabric::instance()->create(cap_fig) );
    }
    for (const auto& fig : board_repr.get_figures())
        figures[fig->get_pos()] = FigureFabric::instance()->create(fig);
}

BoardRepr FigureBoard::get_repr(bool save_all_moves) {
    std::vector<Figure*> fig_vec;
    for (auto& [_, fig] : figures)
        fig_vec.push_back(fig);
    return {
        fig_vec,
        Color::White,
        idw,
        save_all_moves 
            ? move_logger.get_past() 
            : std::vector<MoveRec>{ move_logger.get_last_move() },
        move_logger.get_future(),
        captured_figures
    };
}

void FigureBoard::reset_castling(bool castle_state) {
    castling.clear();
    for (const Color& col : { Color::Black, Color::White }) {
        for (const auto& aspt_to_rook : get_figures_of(col)) {
            if (aspt_to_rook->is(FigureType::Rook)) {
                castling[aspt_to_rook->get_id()] = castle_state;
            }
        }
    }
}

void FigureBoard::reset_castling(const BoardRepr& board_repr) {
    reset_castling(false);
    for (Id& castle_id : board_repr.get_who_can_castle()) {
        castling[castle_id] = true;
    }
}

Figure* FigureBoard::get_fig(Pos position) {
    if (figures.find(position) != figures.end()) {
        return figures[position];
    }
    else {
        return get_default_fig();
    }
}

Figure* FigureBoard::get_fig(Id id) {
    for (const auto& [_, fig] : figures) {
        if (fig->is(id)) {
            return fig;
        }
    }
    return get_default_fig();
}

/// <summary>
/// Показывает содержит ли клетка фигуру
/// </summary>
bool FigureBoard::cont_fig(Pos position) {
    return not get_fig(position)->empty();
}

/// <summary>
/// Показывает не содержит ли клетка фигуру
/// </summary>
bool FigureBoard::is_empty(Pos position) {
    return get_fig(position)->empty();
}

/// <summary>
/// Убирает фигуру с доски и добавляет её к съеденным
/// </summary>
/// <param name="it">Итератор на фигуру</param>
/// <returns>Получилось ли съесть</returns>
bool FigureBoard::capture_figure(Figure* it) {
    if (it->empty()) {
        return false;
    }
    captured_figures.push_back(it);
    figures.erase(it->get_pos());
    return true;
}

/// <summary>
/// Убирает фигуру с доски и добавляет её к съеденным
/// </summary>
/// <param name="it">Идентификатор фигуры</param>
/// <returns>Получилось ли съесть</returns>
bool FigureBoard::capture_figure(const Id id) {
    if (id == ERR_ID) { return false; }
    Figure* fig = get_fig(id);
    captured_figures.push_back(fig);
    figures.erase(fig->get_pos());
    return true;
}

void FigureBoard::delete_fig(Pos pos) {
    delete figures[pos];
    figures.erase(pos);
}

void FigureBoard::place_fig(Figure* fig) {
    if (fig->empty()) throw std::logic_error("trying to place empty figure");
    if (cont_fig(fig->get_pos())) delete_fig(fig->get_pos());
    figures[fig->get_pos()] = fig;
}

bool is_valid_coords(Pos position) {
    int x{ position.x }, y{ position.y };
    return ((x >= 0) && (x < HEIGHT) &&
        (y >= 0) && (y < WIDTH));
}

/// <param name="col">Цвет фигур</param>
/// <returns>Все фигуры определённого цвета</returns>
std::vector<Figure*> FigureBoard::get_figures_of(Color col) {
    std::vector<Figure*> acc{};
    for (const auto& [_, fig] : figures) {
        if (fig->is_col(col)) {
            acc.push_back(fig);
        }
    }
    return acc;
}

Figure* FigureBoard::find_king(Color col) {
    auto map_ptr = std::find_if(
        figures.begin(),
        figures.end(),
        [col](const auto& it)
        { return it.second->is_col(col) && it.second->get_type() == FigureType::King; }
    );
    if (map_ptr == figures.end()) {
        return get_default_fig();
    }
    else {
        return (*map_ptr).second;
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
std::vector<std::pair<bool, Pos>> FigureBoard::expand_broom(const Figure* in_hand, const std::vector<Pos>& to_ignore, const std::vector<Pos>& ours, const std::vector<Pos>& enemies) {
    std::vector<std::pair<bool, Pos>> possible_moves{}; // list { pair{ is_eat, targ }, ... }
    auto in_hand_pos = in_hand->get_pos();
    for (auto& eat_beams : eats[in_hand->get_type()]) {
        for (auto& eat : eat_beams) {
            Pos curr{ in_hand_pos + (in_hand->is_col(Color::White) ? eat : eat.mul_x(-1)) };
            if (not is_valid_coords(curr)) continue;
            if (curr.in(ours)) {
                break; // Врезались в союзника
            }
            else if (curr.in(enemies)) {
                possible_moves.push_back({ true, curr }); // Съели противника
                break; // И врезались...
            }
            else if (cont_fig(curr) && not curr.in(to_ignore)) {
                if (not get_fig(curr)->is_col(in_hand->get_col())) {
                    possible_moves.push_back({ true, curr });
                    break; // Врезались
                }
                else {
                    break; // Врезались
                }
            }
        }
    }
    for (auto& move_beams : moves[in_hand->get_type()]) {
        for (auto& move : move_beams) {
            Pos curr{ in_hand_pos + (in_hand->is_col(Color::White) ? move : move.mul_x(-1)) };
            if (not is_valid_coords(curr)) continue;
            if (not ((not curr.in(to_ignore) && cont_fig(curr)) || curr.in(ours) || curr.in(enemies))) {
                possible_moves.push_back({ false, curr });
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
std::vector<std::pair<bool, Pos>> FigureBoard::get_all_possible_moves(const Figure* in_hand, const std::vector<Pos>& to_ignore, const std::vector<Pos>& ours, const std::vector<Pos>&enemies) {
    std::vector<std::pair<bool, Pos>> all_moves{ expand_broom(in_hand, to_ignore, ours, enemies) };
    Pos in_hand_pos = in_hand->get_pos();
    Figure* in_hand_it{ get_fig(in_hand_pos) };
    if (in_hand->get_type() == FigureType::Pawn) {
        // Ход пешки на 2 (смотрю свои фигуры на 2 линии)
        if (in_hand->is_col(Color::White)) {
            if (in_hand_pos.x == (HEIGHT - 2) && idw && is_empty(in_hand_pos + Pos(-1, 0)) &&
                is_empty(in_hand_pos + Pos(-2, 0))) {
                all_moves.push_back({ false, in_hand_pos + Pos(-2, 0) });
            }
            if (in_hand_pos.x == 1 && not idw && is_empty(in_hand_pos + Pos(1, 0)) &&
                is_empty(in_hand_pos + Pos(2, 0))) {
                all_moves.push_back({ false, in_hand_pos + Pos(2, 0) });
            }
        }
        if (in_hand->is_col(Color::Black)) {
            if (in_hand_pos.x == 1 && idw && is_empty(in_hand_pos + Pos(1, 0)) &&
                is_empty(in_hand_pos + Pos(2, 0))) {
                all_moves.push_back({ false, in_hand_pos + Pos(2, 0) });
            }
            if (in_hand_pos.x == (HEIGHT - 2) && not idw && is_empty(in_hand_pos + Pos(-1, 0)) &&
                is_empty(in_hand_pos + Pos(-2, 0))) {
                all_moves.push_back({ false, in_hand_pos + Pos(-2, 0) });
            }
        }

        // Взятие на проходе
        const MoveRec& last_move = move_logger.get_last_move();
        if (last_move.ms.main_ev == MainEvent::LMOVE && std::abs(last_move.get_who_went_pos().y - in_hand_pos.y) == 1) {
            int shift_y = last_move.get_who_went_pos().y - in_hand_pos.y;
            if (in_hand->is_col(Color::White)) {
                if (in_hand_pos.x == 3 && idw && cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(-1, shift_y))) {
                    all_moves.push_back({ true, in_hand_pos + Pos(-1, shift_y) });
                }
                if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && not idw && cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(1, shift_y))) {
                    all_moves.push_back({ true, in_hand_pos + Pos(1, shift_y) });
                }
            }
            if (in_hand->is_col(Color::Black)) {
                if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && idw && cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(1, shift_y))) {
                    all_moves.push_back({ true, in_hand_pos + Pos(1, shift_y) });
                }
                if (in_hand_pos.x == 3 && not idw && cont_fig(in_hand_pos + Pos(0, shift_y)) && is_empty(in_hand_pos + Pos(-1, shift_y))) {
                    all_moves.push_back({ true, in_hand_pos + Pos(-1, shift_y) });
                }
            }
        }
    }
    if (in_hand->get_type() == FigureType::King) {
        bool is_castling; MoveMessage mm; Figure* king; Figure* rook;
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 6} }, 6, 5);
        if (is_castling && has_castling(rook->get_id()))
            all_moves.push_back({ false, {in_hand_pos.x, 6} });
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 2} }, 2, 3);
        if (is_castling && has_castling(rook->get_id()))
            all_moves.push_back({ false, {in_hand_pos.x, 2} });
    }
    if (in_hand->get_type() == FigureType::Rook) {
        bool is_castling; MoveMessage mm; Figure* king; Figure* rook;
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 5} }, 6, 5);
        if (is_castling && has_castling(rook->get_id()))
            all_moves.push_back({ false, {in_hand_pos.x, 5} });
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 3} }, 2, 3);
        if (is_castling && has_castling(rook->get_id()))
            all_moves.push_back({ false, {in_hand_pos.x, 3} });
    }

    return all_moves;
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
bool FigureBoard::check_for_when(Color col, const std::vector<Pos>& to_ignore, Pos to_defend, const std::vector<Figure*>& ours, const std::vector<Figure*>& enemies) {
    auto king_it = find_king(col);
    if (to_defend == Pos()) {
        if (king_it->empty()) {
            return true; // Нечего защищать
        }
        else {
            to_defend = king_it->get_pos();
        }
    }
    for (const auto& enemy : get_figures_of(what_next(col)) + enemies) {
        if (std::find(to_ignore.begin(), to_ignore.end(), enemy->get_pos()) != to_ignore.end()) {
            if (std::find(enemies.begin(), enemies.end(), enemy) == enemies.end()) // Пока не нужно, но должно быть тут на всякий
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
/// Проверка доски на пат
/// </summary>
/// <param name="col">Цвет, для которого пат проверяется</param>
/// <param name="to_ignore">Фигуры, которые нужно игнорировать</param>
/// <param name="to_defend">Позиция короля или фигуры, которой можно поставить шах</param>
/// <returns>Наличие пата</returns>
bool FigureBoard::stalemate_for(Color col, const std::vector<Pos>& to_ignore, Pos to_defend) {
    auto king_it = find_king(col);
    if (king_it->empty())
        return true; // Нет короля
    if (to_defend == Pos()) to_defend = king_it->get_pos();
    for (auto& aspt : get_figures_of(col)) {
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
                auto tmp = FigureFabric::instance()->submit_on(aspt, curr);
                bool check = (is_eat
                    ? check_for_when(col, to_ignore + aspt->get_pos() + curr, to_defend, { tmp.get()}, {})
                    : check_for_when(col, to_ignore + aspt->get_pos(), to_defend, { tmp.get()}, {})
                    );
                if (not check) {
                    return false;
                }
            }
        }
    }
    return true;
}

/// <summary>
/// Проверка доски на мат
/// </summary>
/// <param name="col">Цвет, для которого мат проверяется</param>
/// <param name="to_ignore">Фигуры, которые нужно игнорировать</param>
/// <param name="to_defend">Позиция короля или фигуры, которой можно поставить шах (мат)</param>
/// <returns>Наличие мата</returns>
bool FigureBoard::checkmate_for(Color col, const std::vector<Pos>& to_ignore, Pos to_defend /*тут, возможно, не король*/) {
    auto king_it = find_king(col);
    if (to_defend == Pos()) {
        if (king_it->empty()) {
            return false; // Нечего защищать
        }
        else {
            to_defend = king_it->get_pos();
        }
    }
    for (const auto& figure : get_figures_of(col)) {
        for (const auto& [is_eat, curr] : expand_broom(figure, to_ignore, { to_defend })) {
            if (figure->get_type() == FigureType::King) {
                if (is_eat
                    ? not check_for_when(col, to_ignore + figure->get_pos() + curr + to_defend, curr)
                    : not check_for_when(col, to_ignore + figure->get_pos() + to_defend, curr)
                    ) {
                    return false;
                }
            }
            else {
                auto tmp = FigureFabric::instance()->submit_on(figure, curr);
                bool check = (is_eat
                    ? check_for_when(col, to_ignore + figure->get_pos() + curr, to_defend, { tmp.get() })
                    : check_for_when(col, to_ignore + figure->get_pos(), to_defend, { tmp.get() })
                    );
                if (not check) {
                    return false;
                }
            }
        }
    }
    return true;
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
/// <param name="move_message">Сообщение хода для модификации</param>
/// <param name="in_hand">Фигура, которой собираются сходить</param>
/// <param name="input">Ввод для проверки</param>
/// <param name="king_end_col">Целевой столбец для короля</param>
/// <param name="rook_end_col">Целевой столбец для ладьи</param>
/// <returns>Валидна ли рокировка</returns>
std::tuple<bool, MoveMessage, Figure*, Figure*> FigureBoard::castling_check(MoveMessage move_message, Figure* in_hand, const Input& input, int king_end_col, int rook_end_col) {
    // Рокировка на g-фланг
    bool castling_can_be_done = true;
    Id must_be_empty = ERR_ID;
    auto king = in_hand->get_type() == FigureType::King ? in_hand : find_king(in_hand->get_col());
    if (king->empty())
        return { false, move_message, get_default_fig(), get_default_fig() };
    Pos king_pos = king->get_pos();
    if (in_hand->get_type() == FigureType::King && input.target.y == king_end_col && in_hand->get_pos().y != king_end_col ||
        king_pos.y == king_end_col && in_hand->get_type() == FigureType::Rook && input.target.y == rook_end_col && in_hand->get_pos().y != rook_end_col) {
        // input правильный (король уже мог быть на g вертикали, так что доступна возможность рокировки от ладьи)
        // Нужно проверить, что все промежуточные позиции (где идёт король) не под шахом
        auto [_, rook] = *figures.begin();
        int step{ king_end_col - king_pos.y > 0 ? 1 : -1 };
        for (Pos rook_aspt_pos{ king_pos };
            is_valid_coords(rook_aspt_pos); rook_aspt_pos.y += step) {
            rook = get_fig(rook_aspt_pos);
            if (rook->is_col(in_hand->get_col()) && rook->get_type() == FigureType::Rook) {
                break;
            }
        }
        if (rook->empty() || rook->get_type() != FigureType::Rook) return {false, move_message, get_default_fig(), get_default_fig()};
        if (king_pos.x != input.target.x && rook->get_pos().x != input.target.x) return { false, move_message, get_default_fig(), get_default_fig() };
        
        Pos rook_pos = rook->get_pos();
        Pos target{ king_pos.x, king_end_col };
        for (int step = king_pos < target ? 1 : -1; king_pos != target; king_pos.y += step) {
            castling_can_be_done &= not check_for_when(in_hand->get_col(), { king_pos, rook_pos }, king_pos);
        }
        for (int step = king_pos < rook_pos ? 1 : -1; king_pos != rook_pos; king_pos.y += step) {
            must_be_empty = get_fig(king_pos)->get_id();
            castling_can_be_done &= must_be_empty == ERR_ID || must_be_empty == king->get_id() || must_be_empty == rook->get_id();
        }
        castling_can_be_done &= not check_for_when(in_hand->get_col(), { king_pos, rook_pos }, { king_pos.x, king_end_col });
        must_be_empty = get_fig({ king_pos.x, rook_end_col })->get_id();
        castling_can_be_done &= must_be_empty == ERR_ID || must_be_empty == king->get_id() || must_be_empty == rook->get_id();
        if (castling_can_be_done) {
            move_message.main_ev = MainEvent::CASTLING;
            if (in_hand->get_type() == FigureType::King) {
                move_message.to_move.push_back({ rook->get_id(), {rook_pos, {rook_pos.x, rook_end_col}}});
            }
            else {
                move_message.to_move.push_back({ king->get_id(), {king_pos, {king_pos.x, king_end_col}}});
            }
            return { true, move_message, king, rook };
        }
    }
    return { false, move_message, get_default_fig(), get_default_fig() };
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
bool FigureBoard::insufficient_material() {
    size_t size = cnt_of_figures();
    if (size <= 2) return true;
    if (size == 3 &&
        std::find_if(
            figures.begin(),
            figures.end(),
            [](const auto& it)
            { return it.second->get_type() == FigureType::Knight || it.second->get_type() == FigureType::Bishop; }
        ) != figures.end()
        ) return true;
    size_t b_cell_bishops_cnt{};
    size_t w_cell_bishops_cnt{};
    for (const auto& fig : all_figures()) {
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

GameEndType FigureBoard::game_end_check(Color col) {
    if (stalemate_for(col)) return GameEndType::Stalemate;
    if (checkmate_for(col)) return GameEndType::Checkmate;
    if (insufficient_material()) return GameEndType::InsufficientMaterial;
    if (move_logger.is_fifty_move_rule_was_triggered()) return GameEndType::FiftyRule;
    if (move_logger.is_moves_repeat_rule_was_triggered()) return GameEndType::MoveRepeat;
    return GameEndType::NotGameEnd;
}

/// <summary>
/// Проверка хода на валидность и составление сообщения хода
/// </summary>
/// <exception cref="std::invalid_argument">Ход нельзя совершить</exception>
/// <param name="in_hand">Фигура, которой собираются ходить</param>
/// <param name="input">Ввод</param>
/// <returns>Сообщение хода</returns>
std::variant<ErrorEvent, MoveMessage> FigureBoard::move_check(Figure* in_hand, Input input) {
    MoveMessage move_message{ MainEvent::E, {} };
    Figure* targ{ get_fig(input.target) };

    if (not (is_valid_coords(input.from) and is_valid_coords(input.target)) ||
        input.from == input.target ||
        not cont_fig(input.from) ||
        get_fig(input.target)->is_col(in_hand->get_col())) {
        return ErrorEvent::INVALID_MOVE;
    }

    if (in_hand->get_type() == FigureType::Rook) {
        move_message.side_evs.push_back(SideEvent::CASTLING_BREAK);
        move_message.what_castling_breaks.push_back(in_hand->get_id());
    }

    if ((in_hand->get_type() == FigureType::King || in_hand->get_type() == FigureType::Rook) &&
        WIDTH == 8) {
        move_message.side_evs.push_back(SideEvent::CASTLING_BREAK);
        for (const auto& fig : get_figures_of(in_hand->get_col())) {
            if (fig->get_type() == FigureType::Rook) {
                // Заполняется в любом случае, но MainEvent::BREAK это отменит, если что
                move_message.what_castling_breaks.push_back(fig->get_id());
            }
        }
        bool is_castling; MoveMessage mm; Figure* king; Figure* rook;
        std::tie(is_castling, mm, king, rook) = castling_check(move_message, in_hand, input, 6, 5);
        if (is_castling && has_castling(rook->get_id())) {
            auto king_tmp = FigureFabric::instance()->submit_on(king, { in_hand->get_pos().x, 6 });
            auto rook_tmp = FigureFabric::instance()->submit_on(rook, { rook->get_pos().x, 5 });
            if (check_for_when(what_next(in_hand->get_col()),
                { input.from, king->get_pos(), rook->get_pos() }, {}, {},
                { king_tmp.get(), rook_tmp.get() })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            return mm;
        }
        std::tie(is_castling, mm, king, rook) = castling_check(move_message, in_hand, input, 2, 3);
        if (is_castling && has_castling(rook->get_id())) {
            auto king_tmp = FigureFabric::instance()->submit_on(king, { king->get_pos().x, 2 });
            auto rook_tmp = FigureFabric::instance()->submit_on(rook, { rook->get_pos().x, 3 });
            if (check_for_when(what_next(in_hand->get_col()),
                { input.from, king->get_pos(), rook->get_pos()}, {}, {},
                { king_tmp.get(), rook_tmp.get() })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            return mm;
        }
    }

    if (in_hand->get_type() == FigureType::Pawn) {
        if (in_hand->is_col(Color::White) &&
                (idw && input.target.x == 0 || not idw && input.target.x == (HEIGHT - 1)) ||
                in_hand->get_col() == Color::Black &&
                (not idw && input.target.x == 0 || idw && input.target.x == (HEIGHT - 1))
                ) {
            move_message.side_evs.push_back(SideEvent::PROMOTION);
        }

        Pos shift{ input.target - input.from };
        // Ход пешки на 2 (смотрю свои фигуры на 2 линии)
        if (shift.y == 0 && is_empty(input.target) && (
            in_hand->get_col() == Color::White && (
                input.from.x == (HEIGHT - 2) && idw && shift.x == -2 && is_empty(input.from + Pos(-1, 0)) ||
                input.from.x == 1 && not idw && shift.x == 2 && is_empty(input.from + Pos(1, 0))
                ) ||
            in_hand->is_col(Color::Black) && (
                input.from.x == 1 && idw && shift.x == 2 && is_empty(input.from + Pos(1, 0)) ||
                input.from.x == (HEIGHT - 2) && not idw && shift.x == -2 && is_empty(input.from + Pos(-1, 0))
                )
            )) {
            auto in_hand_in_targ_tmp = FigureFabric::instance()->submit_on(in_hand, input.target);
            if (check_for_when(what_next(in_hand->get_col()), { input.from }, {}, {}, { in_hand_in_targ_tmp.get()})) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            if (in_hand->get_type() == FigureType::King) {
                if (check_for_when(in_hand->get_col(), { input.from }, input.target)) {
                    return ErrorEvent::CHECK_IN_THAT_TILE;
                }
            }
            else {
                if (check_for_when(in_hand->get_col(), { input.from }, {}, { in_hand_in_targ_tmp.get()}, {})) {
                    return ErrorEvent::UNDER_CHECK;
                }
            }
            move_message.main_ev = MainEvent::LMOVE;
            return move_message;
        }

        // Взятие на проходе (смотрю чужие фигуры на 4 линии)
        // А ещё проверяю прошлый ход
        const MoveRec& last_move = move_logger.get_last_move();
        if (std::abs(shift.y) == 1 && is_empty(input.target) &&
            last_move.ms.main_ev == MainEvent::LMOVE && last_move.get_who_went_pos().y == input.target.y &&
            (
                in_hand->get_col() == Color::White && (
                    input.from.x == 3 && idw && shift.x == -1 && cont_fig(input.from + Pos(0, shift.y)) ||
                    input.from.x == (HEIGHT - EN_PASSANT_INDENT) && not idw && shift.x == 1 && cont_fig(input.from + Pos(0, shift.y))
                    ) ||
                in_hand->get_col() == Color::Black && (
                    input.from.x == (HEIGHT - EN_PASSANT_INDENT) && idw && shift.x == 1 && cont_fig(input.from + Pos(0, shift.y)) ||
                    input.from.x == 3 && not idw && shift.x == -1 && cont_fig(input.from + Pos(0, shift.y))
                    )
                )) {
            auto in_hand_in_targ_tmp = FigureFabric::instance()->submit_on(in_hand, input.target);
            if (check_for_when(what_next(in_hand->get_col()), { input.from, {input.from.x, input.target.y}, targ->get_pos() }, {}, {}, { in_hand_in_targ_tmp.get() })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            if (in_hand->get_type() == FigureType::King) {
                // Фигура на которой сейчас стоим всё ещё учитывается!
                if (check_for_when(in_hand->get_col(), { input.from, {input.from.x, input.target.y}, targ->get_pos() }, input.target)) {
                    return ErrorEvent::CHECK_IN_THAT_TILE;
                }
            }
            else {
                // Фигура на которой сейчас стоим всё ещё учитывается!
                if (check_for_when(in_hand->get_col(), { input.from, {input.from.x, input.target.y}, targ->get_pos() }, {}, { in_hand_in_targ_tmp.get() }, {})) {
                    return ErrorEvent::UNDER_CHECK;
                }
            }
            move_message.main_ev = MainEvent::EN_PASSANT;
            move_message.to_eat.push_back(get_fig({ input.from.x, input.target.y })->get_id());
            move_message.to_eat.push_back(targ->get_id());
            return move_message;
        }
    }

    for (const auto& [is_eat, curr] : expand_broom(in_hand)) {
        if (curr != input.target) continue;
        if (is_eat) {
            if (not targ->empty()) {
                auto in_hand_in_curr_tmp = FigureFabric::instance()->submit_on(in_hand, curr);
                // Фигура на которой сейчас стоим всё ещё учитывается!
                if (check_for_when(what_next(in_hand->get_col()), {input.from, input.target}, {}, {}, { in_hand_in_curr_tmp.get() })) {
                    move_message.side_evs.push_back(SideEvent::CHECK);
                }
                if (in_hand->get_type() == FigureType::King) {
                    // Фигура на которой сейчас стоим всё ещё учитывается!
                    if (check_for_when(in_hand->get_col(), {input.from, input.target}, curr)) {
                        return ErrorEvent::CHECK_IN_THAT_TILE;
                    }
                }
                else {
                    // Фигура на которой сейчас стоим всё ещё учитывается!
                    if (check_for_when(in_hand->get_col(), {input.from, input.target}, {}, { in_hand_in_curr_tmp.get() }, {})) {
                        return ErrorEvent::UNDER_CHECK;
                    }
                }
                move_message.main_ev = MainEvent::EAT;
                move_message.to_eat.push_back(targ->get_id());
                return move_message;
            }
        }
        else {
            if (targ->empty()) {
                auto in_hand_in_curr_tmp = FigureFabric::instance()->submit_on(in_hand, curr);
                if (check_for_when(what_next(in_hand->get_col()), {input.from}, {}, {}, { in_hand_in_curr_tmp.get() })) {
                    move_message.side_evs.push_back(SideEvent::CHECK);
                }
                if (in_hand->get_type() == FigureType::King) {
                    if (check_for_when(in_hand->get_col(), {input.from}, curr)) {
                        return ErrorEvent::CHECK_IN_THAT_TILE;
                    }
                }
                else {
                    if (check_for_when(in_hand->get_col(), {input.from}, {}, { in_hand_in_curr_tmp.get() }, {})) {
                        return ErrorEvent::UNDER_CHECK;
                    }
                }
                move_message.main_ev = MainEvent::MOVE;
                return move_message;
            }
        }
    }
    return ErrorEvent::UNFORESEEN;
}

/// <summary>
/// Берет ход из истории ходов
/// и воспроизводит его в обратном
/// порядке
/// </summary>
/// <returns>Удалось ли отменить ход</returns>
bool FigureBoard::undo_move() {
    if (move_logger.prev_empty()) return false;
    auto last = move_logger.move_last_to_future();
    Figure* in_hand_fig = last.get_who_went();
    auto in_hand = get_fig(in_hand_fig->get_id());
    char chose = last.promotion_choice;
    auto turn = last.turn;
    auto input = last.input;
    MoveMessage ms = last.ms;
    switch (ms.main_ev)
    {
    case MainEvent::MOVE: case MainEvent::LMOVE:
        move_fig(in_hand, input.from);
        break;
    case MainEvent::EN_PASSANT: case MainEvent::EAT:
        move_fig(in_hand, input.from);
        for (const auto& it : ms.to_eat) {
            uncapture_figure(it);
        }
        break;
    case MainEvent::CASTLING:
        move_fig(in_hand, input.from);
        for (const auto& [who, frominto] : ms.to_move) {
            auto who_it = get_fig(who);
            move_fig(who_it, frominto.from);
        }
        break;
    case MainEvent::E:
        throw std::invalid_argument("MainEvent::E");
    }
    for (const auto& s_ev : ms.side_evs) {
        switch (s_ev)
        {
        case SideEvent::CASTLING_BREAK:
            if (not ms.what_castling_breaks.empty() &&
                !has_castling(ms.what_castling_breaks.back())
                ) {
                for (const Id& id : ms.what_castling_breaks) {
                    on_castling(id);
                }
            }
            break;
        case SideEvent::PROMOTION:
            promotion_fig(in_hand, FigureType::Pawn);
            break;
        case SideEvent::CHECK:
            break;
        case SideEvent::E:
            throw std::invalid_argument("SideEvent::E");
        }
    }
    return true;
}

/// <summary>
/// Производит ход
/// </summary>
/// <param name="move_rec">Ход</param>
/// <returns>Удалось ли совершить ход</returns>
bool FigureBoard::provide_move(const MoveRec& move_rec) {
    const auto& choice = move_rec.promotion_choice;
    const auto in_hand_fig = move_rec.get_who_went();
    const auto& in_hand = get_fig(in_hand_fig->get_id());
    const auto& ms = move_rec.ms;
    const auto& turn = move_rec.turn;
    const auto& input = move_rec.input;
    switch (ms.main_ev)
    {
    case MainEvent::MOVE: case MainEvent::LMOVE:
        move_fig(in_hand, input.target);
        break;
    case MainEvent::EN_PASSANT: case MainEvent::EAT:
        move_fig(in_hand, input.target);
        for (const Id& it : ms.to_eat) {
            if (it != ERR_ID && get_fig(it)->get_pos() != input.target)
                capture_figure(it);
        }
        break;
    case MainEvent::CASTLING:
        if (has_castling(ms.to_move.back().first)) {
            move_fig(in_hand, input.target);
            for (const auto& [who, frominto] : ms.to_move) {
                auto who_it = get_fig(who);
                move_fig(who_it, frominto.target);
            }
        }
        else {
            return false;
        }
        break;
    case MainEvent::E:
        throw std::invalid_argument("MainEvent::E");
    }
    for (const auto& s_ev : ms.side_evs) {
        switch (s_ev)
        {
        case SideEvent::CASTLING_BREAK:
            if (not ms.what_castling_breaks.empty() &&
                has_castling(ms.what_castling_breaks.back())
                ) {
                for (const Id& id : ms.what_castling_breaks) {
                    off_castling(id);
                }
            }
            break;
        case SideEvent::PROMOTION:
            promotion_fig(in_hand, char_to_figure_type(choice));
            break;
        case SideEvent::CHECK:
            break;
        case SideEvent::E:
            throw std::invalid_argument("SideEvent::E");
        }
    }

    return true;
}

/// <summary>
/// Совершает отменённый ход
/// </summary>
/// <returns>Удалось ли совершить ход</returns>
bool FigureBoard::restore_move() {
    if (move_logger.future_empty()) return false;
    auto future = move_logger.pop_future_move();
    provide_move(future);
    move_logger.add_without_reset(future);
    return true;
}

/// <summary>
/// Возвращает съеденную фигуру на доску
/// </summary>
/// <exception cref="std::invalid_argument">
/// Фигуры с полученным идендификатором не было в съеденных
/// </exception>
/// <param name="id">Идентификатор фигуры</param>
void FigureBoard::uncapture_figure(const Id id) {
    if (id == ERR_ID) return;
    auto to_resurrect_id = std::find_if(
        captured_figures.begin(), captured_figures.end(),
        [&id] (auto&& val) { return id == val->get_id(); }
    );
    if (to_resurrect_id == captured_figures.end())
        throw std::invalid_argument(std::format("Fig with id->{} can't be resurrected", id));
    Figure* to_resurrect = *to_resurrect_id;
    place_figure(to_resurrect);
    captured_figures.erase(to_resurrect_id);
}

void FigureBoard::promotion_fig(Figure* to_promote, FigureType new_type) {
    Id id = to_promote->get_id();
    Color color = to_promote->get_col();
    Pos position = to_promote->get_pos();
    FigureFabric::instance()->create(position, color, new_type, id, to_promote);
}
