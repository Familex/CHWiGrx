#include "board_logic.h"

#include "../../stuff/debug_print.hpp"
#include "../../variables/mutables.hpp"
#include "../misc/misc.h"
#include "bot/bot.h"

void update_main_window_title(HWND h_wnd) noexcept
{
    /* CHWiGrx vs bot [check] */
    std::wstring title = L"CHWiGrx ";
    title.reserve(64);    // hardcoded (CHWiGrx vs NeuralNetwork bot diff.NaN [Check to Black])

    if (bot_type) {
        title += L"vs ";

        switch (*bot_type) {
            case bot::Type::Random:
                title += L"Random ";
                break;
            case bot::Type::Minimax:
                title += L"Minimax ";
                break;
            case bot::Type::AlphaBeta:
                title += L"AlphaBeta ";
                break;
            case bot::Type::MonteCarlo:
                title += L"MonteCarlo ";
                break;
            case bot::Type::NeuralNetwork:
                title += L"NeuralNetwork ";
                break;
            default:
                title += L"Undefined ";
                break;
        }

        title += L"bot ";

        switch (bot_difficulty) {
            case bot::Difficulty::D0:
                title += L"diff.0 ";
                break;
            case bot::Difficulty::D1:
                title += L"diff.1 ";
                break;
            case bot::Difficulty::D2:
                title += L"diff.2 ";
                break;
            case bot::Difficulty::D3:
                title += L"diff.3 ";
                break;
            default:
                title += L"diff.NaN ";
                break;
        }
    }

    if (board.is_empty()) {
        title += L"[Empty board]";
    }
    else if (board.check_for_when(turn)) {
        title += L"[Check to ";
        title += turn == Color::White ? L"White" : turn == Color::Black ? L"Black" : L"None";
        title += L"]";
    }

    SetWindowText(h_wnd, title.c_str());
}

void on_lbutton_up(
    const HWND h_wnd,
    [[maybe_unused]] WPARAM w_param,
    const LPARAM l_param,
    const Pos where_fig,
    const bool use_move_check_and_log
) noexcept
{
    motion_input.set_lbutton_up();
    motion_input.deactivate_by_pos();
    if (motion_input.is_active_by_click()) {
        if (use_move_check_and_log) {
            make_move(h_wnd);
        }
        else {
            if (!motion_input.is_target_at_input()) {
                board.move_fig(motion_input.get_input(), false);
            }
        }
        motion_input.clear();
        InvalidateRect(h_wnd, nullptr, NULL);
        return;
    }
    else {
        motion_input.set_target(where_fig.x, where_fig.y);
        if (motion_input.is_target_at_input()) {
            if (main_window.get_prev_lbutton_click() != Pos(LOWORD(l_param), HIWORD(l_param))) {
                // Released within the cell, but in another place
                motion_input.clear();
                InvalidateRect(h_wnd, nullptr, NULL);
                return;
            }
            motion_input.prepare(turn);
            motion_input.activate_by_click();
            InvalidateRect(h_wnd, nullptr, NULL);
        }
        else {
            if (motion_input.get_in_hand().has_value()) {
                if (use_move_check_and_log) {
                    make_move(h_wnd);
                }
                else if (is_valid_coords(motion_input.get_input().target)) {
                    board.move_fig(motion_input.get_input(), false);
                }
            }
            motion_input.clear();
        }
    }
}

void on_lbutton_down(const HWND h_wnd, const LPARAM l_param) noexcept
{
    // bot move guard
    if (window_state == WindowState::Game && is_bot_move()) {
        return;
    }

    motion_input.set_lbutton_down();
    main_window.set_prev_lbutton_click(Pos{ LOWORD(l_param), HIWORD(l_param) });
    motion_input.deactivate_by_pos();
    if (motion_input.is_active_by_click()) {
        motion_input.set_target(main_window.divide_by_cell_size(l_param).change_axes());
        InvalidateRect(h_wnd, nullptr, NULL);
    }
    else {
        if (const Pos from = main_window.divide_by_cell_size(l_param).change_axes(); board.cont_fig(from)) {
            motion_input.set_from(from);
            motion_input.prepare(turn);
        }
    }
    InvalidateRect(h_wnd, nullptr, NULL);
};

void restart() noexcept
{
    board_repr::BoardRepr start_board_repr_copy{ start_board_repr };    // explicit copy constructor
    board.reset(std::move(start_board_repr_copy));
    motion_input.clear();
    turn = start_board_repr.turn;
}

bool copy_repr_to_clip(const HWND h_wnd) noexcept
{
    const board_repr::BoardRepr board_repr{ board.get_repr(turn, save_all_moves) };
    const auto board_repr_str = AsString<board_repr::BoardRepr>{}(board_repr);
    return cpy_str_to_clip(h_wnd, board_repr_str);
}

auto take_repr_from_clip(const HWND h_wnd) noexcept -> ParseEither<board_repr::BoardRepr, ParseErrorType>
{
    return FromString<board_repr::BoardRepr>{}(take_str_from_clip(h_wnd));
}

void make_move(const HWND h_wnd) noexcept
{
    if (!is_bot_move() && !motion_input.is_current_turn(turn))
        return;    // "Move out of turn"

    auto in_hand = motion_input.get_in_hand();
    Input input = motion_input.get_input();

    if (is_bot_move()) {
        input = bot::create_move(*bot_type, bot_difficulty, board, turn);
        in_hand = board.get_fig(input.from);
    }

    if (!in_hand) {
        InvalidateRect(h_wnd, nullptr, NULL);
        return;
    }

    const auto move_message_sus = board.provide_move(in_hand.value(), input, turn, [c = chose] { return c; });

    if (!move_message_sus) {
        InvalidateRect(h_wnd, nullptr, NULL);
        return;
    }

    debug_print("Curr move was:", AsString<mvmsg::MoveMessage>{}(*move_message_sus, AsStringMeta{ 0_id, 2, 2 }));

    turn = what_next(turn);
    InvalidateRect(h_wnd, nullptr, NULL);
    UpdateWindow(h_wnd);

    game_end_check(h_wnd, turn);

    if (is_bot_move()) {
        make_move(h_wnd);
    }
}

bool is_bot_move() noexcept { return bot_type && turn == bot_turn; }
