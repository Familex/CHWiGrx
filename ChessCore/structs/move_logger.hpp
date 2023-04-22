#pragma once

#include "move_message.hpp"

class MoveLogger {
    std::vector<mvmsg::MoveMessage> prev_;
    std::vector<mvmsg::MoveMessage> future_;
    
public:
    FN get_last_move() const noexcept -> std::optional<mvmsg::MoveMessage>
    {
        if (prev_.empty()) {
            return std::nullopt;
        }
        return prev_.back();
    }
    
    FN add(const mvmsg::MoveMessage& move_rec) noexcept -> void
    {
        prev_.push_back(move_rec);
        future_.clear();
    }
    
    FN add_without_reset(const mvmsg::MoveMessage& move_rec) noexcept -> void {
        prev_.push_back(move_rec);
    }
    
    FN reset() noexcept -> void
    {
        prev_.clear();
        future_.clear();
    }
    
    FN pop_future_move() noexcept -> std::optional<mvmsg::MoveMessage>
    {
        if (future_.empty()) {
            return std::nullopt;
        }
        mvmsg::MoveMessage future = future_.back();
        future_.pop_back();
        return future;
    }
    
    FN move_last_to_future() noexcept -> std::optional<mvmsg::MoveMessage>
    {
        if (prev_.empty()) {
            return std::nullopt;
        }
        else if (auto last = get_last_move(); last.has_value()) {
            prev_.pop_back();
            future_.push_back(last.value());
            return last.value();
        }
        else {
            return std::nullopt;
        }
    }
    
    FN prev_empty() const noexcept -> bool {
        return prev_.empty();
    }
    
    FN future_empty() const noexcept -> bool {
        return future_.empty();
    }
    
    FN get_past() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
        return prev_;
    }
    
    FN get_future() const noexcept -> const std::vector<mvmsg::MoveMessage>& {
        return future_;
    }
    
    FN set_past(const std::vector<mvmsg::MoveMessage>& past) noexcept -> void {
        prev_ = past;
    }
    
    FN set_future(const std::vector<mvmsg::MoveMessage>& future) noexcept {
        future_ = future;
    }
    
    FN is_fifty_move_rule_was_triggered() const noexcept -> bool
    {
        size_t without_eat_and_pawn_moves = 0;
        for (auto move{ prev_.rbegin() }; move != prev_.rend(); ++move) {
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
