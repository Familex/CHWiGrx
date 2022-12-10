#include "../declarations.hpp"

LRESULT CALLBACK main_default_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT ps{};
    static HBITMAP hbmMem{};
    static HGDIOBJ hOld{};
    static HDC hdcMem{};
    static HDC hdc{};
    switch (window_state) {
        case WindowState::GAME:
            return mainproc::main_game_state_wndproc(hWnd, message, wParam, lParam, ps, hbmMem, hOld, hdcMem, hdc);
        case WindowState::EDIT:
            return mainproc::main_edit_state_wndproc(hWnd, message, wParam, lParam, ps, hbmMem, hOld, hdcMem, hdc);
        default:
            throw std::logic_error("Unexpected main window state: " + std::to_string(static_cast<int>(window_state)));
    }
    return static_cast<LRESULT>(0);
}
