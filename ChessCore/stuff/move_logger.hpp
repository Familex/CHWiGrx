#pragma once

#include "move_rec.hpp"

class MoveLogger {
    std::vector<moverec::MoveRec> prev_moves;
    std::vector<moverec::MoveRec> future_moves;
    
public:
    [[nodiscard]] constexpr moverec::MoveRec get_last_move() const noexcept
    {
        if (prev_moves.empty())
            return moverec::MoveRec{};
        return prev_moves.back();
    }
    
    constexpr void add(const moverec::MoveRec& move_rec) noexcept
    {
        prev_moves.push_back(move_rec);
        future_moves.clear();
    }
    
    constexpr void add_without_reset(const moverec::MoveRec& move_rec) noexcept
        { prev_moves.push_back(move_rec); }
    
    void constexpr reset() noexcept
    {
        prev_moves.clear();
        future_moves.clear();
    }
    
    constexpr moverec::MoveRec pop_future_move() noexcept
    {
        if (future_moves.empty()) return moverec::MoveRec{};
        moverec::MoveRec future = future_moves.back();
        future_moves.pop_back();
        return future;
    }
    
    constexpr moverec::MoveRec move_last_to_future() noexcept
    {
        if (prev_moves.empty()) return moverec::MoveRec{};
        moverec::MoveRec last = get_last_move();
        prev_moves.pop_back();
        future_moves.push_back(last);
        return last;
    }
    
    [[nodiscard]] constexpr bool prev_empty() const noexcept
        { return prev_moves.empty(); }
    
    [[nodiscard]] constexpr bool future_empty() const noexcept
        { return future_moves.empty(); }
    
    [[nodiscard]] constexpr const std::vector<moverec::MoveRec>& get_past() const noexcept
        { return prev_moves; }
    
    [[nodiscard]] constexpr const std::vector<moverec::MoveRec>& get_future() const noexcept
        { return future_moves; }
    
    constexpr void set_past(const std::vector<moverec::MoveRec>& past) noexcept
        { prev_moves = past; }
    
    constexpr void set_future(const std::vector<moverec::MoveRec>& future) noexcept
        { future_moves = future; }
    
    [[nodiscard]] constexpr bool is_fifty_move_rule_was_triggered() const noexcept
    {
        size_t without_eat_and_pawnmoves = 0;
        for (auto move{ prev_moves.rbegin() }; move != prev_moves.rend(); ++move) {
            if (move->who_went.get_type() == FigureType::Pawn || move->ms.main_ev == MainEvent::EAT) {
                break;
            }
            else {
                ++without_eat_and_pawnmoves;
            }
        }
        return without_eat_and_pawnmoves >= 50;
    }
    
    /// @todo
    [[nodiscard]] constexpr bool is_moves_repeat_rule_was_triggered() const noexcept
        {  return false; }
};
