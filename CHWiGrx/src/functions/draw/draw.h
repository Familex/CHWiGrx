#pragma once

#include "../../winapi/framework.hpp"
#include "structs/figure.hpp"
#include "structs/input.hpp"
#include "structs/pos.hpp"

void draw_figure(HDC, const Figure*, const Pos&, bool = true) noexcept;

void draw_figure(HDC, const Figure*, const Pos&, bool, int, int) noexcept;

void draw_board(HDC) noexcept;

void draw_figures_on_board(HDC) noexcept;

void draw_input(HDC, const Input&) noexcept;

void rectangle(HDC, RECT) noexcept;
