#pragma once

#include "../winapi/framework.hpp"
#include "structs/board_repr.hpp"
#include "structs/figure.hpp"

inline HINSTANCE h_inst;
inline const COLORREF CHECKERBOARD_DARK_COLOR{ RGB(0x32, 0x32, 0x32) };
inline const COLORREF CHECKERBOARD_BRIGHT_COLOR{ RGB(0x80, 0x80, 0x80) };
inline const HBRUSH CHECKERBOARD_DARK{ CreateSolidBrush(CHECKERBOARD_DARK_COLOR) };
inline const HBRUSH CHECKERBOARD_BRIGHT{ CreateSolidBrush(CHECKERBOARD_BRIGHT_COLOR) };
inline const auto DEFAULT_CHESS_BOARD_IDW{
    FromString<board_repr::BoardRepr>{}("02H8W8TWC0,7,25,32!"
                                        "0.00BR1.01BH2.02BB3.03BK4.04BQ5.05BB6.06BH7.07BR8.08BP9.09BP10.10BP11.11BP13."
                                        "12BP14.13BP15.14BP16.15BP17.48WP18.49WP19.50WP20.51WP21.52WP22.53WP23.54WP24."
                                        "55WP25.56WR26.57WH27.58WB28.59WK29.60WQ30.61WB31.62WH32.63WR<><>")
        ->value
};
inline const auto DEFAULT_CHESS_BOARD_NIDW{
    FromString<board_repr::BoardRepr>{}("02H8W8FWC0,7,25,32!"
                                        "0.00WR1.01WH2.02WB3.03WQ4.04WK5.05WB6.06WH7.07WR8.08WP9.09WP10.10WP11.11WP12."
                                        "12WP13.13WP14.14WP15.15WP16.48BP17.49BP18.50BP19.51BP20.52BP21.53BP22.54BP23."
                                        "55BP25.56BR26.57BH27.58BB28.59BQ29.60BK30.61BB31.62BH32.63BR<><>")
        ->value
};
inline const auto EMPTY_REPR = board_repr::BoardRepr({}, Color::White, true);
inline const int HEADER_HEIGHT{ GetSystemMetrics(SM_CXPADDEDBORDER) + GetSystemMetrics(SM_CYMENUSIZE) +
                                GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME) };
inline const int SCROLLBAR_THICKNESS{ GetSystemMetrics(SM_CXVSCROLL) };
inline const LPCTSTR FIGURES_LIST_WINDOW_CLASS_NAME{ L"CHWIGRX:LIST" };
inline const LPCTSTR CURR_CHOICE_WINDOW_CLASS_NAME{ L"CHWIGRX:CHOICE" };
inline constexpr Pos FIGURES_LIST_WINDOW_DEFAULT_POS{ CW_USEDEFAULT, CW_USEDEFAULT };
inline constexpr Pos FIGURES_LIST_WINDOW_DEFAULT_DIMENSIONS{ 300, 300 };
inline constexpr auto MAIN_WINDOW_CHOICE_TO_DESTROY_TIMER_ID{ 1 };
inline constexpr auto FIGURES_LIST_CHOICE_TO_DESTROY_TIMER_ID{ 2 };
inline constexpr auto TO_DESTROY_ELAPSE_DEFAULT_IN_MS{ 5 };
inline constexpr COLORREF TRANSPARENCY_PLACEHOLDER{ RGB(0xFF, 0x0, 0x0) };
inline constexpr std::size_t MOVE_LOG_ICONS_WIDTH{ 32 };
inline constexpr std::size_t MOVE_LOG_ICONS_HEIGHT{ 32 };
inline constexpr std::size_t PIECE_SOURCE_WIDTH{ 64 };
inline constexpr std::size_t PIECE_SOURCE_HEIGHT{ 64 };
