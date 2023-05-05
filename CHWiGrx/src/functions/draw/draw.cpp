#include "draw.h"

#pragma comment(lib, "Msimg32.lib")
//      ^^^^^^^^^^^^^^^^^^^^^^^^^^^ for TransparentBlt

#include "../../variables/constants.hpp"
#include "../../variables/mutables.hpp"

void draw_figure(
    const HDC hdc,
    const Figure* figure,
    const Pos& begin_paint,
    const bool is_transparent,
    const int w,
    const int h
) noexcept
{
    const int w_beg = begin_paint.y * w;
    const int h_beg = begin_paint.x * h;
    int h_end = h_beg + h;
    int w_end = w_beg + h;
    const HBITMAP h_bitmap = pieces_bitmaps[col_to_char(figure->get_col())][figure_type_to_char(figure->get_type())];
    BITMAP bm{};
    GetObject(h_bitmap, sizeof(BITMAP), &bm);
    const HDC hdc_mem = CreateCompatibleDC(hdc);
    SelectObject(hdc_mem, h_bitmap);
    SetStretchBltMode(hdc, STRETCH_DELETESCANS);
    if (is_transparent) {
        TransparentBlt(hdc, w_beg, h_beg, w, h, hdc_mem, 0, 0, bm.bmWidth, bm.bmHeight, TRANSPARENCY_PLACEHOLDER);
    }
    else {
        StretchBlt(hdc, w_beg, h_beg, w, h, hdc_mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    }

    // if it's a castling rook, add a star to right top corner
    if (board.has_castling(figure->get_id())) {
        const HBITMAP h_star_bitmap = other_bitmaps["star"];
        GetObject(h_star_bitmap, sizeof(BITMAP), &bm);
        SelectObject(hdc_mem, h_star_bitmap);    // <- new hOldBitmap?
        SetStretchBltMode(hdc, STRETCH_DELETESCANS);
        TransparentBlt(
            hdc,
            w_beg + w * 2 / 3,
            h_beg,
            w / 3,
            h / 3,
            hdc_mem,
            0,
            0,
            bm.bmWidth,
            bm.bmHeight,
            TRANSPARENCY_PLACEHOLDER
        );
    }

    DeleteDC(hdc_mem);
}

void draw_figure(const HDC hdc, const Figure* figure, const Pos& begin_paint, const bool is_transparent) noexcept
{
    draw_figure(hdc, figure, begin_paint, is_transparent, main_window.get_cell_width(), main_window.get_cell_height());
}

/**
 * \brief Draw background board
 * \param hdc Draw context
 */
void draw_board(const HDC hdc) noexcept
{
    for (int i{}; i < HEIGHT; ++i) {
        for (int j{}; j < WIDTH; ++j) {
            const RECT cell = main_window.get_cell(i, j);
            if ((i + j) % 2) {
                FillRect(hdc, &cell, checkerboard_one);
            }
            else {
                FillRect(hdc, &cell, checkerboard_two);
            }
        }
    }
}

/**
 * \brief Draw figures on board (the one that is in hand is not drawn)
 * \param hdc Draw context
 */
void draw_figures_on_board(const HDC hdc) noexcept
{
    for (const auto& figure : board.get_all_figures()) {
        if (!motion_input.is_figure_dragged(figure->get_id())) {
            draw_figure(hdc, figure, figure->get_pos());
        }
    }
}

/**
 * \brief Draw cursor position and selected cell
 * \param hdc_mem Draw context
 * \param input User input to draw
 */
void draw_input(const HDC hdc_mem, const Input& input) noexcept
{
    /// FIXME hardcoded colors
    static const HBRUSH RED{ CreateSolidBrush(RGB(255, 0, 0)) };
    static const HBRUSH BLUE{ CreateSolidBrush(RGB(0, 0, 255)) };
    const RECT from_cell = main_window.get_cell(change_axes(input.from));
    const RECT target_cell = main_window.get_cell(change_axes(input.target));
    FillRect(hdc_mem, &from_cell, RED);
    FillRect(hdc_mem, &target_cell, BLUE);
}

void rectangle(const HDC hdc, const RECT rect) noexcept
{
    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
}
