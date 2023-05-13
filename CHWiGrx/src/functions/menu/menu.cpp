#include "menu.h"

#include "../../variables/mutables.hpp"
#include "bot/bot.h"
#include "stuff/enums.hpp"

void set_menu_checkbox(const HWND h_wnd, const UINT menu_item, const bool value) noexcept
{
    const HMENU h_menu = GetMenu(h_wnd);
    MENUITEMINFO item_info;
    ZeroMemory(&item_info, sizeof(item_info));
    item_info.cbSize = sizeof(item_info);
    item_info.fMask = MIIM_STATE;
    if (!GetMenuItemInfo(h_menu, menu_item, FALSE, &item_info))
        return;
    item_info.fState = value ? MFS_CHECKED : MFS_UNCHECKED;
    SetMenuItemInfoW(h_menu, menu_item, FALSE, &item_info);
}

void update_edit_menu_variables(const HWND h_wnd) noexcept
{
    for (const auto menu_id : { IDM_WHITE_START, IDM_BLACK_START, IDM_IDW_TRUE, IDM_IDW_FALSE }) {
        set_menu_checkbox(h_wnd, menu_id, false);
    }

    if (turn == Color::White)
        set_menu_checkbox(h_wnd, IDM_WHITE_START, true);
    else if (turn == Color::Black)
        set_menu_checkbox(h_wnd, IDM_BLACK_START, true);

    if (board.get_idw() == true)
        set_menu_checkbox(h_wnd, IDM_IDW_TRUE, true);
    else
        set_menu_checkbox(h_wnd, IDM_IDW_FALSE, true);
}

void update_game_menu_variables(const HWND h_wnd) noexcept
{
    update_bot_menu_variables(h_wnd);

    for (const auto menu_id : {
             IDM_TOGGLE_SAVE_ALL_MOVES,
             IDM_SET_CHOICE_TO_QUEEN,
             IDM_SET_CHOICE_TO_ROOK,
             IDM_SET_CHOICE_TO_BISHOP,
             IDM_SET_CHOICE_TO_KNIGHT,
             IDM_WINDOW_MOVELOG,
         })
    {
        set_menu_checkbox(h_wnd, menu_id, false);
    }

    if (save_all_moves)
        set_menu_checkbox(h_wnd, IDM_TOGGLE_SAVE_ALL_MOVES, true);

    switch (chose) {
        case FigureType::Queen:
            set_menu_checkbox(h_wnd, IDM_SET_CHOICE_TO_QUEEN, true);
            break;

        case FigureType::Rook:
            set_menu_checkbox(h_wnd, IDM_SET_CHOICE_TO_ROOK, true);
            break;

        case FigureType::Bishop:
            set_menu_checkbox(h_wnd, IDM_SET_CHOICE_TO_BISHOP, true);
            break;

        case FigureType::Knight:
            set_menu_checkbox(h_wnd, IDM_SET_CHOICE_TO_KNIGHT, true);
            break;

        default:
            assert(("Unknown chose", false));
            break;
    }

    if (moves_list_window) {
        set_menu_checkbox(h_wnd, IDM_WINDOW_MOVELOG, true);
    }
}

void update_bot_menu_variables(const HWND h_wnd) noexcept
{
    for (const auto menu_id : {
             IDM_TOGGLE_BOT,
             IDM_BOTCOLOR_WHITE,
             IDM_BOTCOLOR_BLACK,
             IDM_BOTDIFFICULTY_EASY,
             IDM_BOTDIFFICULTY_NORMAL,
             IDM_BOTDIFFICULTY_HARD,
             IDM_BOTTYPE_RANDOM,
         })
    {
        set_menu_checkbox(h_wnd, menu_id, false);
    }

    if (bot_type) {
        set_menu_checkbox(h_wnd, IDM_TOGGLE_BOT, true);

        if (bot_turn == Color::White)
            set_menu_checkbox(h_wnd, IDM_BOTCOLOR_WHITE, true);
        else if (bot_turn == Color::Black)
            set_menu_checkbox(h_wnd, IDM_BOTCOLOR_BLACK, true);

        if (bot_difficulty == bot::Difficulty::D0)
            set_menu_checkbox(h_wnd, IDM_BOTDIFFICULTY_EASY, true);
        else if (bot_difficulty == bot::Difficulty::D1)
            set_menu_checkbox(h_wnd, IDM_BOTDIFFICULTY_NORMAL, true);
        else if (bot_difficulty == bot::Difficulty::D2)
            set_menu_checkbox(h_wnd, IDM_BOTDIFFICULTY_HARD, true);
        else if (bot_difficulty == bot::Difficulty::D3)
            set_menu_checkbox(h_wnd, IDM_BOTDIFFICULTY_VERYHARD, true);

        if (bot_type == bot::Type::Random)
            set_menu_checkbox(h_wnd, IDM_BOTTYPE_RANDOM, true);
        else if (bot_type == bot::Type::NeuralNetwork)
            ;    // placeholder;
    }
    else {
        // without bot
    }
}
