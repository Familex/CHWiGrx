#pragma once

#include "../../winapi/framework.hpp"
#include "structs/figure.hpp"

#include <string>

INT_PTR CALLBACK about_proc(HWND, UINT, WPARAM, LPARAM) noexcept;

bool cpy_str_to_clip(HWND, std::string_view) noexcept;

std::string take_str_from_clip(HWND) noexcept;

HWND create_curr_choice_window(HWND, Figure*, POINT, int, int, const WNDPROC) noexcept;

void change_checkerboard_color_theme(HWND) noexcept;

HWND create_figures_list_window(HWND) noexcept;
