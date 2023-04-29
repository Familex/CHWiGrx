#pragma once

#include "../../winapi/framework.hpp"
#include "structs/board_repr.hpp"
#include "structs/pos.hpp"

void update_main_window_title(HWND) noexcept;

void on_lbutton_up(HWND, WPARAM, LPARAM, Pos, bool = true) noexcept;

void on_lbutton_down(HWND, LPARAM) noexcept;

void restart() noexcept;

bool copy_repr_to_clip(HWND) noexcept;

auto take_repr_from_clip(HWND) noexcept -> ParseEither<board_repr::BoardRepr, ParseErrorType>;

void make_move(HWND) noexcept;

bool is_bot_move() noexcept;
