#include "declarations.hpp"

#ifdef _DEBUG
#include <fcntl.h>
#include <io.h>
#endif

int APIENTRY wWinMain(
    const HINSTANCE hInstance,
    [[maybe_unused]] const HINSTANCE hPrevInstance,
    [[maybe_unused]] const LPWSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow
)
{
    // init h_inst
    h_inst = hInstance;

    /* init bitmaps */ {
        /* pieces */ {
            pieces_bitmaps['B']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_PAWN));
            pieces_bitmaps['B']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_ROOK));
            pieces_bitmaps['B']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KING));
            pieces_bitmaps['B']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_QUEEN));
            pieces_bitmaps['B']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_BISHOP));
            pieces_bitmaps['B']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KNIGHT));
            pieces_bitmaps['W']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_PAWN));
            pieces_bitmaps['W']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_ROOK));
            pieces_bitmaps['W']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KING));
            pieces_bitmaps['W']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_QUEEN));
            pieces_bitmaps['W']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_BISHOP));
            pieces_bitmaps['W']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KNIGHT));
        }

        /* other */ {
            other_bitmaps["star"] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_STAR));
        }
    }

#ifdef _DEBUG
    if (const auto error = create_console()) {
        const auto error_str = std::to_wstring(error);
        MessageBox(nullptr, error_str.data(), L"Error while creating console", MB_OK | MB_ICONERROR);
    }
#endif    // _DEBUG

    if (!create_window(CreateWindowArgsBuilder{}
                           .set_ex_style(WS_EX_ACCEPTFILES)
                           .set_wc_wndproc(main_default_wndproc)
                           .set_wc_icon(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME_MODE_BIG)))
                           .set_wc_background(reinterpret_cast<HBRUSH>((COLOR_WINDOW)))
                           .set_wc_icon_sm(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL)))
                           .set_title(static_cast<UINT>(IDS_APP_TITLE))
                           .set_class_name(static_cast<UINT>(IDC_CHWIGRX))
                           .set_style(WS_OVERLAPPEDWINDOW)
                           .set_x(main_window.get_window_pos_x())
                           .set_y(main_window.get_window_pos_y())
                           .set_width(main_window.get_width())
                           .set_height(main_window.get_height())
                           .build(hInstance)))
    {
        return FALSE;
    }

    /* message loop */ {
        MSG msg;
        const HACCEL h_accelerators = LoadAccelerators(h_inst, MAKEINTRESOURCE(IDC_CHWIGRX));
        while (const BOOL b_ret = GetMessage(&msg, nullptr, 0, 0)) {
            if (-1 == b_ret)
                break;
            if (!TranslateAccelerator(msg.hwnd, h_accelerators, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        return static_cast<int>(msg.wParam);
    }
}
