#pragma once

#include "../../winapi/framework.hpp"

void set_menu_checkbox(HWND, UINT, bool) noexcept;

void update_edit_menu_variables(HWND) noexcept;

void update_game_menu_variables(HWND) noexcept;

void update_bot_menu_variables(HWND) noexcept;
