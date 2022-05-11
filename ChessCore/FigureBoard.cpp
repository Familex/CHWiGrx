#include "FigureBoard.h"

FigureBoard::FigureBoard(BoardRepr board_repr) {
    reset(board_repr);
}

/// <summary>
/// ��������� �������� moves � eats
/// </summary>
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

    // ������� ������������ ����� ����� idw
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

/// <summary>
/// 
/// </summary>
/// <param name="map">������������� ����� �����</param>
void FigureBoard::reset(const BoardRepr& map) {
    move_logger.reset();
    curr_id = 0;
    figures.clear();
    captured_figures.clear();
    apply_map(map);
    init_figures_moves();
    reset_castling();
}

/// <summary>
/// ������������� ����� � ������� ����� �����
/// </summary>
/// <param name="board_repr">������������� ����� �����</param>
void FigureBoard::apply_map(const BoardRepr& board_repr) {
    idw = board_repr.get_idw();
    move_logger.set_past(board_repr.get_past());
    move_logger.set_future(board_repr.get_future());
    captured_figures = board_repr.get_captured_figures();
    for (const auto& fig : board_repr.get_figures())
        figures[fig->get_pos()] = fig;
}

/// <summary>
/// ���������� ������������� �����
/// </summary>
/// <param name="save_all_moves">����� ��������� ��� ���� �� ������� ��� ������ ���������</param>
/// <returns>������������� ����� �����</returns>
BoardRepr FigureBoard::get_repr(bool save_all_moves) {
    std::string map = "";
    std::vector<MoveRec> past{};
    std::vector<MoveRec> future{};
    for (auto& [position, fig] : figures) {
        map += std::format("{};{};{};{};{};",
            fig->get_id(), position.x, position.y, (char)fig->get_col(), (char)fig->get_type()
        );
    }
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
        map += std::format("{}$", mr.as_string());
    map += ">";
    
    map += "<";
    for (MoveRec& mr : future)
        map += std::format("{}$", mr.as_string());
    map += ">~";
    for (auto& fig : captured_figures) {
        map += std::format("{},{},{},{},{},",
            fig->get_id(), fig->get_pos().x, fig->get_pos().y, (char)fig->get_col(), (char)fig->get_type()
        );
    }
    return map;
}

/// <summary>
/// ���������� ��������� ���� ������
/// </summary>
void FigureBoard::reset_castling() {
    for (const EColor& col : { EColor::Black, EColor::White }) {
        castling[col].clear();
        for (const auto& aspt_to_rook : get_figures_of(col)) {
            if (aspt_to_rook->get_type() == EFigureType::Rook) {
                castling[col][aspt_to_rook->get_id()] = true;
            }
        }
    }
}

/// <summary>
/// ���������� ������ �� �������
/// </summary>
/// <param name="position">������� ������</param>
/// <returns>�������� �� ������</returns>
Figure* FigureBoard::get_fig(pos position) {
    /* ����� ����� �� ����� */
    for (const auto& [_, fig] : figures) {
        if (fig->at(position)) {
            return fig;
        }
    }
    return get_default_fig();
}

/// <summary>
/// ���������� ������ �� ��������������
/// </summary>
/// <param name="id">������������� ������</param>
/// <returns>�������� �� ������</returns>
Figure* FigureBoard::get_fig(Id id) {
    for (const auto& [_, fig] : figures) {
        if (fig->is(id)) {
            return fig;
        }
    }
    return get_default_fig();
}

/// <summary>
/// ���������� �������� �� ������ ������
/// </summary>
bool FigureBoard::cont_fig(pos position) {
    return not get_fig(position)->empty();
}

/// <summary>
/// ���������� �� �������� �� ������ ������
/// </summary>
bool FigureBoard::is_empty(pos position) {
    return get_fig(position)->empty();
}

/// <summary>
/// ������� ������ � ����� � ��������� � � ���������
/// </summary>
/// <param name="it">�������� �� ������</param>
/// <returns>���������� �� ������</returns>
bool FigureBoard::capture_figure(Figure* it) {
    if (it->empty()) {
        return false;
    }
    captured_figures.push_back(it);
    figures.erase(it->get_pos());
    return true;
}

/// <summary>
/// ������� ������ � ����� � ��������� � � ���������
/// </summary>
/// <param name="it">������������� ������</param>
/// <returns>���������� �� ������</returns>
bool FigureBoard::capture_figure(const Id& id) {
    if (id == ERR_ID) { return false; }
    Figure* fig = get_fig(id);
    captured_figures.push_back(fig);
    figures.erase(fig->get_pos());
    return true;
}

bool is_valid_coords(pos position) {
    int x{ position.x }, y{ position.y };
    return ((x >= 0) && (x < HEIGHT) &&
        (y >= 0) && (y < WIDTH));
}

/// <param name="col">���� �����</param>
/// <returns>��� ������ ������������ �����</returns>
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
        { return it.second->is_col(col) && it.second->get_type() == EFigureType::King; }
    );
    return (*map_ptr).second;
}

/// <summary>
/// ��������� ������ ����� ������� ������, ����������
/// ��������� �� ����
/// </summary>
/// <param name="in_hand">������, ������� �����</param>
/// <param name="to_ignore">������, ������� ����� ������������</param>
/// <param name="ours">������, � ������� ���������, �� �� ����� ������</param>
/// <param name="enemies">������, � ������� ��������� � ����� ������</param>
/// <returns>����� �� ��� ��� �� ������ � ���� ��������</returns>
std::vector<std::pair<bool, pos>> FigureBoard::expand_broom(const Figure* in_hand, const std::vector<pos>& to_ignore, const std::vector<pos>& ours, const std::vector<pos>& enemies) {
    std::vector<std::pair<bool, pos>> possible_moves{}; // list { pair{ is_eat, targ }, ... }
    auto in_hand_pos = in_hand->get_pos();
    for (auto& eat_beams : eats[in_hand->get_type().get_data()]) {
        for (auto eat : eat_beams) {
            pos curr{ in_hand_pos + (in_hand->is_col(EColor::White) ? eat : eat.mul_x(-1)) };
            if (not is_valid_coords(curr)) continue;
            if (curr.in(ours)) {
                break; // ��������� � ��������
            }
            else if (curr.in(enemies)) {
                possible_moves.push_back({ true, curr }); // ����� ����������
                break; // � ���������...
            }
            else if (cont_fig(curr) && not curr.in(to_ignore)) {
                if (not get_fig(curr)->is_col(in_hand->get_col())) {
                    possible_moves.push_back({ true, curr });
                    break; // ���������
                }
                else {
                    break; // ���������
                }
            }
        }
    }
    for (auto& move_beams : moves[in_hand->get_type().get_data()]) {
        for (auto move : move_beams) {
            pos curr{ in_hand_pos + (in_hand->is_col(EColor::White) ? move : move.mul_x(-1)) };
            if (not is_valid_coords(curr)) continue;
            if (not ((not curr.in(to_ignore) && cont_fig(curr)) || curr.in(ours) || curr.in(enemies))) {
                possible_moves.push_back({ false, curr });
            }
            else {
                break; // ���������
            }
        }
    }

    return possible_moves;
}

/// <summary>
/// ���������� ��������� ������ ����, ��������� �
/// � ������ ��� � ������ ������
/// </summary>
/// <param name="in_hand">������, ������� �����</param>
/// <param name="to_ignore">������, ������� ����� ������������</param>
/// <param name="ours">������, � ������� ���������, �� �� ����� ������</param>
/// <param name="enemies">������, � ������� ��������� � ����� ������</param>
/// <returns>����� �� ��� ��� �� ������ � ���� ��������</returns>
std::vector<std::pair<bool, pos>> FigureBoard::get_all_possible_moves(const Figure* in_hand, const std::vector<pos>& to_ignore, const std::vector<pos>& ours, const std::vector<pos>&enemies) {
    std::vector<std::pair<bool, pos>> all_possible_moves{ expand_broom(in_hand, to_ignore, ours, enemies) };
    pos in_hand_pos = in_hand->get_pos();
    Figure* in_hand_it{ get_fig(in_hand_pos) };
    if (in_hand->get_type() == EFigureType::Pawn) {
        // ��� ����� �� 2 (������ ���� ������ �� 2 �����)
        if (in_hand->is_col(EColor::White)) {
            if (in_hand_pos.x == (HEIGHT - 2) && idw && is_empty(in_hand_pos + pos(-1, 0)) &&
                is_empty(in_hand_pos + pos(-2, 0))) {
                all_possible_moves.push_back({ false, in_hand_pos + pos(-2, 0) });
            }
            if (in_hand_pos.x == 1 && not idw && is_empty(in_hand_pos + pos(1, 0)) &&
                is_empty(in_hand_pos + pos(2, 0))) {
                all_possible_moves.push_back({ false, in_hand_pos + pos(2, 0) });
            }
        }
        if (in_hand->is_col(EColor::Black)) {
            if (in_hand_pos.x == 1 && idw && is_empty(in_hand_pos + pos(1, 0)) &&
                is_empty(in_hand_pos + pos(2, 0))) {
                all_possible_moves.push_back({ false, in_hand_pos + pos(2, 0) });
            }
            if (in_hand_pos.x == (HEIGHT - 2) && not idw && is_empty(in_hand_pos + pos(-1, 0)) &&
                is_empty(in_hand_pos + pos(-2, 0))) {
                all_possible_moves.push_back({ false, in_hand_pos + pos(-2, 0) });
            }
        }

        // ������ �� �������
        const MoveRec& last_move = move_logger.get_last_move();
        if (last_move.ms.main_ev == MainEvent::LMOVE && std::abs(last_move.get_who_went_pos().y - in_hand_pos.y) == 1) {
            int shift_y = last_move.get_who_went_pos().y - in_hand_pos.y;
            if (in_hand->is_col(EColor::White)) {
                if (in_hand_pos.x == 3 && idw && cont_fig(in_hand_pos + pos(0, shift_y)) && is_empty(in_hand_pos + pos(-1, shift_y))) {
                    all_possible_moves.push_back({ true, in_hand_pos + pos(-1, shift_y) });
                }
                if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && not idw && cont_fig(in_hand_pos + pos(0, shift_y)) && is_empty(in_hand_pos + pos(1, shift_y))) {
                    all_possible_moves.push_back({ true, in_hand_pos + pos(1, shift_y) });
                }
            }
            if (in_hand->is_col(EColor::Black)) {
                if (in_hand_pos.x == (HEIGHT - EN_PASSANT_INDENT) && idw && cont_fig(in_hand_pos + pos(0, shift_y)) && is_empty(in_hand_pos + pos(1, shift_y))) {
                    all_possible_moves.push_back({ true, in_hand_pos + pos(1, shift_y) });
                }
                if (in_hand_pos.x == 3 && not idw && cont_fig(in_hand_pos + pos(0, shift_y)) && is_empty(in_hand_pos + pos(-1, shift_y))) {
                    all_possible_moves.push_back({ true, in_hand_pos + pos(-1, shift_y) });
                }
            }
        }
    }

    if (in_hand->get_type() == EFigureType::King) {
        bool is_castling; MoveMessage mm; Figure* king; Figure* rook;
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 6} }, 6, 5);
        if (is_castling && has_castling(in_hand->get_col(), rook->get_id()))
            all_possible_moves.push_back({ false, {in_hand_pos.x, 6} });
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 2} }, 2, 3);
        if (is_castling && has_castling(in_hand->get_col(), rook->get_id()))
            all_possible_moves.push_back({ false, {in_hand_pos.x, 2} });
    }
    if (in_hand->get_type() == EFigureType::Rook) {
        bool is_castling; MoveMessage mm; Figure* king; Figure* rook;
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 5} }, 6, 5);
        if (is_castling && has_castling(in_hand->get_col(), rook->get_id()))
            all_possible_moves.push_back({ false, {in_hand_pos.x, 5} });
        std::tie(is_castling, mm, king, rook) = castling_check({}, in_hand_it, { in_hand_pos, {in_hand_pos.x, 3} }, 2, 3);
        if (is_castling && has_castling(in_hand->get_col(), rook->get_id()))
            all_possible_moves.push_back({ false, {in_hand_pos.x, 3} });
    }

    return all_possible_moves;
}

/// <summary>
/// ��������� ����� �� ���
/// </summary>
/// <param name="col">����, ��� �������� ��� �����������</param>
/// <param name="to_ignore">������, ������� ����� ������������</param>
/// <param name="to_defend">������� ������ ��� ������, ��� ������� ����������� ���</param>
/// <param name="ours">������, ������� ������������� ���</param>
/// <param name="enemies">������, ������� ��� ����� ���������</param>
/// <returns>������� ����</returns>
bool FigureBoard::check_for_when(Color col, const std::vector<pos>& to_ignore, pos to_defend, const std::vector<Figure*>& ours, const std::vector<Figure*>& enemies) {
    auto king_it = find_king(col);
    if (to_defend == pos()) {
        if (king_it->empty()) {
            return true; // ������ ��������
        }
        else {
            to_defend = king_it->get_pos();
        }
    }
    for (const auto& enemy : get_figures_of(col.what_next()) + enemies) {
        if (std::find(to_ignore.begin(), to_ignore.end(), enemy->get_pos()) != to_ignore.end()) {
            if (std::find(enemies.begin(), enemies.end(), enemy) == enemies.end()) // ���� �� �����, �� ������ ���� ��� �� ������
                continue;
        }
        for (const auto& [is_eat, curr] : expand_broom(enemy, to_ignore, to_pos_vector(enemies), to_pos_vector(ours) + to_defend)) {
            if (is_eat && curr == to_defend) {
                return true;
            }
        }
    }
    return false; // ����� �� �������
}

/// <summary>
/// �������� ����� �� ���
/// </summary>
/// <param name="col">����, ��� �������� ��� �����������</param>
/// <param name="to_ignore">������, ������� ����� ������������</param>
/// <param name="to_defend">������� ������ ��� ������, ������� ����� ��������� ���</param>
/// <returns>������� ����</returns>
bool FigureBoard::stalemate_for(Color col, const std::vector<pos>& to_ignore, pos to_defend) {
    auto king_it = find_king(col);
    if (king_it->empty())
        return true; // ��� ������
    if (to_defend == pos()) to_defend = king_it->get_pos();
    for (auto& aspt : get_figures_of(col)) {
        for (const auto& [is_eat, curr] : expand_broom(aspt)) {
            if (aspt->get_type() == EFigureType::King) {
                if (is_eat
                    ? not check_for_when(col, to_ignore + curr + to_defend, curr)
                    : not check_for_when(col, to_ignore + to_defend, curr)
                    ) {
                    return false;
                }
            }
            else {
                Figure* tmp = FigureFabric::instance()->submit_on(aspt, curr);
                bool check = (is_eat
                    ? check_for_when(col, to_ignore + aspt->get_pos() + curr, to_defend, { tmp }, {})
                    : check_for_when(col, to_ignore + aspt->get_pos(), to_defend, { tmp }, {})
                    );
                delete tmp;
                if (not check) {
                    return false;
                }
            }
        }
    }
    return true;
}

/// <summary>
/// �������� ����� �� ���
/// </summary>
/// <param name="col">����, ��� �������� ��� �����������</param>
/// <param name="to_ignore">������, ������� ����� ������������</param>
/// <param name="to_defend">������� ������ ��� ������, ������� ����� ��������� ��� (���)</param>
/// <returns>������� ����</returns>
bool FigureBoard::checkmate_for(Color col, const std::vector<pos>& to_ignore, pos to_defend /*���, ��������, �� ������*/) {
    auto king_it = find_king(col);
    if (to_defend == pos()) {
        if (king_it->empty()) {
            return false; // ������ ��������
        }
        else {
            to_defend = king_it->get_pos();
        }
    }
    for (const auto& figure : get_figures_of(col)) {
        for (const auto& [is_eat, curr] : expand_broom(figure, to_ignore, { to_defend })) {
            if (figure->get_type() == EFigureType::King) {
                if (is_eat
                    ? not check_for_when(col, to_ignore + figure->get_pos() + curr + to_defend, curr)
                    : not check_for_when(col, to_ignore + figure->get_pos() + to_defend, curr)
                    ) {
                    return false;
                }
            }
            else {
                Figure* tmp = FigureFabric::instance()->submit_on(figure, curr);
                bool check = (is_eat
                    ? check_for_when(col, to_ignore + figure->get_pos() + curr, to_defend, { tmp })
                    : check_for_when(col, to_ignore + figure->get_pos(), to_defend, { tmp })
                    );
                delete tmp;
                if (not check) {
                    return false;
                }
            }
        }
    }
    return true;
}

/// <summary>
/// <para>�������� ���������� ���������</para>
/// <para>��������� ��� � 960 (������ ����������� 8)</para>
/// <para>�������:</para>
/// <para>1. ������ � ���������� ����� �� ������ �����</para>
/// <para>2. ���� ����� ��������� � �������� �������� ������ � ����� �������������� �����</para>
/// <para>3. ������ �� �������� ����� ����� ����, �� ��������� ��� ����� � �� ����� ��� ����</para>
/// <para><i>����� ����� ���� ��� �����</i></para>
/// <para><i>������ ��� ���� ��� ����� �� �����</i></para>
/// <para><i>��������� ���������� ��� ���������</i></para>
/// </summary>
/// <param name="move_message">��������� ���� ��� �����������</param>
/// <param name="in_hand">������, ������� ���������� �������</param>
/// <param name="input">���� ��� ��������</param>
/// <param name="king_end_col">������� ������� ��� ������</param>
/// <param name="rook_end_col">������� ������� ��� �����</param>
/// <returns>������� �� ���������</returns>
std::tuple<bool, MoveMessage, Figure*, Figure*> FigureBoard::castling_check(MoveMessage move_message, Figure* in_hand, const Input& input, int king_end_col, int rook_end_col) {
    // ��������� �� g-�����
    bool castling_can_be_done = true;
    Id must_be_empty = ERR_ID;
    auto king = in_hand->get_type() == EFigureType::King ? in_hand : find_king(in_hand->get_col());
    if (king->empty())
        return { false, move_message, get_default_fig(), get_default_fig() };
    pos king_pos = king->get_pos();
    if (in_hand->get_type() == EFigureType::King && input.target.y == king_end_col && in_hand->get_pos().y != king_end_col ||
        king_pos.y == king_end_col && in_hand->get_type() == EFigureType::Rook && input.target.y == rook_end_col && in_hand->get_pos().y != rook_end_col) {
        // input ���������� (������ ��� ��� ���� �� g ���������, ��� ��� �������� ����������� ��������� �� �����)
        // ����� ���������, ��� ��� ������������� ������� (��� ��� ������) �� ��� �����
        auto [_, rook] = *figures.begin();
        int step{ king_end_col - king_pos.y > 0 ? 1 : -1 };
        for (pos rook_aspt_pos{ king_pos };
            is_valid_coords(rook_aspt_pos); rook_aspt_pos.y += step) {
            rook = get_fig(rook_aspt_pos);
            if (rook->is_col(in_hand->get_col()) && rook->get_type() == EFigureType::Rook) {
                break;
            }
        }
        if (rook->empty() || rook->get_type() != EFigureType::Rook) return {false, move_message, get_default_fig(), get_default_fig()};
        if (king_pos.x != input.target.x && rook->get_pos().x != input.target.x) return { false, move_message, get_default_fig(), get_default_fig() };
        
        pos rook_pos = rook->get_pos();
        pos target{ king_pos.x, king_end_col };
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
            if (in_hand->get_type() == EFigureType::King) {
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
/// <para>�������� �� ������������� �������� ��� ����</para>
/// <para>��������:</para>
/// <para>������ ������ ������</para>
/// <para>������ ������ ������ � ����</para>
/// <para>������ ������ ������ � �����</para>
/// <para>������ � ���� ������ ������ � ������, ��� ��� ����� �� ����� �����</para>
/// </summary>
/// <returns>�� ������� �� ��������� ��� ����</returns>
bool FigureBoard::insufficient_material() {
    size_t size = cnt_of_figures();
    if (size <= 2) return true;
    if (size == 3 &&
        std::find_if(
            figures.begin(),
            figures.end(),
            [](const auto& it)
            { return it.second->get_type() == EFigureType::Knight || it.second->get_type() == EFigureType::Bishop; }
        ) != figures.end()
        ) return true;
    size_t b_cell_bishops_cnt{};
    size_t w_cell_bishops_cnt{};
    for (const auto& fig : all_figures()) {
        if (fig->get_type() == EFigureType::Bishop)
            if ((fig->get_pos().x + fig->get_pos().y) % 2)
                ++b_cell_bishops_cnt;
            else
                ++w_cell_bishops_cnt;
        else if (fig->get_type() != EFigureType::King)
            return false; // ��� ����������� ������� �� ������ ��� ��������� ���� �� goto
    }
    return not (b_cell_bishops_cnt && w_cell_bishops_cnt);
}

bool FigureBoard::game_end(Color col) {
    return stalemate_for(col) ||
        checkmate_for(col) ||
        insufficient_material();
}

/// <summary>
/// �������� ���� �� ���������� � ����������� ��������� ����
/// </summary>
/// <exception cref="std::invalid_argument">��� ������ ���������</exception>
/// <param name="in_hand">������, ������� ���������� ������</param>
/// <param name="input">����</param>
/// <returns>��������� ����</returns>
MoveMessage FigureBoard::move_check(Figure* in_hand, Input input) {
    MoveMessage move_message{ MainEvent::E, {} };
    Figure* targ{ get_fig(input.target) };

    if (not (is_valid_coords(input.from) and is_valid_coords(input.target)) ||
        input.from == input.target ||
        not cont_fig(input.from) ||
        get_fig(input.target)->is_col(in_hand->get_col())) {
        throw std::invalid_argument("Invalid input");
    }

    if (in_hand->get_type() == EFigureType::Rook) {
        move_message.side_evs.push_back(SideEvent::CASTLING_BREAK);
        move_message.what_castling_breaks.push_back(in_hand->get_id());
    }

    if ((in_hand->get_type() == EFigureType::King || in_hand->get_type() == EFigureType::Rook) &&
        WIDTH == 8) {
        move_message.side_evs.push_back(SideEvent::CASTLING_BREAK);
        for (const auto& fig : get_figures_of(in_hand->get_col())) {
            if (fig->get_type() == EFigureType::Rook) {
                // ����������� � ����� ������, �� MainEvent::BREAK ��� �������, ���� ���
                move_message.what_castling_breaks.push_back(fig->get_id());
            }
        }
        bool is_castling; MoveMessage mm; Figure* king; Figure* rook;
        std::tie(is_castling, mm, king, rook) = castling_check(move_message, in_hand, input, 6, 5);
        if (is_castling && has_castling(in_hand->get_col(), rook->get_id())) {
            Figure* king_tmp = FigureFabric::instance()->submit_on(king, { in_hand->get_pos().x, 6 });
            Figure* rook_tmp = FigureFabric::instance()->submit_on(rook, { rook->get_pos().x, 5 });
            if (check_for_when(in_hand->get_col().what_next(),
                { input.from, king->get_pos(), rook->get_pos() }, {}, {},
                { king_tmp, rook_tmp })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            delete king_tmp;
            delete rook_tmp;
            return mm;
        }
        std::tie(is_castling, mm, king, rook) = castling_check(move_message, in_hand, input, 2, 3);
        if (is_castling && has_castling(in_hand->get_col(), rook->get_id())) {
            Figure* king_tmp = FigureFabric::instance()->submit_on(king, { king->get_pos().x, 2 });
            Figure* rook_tmp = FigureFabric::instance()->submit_on(rook, { rook->get_pos().x, 3 });
            if (check_for_when(in_hand->get_col().what_next(),
                { input.from, king->get_pos(), rook->get_pos()}, {}, {},
                { king_tmp, rook_tmp })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            delete king_tmp;
            delete rook_tmp;
            return mm;
        }
    }

    if (in_hand->get_type() == EFigureType::Pawn &&
        (in_hand->is_col(EColor::White) &&
            (idw && input.target.x == 0 || not idw && input.target.x == (HEIGHT - 1)) ||
            in_hand->get_col() == EColor::Black &&
            (not idw && input.target.x == 0 || idw && input.target.x == (HEIGHT - 1))
            )
        ) {
        move_message.side_evs.push_back(SideEvent::PROMOTION);
    }

    if (in_hand->get_type() == EFigureType::Pawn) {
        pos shift{ input.target - input.from };
        // ��� ����� �� 2 (������ ���� ������ �� 2 �����)
        if (shift.y == 0 && is_empty(input.target) && (
            in_hand->get_col() == EColor::White && (
                input.from.x == (HEIGHT - 2) && idw && shift.x == -2 && is_empty(input.from + pos(-1, 0)) ||
                input.from.x == 1 && not idw && shift.x == 2 && is_empty(input.from + pos(1, 0))
                ) ||
            in_hand->is_col(EColor::Black) && (
                input.from.x == 1 && idw && shift.x == 2 && is_empty(input.from + pos(1, 0)) ||
                input.from.x == (HEIGHT - 2) && not idw && shift.x == -2 && is_empty(input.from + pos(-1, 0))
                )
            )) {
            Figure* in_hand_in_targ_tmp = FigureFabric::instance()->submit_on(in_hand, input.target);
            if (check_for_when(in_hand->get_col().what_next(), {input.from}, {}, {}, { in_hand_in_targ_tmp })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            if (in_hand->get_type() == EFigureType::King) {
                if (check_for_when(in_hand->get_col(), {input.from}, input.target)) {
                    throw std::invalid_argument("Check in that tile");
                }
            }
            else {
                if (check_for_when(in_hand->get_col(), {input.from}, {}, { in_hand_in_targ_tmp }, {})) {
                    throw std::invalid_argument("Under check");
                }
            }
            move_message.main_ev = MainEvent::LMOVE;
            delete in_hand_in_targ_tmp;
            return move_message;
        }

        // ������ �� ������� (������ ����� ������ �� 4 �����)
        // � ��� �������� ������� ���
        const MoveRec& last_move = move_logger.get_last_move();
        if (std::abs(shift.y) == 1 && is_empty(input.target) &&
           last_move.ms.main_ev == MainEvent::LMOVE && last_move.get_who_went_pos().y == input.target.y &&
            (
                in_hand->get_col() == EColor::White && (
                    input.from.x == 3 && idw && shift.x == -1 && cont_fig(input.from + pos(0, shift.y)) ||
                    input.from.x == (HEIGHT - EN_PASSANT_INDENT) && not idw && shift.x == 1 && cont_fig(input.from + pos(0, shift.y))
                    ) ||
                in_hand->get_col() == EColor::Black && (
                    input.from.x == (HEIGHT - EN_PASSANT_INDENT) && idw && shift.x == 1 && cont_fig(input.from + pos(0, shift.y)) ||
                    input.from.x == 3 && not idw && shift.x == -1 && cont_fig(input.from + pos(0, shift.y))
                    )
                )) {
            Figure* in_hand_in_targ_tmp = FigureFabric::instance()->submit_on(in_hand, input.target);
            if (check_for_when(in_hand->get_col().what_next(), {input.from, {input.from.x, input.target.y}, targ->get_pos()}, {}, {}, { in_hand_in_targ_tmp })) {
                move_message.side_evs.push_back(SideEvent::CHECK);
            }
            if (in_hand->get_type() == EFigureType::King) {
                // ������ �� ������� ������ ����� �� ��� �����������!
                if (check_for_when(in_hand->get_col(), {input.from, {input.from.x, input.target.y}, targ->get_pos()}, input.target)) {
                    throw std::invalid_argument("Check in that tile");
                }
            }
            else {
                // ������ �� ������� ������ ����� �� ��� �����������!
                if (check_for_when(in_hand->get_col(), {input.from, {input.from.x, input.target.y}, targ->get_pos()}, {}, { in_hand_in_targ_tmp }, {})) {
                    throw std::invalid_argument("Under check");
                }
            }
            move_message.main_ev = MainEvent::EN_PASSANT;
            move_message.to_eat.push_back(get_fig({ input.from.x, input.target.y })->get_id());
            move_message.to_eat.push_back(targ->get_id());
            delete in_hand_in_targ_tmp;
            return move_message;
        }
    }

    for (const auto& [is_eat, curr] : expand_broom(in_hand)) {
        if (curr != input.target) continue;
        if (is_eat) {
            if (not targ->empty()) {
                Figure* in_hand_in_curr_tmp = FigureFabric::instance()->submit_on(in_hand, curr);
                // ������ �� ������� ������ ����� �� ��� �����������!
                if (check_for_when(in_hand->get_col().what_next(), {input.from, input.target}, {}, {}, { in_hand_in_curr_tmp })) {
                    move_message.side_evs.push_back(SideEvent::CHECK);
                }
                if (in_hand->get_type() == EFigureType::King) {
                    // ������ �� ������� ������ ����� �� ��� �����������!
                    if (check_for_when(in_hand->get_col(), {input.from, input.target}, curr)) {
                        delete in_hand_in_curr_tmp;
                        throw std::invalid_argument("Check in that tile");
                    }
                }
                else {
                    // ������ �� ������� ������ ����� �� ��� �����������!
                    if (check_for_when(in_hand->get_col(), {input.from, input.target}, {}, { in_hand_in_curr_tmp }, {})) {
                        delete in_hand_in_curr_tmp;
                        throw std::invalid_argument("Under check");
                    }
                }
                move_message.main_ev = MainEvent::EAT;
                move_message.to_eat.push_back(targ->get_id());
                delete in_hand_in_curr_tmp;
                return move_message;
            }
        }
        else {
            if (targ->empty()) {
                Figure* in_hand_in_curr_tmp = FigureFabric::instance()->submit_on(in_hand, curr);
                if (check_for_when(in_hand->get_col().what_next(), {input.from}, {}, {}, { in_hand_in_curr_tmp })) {
                    move_message.side_evs.push_back(SideEvent::CHECK);
                }
                if (in_hand->get_type() == EFigureType::King) {
                    if (check_for_when(in_hand->get_col(), {input.from}, curr)) {
                        delete in_hand_in_curr_tmp;
                        throw std::invalid_argument("Check in that tile");
                    }
                }
                else {
                    if (check_for_when(in_hand->get_col(), {input.from}, {}, { in_hand_in_curr_tmp }, {})) {
                        delete in_hand_in_curr_tmp;
                        throw std::invalid_argument("Under check");
                    }
                }
                move_message.main_ev = MainEvent::MOVE;
                delete in_hand_in_curr_tmp;
                return move_message;
            }
        }
    }
    throw std::invalid_argument("Unforeseen move");
}

/// <summary>
/// ����� ��� �� ������� �����
/// � ������������� ��� � ��������
/// �������
/// </summary>
/// <returns>������� �� �������� ���</returns>
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
                !has_castling(turn, ms.what_castling_breaks.back())
                ) {
                for (const Id& id : ms.what_castling_breaks) {
                    on_castling(turn, id);
                }
            }
            break;
        case SideEvent::PROMOTION:
            promotion_fig(in_hand, EFigureType::Pawn);
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
/// ���������� ���
/// </summary>
/// <param name="move_rec">���</param>
/// <returns>������� �� ��������� ���</returns>
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
        for (const Id& it : ms.to_eat) capture_figure(it);
        break;
    case MainEvent::CASTLING:
        if (has_castling(turn, ms.to_move.back().first)) {
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
                has_castling(turn, ms.what_castling_breaks.back())
                ) {
                for (const Id& id : ms.what_castling_breaks) {
                    off_castling(turn, id);
                }
            }
            break;
        case SideEvent::PROMOTION:
            promotion_fig(in_hand, choice);
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
/// ��������� ���������� ���
/// </summary>
/// <returns>������� �� ��������� ���</returns>
bool FigureBoard::restore_move() {
    if (move_logger.future_empty()) return false;
    auto future = move_logger.pop_future_move();
    provide_move(future);
    move_logger.add_without_reset(future);
    return true;
}

/// <summary>
/// ���������� ��������� ������ �� �����
/// </summary>
/// <exception cref="std::invalid_argument">
/// ������ � ���������� ��������������� �� ���� � ���������
/// </exception>
/// <param name="id">������������� ������</param>
void FigureBoard::uncapture_figure(const Id& id) {
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
    pos position = to_promote->get_pos();
    FigureFabric::instance()->create(position, color, new_type, id, to_promote);
}
