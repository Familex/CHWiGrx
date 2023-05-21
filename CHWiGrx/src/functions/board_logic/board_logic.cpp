#include "board_logic.h"

#include "../../stuff/debug_print.hpp"
#include "../../variables/mutables.hpp"
#include "../misc/misc.h"
#include "bot/bot.h"

void update_main_window_title(const HWND h_wnd) noexcept
{
    /* CHWiGrx vs bot [check] */
    auto title = misc::load_resource_string(IDS_GAME_MODE_TITLE_BEGIN) + TEXT(' ');

    if (mutables::bot_type) {
        title += misc::load_resource_string(IDS_GAME_MODE_TITLE_VERSUS) + TEXT(' ');

        switch (*mutables::bot_type) {
            case bot::Type::Random:
                title += misc::load_resource_string(IDS_BOT_RANDOM) + TEXT(' ');
                break;
            case bot::Type::Minimax:
                title += misc::load_resource_string(IDS_BOT_MINIMAX) + TEXT(' ');
                break;
            case bot::Type::AlphaBeta:
                title += misc::load_resource_string(IDS_BOT_ALPHABETA) + TEXT(' ');
                break;
            case bot::Type::MonteCarlo:
                title += misc::load_resource_string(IDS_BOT_MONTECARLO) + TEXT(' ');
                break;
            case bot::Type::NeuralNetwork:
                title += misc::load_resource_string(IDS_BOT_NEURALNETWORK) + TEXT(' ');
                break;
            default:
                title += misc::load_resource_string(NULL) + TEXT(' ');
                break;
        }

        switch (mutables::bot_difficulty) {
            case bot::Difficulty::D0:
                title += misc::load_resource_string(IDS_BOT_DIFFICULTY_0) + TEXT(' ');
                break;
            case bot::Difficulty::D1:
                title += misc::load_resource_string(IDS_BOT_DIFFICULTY_1) + TEXT(' ');
                break;
            case bot::Difficulty::D2:
                title += misc::load_resource_string(IDS_BOT_DIFFICULTY_2) + TEXT(' ');
                break;
            case bot::Difficulty::D3:
                title += misc::load_resource_string(IDS_BOT_DIFFICULTY_3) + TEXT(' ');
                break;
            default:
                title += misc::load_resource_string(NULL) + TEXT(' ');
                break;
        }
    }

    if (mutables::board.is_empty()) {
        title += TEXT('[') + misc::load_resource_string(IDS_GAME_MODE_TITLE_EMPTY) + TEXT(']');
    }
    else if (mutables::board.check_for_when(mutables::turn)) {
        title += TEXT('{') +
                 misc::load_resource_string(
                     mutables::turn == Color::White ? IDS_GAME_MODE_TITLE_WHITE_CHECK : IDS_GAME_MODE_TITLE_BLACK_CHECK
                 ) +
                 TEXT('}');
    }

    SetWindowText(h_wnd, title.c_str());
}

void on_lbutton_up(
    const HWND h_wnd,
    [[maybe_unused]] WPARAM w_param,
    const LPARAM l_param,
    const Pos& where_fig,
    const bool use_move_check_and_log
) noexcept
{
    mutables::motion_input.set_lbutton_up();
    mutables::motion_input.deactivate_by_pos();
    if (mutables::motion_input.is_active_by_click()) {
        if (use_move_check_and_log) {
            make_move(h_wnd);
        }
        else {
            if (!mutables::motion_input.is_target_at_input()) {
                mutables::board.move_fig(mutables::motion_input.get_input(), false);
            }
        }
        mutables::motion_input.clear();
        InvalidateRect(h_wnd, nullptr, NULL);
        return;
    }
    else {
        mutables::motion_input.set_target(where_fig.x, where_fig.y);
        if (mutables::motion_input.is_target_at_input()) {
            if (mutables::main_window.get_prev_lbutton_click() != Pos(LOWORD(l_param), HIWORD(l_param))) {
                // Released within the cell, but in another place
                mutables::motion_input.clear();
                InvalidateRect(h_wnd, nullptr, NULL);
                return;
            }
            mutables::motion_input.prepare(mutables::turn);
            mutables::motion_input.activate_by_click();
            InvalidateRect(h_wnd, nullptr, NULL);
        }
        else {
            if (mutables::motion_input.get_in_hand().has_value()) {
                if (use_move_check_and_log) {
                    make_move(h_wnd);
                }
                else if (is_valid_coords(mutables::motion_input.get_input().target)) {
                    mutables::board.move_fig(mutables::motion_input.get_input(), false);
                }
            }
            mutables::motion_input.clear();
        }
    }
}

void on_lbutton_down(const HWND h_wnd, const LPARAM l_param) noexcept
{
    // bot move guard
    if (mutables::window_state == WindowState::Game && is_bot_move()) {
        return;
    }

    mutables::motion_input.set_lbutton_down();
    mutables::main_window.set_prev_lbutton_click(Pos{ LOWORD(l_param), HIWORD(l_param) });
    mutables::motion_input.deactivate_by_pos();
    if (mutables::motion_input.is_active_by_click()) {
        mutables::motion_input.set_target(mutables::main_window.divide_by_cell_size(l_param).change_axes());
        InvalidateRect(h_wnd, nullptr, NULL);
    }
    else {
        if (const Pos from = mutables::main_window.divide_by_cell_size(l_param).change_axes();
            mutables::board.cont_fig(from))
        {
            mutables::motion_input.set_from(from);
            mutables::motion_input.prepare(mutables::turn);
        }
    }
    InvalidateRect(h_wnd, nullptr, NULL);
};

void restart() noexcept
{
    board_repr::BoardRepr start_board_repr_copy{ mutables::start_board_repr };    // explicit copy constructor
    mutables::board.reset(std::move(start_board_repr_copy));
    mutables::motion_input.clear();
    mutables::turn = mutables::start_board_repr.turn;
    misc::on_game_board_change(mutables::board);
}

bool copy_repr_to_clip(const HWND h_wnd) noexcept
{
    const board_repr::BoardRepr board_repr{ mutables::board.get_repr(mutables::turn, mutables::save_all_moves) };
    const auto board_repr_str = AsString<board_repr::BoardRepr>{}(board_repr);
    return misc::cpy_str_to_clip(h_wnd, board_repr_str);
}

auto take_repr_from_clip(const HWND h_wnd) noexcept -> ParseEither<board_repr::BoardRepr, ParseErrorType>
{
    return FromString<board_repr::BoardRepr>{}(misc::take_str_from_clip(h_wnd));
}

void make_move(const HWND h_wnd) noexcept
{
    if (!is_bot_move() && !mutables::motion_input.is_current_turn(mutables::turn))
        return;    // "Move out of mutables::turn"

    auto in_hand = mutables::motion_input.get_in_hand();
    Input input = mutables::motion_input.get_input();

    if (is_bot_move()) {
        input = bot::create_move(*mutables::bot_type, mutables::bot_difficulty, mutables::board, mutables::turn);
        in_hand = mutables::board.get_fig(input.from);
    }

    if (!in_hand) {
        InvalidateRect(h_wnd, nullptr, NULL);
        return;
    }

    const auto move_message_sus =
        mutables::board.provide_move(*in_hand, input, mutables::turn, [c = mutables::chose] { return c; });

    if (!move_message_sus) {
        InvalidateRect(h_wnd, nullptr, NULL);
        return;
    }

    debug_print("Curr move was:", AsString<mvmsg::MoveMessage>{}(*move_message_sus, AsStringMeta{ 0_id, 2, 2 }));

    misc::update_moves_list(mutables::moves_list_list_view, mutables::board);

    mutables::turn = what_next(mutables::turn);
    InvalidateRect(h_wnd, nullptr, NULL);
    UpdateWindow(h_wnd);

    misc::game_end_check(h_wnd, mutables::turn);

    if (is_bot_move()) {
        make_move(h_wnd);
    }
}

bool is_bot_move() noexcept { return mutables::bot_type && mutables::turn == mutables::bot_turn; }
