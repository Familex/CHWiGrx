#include "misc.h"

#include "../../declarations.hpp"

INT_PTR CALLBACK about_proc(const HWND h_dlg, const UINT message, const WPARAM w_param, const LPARAM l_param) noexcept
{
    UNREFERENCED_PARAMETER(l_param);
    switch (message) {
        case WM_INITDIALOG:
            return static_cast<INT_PTR>(TRUE);

        case WM_COMMAND:
            if (LOWORD(w_param) == IDOK || LOWORD(w_param) == IDCANCEL) {
                EndDialog(h_dlg, LOWORD(w_param));
                return static_cast<INT_PTR>(TRUE);
            }
            break;
    }
    return static_cast<INT_PTR>(FALSE);
}

bool cpy_str_to_clip(const HWND h_wnd, const std::string_view s) noexcept
{
    if (!OpenClipboard(h_wnd)) {
        debug_print("Failed to open clipboard");
        return false;
    }

    if (!EmptyClipboard()) {
        debug_print("Failed to empty clipboard");
        CloseClipboard();
        return false;
    }

    const HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
    if (!hg) {
        debug_print("Failed to allocate memory");
        CloseClipboard();
        return false;
    }

    const LPVOID p = GlobalLock(hg);
    if (!p) {
        debug_print("Failed to lock memory");
        GlobalFree(hg);
        CloseClipboard();
        return false;
    }

    memcpy(p, s.data(), s.size() + 1);
    GlobalUnlock(hg);

    if (!SetClipboardData(CF_TEXT, hg)) {
        debug_print("Failed to set clipboard data");
        GlobalFree(hg);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

std::string take_str_from_clip(const HWND h_wnd) noexcept
{
    if (!IsClipboardFormatAvailable(CF_TEXT)) {
        debug_print("CF_TEXT format not available");
        return "";
    }

    if (!OpenClipboard(h_wnd)) {
        debug_print("Failed to open clipboard");
        return "";
    }

    const HANDLE h_data = GetClipboardData(CF_TEXT);
    if (!h_data) {
        debug_print("Failed to get clipboard data");
        CloseClipboard();
        return "";
    }

    const auto psz_text = static_cast<const char*>(GlobalLock(h_data));
    if (!psz_text) {
        debug_print("Failed to lock memory");
        CloseClipboard();
        return "";
    }

    std::string text(psz_text);
    GlobalUnlock(h_data);

    if (!CloseClipboard()) {
        debug_print("Failed to close clipboard");
        return "";
    }

    return text;
}

/**
 * \brief Create a curr choice window
 * \param parent Parent window
 * \param in_hand Figure, what will be drawn in curr choice window
 * \param mouse Mouse initial position
 * \param w Curr choice window width
 * \param h Curr choice window height
 * \param callback Curr choice window wndproc
 * \return Curr choice window handle
 */
HWND create_curr_choice_window(HWND parent, Figure* in_hand, POINT mouse, int w, int h, const WNDPROC callback) noexcept
{
    return *create_window(
        CreateWindowArgsBuilder{}
            .set_wc_wndproc(callback)
            .set_wc_icon(LoadIcon(nullptr, MAKEINTRESOURCE(IDI_GAME_MODE_BIG)))
            .set_wc_icon_sm(LoadIcon(nullptr, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL)))
            .set_wc_wnd_extra(static_cast<int>(sizeof(in_hand)))
            .set_x(mouse.x - w / 2)
            .set_y(mouse.y - h / 2)
            .set_width(w)
            .set_height(h)
            .set_parent(parent)
            .set_style(WS_POPUP | WS_EX_TRANSPARENT | WS_EX_LAYERED)
            .set_class_name(CURR_CHOICE_WINDOW_CLASS_NAME)
            .set_title(L"")
            .set_after_create([](HWND h_wnd) {
                SetWindowPos(h_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                SetWindowLongPtr(h_wnd, GWL_EXSTYLE, GetWindowLongPtr(h_wnd, GWL_EXSTYLE) | WS_EX_LAYERED);
                SetLayeredWindowAttributes(h_wnd, TRANSPARENCY_PLACEHOLDER, 0xFF, LWA_COLORKEY);
            })
            .set_wnd_extra_data(
                reinterpret_cast<LONG_PTR>(figfab::FigureFabric::instance().create(in_hand, true).release())
            )
            .build(h_inst)
    );
}

void change_checkerboard_color_theme(const HWND h_wnd) noexcept
{
    if (window_state == WindowState::Edit) {
        std::swap(checkerboard_one, checkerboard_two);
    }
    else if (window_state == WindowState::Game) {
        std::swap(checkerboard_one, checkerboard_two);
    }
    else {
        assert(false);
    }
    InvalidateRect(h_wnd, nullptr, NULL);
}

HWND create_figures_list_window(HWND parent) noexcept
{
    return *create_window(CreateWindowArgsBuilder{}
                              .set_wc_wndproc(figures_list_wndproc)
                              .set_wc_icon(LoadIcon(h_inst, MAKEINTRESOURCE(IDI_FIGURES_LIST)))
                              .set_wc_icon_sm(LoadIcon(h_inst, MAKEINTRESOURCE(IDI_FIGURES_LIST)))
                              .set_class_name(FIGURES_LIST_WINDOW_CLASS_NAME)
                              .set_title(FIGURES_LIST_WINDOW_TITLE)
                              .set_style(WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL)
                              .set_x(FIGURES_LIST_WINDOW_DEFAULT_POS.x)
                              .set_y(FIGURES_LIST_WINDOW_DEFAULT_POS.y)
                              .set_width(figures_list.get_width_with_extra())
                              .set_height(figures_list.get_height_with_extra())
                              .set_parent(parent)
                              .build(h_inst));
}

bool game_end_check(HWND h_wnd, Color turn) noexcept
{
    if (const GameEndType curr_game_end_state = board.game_end_check(turn);
        curr_game_end_state != GameEndType::NotGameEnd)
    {
        std::wstring body;
        const std::wstring head = L"Game end";
        switch (curr_game_end_state) {
            case GameEndType::Checkmate:
            {
                const auto who_next = what_next(turn);
                body = who_next == Color::White   ? L"White wins!"
                       : who_next == Color::Black ? L"Black wins!"
                                                  : L"None wins!";
            } break;

            case GameEndType::Stalemate:
                body = turn == Color::White   ? L"Stalemate to white!"
                       : turn == Color::Black ? L"Stalemate to black!"
                                              : L"Stalemate?";
                break;

            case GameEndType::FiftyRule:
                body = L"Fifty rule";
                break;

            case GameEndType::InsufficientMaterial:
                body = L"Insufficient material";
                break;

            case GameEndType::MoveRepeat:
                body = L"Move repeat rule";
                break;

            default:
                assert(!"unexpected game end");
                break;
        }
        if (const auto result = MessageBox(h_wnd, (body + L"\nCopy board to clip?").c_str(), head.c_str(), MB_YESNO);
            result == IDYES)
        {
            copy_repr_to_clip(h_wnd);
        }
        restart();
        InvalidateRect(h_wnd, nullptr, NULL);

        update_main_window_title(h_wnd);
        return true;
    }

    update_main_window_title(h_wnd);

    return false;
}
