#pragma once

#include "move_message.hpp"

class MoveLogger {
    std::vector<mvmsg::MoveMessage> prev_moves;
    std::vector<mvmsg::MoveMessage> future_moves;
    
public:
    FN get_last_move() const noexcept -> std::optional<mvmsg::MoveMessage>
    {
        if (prev_moves.empty()) {
            return std::nullopt;
        }
        return prev_moves.back();
    }
    
    FN add(const mvmsg::MoveMessage& move_rec) noexcept -> void
    {
        prev_moves.push_back(move_rec);
        future_moves.clear();
    }
    
    FN add_without_reset(const mvmsg::MoveMessage& move_rec) noexcept -> void {
        prev_moves.push_back(move_rec);
    }
    
    FN reset() noexcept -> void
    {
        prev_moves.clear();
        future_moves.clear();
    }
    
    FN pop_future_move() noexcept -> std::optional<mvmsg::MoveMessage>
    {
        if (future_moves.empty()) {
            return std::nullopt;
        }
        mvmsg::MoveMessage future = future_moves.back();
        future_moves.pop_back();
        return future;
    }
    
    FN move_last_to_future() noexcept -> std::optional<mvmsg::MoveMessage>
    {
        if (prev_moves.empty()) {
            return std::nullopt;
        }
        else if (auto last = get_last_move(); last.has_value()) {
            prev_moves.pop_back();
            future_moves.push_back(last.value());
            return last.value();
        }
        else {
            return std::nullopt;
        }
    }
    
    FN prev_empty() const noexcept -> bool {
        return prev_moves.empty();
    }
    
    FN future_empty() const noexcept -> bool {
        return future_moves.empty();
    }
    
    FN get_past() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
        return prev_moves;
    }
    
    FN get_future() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
        return future_moves;
    }
    
    FN set_past(const std::vector<mvmsg::MoveMessage>& past) noexcept -> void {
        prev_moves = past;
    }
    
    FN set_future(const std::vector<mvmsg::MoveMessage>& future) noexcept {
        future_moves = future;
    }
    
    FN is_fifty_move_rule_was_triggered() const noexcept -> bool
    {
        size_t without_eat_and_pawnmoves = 0;
        for (auto move{ prev_moves.rbegin() }; move != prev_moves.rend(); ++move) {
            if (move->first.get_type() == FigureType::Pawn
                || std::holds_alternative<mvmsg::Eat>(move->main_event))
            {
                break;
            }
            else {
                ++without_eat_and_pawnmoves;
            }
        }
        return without_eat_and_pawnmoves >= 50;
    }
    
    /// @todo
    FN is_moves_repeat_rule_was_triggered() const noexcept -> bool {
        return false;
    }
};
