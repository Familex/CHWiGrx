#include "move_logger.h"

// ���������� ��������� ��������� ���
MoveRec MoveLogger::get_last_move() {
    if (prev_moves.empty())
        return {};
    return prev_moves.back();
}

// ��������� � ��� ���
void MoveLogger::add(const MoveRec& move_rec) {
    prev_moves.push_back(move_rec);
    future_moves.clear();
}

// �������� ���
void MoveLogger::reset() {
    prev_moves.clear();
    future_moves.clear();
}

// ������� �� ������� ����� ���������
MoveRec MoveLogger::pop_future_move() {
    if (future_moves.empty()) return {};
    MoveRec future = future_moves.back();
    future_moves.pop_back();
    return future;
}

// ��������� ��������� ��� � ��������� � ���������� ���
MoveRec MoveLogger::move_last_to_future() {
    if (prev_moves.empty()) return {};
    auto last = get_last_move();
    prev_moves.pop_back();
    future_moves.push_back(last);
    return last;
}

bool MoveLogger::is_fifty_move_rule_was_triggered() {
    size_t without_eat_and_pawnmoves = 0;
    for (auto move{ prev_moves.rbegin() }; move != prev_moves.rend(); ++move) {
        if (move->get_who_went()->get_type() == FigureType::Pawn || move->ms.main_ev == MainEvent::EAT) {
            break;
        }
        else {
            ++without_eat_and_pawnmoves;
        }
    }
    return without_eat_and_pawnmoves >= 50;
}

bool MoveLogger::is_moves_repeat_rule_was_triggered() {
    return false;
}
