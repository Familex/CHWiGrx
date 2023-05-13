#pragma once

#include "../../winapi/framework.hpp"

namespace mainproc
{

LRESULT CALLBACK main_game_state_wndproc(HWND, UINT, WPARAM, LPARAM) noexcept;

LRESULT CALLBACK main_edit_state_wndproc(HWND, UINT, WPARAM, LPARAM) noexcept;

}    // namespace mainproc

LRESULT CALLBACK main_default_wndproc(HWND, UINT, WPARAM, LPARAM) noexcept;

LRESULT CALLBACK figures_list_wndproc(HWND, UINT, WPARAM, LPARAM) noexcept;

LRESULT CALLBACK curr_choice_figures_list_wndproc(HWND, UINT, WPARAM, LPARAM) noexcept;

LRESULT CALLBACK curr_choice_game_mode_wndproc(HWND, UINT, WPARAM, LPARAM) noexcept;

LRESULT CALLBACK curr_choice_edit_mode_wndproc(HWND, UINT, WPARAM, LPARAM) noexcept;

LRESULT CALLBACK moves_list_wndproc(HWND, UINT, WPARAM, LPARAM) noexcept;
