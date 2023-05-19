#pragma once

#include "../../winapi/framework.hpp"
#include "chess_game.hpp"

#include <string>

namespace misc
{
namespace bitmap
{
class Wrapper final
{
    HBITMAP value_;

public:
    explicit Wrapper(HBITMAP&& value) noexcept;

    explicit Wrapper(Wrapper&&) noexcept = default;
    Wrapper& operator=(Wrapper&&) noexcept = default;

    ~Wrapper() noexcept;

    operator HBITMAP&() noexcept;
    operator HBITMAP const&() const noexcept;
};

[[nodiscard]] Wrapper create_mask(HBITMAP, COLORREF) noexcept;

[[nodiscard]] Wrapper resize(HBITMAP, std::size_t, std::size_t, std::size_t, std::size_t) noexcept;

}    // namespace bitmap

namespace new_window
{
HWND curr_choice(HWND, const Figure*, POINT, int, int, WNDPROC) noexcept;

HWND figures_list(HWND) noexcept;

HWND moves_log(HWND) noexcept;
}    // namespace new_window

INT_PTR CALLBACK about_proc(HWND, UINT, WPARAM, LPARAM) noexcept;

bool cpy_str_to_clip(HWND, std::string_view) noexcept;

std::string take_str_from_clip(HWND) noexcept;

void change_checkerboard_color_theme(HWND) noexcept;

bool game_end_check(HWND, Color) noexcept;

DWORD create_console() noexcept;

[[nodiscard]] std::size_t get_icon(const mvmsg::MoveMessage&) noexcept;

HIMAGELIST init_move_log_bitmaps() noexcept;

void update_moves_list(HWND, const ChessGame&) noexcept;

void on_game_board_change(const ChessGame&) noexcept;

[[nodiscard]] std::wstring to_wstring(FigureType) noexcept;

[[nodiscard]] HCURSOR load_animated_cursor(UINT, LPCTSTR) noexcept;

}    // namespace misc
