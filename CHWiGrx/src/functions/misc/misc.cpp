#include "misc.h"

#include "../../declarations.hpp"

INT_PTR CALLBACK about_proc(const HWND h_dlg, const UINT message, const WPARAM w_param, const LPARAM l_param) noexcept
{
    UNREFERENCED_PARAMETER(l_param);
    switch (message) {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            if (LOWORD(w_param) == IDOK || LOWORD(w_param) == IDCANCEL) {
                EndDialog(h_dlg, LOWORD(w_param));
                return TRUE;
            }
            break;
    }
    return FALSE;
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

void change_checkerboard_color_theme(const HWND h_wnd) noexcept
{
    std::swap(checkerboard_one, checkerboard_two);
    InvalidateRect(h_wnd, nullptr, NULL);
}

namespace new_window
{

HWND figures_list(const HWND parent) noexcept
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
                              .set_width(::figures_list.get_width_with_extra())
                              .set_height(::figures_list.get_height_with_extra())
                              .set_parent(parent)
                              .build(h_inst));
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
HWND curr_choice(
    const HWND parent,
    const Figure* const in_hand,
    const POINT mouse,
    const int w,
    const int h,
    const WNDPROC callback
) noexcept
{
    return *create_window(CreateWindowArgsBuilder{}
                              .set_ex_style(WS_EX_LAYERED | WS_EX_NOACTIVATE)
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
                              .set_after_create([](const HWND h_wnd) {
                                  SetWindowPos(h_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                                  SetLayeredWindowAttributes(h_wnd, TRANSPARENCY_PLACEHOLDER, 0xFF, LWA_COLORKEY);
                              })
                              .set_wnd_extra_data(reinterpret_cast<LONG_PTR>(
                                  figfab::FigureFabric::instance().create(in_hand, true).release()
                              ))
                              .build(h_inst));
}

HWND moves_log(const HWND parent) noexcept
{
    return *create_window(
        CreateWindowArgsBuilder{}
            .register_class(false)
            .set_wc_wndproc(moves_list_wndproc)
            .set_ex_style(WS_EX_CLIENTEDGE)
            .set_class_name(WC_LISTVIEW)
            .set_title(TEXT("Moves log"))
            .set_style(WS_TABSTOP | WS_CHILD | WS_BORDER | WS_VISIBLE | LVS_AUTOARRANGE | LVS_REPORT | LVS_OWNERDATA)
            .set_parent(parent)
            .build(h_inst)
    );
}

}    // namespace new_window

bool game_end_check(const HWND h_wnd, const Color turn) noexcept
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

// https://stackoverflow.com/a/57241985
// does not work for wcout...
DWORD create_console() noexcept
{
    if (!AllocConsole()) {
        return GetLastError();
    }

#ifdef _DEBUG
    // std::cout, std::clog, std::cerr, std::cin
    FILE* f_dummy;
    freopen_s(&f_dummy, "CONOUT$", "w", stdout);
    freopen_s(&f_dummy, "CONOUT$", "w", stderr);
    freopen_s(&f_dummy, "CONIN$", "r", stdin);
    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();

    // std::wcout, std::wclog, std::wcerr, std::wcin
    const HANDLE con_out = CreateFile(
        TEXT("CONOUT$"),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (con_out == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
    const HANDLE con_in = CreateFile(
        TEXT("CONIN$"),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (con_in == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }
    if (!SetStdHandle(STD_OUTPUT_HANDLE, con_out)) {
        return GetLastError();
    }
    if (!SetStdHandle(STD_ERROR_HANDLE, con_out)) {
        return GetLastError();
    }
    if (!SetStdHandle(STD_INPUT_HANDLE, con_in)) {
        return GetLastError();
    }
    std::wcout.clear();
    std::wclog.clear();
    std::wcerr.clear();
    std::wcin.clear();

#endif

    return ERROR_SUCCESS;
}

std::size_t get_icon(const mvmsg::MoveMessage& mm) noexcept
{
    const auto& by_color = pieces_bitmaps.find(col_to_char(mm.first.get_col()));
    const auto& by_type = by_color->second.find(figure_type_to_char(mm.first.get_type()));
    return std::distance(std::begin(pieces_bitmaps), by_color) * PLAYABLE_FIGURES.size() +
           std::distance(std::begin(by_color->second), by_type);
}

HIMAGELIST init_move_log_bitmaps() noexcept
{
    auto list = ImageList_Create(64, 64, ILC_COLORDDB | ILC_MASK, 1, 0);
    for (const auto& [key_col, val_outher] : pieces_bitmaps) {
        for (const auto& [key_type, val_bitmap] : val_outher) {
            /* does not work
            BITMAP mask{};
            GetObject(val_bitmap, sizeof(mask), &mask);
            unsigned char* pixels = new unsigned char[mask.bmWidth * 4 * mask.bmHeight];
            const auto h_mask = CreateBitmapIndirect(&mask);
            BITMAPINFOHEADER header{ .biSize = sizeof(BITMAPINFOHEADER),
                                     .biWidth = mask.bmWidth,
                                     .biHeight = mask.bmHeight,
                                     .biPlanes = 1,
                                     .biBitCount = 32,
                                     .biCompression = BI_RGB,
                                     .biSizeImage =
                                         4 * static_cast<DWORD>(mask.bmWidth) * static_cast<DWORD>(mask.bmHeight),
                                     .biClrUsed = 0,
                                     .biClrImportant = 0 };
            GetDIBits(
                CreateCompatibleDC(nullptr),
                h_mask,
                0,
                mask.bmHeight,
                pixels,
                reinterpret_cast<BITMAPINFO*>(&header),
                DIB_RGB_COLORS
            );
            for (auto i = 0; i < mask.bmWidth; ++i) {
                for (auto j = 0; j < mask.bmHeight; ++j) {
                    auto& r = pixels[(mask.bmWidth * j + i) + 2];
                    auto& g = pixels[(mask.bmWidth * j + i) + 1];
                    auto& b = pixels[(mask.bmWidth * j + i) + 0];
                    const auto color = RGB(r, g, b);
                    if (color != TRANSPARENCY_PLACEHOLDER) {
                        r = 0;
                        g = 0;
                        b = 0;
                    }
                    else {
                        r = 0xFF;
                        g = 0xFF;
                        b = 0xFF;
                    }
                }
            }
            */
            ImageList_Add(list, val_bitmap, nullptr);
            /*
            delete[] pixels;
            DeleteObject(h_mask);
            */
        }
    }
    return list;
}

void update_moves_list() noexcept
{
    if (moves_list_list_view) {
        ListView_SetItemCount(moves_list_list_view, board.get_last_moves().size() + board.get_future_moves().size());
    }
}
