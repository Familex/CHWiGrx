#pragma once

#include "move_rec.h"

class MoveLogger {
public:
    MoveRec get_last_move();
    void add(const MoveRec& move_rec);
    void add_without_reset(const MoveRec& move_rec) { prev_moves.push_back(move_rec); }
    void reset();
    MoveRec pop_future_move();
    MoveRec move_last_to_future();
    bool prev_empty() const { return prev_moves.empty(); }
    bool future_empty() const { return future_moves.empty(); }
    std::vector<MoveRec> get_past() { return prev_moves; }
    std::vector<MoveRec> get_future() { return future_moves; }
    void   set_past(const std::vector<MoveRec>& past) { prev_moves = past; }
    void set_future(const std::vector<MoveRec>& future) { future_moves = future; }
    bool is_fifty_move_rule_was_triggered();
    bool is_moves_repeat_rule_was_triggered();
private:
    std::vector<MoveRec> prev_moves;
    std::vector<MoveRec> future_moves;
};
