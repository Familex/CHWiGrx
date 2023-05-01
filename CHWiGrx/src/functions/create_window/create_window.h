#pragma once

#include "../../winapi/framework.hpp"

#include <expected>
#include <utility>
#include <variant>

namespace create_window_nc
{
inline constexpr auto max_load_string = 100;
}

struct CreateWindowArgs
{
    WNDCLASSEX wc{ .cbSize = sizeof(WNDCLASSEX) };
    std::variant<LPCTSTR, UINT> class_name{};
    std::variant<LPCTSTR, UINT> title{};
    DWORD style{ CS_VREDRAW | CS_HREDRAW };
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

    // through other setters
    TCHAR sz_title[create_window_nc::max_load_string]{};
    TCHAR sz_class_name[create_window_nc::max_load_string]{};
};

struct CreateWindowArgsBuilder
{
    // clang-format off
    #define DECLARE_SETTER(name, field)                                                                                \
        template <typename T>                                                                                          \
        [[nodiscard]] constexpr auto name(const T value) && noexcept -> CreateWindowArgsBuilder                        \
        {                                                                                                              \
            result.field = value;                                                                                      \
            return *this;                                                                                              \
        }
    // clang-format on

    CreateWindowArgs result{};

    [[nodiscard]] auto build(HINSTANCE h_inst) && noexcept -> CreateWindowArgs
    {
        result.wc.hInstance = h_inst;
        result.wc.hCursor = LoadCursor(h_inst, MAKEINTRESOURCE(IDC_MINIMAL_CURSOR));

        /* class name */ {
            if (std::holds_alternative<UINT>(result.class_name)) {
                LoadString(
                    h_inst, std::get<UINT>(result.class_name), result.sz_class_name, create_window_nc::max_load_string
                );
            }
            else {
                wcsncpy_s(
                    result.sz_class_name, std::get<LPCTSTR>(result.class_name), create_window_nc::max_load_string
                );
                result.sz_class_name[create_window_nc::max_load_string - 1] = '\0';
            }

            result.wc.lpszClassName = result.sz_class_name;

            if (std::holds_alternative<UINT>(result.class_name)) {
                result.wc.lpszMenuName = MAKEINTRESOURCE(std::get<UINT>(result.class_name));
            }
        }

        /* title */ {
            if (std::holds_alternative<UINT>(result.title)) {
                LoadString(h_inst, std::get<UINT>(result.title), result.sz_title, create_window_nc::max_load_string);
            }
            else {
                wcsncpy_s(result.sz_title, std::get<LPCTSTR>(result.title), create_window_nc::max_load_string);
                result.sz_title[create_window_nc::max_load_string - 1] = '\0';
            }
        }

        return result;
    }

    DECLARE_SETTER(set_wc_style, wc.style)
    DECLARE_SETTER(set_wc_wndproc, wc.lpfnWndProc)
    DECLARE_SETTER(set_wc_cls_extra, wc.cbClsExtra)
    DECLARE_SETTER(set_wc_wnd_extra, wc.cbWndExtra)
    DECLARE_SETTER(set_wc_icon, wc.hIcon)
    DECLARE_SETTER(set_wc_cursor, wc.hCursor)
    DECLARE_SETTER(set_wc_background, wc.hbrBackground)
    DECLARE_SETTER(set_wc_menu_name, wc.lpszMenuName)
    DECLARE_SETTER(set_wc_icon_sm, wc.hIconSm)
    [[nodiscard]] constexpr auto set_class_name(const auto value) && noexcept -> CreateWindowArgsBuilder
    {
        result.class_name = value;
        return *this;
    }
    [[nodiscard]] constexpr auto set_title(const auto value) && noexcept -> CreateWindowArgsBuilder
    {
        result.title = value;
        return *this;
    }
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

#undef DECLARE_SETTER
};

auto create_window(CreateWindowArgs&&) noexcept -> std::expected<HWND, DWORD>;

inline void destroy_window(HWND&) noexcept;
