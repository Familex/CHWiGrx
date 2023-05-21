#include "../../declarations.hpp"
#include "wndproc.h"

#include <format>
#include <stdio.h>
#include <tchar.h>
#include <vector>

LRESULT CALLBACK
moves_list_wndproc(const HWND h_wnd, const UINT u_msg, const WPARAM w_param, const LPARAM l_param) noexcept
{
    enum class Column : std::size_t {
        Figure,
        From,
        Target
    };

    switch (u_msg) {
        case WM_CREATE:
        {
            /* list view */ {
                moves_list_list_view = misc::new_window::move_log_list_view(h_wnd);
                ListView_DeleteAllItems(moves_list_list_view);

                /* columns */ {
                    using TextCol = std::pair<LPTSTR, Column>;

                    /* init */ {
                        auto str_from = misc::load_resource_string(IDS_MOVE_LOG_FROM);
                        auto str_target = misc::load_resource_string(IDS_MOVE_LOG_TARGET);
                        auto str_figure = misc::load_resource_string(IDS_MOVE_LOG_FIGURE);
                        for (auto [name, ind] : { TextCol{ str_from.data(), Column::From },
                                                  TextCol{ str_target.data(), Column::Target },
                                                  TextCol{ str_figure.data(), Column::Figure } })
                        {
                            LV_COLUMN lv_col{ .mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM,
                                              .fmt = LVCFMT_LEFT,
                                              .cx = 120,
                                              .pszText = name };

                            ListView_InsertColumn(moves_list_list_view, static_cast<std::size_t>(ind), &lv_col);
                        }
                    }

                    /* width */ {
                        ListView_SetColumnWidth(moves_list_list_view, static_cast<std::size_t>(Column::From), 45);
                        ListView_SetColumnWidth(moves_list_list_view, static_cast<std::size_t>(Column::Target), 52);
                        ListView_SetColumnWidth(
                            moves_list_list_view, static_cast<std::size_t>(Column::Figure), MOVE_LOG_ICONS_WIDTH + 48
                        );
                    }
                }

                /* item count */
                misc::update_moves_list(moves_list_list_view, board);

                /* icons */ {
                    if (const auto& imglst = misc::init_move_log_bitmaps()) {
                        ListView_SetImageList(moves_list_list_view, imglst, LVSIL_NORMAL);
                        ListView_SetImageList(moves_list_list_view, imglst, LVSIL_SMALL);
                    }
                }
            }

            break;
        }

        case WM_NOTIFY:
        {
            LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(l_param);
            const auto insert_text = [&](LVITEM& item, const TCHAR* str) {
                _tcsncpy_s(item.pszText, item.cchTextMax, str, _TRUNCATE);
            };

            switch (lpnmh->code) {
                case LVN_GETDISPINFO:
                {
                    LV_DISPINFO* in = reinterpret_cast<LV_DISPINFO*>(l_param);
                    TCHAR sz_string[MAX_PATH]{};

                    const auto& prev = board.get_last_moves();
                    const auto& future = board.get_future_moves();
                    const std::optional<mvmsg::MoveMessage> rec =
                        prev.empty() != future.empty()
                            ? (in->item.iItem < prev.size()
                                   ? std::optional{ prev.at(in->item.iItem) }
                                   : std::optional{ future.at(future.size() - (in->item.iItem - prev.size()) - 1) })
                            : (in->item.iItem == prev.size() ? std::nullopt
                               : in->item.iItem < prev.size()
                                   ? std::optional{ prev.at(in->item.iItem) }
                                   : std::optional{ future.at(future.size() - (in->item.iItem - prev.size() - 1) - 1) }
                              );

                    if (in->item.iSubItem) {
                        if (in->item.mask & LVIF_TEXT) {
                            switch (static_cast<Column>(in->item.iSubItem)) {
                                case Column::From:
                                {
                                    const auto text = rec ? board.pos_to_string(rec->input.from) : "";
                                    insert_text(in->item, misc::to_wstring(text).c_str());

                                    break;
                                }

                                case Column::Target:
                                {
                                    const auto text = rec ? board.pos_to_string(rec->input.target) : "";
                                    insert_text(in->item, misc::to_wstring(text).c_str());

                                    break;
                                }
                            }
                        }
                    }
                    else {
                        if (in->item.mask & LVIF_TEXT) {
                            insert_text(in->item, rec ? misc::to_wstring(rec->first.get_type()).c_str() : L"");
                        }

                        if (in->item.mask & LVIF_IMAGE) {
                            if (rec) {
                                in->item.iImage = static_cast<int>(misc::get_icon(*rec));
                            }
                        }
                    }

                    break;
                }

                // https://stackoverflow.com/a/23449467
                case NM_CUSTOMDRAW:
                {
                    auto lvcd{ reinterpret_cast<LPNMLVCUSTOMDRAW>(l_param) };
                    switch (lvcd->nmcd.dwDrawStage) {
                        case CDDS_PREPAINT:
                            return CDRF_NOTIFYITEMDRAW;
                        case CDDS_ITEMPREPAINT:
                            lvcd->clrTextBk = CHECKERBOARD_BRIGHT_COLOR;
                            return CDRF_NEWFONT;
                        case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
                            return CDRF_NEWFONT;
                    }
                    return TRUE;
                }
            }

            break;
        }

        case WM_SIZE:
        {
            RECT rc;
            GetClientRect(h_wnd, &rc);
            MoveWindow(moves_list_list_view, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

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
            moves_list_window = nullptr;
            moves_list_list_view = nullptr;

            break;
        }

        default:
            break;
    }

    return DefWindowProc(h_wnd, u_msg, w_param, l_param);
}
