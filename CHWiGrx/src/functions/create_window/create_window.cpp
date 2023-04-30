#include "create_window.h"

auto create_window(HWND& wnd, const HINSTANCE h_inst, const CreateWindowInput& input) noexcept -> DWORD
{
    UnregisterClass(input.class_name, GetModuleHandle(nullptr));
    RegisterClassEx(&input.wc);
    wnd = CreateWindow(
        input.class_name,
        input.title,
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
        return GetLastError();
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
    return 0ull;
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
