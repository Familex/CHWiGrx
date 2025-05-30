#include "../../declarations.hpp"
#include "wndproc.h"

#include <map>

LRESULT CALLBACK
figures_list_wndproc(const HWND h_wnd, const UINT message, const WPARAM w_param, const LPARAM l_param) noexcept
{
    SCROLLINFO si{ .cbSize = sizeof(SCROLLINFO) };
    static PAINTSTRUCT ps;
    static std::map<Color, std::vector<std::unique_ptr<Figure>>> figures_prototypes;
    static auto curr_color{ Color::White };
    static bool is_resizes = true;    // also to horizontal scrollbar
    static bool is_scrolls = false;

    switch (message) {
        case WM_CREATE:
        {
            for (Color col : PLAYABLE_COLORS) {
                for (const FigureType type : PLAYABLE_FIGURES) {
                    figures_prototypes[col].push_back(figfab::FigureFabric::instance().create(Pos{}, col, type));
                }
            }

            RECT window_rect{};
            GetWindowRect(h_wnd, &window_rect);
            mutables::figures_list.set_rect(window_rect);
            mutables::figures_list.clear_scrolling();
            is_resizes = true;
            is_scrolls = false;

            si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            si.nPos = static_cast<int>(mutables::figures_list.get_figures_in_row());
            si.nMax = static_cast<int>(figures_prototypes[curr_color].size());
            si.nMin = 1;
            si.nPage = 1;
            SetScrollInfo(h_wnd, SB_HORZ, &si, TRUE);

            si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            si.nPos = 0;
            si.nMax = static_cast<int>(mutables::figures_list.get_max_scroll());
            si.nMin = 0;
            si.nPage = mutables::figures_list.get_height();
            SetScrollInfo(h_wnd, SB_VERT, &si, TRUE);

            break;
        }

        case WM_HSCROLL:
        {
            is_resizes = true;

            switch (LOWORD(w_param)) {
                case SB_PAGELEFT:
                case SB_LINELEFT:
                    mutables::figures_list.dec_figures_in_row();
                    break;
                case SB_PAGERIGHT:
                case SB_LINERIGHT:
                    mutables::figures_list.inc_figures_in_row();
                    break;
            }
            si.fMask = SIF_POS;
            si.nPos = static_cast<int>(mutables::figures_list.get_figures_in_row());
            SetScrollInfo(h_wnd, SB_HORZ, &si, TRUE);

            si.fMask = SIF_RANGE | SIF_PAGE;
            si.nMin = 0;
            si.nMax = static_cast<int>(mutables::figures_list.get_max_scroll());
            si.nPage = mutables::figures_list.get_height();
            SetScrollInfo(h_wnd, SB_VERT, &si, TRUE);

            SetWindowPos(
                h_wnd,
                nullptr,
                NULL,
                NULL,
                mutables::figures_list.get_width_with_extra(),
                mutables::figures_list.get_height_with_extra(),
                SWP_NOMOVE
            );

            InvalidateRect(h_wnd, nullptr, FALSE);
            break;
        }

        case WM_VSCROLL:
        {
            is_scrolls = true;
            int delta = 0;
            switch (LOWORD(w_param)) {
                case SB_PAGEUP:
                    delta = mutables::figures_list.add_to_curr_scroll(-30);
                    break;
                case SB_LINEUP:
                    delta = mutables::figures_list.add_to_curr_scroll(-10);
                    break;
                case SB_PAGEDOWN:
                    delta = mutables::figures_list.add_to_curr_scroll(30);
                    break;
                case SB_LINEDOWN:
                    delta = mutables::figures_list.add_to_curr_scroll(10);
                    break;
                case SB_THUMBPOSITION:
                case SB_THUMBTRACK:
                    delta = mutables::figures_list.set_curr_scroll(HIWORD(w_param));
                    break;
            }

            si.fMask = SIF_POS;
            si.nPos = mutables::figures_list.get_curr_scroll();
            SetScrollInfo(h_wnd, SB_VERT, &si, TRUE);

            InvalidateRect(h_wnd, nullptr, FALSE);
            break;
        }

        case WM_SIZE:
        {
            is_resizes = true;
            mutables::figures_list.set_size(l_param);
            si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
            si.nMax = static_cast<int>(mutables::figures_list.get_all_figures_height());
            si.nPage = mutables::figures_list.get_height();
            si.nPos = mutables::figures_list.get_curr_scroll();
            SetScrollInfo(h_wnd, SB_VERT, &si, TRUE);

            InvalidateRect(h_wnd, nullptr, FALSE);
            break;
        }

        case WM_GETMINMAXINFO:
        {
            const auto lp_mmi = reinterpret_cast<LPMINMAXINFO>(l_param);
            lp_mmi->ptMinTrackSize.y =
                static_cast<LONG>(mutables::figures_list.get_cell_height() + constants::SCROLLBAR_THICKNESS);
            lp_mmi->ptMaxTrackSize.y = static_cast<int>(mutables::figures_list.get_all_figures_height()) +
                                       constants::HEADER_HEIGHT - constants::SCROLLBAR_THICKNESS;

            break;
        }

        case WM_PAINT:
        {
            static HDC hdc;
            static HDC hdc_mem;
            static HBITMAP hbm_mem;
            hdc = BeginPaint(h_wnd, &ps);
            const PRECT prect = &ps.rcPaint;

            const int height = mutables::figures_list.get_height();
            const int width = mutables::figures_list.get_width();
            const int curr_scroll = mutables::figures_list.get_curr_scroll();
            const auto all_figures_height = mutables::figures_list.get_all_figures_height();

            hdc_mem = CreateCompatibleDC(hdc);
            hbm_mem = CreateCompatibleBitmap(hdc, width, static_cast<int>(all_figures_height));
            const auto h_old = SelectObject(hdc_mem, hbm_mem);

            // background
            // FIXME use wc.hbrBackground
            {
                const RECT full_rect = { .left = prect->left,
                                         .top = prect->top + curr_scroll,
                                         .right = prect->right,
                                         .bottom = prect->bottom + curr_scroll };
                FillRect(hdc_mem, &full_rect, constants::CHECKERBOARD_DARK);
            }

            /* Figure draw */
            for (std::size_t index = 0; index < figures_prototypes[curr_color].size(); ++index) {
                const int x = static_cast<int>(index / mutables::figures_list.get_figures_in_row());
                const int y = static_cast<int>(index % mutables::figures_list.get_figures_in_row());
                const auto& fig_to_draw = figures_prototypes[curr_color][index];
                draw_figure(
                    hdc_mem,
                    fig_to_draw.get(),
                    Pos{ x, y },
                    true,
                    static_cast<int>(mutables::figures_list.get_cell_height()),
                    static_cast<int>(mutables::figures_list.get_cell_width())
                );
            }

            // Copy the off-screen bitmap onto the screen.
            if (is_scrolls) {
                BitBlt(
                    ps.hdc,
                    prect->left,
                    prect->top,
                    width,
                    height,
                    hdc_mem,
                    prect->left,
                    prect->top + curr_scroll,
                    SRCCOPY
                );
                is_scrolls = false;
            }
            else if (is_resizes) {
                BitBlt(ps.hdc, 0, 0, width, static_cast<int>(all_figures_height), hdc_mem, 0, curr_scroll, SRCCOPY);
                is_resizes = false;
            }
            else {
                // When the window is expanded, it comes here
                BitBlt(ps.hdc, 0, 0, width, static_cast<int>(all_figures_height), hdc_mem, 0, curr_scroll, SRCCOPY);
            }

            SelectObject(hdc_mem, h_old);
            DeleteObject(hbm_mem);
            DeleteDC(hdc_mem);
            EndPaint(h_wnd, &ps);

            break;
        }

        case WM_LBUTTONDOWN:
        {
            mutables::motion_input.set_lbutton_down();
            const Pos figure_to_drag = Pos((LOWORD(l_param)) / mutables::figures_list.get_cell_width(),
                                           (HIWORD(l_param) + mutables::figures_list.get_curr_scroll()) /
                                               mutables::figures_list.get_cell_width())
                                           .change_axes();
            const auto index =
                static_cast<std::size_t>(figure_to_drag.x) * mutables::figures_list.get_figures_in_row() +
                static_cast<std::size_t>(figure_to_drag.y);
            if (index >= figures_prototypes[curr_color].size())
                break;    // there was a click in a non-standard part of the window => ignore
            mutables::motion_input.set_in_hand(
                figfab::FigureFabric::instance().create(figures_prototypes[curr_color][index].get(), false).release()
            );

            break;
        }

        case WM_RBUTTONUP:
        {
            curr_color = what_next(curr_color);
            InvalidateRect(h_wnd, nullptr, NULL);
            break;
        }

        case WM_MOUSEMOVE:
        {
            if (mutables::motion_input.is_drags()) {
                mutables::motion_input.init_curr_choice_window(
                    h_wnd,
                    curr_choice_figures_list_wndproc,
                    Pos{ mutables::main_window.get_cell_width(), mutables::main_window.get_cell_height() }
                );
            }

            break;
        }

        case WM_DESTROY:
        {
            figures_prototypes.clear();
            mutables::figures_list_window = nullptr;
            const HWND owner = GetWindow(h_wnd, GW_OWNER);    // It should be GetParent, but it returns NULL.
            set_menu_checkbox(owner, IDM_TOGGLE_LIST_WINDOW, false);

            break;
        }

        default:
            return DefWindowProc(h_wnd, message, w_param, l_param);
    }

    return static_cast<LRESULT>(0);
}
