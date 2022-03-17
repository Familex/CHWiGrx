// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "_FigureBoard.h"

FigureBoard::FigureBoard(BoardRepr board_repr) {
    reset(board_repr);
}

void FigureBoard::init_figures_moves() {
    std::vector<pos>temp_left_up;
    std::vector<pos>temp_left_down;
    std::vector<pos>temp_right_up;
    std::vector<pos>temp_right_down;
    std::vector<pos>temp_left;
    std::vector<pos>temp_right;
    std::vector<pos>temp_up;
    std::vector<pos>temp_down;

    for (int ij = 1; ij < min(WIDTH, HEIGHT); ++ij) {
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
    moves[EFigureType::Pawn] = { {{ (idw ? -1 : 1), 0}} };
    eats[EFigureType::Pawn] = { {{ (idw ? -1 : 1), -1}}, {{ (idw ? -1 : 1), 1}} };

    moves[EFigureType::Knight] = { {{-1, 2}}, {{-1, -2}},
        {{1, -2}}, {{1, 2}}, {{2, -1}}, {{-2, -1}},
        {{2, 1}}, {{-2, 1}}, };
    eats[EFigureType::Knight] = moves[EFigureType::Knight];

    moves[EFigureType::King] = { {{-1, -1}}, {{-1, 1}}, {{-1, 0}},
        {{0, -1}}, {{0, 1}}, {{1, 1}}, {{1, -1}},
        {{1, 0}} };
    eats[EFigureType::King] = moves[EFigureType::King];

    moves[EFigureType::Bishop] = { temp_left_up, temp_left_down, temp_right_up, temp_right_down };
    eats[EFigureType::Bishop] = moves[EFigureType::Bishop];

    moves[EFigureType::Rook] = { temp_left, temp_right, temp_up, temp_down };
    eats[EFigureType::Rook] = moves[EFigureType::Rook];

    moves[EFigureType::Queen] = { temp_left_up, temp_left_down, temp_right_up, temp_right_down, temp_left, temp_right, temp_up, temp_down };
    eats[EFigureType::Queen] = moves[EFigureType::Queen];
}

void FigureBoard::reset(BoardRepr map) {
    move_logger.reset();
    curr_id = 0;
    figures.clear();
    figures.push_back({}); // Заглушка
    captured_figures.clear();
    if (map.empty()) {
        if (idw) {
            map.set_figures("bRbHbBbQbKbBbHbR8bP32E8wPwRwHwBwQwKwBwHwR");
            map.set_idw(true);
        }
        else {
            map.set_figures("wRwHwBwQwKwBwHwR8wP32E8bPbRbHbBbQbKbBbHbR");
            map.set_idw(false);
        }
    }
    init_figures_moves();
    apply_map(map);
    reset_castling();
}

void FigureBoard::apply_map(const BoardRepr& board_repr) {
    /*
    кол-во (1 по умолчанию) -> цвет -> тип
    Типы не из ctoft игнорируются, то же с ctoc
    Фигуры, не вошедшие на поле, игнорируются
    */
    int curr_flat{ -1 };
    int num{};
    char type;
    char color;
    const size_t npos = std::string::npos;
    
    idw = board_repr.get_idw();
    move_logger.set_past(board_repr.get_past());
    move_logger.set_future(board_repr.get_future());
    const std::string map = board_repr.get_figures();

    std::regex board_split("(\\d*)([bBwW]?)(\\w)");
    const int IND_NUM{ 1 };
    const int IND_COLOR{ 2 };
    const int IND_TYPE{ 3 };
    auto begin{ map.cbegin() };
    auto end{ map.cend() };
    std::smatch match{};
    while (std::regex_search(begin, end, match, board_split)) {
        num = !match.str(IND_NUM).empty()
            ? std::stoi(match.str(IND_NUM))
            : 1;

        color = !match.str(IND_COLOR).empty()
            ? match.str(IND_COLOR)[0]
            : 'N';

        type = match.str(IND_TYPE)[0];

        while (num-- && num < MAX_FIGURES_AMOUNT) {
            ++curr_flat;
            if (NOT_FIGURES.find(std::toupper(type))  != npos) continue;
            if (COLOR_CHARS.find(std::toupper(color)) == npos) continue;
            if (ALL_FIGURES.find(std::toupper(type))  == npos) continue;
            figures.push_back({
                ++curr_id,
                {(curr_flat - (curr_flat % WIDTH)) / HEIGHT,
                curr_flat % WIDTH},
                color,
                type
            });
        }

        begin += match.position() + match.length();
    }
}

// Вроде, работает, хоть и написано не очень
BoardRepr FigureBoard::get_repr(bool save_all_moves) {
    std::string map = "";
    std::vector<MoveRec> past{};
    std::vector<MoveRec> future{};
    int num{ -1 };
    char prev_type{ 'N' };
    char prev_color{ 'N' };
    auto fig{ get_default_fig() };
    for (int i{}; i < HEIGHT; ++i) {
        for (int j{}; j < WIDTH; ++j) {
            fig = get_fig({ i, j });
            char curr_type = fig->id == ERR_ID
                ? 'E'
                : (char)fig->type;

            char curr_color = fig->id == ERR_ID
                ? 'N'
                : (char)fig->color;

            if (curr_type == prev_type && curr_color == prev_color) {
                ++num;
            }
            else {
                if (num > 1)
                    map += std::format("{}", num);
                if (prev_color != 'N')
                    map += std::format("{}", prev_color);
                if (prev_type != 'N')
                    map += std::format("{}", prev_type);
                num = 1;
                prev_color = curr_color;
                prev_type = curr_type;
            }
        }
    }
    if (num > 1)
        map += std::format("{}", num);
    if (prev_color != 'N')
        map += std::format("{}", prev_color);
    if (prev_type != 'N')
        map += std::format("{}", prev_type);
    map += (idw ? "[t]" : "[f]");

    if (save_all_moves) {
        past = move_logger.get_past();
        future = move_logger.get_future();
    }
    else {
        past = { move_logger.get_last_move() };
    }
    map += "<";
    for (MoveRec& mr : past)
        map += std::format("{}, ", mr.as_string());
    map += ">";
    
    map += "<";
    for (MoveRec& mr : future)
        map += std::format("{}, ", mr.as_string());
    map += ">";
    return map;
}

void FigureBoard::reset_castling() {
    for (const EColor& col : { EColor::Black, EColor::White }) {
        castling[col].clear();
        for (const auto& aspt_to_rook : get_figures_of(col)) {
            if (aspt_to_rook.type == EFigureType::Rook) {
                castling[col][aspt_to_rook.id] = true;
            }
        }
    }
}

std::list<Figure>::iterator FigureBoard::get_fig(pos position) {
    for (auto it{ ++figures.begin() }; it != figures.end(); ++it) {
        if (it->position == position) {
            return it;
        }
    }
    return figures.begin();
    // throw std::invalid_argument("pos: " + std::to_string(position.x) + " " + std::to_string(position.y));
}

std::list<Figure>::iterator FigureBoard::get_fig(Id id) {
    for (auto it{ ++figures.begin() }; it != figures.end(); ++it) {
        if (it->id == id) {
            return it;
        }
    }
    return figures.begin();
}

bool FigureBoard::cont_fig(pos position) {
    return get_fig(position)->id != ERR_ID;
}

bool FigureBoard::is_empty(pos position) {
    return get_fig(position)->id == ERR_ID;
}

bool FigureBoard::capture_figure(std::list<Figure>::iterator it) {
    if (it->id == ERR_ID) {
        return false;
    }
    captured_figures.push_back(*it);
    figures.erase(it);
    return true;
}

bool FigureBoard::capture_figure(const Id& id) {
    if (id == ERR_ID) { return false; }
    std::list<Figure>::iterator it = get_fig(id);
    captured_figures.push_back(*it);
    figures.erase(it);
    return true;
}

bool is_valid_coords(pos position) {
    int x{ position.x }, y{ position.y };
    return ((x >= 0) && (x < HEIGHT) &&
        (y >= 0) && (y < WIDTH));
}

std::vector<Figure> FigureBoard::get_figures_of(Color col) {
    std::vector<Figure> acc{};
    for (auto it{ ++figures.begin() }; it != figures.end(); ++it) {
        if (it->color == col) {
            acc.push_back(*it);
        }
    }
    return acc;
}

std::list<Figure>::iterator FigureBoard::find_king(Color col) {
    /* тут можно замемоизировать королей
        Хотя лучше это делать сразу в конструкторе
            И вообще их не искать перебором */
    return std::find_if(
        figures.begin(),
        figures.end(),
        [col](const auto& it)
        { return it.color == col && it.type == EFigureType::King; }
    );
}

// Вроде, нормально работает... не нормально...
std::vector<std::pair<bool, pos>> FigureBoard::expand_broom(const Figure& in_hand, const std::vector<pos>& to_ignore, const std::vector<pos>& ours, const std::vector<pos>& enemies) {
    std::vector<std::pair<bool, pos>> possible_moves{}; // list { pair{ is_eat, targ }, ... }
    auto in_hand_pos = in_hand.position;
    for (auto& eat_beams : eats[in_hand.type.get_data()]) {
        for (auto eat : eat_beams) {
            pos curr{ in_hand_pos + (in_hand.color == EColor::White ? eat : eat.mul_x(-1)) };
            if (not is_valid_coords(curr)) continue;
            if (curr.in(ours)) {
                break; // Врезались в союзника
            }
            else if (curr.in(enemies)) {
                possible_moves.push_back({ true, curr }); // Съели противника
                break; // И врезались...
            }
            else if (cont_fig(curr) && not curr.in(to_ignore)) {
                if (get_fig(curr)->color != in_hand.color) {
                    possible_moves.push_back({ true, curr });
                    break; // И тут, конечно, врезались!
                }
                else {
                    break; // Врезались
                }
            }
        }
    }
    for (auto& move_beams : moves[in_hand.type.get_data()]) {
        for (auto move : move_beams) {
            pos curr{ in_hand_pos + (in_hand.color == EColor::White ? move : move.mul_x(-1)) };
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

std::vector<std::pair<bool, pos>> FigureBoard::get_all_possible_moves(const Figure& in_hand, const std::vector<pos>& to_ignore, const std::vector<pos>& ours, const std::vector<pos>&enemies) {
    std::vector<std::pair<bool, pos>> all_possible_moves{ expand_broom(in_hand, to_ignore, ours, enemies) };
    pos in_hand_pos = in_hand.position;
    std::list<Figure>::iterator in_hand_it{ get_fig(in_hand_pos) };
    if (in_hand.type == EFigureType::Pawn) {
        // Ход пешки на 2 (смотрю свои фигуры на 2 линии)
        if (in_hand.color == EColor::White) {
            if (in_hand_pos.x == (HEIGHT - 2) && idw && is_empty(in_hand_pos + pos(-1, 0)) &&
                is_empty(in_hand_pos + pos(-2, 0))) {
                all_possible_moves.push_back({ false, in_hand_pos + pos(-2, 0) });
            }
            if (in_hand_pos.x == 1 && not idw && is_empty(in_hand_pos + pos(1, 0)) &&
                is_empty(in_hand_pos + pos(2, 0))) {
                all_possible_moves.push_back({ false, in_hand_pos + pos(2, 0) });
            }
        }
        if (in_hand.color == EColor::Black) {
            if (in_hand_pos.x == 1 && idw && is_empty(in_hand_pos + pos(1, 0)) &&
                is_empty(in_hand_pos + pos(2, 0))) {
                all_possible_moves.push_back({ false, in_hand_pos + pos(2, 0) });
            }
            if (in_hand_pos.x == (HEIGHT - 2) && not idw && is_empty(in_hand_pos + pos(-1, 0)) &&
                is_empty(in_hand_pos + pos(-2, 0))) {
                all_possible_moves.push_back({ false, in_hand_pos + pos(-2, 0) });
            }
        }

        // Взятие на проходе
        const MoveRec& last_move = move_logger.get_last_move();
        if (last_move.ms.main_ev == MainEvent::LMOVE && std::abs(last_move.who_went.position.y - in_hand_pos.y) == 1) {
            int shift_y = last_move.who_went.position.y - in_hand_pos.y;
            if (in_hand.color == EColor::White) {
                if (in_hand_pos.x == 3 && idw && cont_fig(in_hand_pos + pos(0, shift_y)) && is_empty(in_hand_pos + pos(-1, shift_y))) {
                    all_possible_moves.push_back({ true, in_hand_pos + pos(-1, shift_y) });
                }
                if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && not idw && cont_fig(in_hand_pos + pos(0, shift_y)) && is_empty(in_hand_pos + pos(1, shift_y))) {
                    all_possible_moves.push_back({ true, in_hand_pos + pos(1, shift_y) });
                }
            }
            if (in_hand.color == EColor::Black) {
                if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && idw && cont_fig(in_hand_pos + pos(0, shift_y)) && is_empty(in_hand_pos + pos(1, shift_y))) {
                    all_possible_moves.push_back({ true, in_hand_pos + pos(1, shift_y) });
                }
                if (in_hand_pos.x == 3 && not idw && cont_fig(in_hand_pos + pos(0, shift_y)) && is_empty(in_hand_pos + pos(-1, shift_y))) {
                    all_possible_moves.push_back({ true, in_hand_pos + pos(-1, shift_y) });
                }
            }
        }
    }

    if (in_hand.type == EFigureType::King) {
        bool is_castling; MoveMessage mm; std::list<Figure>::iterator king; std::list<Figure>::iterator rook;
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 6} }, 6, 5);
        if (is_castling && has_castling(in_hand.color, rook->id))
            all_possible_moves.push_back({ false, {in_hand_pos.x, 6} });
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 2} }, 2, 3);
        if (is_castling && has_castling(in_hand.color, rook->id))
            all_possible_moves.push_back({ false, {in_hand_pos.x, 2} });
    }
    if (in_hand.type == EFigureType::Rook) {
        bool is_castling; MoveMessage mm; std::list<Figure>::iterator king; std::list<Figure>::iterator rook;
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 5} }, 6, 5);
        if (is_castling && has_castling(in_hand.color, rook->id))
            all_possible_moves.push_back({ false, {in_hand_pos.x, 5} });
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 3} }, 2, 3);
        if (is_castling && has_castling(in_hand.color, rook->id))
            all_possible_moves.push_back({ false, {in_hand_pos.x, 3} });
    }

    return all_possible_moves;
}

// Мастер функция
bool FigureBoard::check_for_when(Color col, const std::vector<pos>& to_ignore, pos to_defend, const std::vector<Figure>& ours, const std::vector<Figure>& enemies) {
    auto king_it = find_king(col);
    if (to_defend == pos()) {
        if (king_it == figures.end()) {
            return true; // Нечего защищать
        }
        else {
            to_defend = king_it->position;
        }
    }
    for (const auto& enemy : get_figures_of(col.what_next()) + enemies) {
        if (std::find(to_ignore.begin(), to_ignore.end(), enemy.position) != to_ignore.end()) {
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

bool FigureBoard::stalemate_for(Color col, const std::vector<pos>& to_ignore, pos to_defend) {
    auto king_it = find_king(col);
    if (king_it == figures.end()) return true; // Нет короля
    if (to_defend == pos()) to_defend = king_it->position;
    for (auto& aspt : get_figures_of(col)) {
        for (const auto& [is_eat, curr] : expand_broom(aspt)) {
            if (aspt.type == EFigureType::King
                ? (is_eat
                    ? not check_for_when(col, to_ignore + curr + to_defend, curr)
                    : not check_for_when(col, to_ignore + to_defend, curr)
                    )
                : (is_eat
                    ? not check_for_when(col, to_ignore + aspt.position + curr, to_defend, { aspt.submit_on(curr) }, {})
                    : not check_for_when(col, to_ignore + aspt.position, to_defend, { aspt.submit_on(curr) }, {})
                    )
                ) {
                return false;
            }
        }
    }
    return true;
}

bool FigureBoard::checkmate_for(Color col, const std::vector<pos>& to_ignore, pos to_defend /*тут, возможно, не король*/) {
    auto king_it = find_king(col);
    if (to_defend == pos()) {
        if (king_it == figures.end()) {
            return false; // Нечего защищать
        }
        else {
            to_defend = king_it->position;
        }
    }
    for (const auto& figure : get_figures_of(col)) {
        for (const auto& [is_eat, curr] : expand_broom(figure, to_ignore, { to_defend })) {
            if (figure.type == EFigureType::King
                ? (is_eat
                    ? not check_for_when(col, to_ignore + figure.position + curr + to_defend, curr)
                    : not check_for_when(col, to_ignore + figure.position + to_defend, curr)
                    )
                : (is_eat
                    ? not check_for_when(col, to_ignore + figure.position + curr, to_defend, {figure.submit_on(curr)})
                    : not check_for_when(col, to_ignore + figure.position, to_defend, {figure.submit_on(curr)})
                    )
                )
                return false;
        }
    }
    return true;
}

std::tuple<bool, MoveMessage, std::list<Figure>::iterator, std::list<Figure>::iterator> FigureBoard::castling_check(MoveMessage move_message, std::list<Figure>::iterator in_hand, const Input& input, int king_end_col, int rook_end_col) {
    /* Рокировка как в 960 (пока что ширина обязательно 8)
        Условия:
    * Король и рокируемая ладья не ходили ранее;
    * Поля между начальной и конечной позицией короля и ладьи соответственно пусты;
    * Король не проходит через битое поле, не находится под шахом и не встаёт под него;
      Ладья может быть под шахом
      Король мог быть под шахом до этого

    Рокировка сбрасывает все рокировки
    */
    // Рокировка на g-фланг
    bool castling_can_be_done = true;
    Id must_be_empty = ERR_ID;
    auto king = in_hand->type == EFigureType::King ? in_hand : find_king(in_hand->color);
    if (king->id == ERR_ID) return { false, move_message, get_default_fig(), get_default_fig() };
    pos king_pos = king->position;
    if (in_hand->type == EFigureType::King && input.target.y == king_end_col && in_hand->position.y != king_end_col ||
        king_pos.y == king_end_col && in_hand->type == EFigureType::Rook && input.target.y == rook_end_col && in_hand->position.y != rook_end_col) {
        // input правильный (король уже мог быть на g вертикали, так что доступна возможность рокировки от ладьи)
        // Нужно проверить, что все промежуточные позиции (где идёт король) не под шахом
        std::list<Figure>::iterator rook{ figures.begin() };
        int step{ king_end_col - king_pos.y > 0 ? 1 : -1 };
        for (pos rook_aspt_pos{ king_pos };
            is_valid_coords(rook_aspt_pos); rook_aspt_pos.y += step) {
            rook = get_fig(rook_aspt_pos);
            if (rook->color == in_hand->color && rook->type == EFigureType::Rook) {
                break;
            }
        }
        if (rook->id == ERR_ID) return { false, move_message, get_default_fig(), get_default_fig()};
        if (king_pos.x != input.target.x && rook->position.x != input.target.x) return { false, move_message, get_default_fig(), get_default_fig() };
        
        pos rook_pos = rook->position;
        pos target{ king_pos.x, king_end_col };
        for (int step = king_pos < target ? 1 : -1; king_pos != target; king_pos.y += step) {
            castling_can_be_done &= not check_for_when(in_hand->color, { king_pos, rook_pos }, king_pos);
        }
        for (int step = king_pos < rook_pos ? 1 : -1; king_pos != rook_pos; king_pos.y += step) {
            must_be_empty = get_fig(king_pos)->id;
            castling_can_be_done &= must_be_empty == ERR_ID || must_be_empty == king->id || must_be_empty == rook->id;
        }
        castling_can_be_done &= not check_for_when(in_hand->color, { king_pos, rook_pos }, { king_pos.x, king_end_col });
        must_be_empty = get_fig({ king_pos.x, rook_end_col })->id;
        castling_can_be_done &= must_be_empty == ERR_ID || must_be_empty == king->id || must_be_empty == rook->id;
        if (castling_can_be_done) {
            move_message.main_ev = MainEvent::CASTLING;
            if (in_hand->type == EFigureType::King) {
                move_message.to_move.push_back({ rook->id, {rook_pos, {rook_pos.x, rook_end_col}} });
            }
            else {
                move_message.to_move.push_back({ king->id, {king_pos, {king_pos.x, king_end_col}} });
            }
            return { true, move_message, king, rook };
        }
    }
    return { false, move_message, get_default_fig(), get_default_fig() };
}

bool FigureBoard::insufficient_material() {
    /*
    k vs. k
    k vs. kn
    k vs. kb
    kb vs. kb where any number of bishops are all on the same color
    */
    size_t size = cnt_of_figures();
    if (size <= 2) return true;
    if (size == 3 &&
        std::find_if(
            figures.begin(),
            figures.end(),
            [](const auto& it)
            { return it.type == EFigureType::Knight || it.type == EFigureType::Bishop; })
        != figures.end()
        ) return true;
    size_t b_cell_bishops_cnt{};
    size_t w_cell_bishops_cnt{};
    for (const auto& fig : all_figures()) {
        if (fig.type == EFigureType::Bishop)
            if ((fig.position.x + fig.position.y) % 2)
                ++b_cell_bishops_cnt;
            else
                ++w_cell_bishops_cnt;
        else if (fig.type != EFigureType::King)
            return false; // при модификации функции не забыть тут поставить хотя бы goto
    }
    if (not (b_cell_bishops_cnt && w_cell_bishops_cnt)) return true;
    return false;
}

bool FigureBoard::game_end(Color col) {
    return stalemate_for(col) ||
        checkmate_for(col) ||
        insufficient_material();
}

MoveMessage FigureBoard::move_check(std::list<Figure>::iterator in_hand, Input input) {
    MoveMessage move_message{ MainEvent::E, {} };
    std::list<Figure>::iterator targ{ get_fig(input.target) };

    if (not (is_valid_coords(input.from) and is_valid_coords(input.target)) ||
        input.from == input.target ||
        not cont_fig(input.from) ||
        get_fig(input.target)->color == in_hand->color) {
        throw std::invalid_argument("Invalid input");
    }

    if (in_hand->type == EFigureType::Rook) {
        move_message.side_evs.push_back(SideEvent::CASTLING_BREAK);
        move_message.what_castling_breaks.push_back(in_hand->id);
    }

    if ((in_hand->type == EFigureType::King || in_hand->type == EFigureType::Rook) &&
        WIDTH == 8) {
        move_message.side_evs.push_back(SideEvent::CASTLING_BREAK);
        for (const auto& fig : get_figures_of(in_hand->color)) {
            if (fig.type == EFigureType::Rook) {
                // Заполняется в любом случае, но MainEvent::BREAK это отменит, если что
                move_message.what_castling_breaks.push_back(fig.id);
            }
        }
        bool is_castling; MoveMessage mm; std::list<Figure>::iterator king; std::list<Figure>::iterator rook;
        std::tie(is_castling, mm, king, rook) = castling_check(move_message, in_hand, input, 6, 5);
        if (is_castling && has_castling(in_hand->color, rook->id)) {
            if (check_for_when(in_hand->color.what_next(),
                { input.from, king->position, rook->position }, {}, {},
                { king->submit_on({in_hand->position.x, 6 }), rook->submit_on({rook->position.x, 5})})) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            return mm;
        }
        std::tie(is_castling, mm, king, rook) = castling_check(move_message, in_hand, input, 2, 3);
        if (is_castling && has_castling(in_hand->color, rook->id)) {
            if (check_for_when(in_hand->color.what_next(),
                { input.from, king->position, rook->position }, {}, {},
                { king->submit_on({king->position.x, 2 }), rook->submit_on({rook->position.x, 3}) })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            return mm;
        }
    }

    if (in_hand->type == EFigureType::Pawn &&
        (in_hand->color == EColor::White &&
            (idw && input.target.x == 0 || not idw && input.target.x == (HEIGHT - 1)) ||
            in_hand->color == EColor::Black &&
            (not idw && input.target.x == 0 || idw && input.target.x == (HEIGHT - 1))
            )
        ) {
        move_message.side_evs.push_back(SideEvent::PROMOTION);
    }

    if (in_hand->type == EFigureType::Pawn) {
        pos shift{ input.target - input.from };
        // Ход пешки на 2 (смотрю свои фигуры на 2 линии)
        if (shift.y == 0 && is_empty(input.target) && (
            in_hand->color == EColor::White && (
                input.from.x == (HEIGHT - 2) && idw && shift.x == -2 && is_empty(input.from + pos(-1, 0)) ||
                input.from.x == 1 && not idw && shift.x == 2 && is_empty(input.from + pos(1, 0))
                ) ||
            in_hand->color == EColor::Black && (
                input.from.x == 1 && idw && shift.x == 2 && is_empty(input.from + pos(1, 0)) ||
                input.from.x == (HEIGHT - 2) && not idw && shift.x == -2 && is_empty(input.from + pos(-1, 0))
                )
            )) {
            if (check_for_when(in_hand->color.what_next(), { input.from }, {}, {}, { in_hand->submit_on(input.target) })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            if (in_hand->type == EFigureType::King) {
                if (check_for_when(in_hand->color, { input.from }, input.target)) {
                    throw std::invalid_argument("Check in that tile");
                }
            }
            else {
                if (check_for_when(in_hand->color, { input.from }, {}, { in_hand->submit_on(input.target) }, {})) {
                    throw std::invalid_argument("Under check");
                }
            }
            move_message.main_ev = MainEvent::LMOVE;
            return move_message;
        }

        // Взятие на проходе (смотрю чужие фигуры на 4 линии)
        // А ещё проверяю прошлый ход
        const MoveRec& last_move = move_logger.get_last_move();
        if (std::abs(shift.y) == 1 && is_empty(input.target) &&
           last_move.ms.main_ev == MainEvent::LMOVE && last_move.who_went.position.y == input.target.y &&
            (
                in_hand->color == EColor::White && (
                    input.from.x == 3 && idw && shift.x == -1 && cont_fig(input.from + pos(0, shift.y)) ||
                    input.from.x == (HEIGHT - EN_PASSANT_INDENT) && not idw && shift.x == 1 && cont_fig(input.from + pos(0, shift.y))
                    ) ||
                in_hand->color == EColor::Black && (
                    input.from.x == (HEIGHT - EN_PASSANT_INDENT) && idw && shift.x == 1 && cont_fig(input.from + pos(0, shift.y)) ||
                    input.from.x == 3 && not idw && shift.x == -1 && cont_fig(input.from + pos(0, shift.y))
                    )
                )) {
            if (check_for_when(in_hand->color.what_next(), { input.from, { input.from.x, input.target.y }, targ->position }, {}, {}, { in_hand->submit_on(input.target) })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            if (in_hand->type == EFigureType::King) {
                // Фигура на которой сейчас стоим всё ещё учитывается!
                if (check_for_when(in_hand->color, { input.from, { input.from.x, input.target.y }, targ->position }, input.target)) {
                    throw std::invalid_argument("Check in that tile");
                }
            }
            else {
                // Фигура на которой сейчас стоим всё ещё учитывается!
                if (check_for_when(in_hand->color, { input.from, { input.from.x, input.target.y }, targ->position }, {}, { in_hand->submit_on(input.target) }, {})) {
                    throw std::invalid_argument("Under check");
                }
            }
            move_message.main_ev = MainEvent::EN_PASSANT;
            move_message.to_eat.push_back(get_fig({ input.from.x, input.target.y })->id);
            move_message.to_eat.push_back(targ->id);
            return move_message;
        }
    }

    for (const auto& [is_eat, curr] : expand_broom(*in_hand)) {
        if (curr != input.target) continue;
        if (is_eat) {
            if (targ->id != ERR_ID) {
                // Фигура на которой сейчас стоим всё ещё учитывается!
                if (check_for_when(in_hand->color.what_next(), { input.from, input.target }, {}, {}, { in_hand->submit_on(curr) })) {
                    move_message.side_evs.push_back(SideEvent::CHECK);
                }
                if (in_hand->type == EFigureType::King) {
                    // Фигура на которой сейчас стоим всё ещё учитывается!
                    if (check_for_when(in_hand->color, { input.from, input.target }, curr)) {
                        throw std::invalid_argument("Check in that tile");
                    }
                }
                else {
                    // Фигура на которой сейчас стоим всё ещё учитывается!
                    if (check_for_when(in_hand->color, { input.from, input.target }, {}, { in_hand->submit_on(curr) }, {})) {
                        throw std::invalid_argument("Under check");
                    }
                }
                move_message.main_ev = MainEvent::EAT;
                move_message.to_eat.push_back(targ->id);
                return move_message;
            }
        }
        else {
            if (targ->id == ERR_ID) {
                if (check_for_when(in_hand->color.what_next(), { input.from }, {}, {}, { in_hand->submit_on(curr) })) {
                    move_message.side_evs.push_back(SideEvent::CHECK);
                }
                if (in_hand->type == EFigureType::King) {
                    if (check_for_when(in_hand->color, { input.from }, curr)) {
                        throw std::invalid_argument("Check in that tile");
                    }
                }
                else {
                    if (check_for_when(in_hand->color, { input.from }, {}, { in_hand->submit_on(curr) }, {})) {
                        throw std::invalid_argument("Under check");
                    }
                }
                move_message.main_ev = MainEvent::MOVE;
                return move_message;
            }
        }
    }
    throw std::invalid_argument("Unforeseen move");
}

bool FigureBoard::undo_move() {
    if (move_logger.prev_empty()) return false;
    auto last = move_logger.move_last_to_future();
    Figure in_hand_fig = last.who_went;
    auto in_hand = get_fig(in_hand_fig.id);
    char chose = last.promotion_choice;
    auto turn = last.turn;
    auto input = last.input;
    MoveMessage ms = last.ms;
    switch (ms.main_ev)
    {
    case MainEvent::MOVE: case MainEvent::LMOVE:
        in_hand->move_to(input.from);
        break;
    case MainEvent::EN_PASSANT: case MainEvent::EAT:
        in_hand->move_to(input.from);
        for (const auto& it : ms.to_eat) {
            uncapture_figure(it);
        }
        break;
    case MainEvent::CASTLING:
        in_hand->move_to(input.from);
        for (const auto& [who, frominto] : ms.to_move) {
            auto who_it = get_fig(who);
            who_it->move_to(frominto.from);
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
                !has_castling(turn, ms.what_castling_breaks.back())
                ) {
                for (const Id& id : ms.what_castling_breaks) {
                    on_castling(turn, id);
                }
            }
            break;
        case SideEvent::PROMOTION:
            in_hand->type = EFigureType::Pawn;
            break;
        case SideEvent::CHECK:
            break;
        case SideEvent::E:
            throw std::invalid_argument("SideEvent::E");
        }
    }
    return true;
}

bool FigureBoard::provide_move(const MoveRec& move_rec) {
    auto choice = move_rec.promotion_choice;
    auto in_hand_fig = move_rec.who_went;
    auto in_hand = get_fig(in_hand_fig.id);
    auto ms = move_rec.ms;
    auto turn = move_rec.turn;
    auto input = move_rec.input;
    switch (ms.main_ev)
    {
    case MainEvent::MOVE: case MainEvent::LMOVE:
        in_hand->move_to(input.target);
        break;
    case MainEvent::EN_PASSANT: case MainEvent::EAT:
        in_hand->move_to(input.target);
        for (const Id& it : ms.to_eat) capture_figure(it);
        break;
    case MainEvent::CASTLING:
        if (has_castling(turn, ms.to_move.back().first)) {
            in_hand->move_to(input.target);
            for (const auto& [who, frominto] : ms.to_move) {
                auto who_it = get_fig(who);
                who_it->move_to(frominto.target);
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
                has_castling(turn, ms.what_castling_breaks.back())
                ) {
                for (const Id& id : ms.what_castling_breaks) {
                    off_castling(turn, id);
                }
            }
            break;
        case SideEvent::PROMOTION:
            in_hand->type = choice;
            break;
        case SideEvent::CHECK:
            break;
        case SideEvent::E:
            throw std::invalid_argument("SideEvent::E");
        }
    }

    return true;
}

bool FigureBoard::restore_move() {
    if (move_logger.future_empty()) return false;
    auto future = move_logger.pop_future_move();
    provide_move(future);
    move_logger.add_without_reset(future);
    return true;
}

void FigureBoard::uncapture_figure(const Id& id) {
    if (id == ERR_ID) return;
    std::list<Figure>::iterator to_resurrect = std::find_if(
        captured_figures.begin(), captured_figures.end(),
        [&id] (auto&& val) { return id == val.id; }
    );
    if (to_resurrect == captured_figures.end())
        throw std::invalid_argument(std::format("Fig with id->{} can't be resurrected", id));
    place_figure(*to_resurrect);
    captured_figures.erase(to_resurrect);
}