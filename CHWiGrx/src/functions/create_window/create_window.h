#pragma once

#include "../../winapi/framework.hpp"

#include <expected>
#include <utility>
#include <variant>

namespace create_window_nc
{
inline constexpr auto MAX_LOAD_STRING = 100;
}

struct CreateWindowArgs
{
    // data
    DWORD ex_style{};
    WNDCLASSEX wc{ .cbSize = sizeof(WNDCLASSEX), .style = CS_VREDRAW | CS_HREDRAW };
    std::variant<LPCTSTR, UINT> class_name{};
    std::variant<LPCTSTR, UINT> title{};
    DWORD style{ WS_OVERLAPPEDWINDOW };
    int x{ CW_USEDEFAULT };
    int y{ CW_USEDEFAULT };
    int width{ CW_USEDEFAULT };
    int height{ CW_USEDEFAULT };
    HWND parent{};
    HMENU h_menu{};
    LPVOID lp_param{};
    void (*after_create)(HWND){};
    LONG_PTR wnd_extra_data{};
    LONG_PTR cls_extra_data{};

    // logic
    bool register_class{ true };
    bool unregister_class{ true };

    // through other setters
    TCHAR sz_title[create_window_nc::MAX_LOAD_STRING]{};
    TCHAR sz_class_name[create_window_nc::MAX_LOAD_STRING]{};
};

struct CreateWindowArgsBuilder
{
#define DECLARE_SETTER(name, field)                                                                                    \
    template <typename T>                                                                                              \
    [[nodiscard]] constexpr auto name(const T value)&& noexcept -> CreateWindowArgsBuilder                             \
    {                                                                                                                  \
        result.field = value;                                                                                          \
        return *this;                                                                                                  \
    }

    CreateWindowArgs result{};

    [[nodiscard]] CreateWindowArgs build(HINSTANCE) && noexcept;

    DECLARE_SETTER(set_ex_style, ex_style)
    DECLARE_SETTER(set_wc_style, wc.style)
    DECLARE_SETTER(set_wc_wndproc, wc.lpfnWndProc)
    DECLARE_SETTER(set_wc_cls_extra, wc.cbClsExtra)
    DECLARE_SETTER(set_wc_wnd_extra, wc.cbWndExtra)
    DECLARE_SETTER(set_wc_icon, wc.hIcon)
    DECLARE_SETTER(set_wc_cursor, wc.hCursor)
    DECLARE_SETTER(set_wc_background, wc.hbrBackground)
    DECLARE_SETTER(set_wc_menu_name, wc.lpszMenuName)
    DECLARE_SETTER(set_wc_icon_sm, wc.hIconSm)
    DECLARE_SETTER(set_class_name, class_name)
    DECLARE_SETTER(set_title, title)
    DECLARE_SETTER(set_style, style)
    DECLARE_SETTER(set_x, x)
    DECLARE_SETTER(set_y, y)
    DECLARE_SETTER(set_width, width)
    DECLARE_SETTER(set_height, height)
    DECLARE_SETTER(set_parent, parent)
    DECLARE_SETTER(set_menu, h_menu)
    DECLARE_SETTER(set_lp_param, lp_param)
    DECLARE_SETTER(set_after_create, after_create)
    DECLARE_SETTER(set_wnd_extra_data, wnd_extra_data)
    DECLARE_SETTER(set_cls_extra_data, cls_extra_data)

    DECLARE_SETTER(register_class, register_class)
    DECLARE_SETTER(unregister_class, unregister_class)

#undef DECLARE_SETTER
};

auto create_window(CreateWindowArgs&&) noexcept -> std::expected<HWND, DWORD>;

void destroy_window(HWND&) noexcept;
