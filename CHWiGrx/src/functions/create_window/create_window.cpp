#include "create_window.h"

#include "../../declarations.hpp"

[[nodiscard]] CreateWindowArgs CreateWindowArgsBuilder::build(const HINSTANCE h_inst) && noexcept
{
    result.wc.hInstance = h_inst;
    if (!result.wc.hCursor) {
        result.wc.hCursor = misc::load_animated_cursor(IDC_MINIMAL_CURSOR, RT_ANICURSOR);
    }

    /* class name */ {
        if (std::holds_alternative<UINT>(result.class_name)) {
            const auto res = LoadString(
                h_inst, std::get<UINT>(result.class_name), result.sz_class_name, create_window_nc::MAX_LOAD_STRING
            );
        }
        else {
            wcsncpy_s(result.sz_class_name, std::get<LPCTSTR>(result.class_name), create_window_nc::MAX_LOAD_STRING);
            result.sz_class_name[create_window_nc::MAX_LOAD_STRING - 1] = '\0';
        }

        result.wc.lpszClassName = result.sz_class_name;

        if (std::holds_alternative<UINT>(result.class_name)) {
            result.wc.lpszMenuName = MAKEINTRESOURCE(std::get<UINT>(result.class_name));
        }
    }

    /* title */ {
        if (std::holds_alternative<UINT>(result.title)) {
            const auto res =
                LoadString(h_inst, std::get<UINT>(result.title), result.sz_title, create_window_nc::MAX_LOAD_STRING);
        }
        else {
            wcsncpy_s(result.sz_title, std::get<LPCTSTR>(result.title), create_window_nc::MAX_LOAD_STRING);
            result.sz_title[create_window_nc::MAX_LOAD_STRING - 1] = '\0';
        }
    }

    return result;
}

/**
 * \breif Registers a window class and creates a window
 * \param args Parameters for the window creation
 * \return Window handle if successful, otherwise the winapi error code
 */
auto create_window(CreateWindowArgs&& args) noexcept -> std::expected<HWND, DWORD>
{
    if (args.register_class) {
        if (!UnregisterClass(args.sz_class_name, GetModuleHandle(nullptr))) {
            if (const auto error = GetLastError(); error != ERROR_CLASS_DOES_NOT_EXIST) {
                return std::unexpected{ error };
            }
        }
        if (!RegisterClassEx(&args.wc)) {
            return std::unexpected{ GetLastError() };
        }
    }
    HWND wnd = CreateWindowEx(
        args.ex_style,
        args.sz_class_name,
        args.sz_title,
        args.style,
        args.x,
        args.y,
        args.width,
        args.height,
        args.parent,
        args.h_menu,
        args.wc.hInstance,
        args.lp_param
    );
    if (!wnd) {
        return std::unexpected{ GetLastError() };
    }
    if (args.after_create) {
        args.after_create(wnd);
    }
    if (args.wnd_extra_data) {
        SetLastError(NULL);
        SetWindowLongPtr(wnd, GWLP_USERDATA, args.wnd_extra_data);
        if (const auto error = GetLastError()) {
            return std::unexpected{ error };
        }
    }
    ShowWindow(wnd, SW_SHOWDEFAULT);
    if (!UpdateWindow(wnd)) {
        return std::unexpected{ GetLastError() };
    }
    if (args.cls_extra_data) {
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
}
