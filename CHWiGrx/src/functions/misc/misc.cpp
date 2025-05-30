#include "misc.h"

#include "../../declarations.hpp"

misc::bitmap::Wrapper::Wrapper(HBITMAP&& value) noexcept
  : value_{ value }
{ }

misc::bitmap::Wrapper::~Wrapper() noexcept { DeleteObject(value_); }

misc::bitmap::Wrapper::operator HBITMAP&() noexcept { return value_; }

misc::bitmap::Wrapper::operator HBITMAP const&() const noexcept { return value_; }

// http://www.winprog.org/tutorial/transparency.html
misc::bitmap::Wrapper misc::bitmap::create_mask(const HBITMAP bitmap, const COLORREF transparent) noexcept
{
    // create monochrome (1 bit) mask bitmap
    BITMAP mask{};
    GetObject(bitmap, sizeof(BITMAP), &mask);
    auto h_mask = CreateBitmap(mask.bmWidth, mask.bmHeight, 1, 1, NULL);

    auto dc_bitmap = CreateCompatibleDC(nullptr);
    auto dc_mask = CreateCompatibleDC(nullptr);

    SelectObject(dc_bitmap, bitmap);
    SelectObject(dc_mask, h_mask);

    SetBkColor(dc_bitmap, transparent);
    BitBlt(dc_mask, 0, 0, mask.bmWidth, mask.bmHeight, dc_bitmap, 0, 0, SRCCOPY);
    // original say that with that mask will be more transparent, but it modifies original bitmap
    // BitBlt(dc_bitmap, 0, 0, mask.bmWidth, mask.bmHeight, dc_mask, 0, 0, SRCINVERT);

    // clean up
    DeleteDC(dc_bitmap);
    DeleteDC(dc_mask);

    return Wrapper{ std::move(h_mask) };
}

misc::bitmap::Wrapper misc::bitmap::resize(
    HBITMAP source,
    std::size_t source_width,
    std::size_t source_height,
    std::size_t width,
    std::size_t height
) noexcept
{
    auto source_dc = CreateCompatibleDC(nullptr);
    SelectObject(source_dc, source);

    auto result_dc = CreateCompatibleDC(nullptr);
    auto result = CreateCompatibleBitmap(source_dc, static_cast<int>(width), static_cast<int>(height));
    SelectObject(result_dc, result);

    SetStretchBltMode(result_dc, STRETCH_DELETESCANS);

    StretchBlt(
        result_dc,
        0,
        0,
        static_cast<int>(width),
        static_cast<int>(height),
        source_dc,
        0,
        0,
        static_cast<int>(source_width),
        static_cast<int>(source_height),
        SRCCOPY
    );

    // clean up
    DeleteDC(source_dc);
    DeleteDC(result_dc);

    auto deleter = [](HBITMAP m) {
        DeleteObject(m);
    };

    return Wrapper{ std::move(result) };
}

HWND misc::new_window::figures_list(const HWND parent) noexcept
{
    return *create_window(CreateWindowArgsBuilder{}
                              .set_wc_wndproc(figures_list_wndproc)
                              .set_wc_icon(LoadIcon(constants::h_inst, MAKEINTRESOURCE(IDI_FIGURES_LIST)))
                              .set_wc_icon_sm(LoadIcon(constants::h_inst, MAKEINTRESOURCE(IDI_FIGURES_LIST)))
                              .set_wc_background(constants::CHECKERBOARD_DARK)
                              .set_class_name(constants::FIGURES_LIST_WINDOW_CLASS_NAME)
                              .set_title(static_cast<UINT>(IDS_FIGURES_LIST_TITLE))
                              .set_style(WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL)
                              .set_x(constants::FIGURES_LIST_WINDOW_DEFAULT_POS.x)
                              .set_y(constants::FIGURES_LIST_WINDOW_DEFAULT_POS.y)
                              .set_width(mutables::figures_list.get_width_with_extra())
                              .set_height(mutables::figures_list.get_height_with_extra())
                              .set_parent(parent)
                              .unregister_class(false)
                              .build(constants::h_inst));
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
HWND misc::new_window::curr_choice(
    const HWND parent,
    const Figure* const in_hand,
    const POINT mouse,
    const int w,
    const int h,
    const WNDPROC callback
) noexcept
{
    return *create_window(
        CreateWindowArgsBuilder{}
            .set_ex_style(WS_EX_LAYERED | WS_EX_NOACTIVATE)
            .set_wc_wndproc(callback)
            .set_wc_icon(LoadIcon(constants::h_inst, MAKEINTRESOURCE(IDI_GAME_MODE_BIG)))
            .set_wc_icon_sm(LoadIcon(constants::h_inst, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL)))
            .set_wc_wnd_extra(static_cast<int>(sizeof(in_hand)))
            .set_wc_style(CS_SAVEBITS)
            .set_x(mouse.x - w / 2)
            .set_y(mouse.y - h / 2)
            .set_width(w)
            .set_height(h)
            .set_parent(parent)
            .set_style(WS_POPUP | WS_EX_TRANSPARENT | WS_EX_LAYERED)
            .set_class_name(constants::CURR_CHOICE_WINDOW_CLASS_NAME)
            .set_title(L"")
            .set_after_create([](const HWND h_wnd) {
                SetWindowPos(h_wnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                SetLayeredWindowAttributes(h_wnd, constants::TRANSPARENCY_PLACEHOLDER, 0xFF, LWA_COLORKEY);
            })
            .set_wnd_extra_data(
                reinterpret_cast<LONG_PTR>(figfab::FigureFabric::instance().create(in_hand, true).release())
            )
            .build(constants::h_inst)
    );
}

HWND misc::new_window::move_log(const HWND parent) noexcept
{
    return *create_window(CreateWindowArgsBuilder{}
                              .set_class_name(TEXT("CHWIGRX:MOVES_LOG"))
                              .set_style(WS_OVERLAPPEDWINDOW)
                              .set_title(static_cast<UINT>(IDS_MOVE_LOG_TITLE))
                              .set_wc_wndproc(moves_list_wndproc)
                              .set_parent(parent)
                              .build(constants::h_inst));
}

HWND misc::new_window::move_log_list_view(const HWND parent) noexcept
{
    return *create_window(
        CreateWindowArgsBuilder{}
            .register_class(false)
            .set_ex_style(WS_EX_CLIENTEDGE)
            .set_class_name(WC_LISTVIEW)
            .set_style(WS_TABSTOP | WS_CHILD | WS_BORDER | WS_VISIBLE | LVS_AUTOARRANGE | LVS_REPORT | LVS_OWNERDATA)
            .set_after_create([](HWND self) { ListView_SetBkColor(self, constants::CHECKERBOARD_BRIGHT_COLOR); })
            .set_parent(parent)
            .build(constants::h_inst)
    );
}

INT_PTR CALLBACK misc::about_proc(
    const HWND h_dlg,
    const UINT message,
    const WPARAM w_param,
    [[maybe_unused]] const LPARAM l_param
) noexcept
{
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

bool misc::cpy_str_to_clip(const HWND h_wnd, const std::string_view s) noexcept
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

std::string misc::take_str_from_clip(const HWND h_wnd) noexcept
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

void misc::change_checkerboard_color_theme(const HWND h_wnd) noexcept
{
    std::swap(mutables::checkerboard_one, mutables::checkerboard_two);
    InvalidateRect(h_wnd, nullptr, NULL);
}

bool misc::game_end_check(const HWND h_wnd, const Color turn) noexcept
{
    if (const GameEndType curr_game_end_state = mutables::board.game_end_check(turn);
        curr_game_end_state != GameEndType::NotGameEnd)
    {
        std::wstring body;
        switch (curr_game_end_state) {
            case GameEndType::Checkmate:
                body = misc::load_resource_string(
                    what_next(turn) == Color::White ? IDS_GAME_END_WHITE_WON : IDS_GAME_END_BLACK_WON
                );
                break;

            case GameEndType::Stalemate:
                body = misc::load_resource_string(
                    turn == Color::White ? IDS_GAME_END_WHITE_STALEMATE : IDS_GAME_END_BLACK_STALEMATE
                );

                break;

            case GameEndType::FiftyRule:
                body = misc::load_resource_string(IDS_GAME_END_FIFTY_RULE);
                break;

            case GameEndType::InsufficientMaterial:
                body = misc::load_resource_string(IDS_GAME_END_INSUFFICIENT_MATERIAL);
                break;

            case GameEndType::MoveRepeat:
                body = misc::load_resource_string(IDS_GAME_END_MOVE_REPEAT_RULE);
                break;

            default:
                assert(!"unexpected game end");
                break;
        }
        if (const auto result = MessageBox(
                h_wnd,
                (body + TEXT('\n') + misc::load_resource_string(IDS_COPY_BOARD_REQUEST) + TEXT('?')).c_str(),
                misc::load_resource_string(IDS_GAME_END_TITLE).c_str(),
                MB_YESNO
            );
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
DWORD misc::create_console() noexcept
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

std::size_t misc::get_icon(const mvmsg::MoveMessage& mm) noexcept
{
    const auto& by_color = mutables::pieces_bitmaps.find(col_to_char(mm.first.get_col()));
    const auto& by_type = by_color->second.find(figure_type_to_char(mm.first.get_type()));
    return std::distance(std::begin(mutables::pieces_bitmaps), by_color) * PLAYABLE_FIGURES.size() +
           std::distance(std::begin(by_color->second), by_type);
}

HIMAGELIST misc::init_move_log_bitmaps() noexcept
{
    auto list = ImageList_Create(
        constants::MOVE_LOG_ICONS_WIDTH, constants::MOVE_LOG_ICONS_HEIGHT, ILC_COLORDDB | ILC_MASK, 1, 0
    );
    for (const auto& [key_col, val_outher] : mutables::pieces_bitmaps) {
        for (const auto& [key_type, val_bitmap] : val_outher) {
            const auto resized = bitmap::resize(
                val_bitmap,
                constants::PIECE_SOURCE_WIDTH,
                constants::PIECE_SOURCE_HEIGHT,
                constants::MOVE_LOG_ICONS_WIDTH,
                constants::MOVE_LOG_ICONS_HEIGHT
            );
            const auto mask = bitmap::create_mask(resized, constants::TRANSPARENCY_PLACEHOLDER);
            ImageList_Add(list, resized, mask);
        }
    }
    return list;
}

void misc::update_moves_list(const HWND moves_list_list_view, const ChessGame& board) noexcept
{
    const auto& prev = board.get_last_moves();
    const auto& future = board.get_future_moves();
    const auto separator = static_cast<int>(!prev.empty() && !future.empty());
    if (moves_list_list_view) {
        ListView_SetItemCount(moves_list_list_view, prev.size() + future.size() + separator);
    }
}

void misc::on_game_board_change(const ChessGame& board) noexcept
{
    update_moves_list(mutables::moves_list_list_view, board);
}

std::wstring misc::load_resource_string(const UINT id) noexcept
{
    constexpr auto max_len = MAX_PATH;
    TCHAR result[max_len]{ TEXT("Unknown resource id") };
    LoadString(constants::h_inst, id, result, max_len);
    return result;
}

std::wstring misc::to_wstring(const FigureType type) noexcept
{
    switch (type) {
        case FigureType::Pawn:
            return load_resource_string(IDS_FIGURE_TYPE_PAWN);
        case FigureType::Bishop:
            return load_resource_string(IDS_FIGURE_TYPE_BISHOP);
        case FigureType::Rook:
            return load_resource_string(IDS_FIGURE_TYPE_ROOK);
        case FigureType::Knight:
            return load_resource_string(IDS_FIGURE_TYPE_KNIGHT);
        case FigureType::Queen:
            return load_resource_string(IDS_FIGURE_TYPE_QUEEN);
        case FigureType::King:
            return load_resource_string(IDS_FIGURE_TYPE_KING);
    }
    return load_resource_string(NULL);
}

std::wstring misc::to_wstring(const std::string& source) noexcept
{
    constexpr auto max_len = 0x100;
    WCHAR result[max_len]{};

    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, source.c_str(), static_cast<int>(source.length()), result, max_len);

    return result;
}

HCURSOR misc::load_animated_cursor(UINT nID, LPCTSTR pszResouceType) noexcept
{
    HCURSOR cursor = nullptr;
    HRSRC resource = FindResource(constants::h_inst, MAKEINTRESOURCE(nID), pszResouceType);
    if (resource) {
        const auto resource_size = SizeofResource(constants::h_inst, resource);
        if (resource_size > 0) {
            const auto resource_global = LoadResource(constants::h_inst, resource);
            if (resource_global) {
                const auto ptr_resource = reinterpret_cast<LPBYTE>(LockResource(resource_global));
                if (ptr_resource) {
                    cursor =
                        reinterpret_cast<HCURSOR>(CreateIconFromResource(ptr_resource, resource_size, FALSE, 0x00030000)
                        );
                    UnlockResource(ptr_resource);
                }
                FreeResource(resource_global);
            }
        }
    }
    return cursor;
}

void misc::set_window_mode(const HWND wnd, const WindowState mode) noexcept
{
    mutables::window_state = mode;
    misc::change_checkerboard_color_theme(wnd);

    switch (mode) {
        case WindowState::Game:
        {
            const auto h_big_game_icon = LoadImage(
                constants::h_inst, MAKEINTRESOURCE(IDI_GAME_MODE_BIG), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED
            );
            const auto h_mini_game_icon = LoadImage(
                constants::h_inst, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED
            );
            SendMessage(wnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(h_big_game_icon));
            SendMessage(wnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(h_mini_game_icon));

            SetMenu(wnd, LoadMenu(constants::h_inst, MAKEINTRESOURCE(IDC_CHWIGRX)));

            update_bot_menu_variables(wnd);

            destroy_window(mutables::figures_list_window);

            break;
        }

        case WindowState::Edit:
        {
            const auto h_edit_icon = LoadImage(
                constants::h_inst, MAKEINTRESOURCE(IDI_EDIT_MODE), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED
            );
            SendMessage(wnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(h_edit_icon));
            SendMessage(wnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(h_edit_icon));

            SetMenu(wnd, LoadMenu(constants::h_inst, MAKEINTRESOURCE(IDR_CHWIGRX_EDIT_MENU)));

            mutables::motion_input.clear();
            mutables::board.reset_move_logger();

            update_edit_menu_variables(wnd);

            mutables::figures_list_window = misc::new_window::figures_list(wnd);
            destroy_window(mutables::moves_list_window);

            break;
        }

        default:
            std::unreachable();
            break;
    }
}
