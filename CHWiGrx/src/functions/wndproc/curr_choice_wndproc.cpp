#include "../../declarations.hpp"
#include "wndproc.h"

#include <array>

namespace
{

/**
 * \brief Curr choice wndproc base function
 * \tparam target_pos_game_check Whether to check target position for game validity
 * \tparam get_to_ignore RECTs what prevent from getting into success branch
 * \tparam get_main_window Main window from this window getter
 * \tparam on_success Procedure on success
 * \tparam on_fail Procedure on fail
 * \tparam after_check Procedure after check
 * \param h_wnd Curr choice window handle
 * \param u_msg Winapi message
 * \param w_param Winapi message argument
 * \param l_param Winapi message argument
 * \return Wndproc result
 */
template <
    bool target_pos_game_check,
    auto get_to_ignore,
    auto get_main_window,
    auto on_success,
    auto on_fail,
    auto after_check>
// clang-format off
    requires requires(HWND wnd, Pos p, WPARAM w, LPARAM l, Figure* f) {
        get_to_ignore(wnd).begin();
        get_to_ignore(wnd).end();
        { get_to_ignore(wnd)[0] } -> std::same_as<RECT&>;
        { get_main_window(wnd) } -> std::same_as<HWND>;
        on_success(wnd, p, target_pos_game_check, w, l, f);
        on_fail(wnd, p, target_pos_game_check, w, l, f);
        after_check(f);
    }
// clang-format on
LRESULT CALLBACK
curr_choice_wndproc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) noexcept
{
    static constexpr int TO_DESTROY_TIMER_ID{ MAIN_WINDOW_CHOICE_TO_DESTROY_TIMER_ID };
    switch (u_msg) {
        case WM_CREATE:
            SetTimer(h_wnd, TO_DESTROY_TIMER_ID, TO_DESTROY_ELAPSE_DEFAULT_IN_MS, nullptr);
            break;
        case WM_TIMER:
            if (w_param == TO_DESTROY_TIMER_ID) {
                KillTimer(h_wnd, TO_DESTROY_TIMER_ID);
                SendMessage(h_wnd, WM_EXITSIZEMOVE, NULL, NULL);
            }
            break;
        case WM_ENTERSIZEMOVE:    // On figure start dragging
            KillTimer(h_wnd, TO_DESTROY_TIMER_ID);
            break;
        case WM_EXITSIZEMOVE:    // On figure release
        {
            const HWND main_wnd = get_main_window(h_wnd);

            POINT cur_pos{};
            GetCursorPos(&cur_pos);

            const auto stored_fig = reinterpret_cast<Figure*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA));

            const auto to_ignore = get_to_ignore(h_wnd);

            if (const Pos where_fig = main_window.get_figure_under_mouse(cur_pos);
                (target_pos_game_check || is_valid_coords(where_fig)) &&
                !std::ranges::any_of(to_ignore, [cur_pos](const RECT rc) {
                    return rc.top <= cur_pos.y && cur_pos.y <= rc.bottom && rc.left <= cur_pos.x &&
                           cur_pos.x <= rc.right;
                }))
            {
                // Success branch
                on_success(h_wnd, where_fig, target_pos_game_check, w_param, l_param, stored_fig);
            }
            else {
                // Fail branch
                on_fail(h_wnd, where_fig, target_pos_game_check, w_param, l_param, stored_fig);
            }
            after_check(stored_fig);
            InvalidateRect(main_wnd, nullptr, NULL);
            DestroyWindow(h_wnd);
        } break;
        case WM_NCHITTEST:    // When intercepting mouse clicks, we simulate dragging.
            return HTCAPTION;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            const HDC hdc = BeginPaint(h_wnd, &ps);
            if (const auto in_hand = reinterpret_cast<Figure*>(GetWindowLongPtr(h_wnd, GWLP_USERDATA))) {
                draw_figure(hdc, in_hand, Pos(0, 0), false);
            }
            EndPaint(h_wnd, &ps);

            break;
        }

        default:
            return DefWindowProc(h_wnd, u_msg, w_param, l_param);
    }

    return 0;
}

namespace figures_list
{

[[nodiscard]] auto get_figures_list_rect(const HWND h_wnd) noexcept -> std::array<RECT, 1>
{
    RECT figures_list;
    GetWindowRect(GetParent(h_wnd), &figures_list);
    return { figures_list };
}

[[nodiscard]] auto get_main_window(const HWND h_wnd) noexcept -> HWND { return GetWindow(GetParent(h_wnd), GW_OWNER); }

void on_success(HWND, const Pos& where_fig, bool, WPARAM, LPARAM, Figure* stored_fig) noexcept
{
    stored_fig->move_to(where_fig);
    board.place_fig(stored_fig);
    if (stored_fig->is(FigureType::Rook)) {
        board.on_castling(stored_fig->get_id());
    }
}

void on_fail(HWND, const Pos&, bool, WPARAM, LPARAM, const Figure* stored_fig) noexcept { delete stored_fig; }

void after_check(Figure*) noexcept { motion_input.clear(); }

}    // namespace figures_list

namespace main_window
{

auto get_empty_array(HWND) noexcept -> std::array<RECT, 0> { return {}; }

auto get_main_window(const HWND wnd) noexcept -> HWND { return GetParent(wnd); }

void on_success(const HWND wnd, const Pos& where_fig, const bool target_pos_game_check, WPARAM w, LPARAM l, Figure*) noexcept
{
    on_lbutton_up(get_main_window(wnd), w, l, where_fig, target_pos_game_check);
}

void on_fail(HWND, const Pos&, bool, WPARAM, LPARAM, const Figure* const stored_fig) noexcept
{
    // Moved figure out of the board without checking validity => delete it from the board
    board.delete_fig(stored_fig->get_pos());
    motion_input.clear();
}

void after_check(const Figure* const stored_fig) noexcept { delete stored_fig; }

}    // namespace main_window

}    // namespace

LRESULT CALLBACK
curr_choice_edit_mode_wndproc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) noexcept
{
    using namespace main_window;

    return curr_choice_wndproc<false, get_empty_array, get_main_window, on_success, on_fail, after_check>(
        h_wnd, u_msg, w_param, l_param
    );
}

LRESULT CALLBACK
curr_choice_game_mode_wndproc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) noexcept
{
    using namespace main_window;

    return curr_choice_wndproc<true, get_empty_array, get_main_window, on_success, on_fail, after_check>(
        h_wnd, u_msg, w_param, l_param
    );
}

LRESULT CALLBACK curr_choice_figures_list_wndproc(
    const HWND h_wnd,
    const UINT u_msg,
    const WPARAM w_param,
    const LPARAM l_param
) noexcept
{
    using namespace figures_list;

    return curr_choice_wndproc<false, get_figures_list_rect, get_main_window, on_success, on_fail, after_check>(
        h_wnd, u_msg, w_param, l_param
    );
}
