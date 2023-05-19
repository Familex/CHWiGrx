#pragma once

#include "../../winapi/framework.hpp"
#include "chess_game.hpp"

#include <string>

INT_PTR CALLBACK about_proc(HWND, UINT, WPARAM, LPARAM) noexcept;

bool cpy_str_to_clip(HWND, std::string_view) noexcept;

std::string take_str_from_clip(HWND) noexcept;

namespace new_window
{
HWND curr_choice(HWND, const Figure*, POINT, int, int, WNDPROC) noexcept;

HWND figures_list(HWND) noexcept;

HWND moves_log(HWND) noexcept;
}    // namespace new_window

void change_checkerboard_color_theme(HWND) noexcept;

bool game_end_check(HWND, Color) noexcept;

DWORD create_console() noexcept;

[[nodiscard]] std::size_t get_icon(const mvmsg::MoveMessage&) noexcept;

HIMAGELIST init_move_log_bitmaps() noexcept;

void update_moves_list(HWND, const ChessGame&) noexcept;

[[nodiscard]] HBITMAP generate_mask_from_bitmap(HBITMAP, COLORREF) noexcept;

namespace misc
{
[[nodiscard]] std::wstring to_wstring(FigureType) noexcept;
}    // namespace misc
