#pragma once

#include "move_message.hpp"

class MoveLogger {
    std::vector<mvmsg::MoveMessage> prev_moves_;
    std::vector<mvmsg::MoveMessage> future_moves_;
    
public:
    FN get_last_move() const noexcept -> std::optional<mvmsg::MoveMessage>
    {
        if (prev_moves_.empty()) {
            return std::nullopt;
        }
        return prev_moves_.back();
    }
    
    FN add(const mvmsg::MoveMessage& move_rec) noexcept -> void
    {
        prev_moves_.push_back(move_rec);
        future_moves_.clear();
    }
    
    FN add_without_reset(const mvmsg::MoveMessage& move_rec) noexcept -> void {
        prev_moves_.push_back(move_rec);
    }
    
    FN reset() noexcept -> void
    {
        prev_moves_.clear();
        future_moves_.clear();
    }
    
    FN pop_future_move() noexcept -> std::optional<mvmsg::MoveMessage>
    {
        if (future_moves_.empty()) {
            return std::nullopt;
        }
        mvmsg::MoveMessage future = future_moves_.back();
        future_moves_.pop_back();
        return future;
    }
    
    FN move_last_to_future() noexcept -> std::optional<mvmsg::MoveMessage>
    {
        if (prev_moves_.empty()) {
            return std::nullopt;
        }
        else if (auto last = get_last_move(); last.has_value()) {
            prev_moves_.pop_back();
            future_moves_.push_back(last.value());
            return last.value();
        }
        else {
            return std::nullopt;
        }
    }
    
    FN prev_empty() const noexcept -> bool {
        return prev_moves_.empty();
    }
    
    FN future_empty() const noexcept -> bool {
        return future_moves_.empty();
    }
    
    FN get_past() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
        return prev_moves_;
    }
    
    FN get_future() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
        return future_moves_;
    }
    
    FN set_past(const std::vector<mvmsg::MoveMessage>& past) noexcept -> void {
        prev_moves_ = past;
    }
    
    FN set_future(const std::vector<mvmsg::MoveMessage>& future) noexcept {
        future_moves_ = future;
    }
    
    FN is_fifty_move_rule_was_triggered() const noexcept -> bool
    {
        size_t without_eat_and_pawn_moves = 0;
        for (auto move{ prev_moves_.rbegin() }; move != prev_moves_.rend(); ++move) {
            if (move->first.get_type() == FigureType::Pawn
                || std::holds_alternative<mvmsg::Eat>(move->main_event))
            {
                break;
            }
            else {
                ++without_eat_and_pawn_moves;
            }
        }
        return without_eat_and_pawn_moves >= 50;
    }
    
    /// FIXME implement 
    FN is_moves_repeat_rule_was_triggered() const noexcept -> bool {
        return false;
    }
};
