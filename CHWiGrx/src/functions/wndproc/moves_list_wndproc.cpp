#include "../../declarations.hpp"
#include "wndproc.h"

#include <vector>

LRESULT CALLBACK
moves_list_wndproc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) noexcept
{
    static std::vector<mvmsg::MoveMessage> display_prev_move_recs;
    static std::vector<mvmsg::MoveMessage> display_future_move_recs;

    switch (u_msg) {

        default:
            break;
    }

    return DefWindowProc(h_wnd, u_msg, w_param, l_param);
}
