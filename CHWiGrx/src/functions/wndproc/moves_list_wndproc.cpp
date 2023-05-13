#include "../../declarations.hpp"
#include "wndproc.h"

#include <stdio.h>
#include <tchar.h>
#include <vector>

LRESULT CALLBACK
moves_list_wndproc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) noexcept
{
    static std::vector<mvmsg::MoveMessage> display_prev_move_recs;
    static std::vector<mvmsg::MoveMessage> display_future_move_recs;
    static HWND list_view{};

    switch (u_msg) {
        case WM_CREATE:
        {
            /* stole data from board */ {
                display_prev_move_recs = board.get_last_moves();
                display_future_move_recs = board.get_future_moves();
            }

            /* list view */ {
                list_view = new_window::moves_log(h_wnd);
                ListView_DeleteAllItems(list_view);

                /* cols + item count */ {
                    LV_COLUMN lv_col{ .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
                                      .fmt = LVCFMT_LEFT,
                                      .cx = 120,
                                      .pszText = const_cast<LPTSTR>(TEXT("Main Col")) };

                    ListView_InsertColumn(list_view, 0, &lv_col);
                    ListView_SetItemCount(list_view, display_prev_move_recs.size() + display_future_move_recs.size());
                }
            }

            break;
        }

        case WM_NOTIFY:
        {
            LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(l_param);

            switch (lpnmh->code) {
                case LVN_GETDISPINFO:
                {
                    LV_DISPINFO* in = reinterpret_cast<LV_DISPINFO*>(l_param);
                    TCHAR sz_string[MAX_PATH]{};

                    const auto& rec = in->item.iItem < display_prev_move_recs.size()
                                          ? display_prev_move_recs.at(in->item.iItem)
                                          : display_future_move_recs.at(in->item.iItem);

                    if (in->item.iSubItem) {
                        if (in->item.mask & LVIF_TEXT) {
                            // detailed stuff
                        }
                    }
                    else {
                        if (in->item.mask & LVIF_TEXT) {
                            _sntprintf_s(
                                sz_string,
                                MAX_PATH,
                                _TRUNCATE,
                                TEXT("Move from (%d, %d) to (%d, %d)"),
                                rec.input.from.x,
                                rec.input.from.y,
                                rec.input.target.x,
                                rec.input.target.y
                            );

                            _tcsncpy_s(in->item.pszText, in->item.cchTextMax, sz_string, _TRUNCATE);
                        }

                        if (in->item.mask & LVIF_IMAGE) {
                            in->item.iImage = get_icon_from_type(rec.first.get_type());
                        }
                    }

                    break;
                }

                case LVN_ODCACHEHINT:
                {
                    LPNMLVCACHEHINT lpCacheHint = (LPNMLVCACHEHINT)l_param;
                    /* FIXME
                    this is sent when the ListView is about to ask for a range of items. On this notification,
                    you should load the specified items into your local cache. It is still
                    possible to get an LVN_GETDISPINFO for an item that has not been cached,
                    therefore, your application must take into account the chance of this
                    occurring.
                    */

                    break;
                }

                case LVN_ODFINDITEM:
                {
                    LPNMLVFINDITEM lpFindItem = (LPNMLVFINDITEM)l_param;
                    /* FIXME 
                    this is sent when ListView needs a particular item. Return -1 if the item is not found.
                    */

                    break;
                }
            }

            break;
        }

        case WM_SIZE:
        {
            RECT rc;
            GetClientRect(h_wnd, &rc);
            MoveWindow(list_view, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

            break;
        }

        case WM_GETMINMAXINFO:
        {
            const auto lp_mmi = reinterpret_cast<LPMINMAXINFO>(l_param);
            lp_mmi->ptMinTrackSize.x = 250;
            lp_mmi->ptMaxTrackSize.x = 350;

            break;
        }

        case WM_CLOSE:
        {
            const HWND owner = GetWindow(h_wnd, GW_OWNER);    // It should be GetParent, but it returns NULL 😢.
            set_menu_checkbox(owner, IDM_WINDOW_MOVELOG, false);

            break;
        }

        default:
            break;
    }

    return DefWindowProc(h_wnd, u_msg, w_param, l_param);
}
