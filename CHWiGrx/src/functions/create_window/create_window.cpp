#include "create_window.h"

/**
 * \breif Registers a window class and creates a window
 * \param h_inst
 * \param input Parameters for the window creation
 * \return Window handle if successful, otherwise the winapi error code
 */
auto create_window(const HINSTANCE h_inst, CreateWindowParameters&& input) noexcept -> std::expected<HWND, DWORD>
{
    input.wc.hInstance = h_inst;

    constexpr auto max_load_string = 100;
    TCHAR sz_title[max_load_string];
    TCHAR sz_window_class[max_load_string];

    if (std::holds_alternative<UINT>(input.title)) {
        LoadString(h_inst, std::get<UINT>(input.title), sz_title, max_load_string);
    }
    else {
        wcsncpy_s(sz_title, std::get<LPCTSTR>(input.title), max_load_string);
        sz_title[max_load_string - 1] = '\0';
    }
    if (std::holds_alternative<UINT>(input.class_name)) {
        LoadString(h_inst, std::get<UINT>(input.class_name), sz_window_class, max_load_string);
    }
    else {
        wcsncpy_s(sz_window_class, std::get<LPCTSTR>(input.class_name), max_load_string);
        sz_window_class[max_load_string - 1] = '\0';
    }

    input.wc.lpszClassName = sz_window_class;
    if (std::holds_alternative<UINT>(input.class_name)) {
        input.wc.lpszMenuName = MAKEINTRESOURCE(std::get<UINT>(input.class_name));
    }

    UnregisterClass(sz_title, GetModuleHandle(nullptr));
    if (!RegisterClassEx(&input.wc)) {
        return std::unexpected{ GetLastError() };
    }
    HWND wnd = CreateWindow(
        sz_window_class,
        sz_title,
        input.style,
        input.x,
        input.y,
        input.width,
        input.height,
        input.parent,
        input.h_menu,
        h_inst,
        input.lp_param
    );
    if (!wnd) {
        return std::unexpected{ GetLastError() };
    }
    if (input.after_create) {
        input.after_create(wnd);
    }
    ShowWindow(wnd, SW_SHOWDEFAULT);
    UpdateWindow(wnd);
    if (input.get_wnd_extra) {
        SetWindowLongPtr(wnd, GWLP_USERDATA, input.get_wnd_extra());
    }
    if (input.get_cls_extra) {
        std::unreachable();    // 💀
    }
    return wnd;
}

void destroy_window(HWND& wnd) noexcept
{
    if (wnd) {
        DestroyWindow(wnd);
        wnd = nullptr;
    }
    else {
        std::unreachable();
    }
}
