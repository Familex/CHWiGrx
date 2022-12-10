#include "../declarations.hpp"

LRESULT CALLBACK figures_list_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    SCROLLINFO si{.cbSize=sizeof(SCROLLINFO)};
    static PAINTSTRUCT ps;
    static HBITMAP hbmMem;
    static HGDIOBJ hOld;
    static HDC hdcMem;
    static HDC hdc;
    static std::map<Color, std::vector<Figure*>> figures_prototypes;
    static Color curr_color{ Color::White };
    static bool is_resizes = true;  // also to horizontal scrollbar
    static bool is_scrolls = false;

    switch (message) {
        case WM_CREATE:
        {
            for (Color col : PLAYABLE_COLORS) {
                for (FigureType type : PLAYABLE_FIGURES) {
                    figures_prototypes[col].push_back(
                        FigureFabric::instance()->create({}, col, type)
                    );
                }
            }

            RECT window_rect{};
            GetWindowRect(hWnd, &window_rect);
            figures_list.set_rect(window_rect);
            figures_list.clear_scrolling();
            is_resizes = true;
            is_scrolls = false;

            si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            si.nPos = static_cast<int>(figures_list.get_figures_in_row());
            si.nMax = static_cast<int>(figures_prototypes[curr_color].size());
            si.nMin = 1;
            si.nPage = 1;
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

            figures_list.recalculate_dimensions();

            si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
            si.nPos = 0;
            si.nMax = static_cast<int>(figures_list.get_max_scroll());
            si.nMin = 0;
            si.nPage = figures_list.get_height();
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }
            break;
        case WM_HSCROLL:
        {
            is_resizes = true;

            switch (LOWORD(wParam))
            {
                case SB_PAGELEFT: case SB_LINELEFT:
                    figures_list.dec_figures_in_row();
                    break;
                case SB_PAGERIGHT: case SB_LINERIGHT:
                    figures_list.inc_figures_in_row();
                    break;
            }
            si.fMask = SIF_POS;
            si.nPos = static_cast<int>(figures_list.get_figures_in_row());
            SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

            figures_list.recalculate_dimensions();

            si.fMask = SIF_RANGE | SIF_PAGE;
            si.nMin = 0;
            si.nMax = static_cast<int>(figures_list.get_max_scroll());
            si.nPage = figures_list.get_height();
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

            SetWindowPos(hWnd, NULL, NULL, NULL,
                figures_list.get_width_with_extra(), figures_list.get_height_with_extra(), SWP_NOMOVE);
        }
            break;
        case WM_VSCROLL:
        {
            is_scrolls = true;
            int delta = 0;
            switch (LOWORD(wParam))
            {
            case SB_PAGEUP:
                delta = figures_list.add_to_curr_scroll(-30);
                break;
            case SB_LINEUP:
                delta = figures_list.add_to_curr_scroll(-10);
                break;
            case SB_PAGEDOWN:
                delta = figures_list.add_to_curr_scroll(30);
                break;
            case SB_LINEDOWN:
                delta = figures_list.add_to_curr_scroll(10);
                break;
            case SB_THUMBPOSITION:
                delta = figures_list.set_curr_scroll(HIWORD(wParam));
                break;
            }

            ScrollWindowEx(hWnd, 0, -delta, (CONST RECT*) NULL,
                (CONST RECT*) NULL, (HRGN)NULL, (PRECT)NULL,
                SW_INVALIDATE);
            UpdateWindow(hWnd);
            
            si.fMask = SIF_POS;
            si.nPos = figures_list.get_curr_scroll();
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }
            break;
        case WM_SIZE:
        {
            is_resizes = true;
            figures_list.set_size(lParam);
            si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
            si.nMax = static_cast<int>(figures_list.get_all_figures_height());
            si.nPage = figures_list.get_height();
            si.nPos = figures_list.get_curr_scroll();
            SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
        }
            break;
        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO lpMMI = reinterpret_cast<LPMINMAXINFO>(lParam);
            lpMMI->ptMinTrackSize.y = static_cast<LONG>(figures_list.get_cell_height() + SCROLLBAR_THICKNESS);
            lpMMI->ptMaxTrackSize.y = static_cast<int>(figures_list.get_all_figures_height()) + HEADER_HEIGHT - SCROLLBAR_THICKNESS;
        }
        break;
        case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            PRECT prect = &ps.rcPaint;

            int height = figures_list.get_height();
            int width = figures_list.get_width();
            int curr_scroll = figures_list.get_curr_scroll();
            size_t all_figures_height = figures_list.get_all_figures_height();

            hdcMem = CreateCompatibleDC(hdc);
            hbmMem = CreateCompatibleBitmap(hdc, static_cast<int>(width), static_cast<int>(all_figures_height));
            hOld = SelectObject(hdcMem, hbmMem);

            /* фон */
            RECT full_rect = { .left = prect->left, .top = prect->top + curr_scroll,
                .right = prect->right, .bottom = prect->bottom + curr_scroll };
            FillRect(hdcMem, &full_rect, CHECKERBOARD_DARK);

            /* Отрисовка фигур */
            for (int index = 0; index < figures_prototypes[curr_color].size(); ++index) {
                int x = static_cast<int>(index / figures_list.get_figures_in_row());
                int y = static_cast<int>(index % figures_list.get_figures_in_row());
                Figure* fig_to_draw = figures_prototypes[curr_color][index];
                draw_figure(hdcMem, fig_to_draw->get_col(), fig_to_draw->get_type(), { x, y },
                    true, static_cast<int>(figures_list.get_cell_height()), static_cast<int>(figures_list.get_cell_width()));
            }

            /* Копирование временного буфера в основной (взято из майкросовтовской документации) */
            if (is_scrolls) {
                BitBlt(ps.hdc,
                    prect->left, prect->top,
                    width,
                    height,
                    hdcMem,
                    prect->left,
                    prect->top + curr_scroll,
                    SRCCOPY);
                is_scrolls = false;
            }
            else if (is_resizes) {
                BitBlt(ps.hdc,
                    0, 0,
                    width, static_cast<int>(all_figures_height),
                    hdcMem,
                    0, curr_scroll,
                    SRCCOPY);
                is_resizes = false;
            }
            else {
                // При разворачивании окна заходит сюда
                BitBlt(ps.hdc,
                    0, 0,
                    width, static_cast<int>(all_figures_height),
                    hdcMem,
                    0, curr_scroll,
                    SRCCOPY);
            }

            SelectObject(hdcMem, hOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            EndPaint(hWnd, &ps);
        }
            break;
        case WM_LBUTTONDOWN:
        {
            motion_input.set_lbutton_down();
            Pos figure_to_drag = Pos(
                (LOWORD(lParam)) / static_cast<int>(figures_list.get_cell_width()),
                (HIWORD(lParam) + figures_list.get_curr_scroll()) / static_cast<int>(figures_list.get_cell_width())
            ).change_axes();
            size_t index = figure_to_drag.x * figures_list.get_figures_in_row() + figure_to_drag.y;
            if (index >= figures_prototypes[curr_color].size()) break;  // there was a click in a non-standard part of the window => ignore
            motion_input.set_in_hand(FigureFabric::instance()->create(figures_prototypes[curr_color][index], false));
        }
            break;
        case WM_RBUTTONUP:
        {
            curr_color = what_next(curr_color);
        }
            InvalidateRect(hWnd, NULL, NULL);
            break;
        case WM_MOUSEMOVE:
        {
            if (motion_input.is_drags()) {
                motion_input.init_curr_choice_window(hWnd, curr_choice_figures_list_wndproc);
            }
        }
            break;
        case WM_DESTROY:
        {
            for (Color col : PLAYABLE_COLORS) {
                for (Figure* figure : figures_prototypes[col]) {
                    delete figure;
                }
            }
            figures_prototypes.clear();
            figures_list_window = NULL;
            HWND owner = GetWindow(hWnd, GW_OWNER); // это должен быть GetParent, но оный возвращает NULL
            set_menu_checkbox(owner, IDM_TOGGLE_LIST_WINDOW, false);
        }
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return static_cast<LRESULT>(0);
}
