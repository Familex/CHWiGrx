#include "declarations.hpp"

#include <cstdlib>

#ifdef _DEBUG
#include <fcntl.h>
#include <io.h>
#endif

int APIENTRY wWinMain(
    const HINSTANCE hInstance,
    [[maybe_unused]] const HINSTANCE hPrevInstance,
    [[maybe_unused]] const LPTSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow
)
{
    constants::h_inst = hInstance;

    /* winapi stuff init */ {
        constexpr auto i =
            INITCOMMONCONTROLSEX{ .dwSize = sizeof(INITCOMMONCONTROLSEX), .dwICC = ICC_LISTVIEW_CLASSES };
        InitCommonControlsEx(&i);
    }

    /* init bitmaps */ {
        /* pieces */ {
            mutables::pieces_bitmaps['B']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_PAWN));
            mutables::pieces_bitmaps['B']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_ROOK));
            mutables::pieces_bitmaps['B']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KING));
            mutables::pieces_bitmaps['B']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_QUEEN));
            mutables::pieces_bitmaps['B']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_BISHOP));
            mutables::pieces_bitmaps['B']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BLACK_KNIGHT));
            mutables::pieces_bitmaps['W']['P'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_PAWN));
            mutables::pieces_bitmaps['W']['R'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_ROOK));
            mutables::pieces_bitmaps['W']['K'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KING));
            mutables::pieces_bitmaps['W']['Q'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_QUEEN));
            mutables::pieces_bitmaps['W']['B'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_BISHOP));
            mutables::pieces_bitmaps['W']['H'] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_WHITE_KNIGHT));
        }

        /* other */ {
            mutables::other_bitmaps["star"] = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_STAR));
        }
    }

#ifdef _DEBUG
    if (const auto error = misc::create_console()) {
        const auto error_str = std::to_wstring(error);
        MessageBox(
            nullptr,
            error_str.data(),
            misc::load_resource_string(IDS_CREATE_CONSOLE_ERROR).c_str(),
            MB_OK | MB_ICONERROR
        );
    }
#endif    // _DEBUG

    auto const result = create_window(CreateWindowArgsBuilder{}
                                          .set_ex_style(WS_EX_ACCEPTFILES)
                                          .set_wc_wndproc(main_default_wndproc)
                                          .set_wc_icon(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME_MODE_BIG)))
                                          .set_wc_background(reinterpret_cast<HBRUSH>((COLOR_WINDOW)))
                                          .set_wc_icon_sm(LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME_MODE_SMALL)))
                                          .set_title(static_cast<UINT>(IDS_APP_TITLE))
                                          .set_class_name(static_cast<UINT>(IDC_CHWIGRX))
                                          .set_style(WS_OVERLAPPEDWINDOW - WS_MAXIMIZEBOX)
                                          .set_x(mutables::main_window.get_window_pos_x())
                                          .set_y(mutables::main_window.get_window_pos_y())
                                          .set_width(mutables::main_window.get_width())
                                          .set_height(mutables::main_window.get_height())
                                          .build(hInstance));
    if (!result) {
        TCHAR msg[32]{};
        swprintf(msg, 32, TEXT("GetLastError(): %lu"), result.error());
        MessageBox(NULL, msg, TEXT("Failed to create window"), MB_OK);
        return EXIT_FAILURE;
    }

    /* message loop */ {
        MSG msg;
        const HACCEL h_accelerators = LoadAccelerators(constants::h_inst, MAKEINTRESOURCE(IDC_CHWIGRX));
        while (const BOOL b_ret = GetMessage(&msg, nullptr, 0, 0)) {
            if (-1 == b_ret) {
                break;
            }
            if (!TranslateAccelerator(msg.hwnd, h_accelerators, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        return static_cast<int>(msg.wParam);
    }

    return EXIT_FAILURE;
}
