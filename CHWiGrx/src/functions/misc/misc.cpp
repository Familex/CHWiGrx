#include "misc.h"

#include "../../stuff/debug_print.hpp"
#include "../../variables/constants.hpp"
#include "../../variables/mutables.hpp"
#include "../wndproc/wndproc.h"

bool create_main_window(
    const HINSTANCE h_instance,
    const LPTSTR sz_title,
    const LPTSTR sz_window_class,
    const int n_cmd_show
) noexcept
{
    const HWND h_wnd = CreateWindowW(
        sz_window_class,
        sz_title,
        WS_OVERLAPPEDWINDOW,
        main_window.get_window_pos_x(),
        main_window.get_window_pos_y(),
        main_window.get_width_with_extra(),
        main_window.get_height_with_extra(),
        nullptr,
        nullptr,
        h_instance,
        nullptr
    );

    if (!h_wnd) {
        return false;
    }

    ShowWindow(h_wnd, n_cmd_show);
    UpdateWindow(h_wnd);

    return true;
}

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
    UnregisterClass(CURR_CHOICE_WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
    WNDCLASSEX wc { sizeof(WNDCLASSEX) };
    // FIXME need to copy figure (Don't forget to delete in callback too)
    // because in main program in_hand will be used too where it can be destroyed.
    Figure* for_storage = in_hand;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(in_hand);
    wc.hbrBackground = nullptr;
    wc.hCursor = LoadCursor(nullptr, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
    wc.hIcon = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_GAME_MODE_BIG));
    wc.hIconSm = LoadIcon(nullptr, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL));
    wc.lpfnWndProc = callback;
    wc.lpszClassName = CURR_CHOICE_WINDOW_CLASS_NAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    const auto create_window = [&parent, &mouse, &w, &h, for_storage]() -> HWND {
        if (const HWND h_window = CreateWindow(
                CURR_CHOICE_WINDOW_CLASS_NAME,
                L"",
                WS_POPUP | WS_EX_TRANSPARENT | WS_EX_LAYERED,
                mouse.x - w / 2,
                mouse.y - h / 2,
                w,
                h,
                parent,
                nullptr,
                nullptr,
                nullptr
            );
            h_window)
        {
            SetWindowPos(h_window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetWindowLongPtr(h_window, GWL_EXSTYLE, GetWindowLongPtr(h_window, GWL_EXSTYLE) | WS_EX_LAYERED);
            SetLayeredWindowAttributes(h_window, TRANSPARENCY_PLACEHOLDER, 0xFF, LWA_COLORKEY);
            SetWindowLongPtr(h_window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(for_storage));
            ShowWindow(h_window, SW_SHOWDEFAULT);
            UpdateWindow(h_window);
            return h_window;
        }
        else {
            return nullptr;
        }
    };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}

bool prepare_window(
    const HINSTANCE h_instance,
    const int n_cmd_show,
    const UINT title_id,
    const UINT window_class_id,
    WNDCLASSEX wcex
) noexcept
{
    constexpr auto max_load_string = 100;

    WCHAR sz_title[max_load_string];
    WCHAR sz_window_class[max_load_string];

    LoadStringW(h_instance, title_id, sz_title, max_load_string);
    LoadStringW(h_instance, window_class_id, sz_window_class, max_load_string);

    wcex.lpszClassName = sz_window_class;
    wcex.lpszMenuName = MAKEINTRESOURCE(window_class_id);    // ... = szWindowClass does not work 

    RegisterClassExW(&wcex);

    if (!create_main_window(h_instance, sz_title, sz_window_class, n_cmd_show)) {
        return false;
    }
    return true;
}

int window_loop(const HINSTANCE h_instance) noexcept
{
    MSG msg;
    const HACCEL h_accelerators = LoadAccelerators(h_instance, MAKEINTRESOURCE(IDC_CHWIGRX));
    while (const BOOL b_ret = GetMessage(&msg, nullptr, 0, 0)) {
        if (-1 == b_ret)
            break;
        if (!TranslateAccelerator(msg.hwnd, h_accelerators, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
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
    UnregisterClass(FIGURES_LIST_WINDOW_CLASS_NAME, GetModuleHandle(nullptr));
    WNDCLASSEX wc { sizeof(WNDCLASSEX) };
    HWND h_window {};
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = nullptr;
    wc.hCursor = LoadCursor(h_inst, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));
    wc.hIcon = LoadIcon(h_inst, MAKEINTRESOURCE(IDI_FIGURES_LIST));
    wc.hIconSm = LoadIcon(h_inst, MAKEINTRESOURCE(IDI_FIGURES_LIST));
    wc.lpfnWndProc = figures_list_wndproc;
    wc.lpszClassName = FIGURES_LIST_WINDOW_CLASS_NAME;
    wc.style = CS_VREDRAW | CS_HREDRAW;
    const auto create_window = [&h_window, &parent]() -> HWND {
        if (h_window = CreateWindow(
                FIGURES_LIST_WINDOW_CLASS_NAME,
                FIGURES_LIST_WINDOW_TITLE,
                WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
                FIGURES_LIST_WINDOW_DEFAULT_POS.x,
                FIGURES_LIST_WINDOW_DEFAULT_POS.y,
                figures_list.get_width_with_extra(),
                figures_list.get_height_with_extra(),
                parent,
                nullptr,
                h_inst,
                nullptr
            ),
            !h_window)
            return nullptr;
        ShowWindow(h_window, SW_SHOWDEFAULT);
        UpdateWindow(h_window);
        return h_window;
    };

    if (!RegisterClassEx(&wc))
        return create_window();

    return create_window();
}