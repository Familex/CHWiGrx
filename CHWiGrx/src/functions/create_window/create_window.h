#pragma once

#include "../../winapi/framework.hpp"

#include <utility>

struct CreateWindowInput
{
    WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
    LPTSTR class_name{};
    LPTSTR title{};
    DWORD style{};
    int x{};
    int y{};
    int width{};
    int height{};
    HWND parent{};
    HMENU h_menu{};
    HINSTANCE h_inst{};
    LPVOID lp_param{};
    void (*after_create)(HWND){};
    LONG_PTR (*get_wnd_extra)(){};
    LONG_PTR (*get_cls_extra)(){};
};

struct CreateWindowInputBuilder
{
    CreateWindowInput result{};

    [[nodiscard]] constexpr auto build() const&& noexcept -> CreateWindowInput { return result; }

    constexpr void set_wc_style(const UINT value) && noexcept { result.wc.style = value; }

    constexpr void set_wc_wndproc(const WNDPROC value) && noexcept { result.wc.lpfnWndProc = value; }

    constexpr void set_wc_cls_extra(const int value) && noexcept { result.wc.cbClsExtra = value; }

    constexpr void set_wc_wnd_extra(const int value) && noexcept { result.wc.cbWndExtra = value; }

    constexpr void set_wc_icon(const HICON value) && noexcept { result.wc.hIcon = value; }

    constexpr void set_wc_cursor(const HCURSOR value) && noexcept { result.wc.hCursor = value; }

    constexpr void set_wc_background(const HBRUSH value) && noexcept { result.wc.hbrBackground = value; }

    constexpr void set_wc_menu_name(const LPCTSTR value) && noexcept { result.wc.lpszMenuName = value; }

    constexpr void set_wc_class_name(const LPCTSTR value) && noexcept { result.wc.lpszClassName = value; }

    constexpr void set_wc_icon_sm(const HICON value) && noexcept { result.wc.hIconSm = value; }
};

auto create_window(HWND&, HINSTANCE, const CreateWindowInput& input) noexcept -> DWORD;

inline void destroy_window(HWND&) noexcept;
