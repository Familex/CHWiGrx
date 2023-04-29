#pragma once

#include "../structs/figures_list_stats.hpp"
#include "../structs/motion_input.hpp"
#include "../structs/window_state.hpp"
#include "bot/bot.h"
#include "chess_game.hpp"
#include "constants.hpp"
#include "structs/board_repr.hpp"

inline WindowState window_state = WindowState::Game;
inline board_repr::BoardRepr start_board_repr { DEFAULT_CHESS_BOARD_IDW };
inline ChessGame board { board_repr::BoardRepr { start_board_repr } /* <- explicit copy */ };
inline Color turn { Color::White };
inline FigureType chose { FigureType::Queen };
inline std::map<char, std::map<char, HBITMAP>> pieces_bitmaps;
inline std::map<const char*, HBITMAP> other_bitmaps;
inline bool save_all_moves = true;
inline HBRUSH checkerboard_one = CHECKERBOARD_DARK;
inline HBRUSH checkerboard_two = CHECKERBOARD_BRIGHT;
inline HWND figures_list_window = nullptr;
inline std::optional<bot::Type> bot_type = std::nullopt;
inline bot::Difficulty bot_difficulty = bot::Difficulty::D0;
inline Color bot_turn = Color::Black;

inline WindowStats main_window { Pos { 300, 300 }, Pos { 498, 498 } };
inline FiguresListStats figures_list { FIGURES_LIST_WINDOW_DEFAULT_POS,
                                       FIGURES_LIST_WINDOW_DEFAULT_DIMENSIONS,
                                       2,
                                       PLAYABLE_FIGURES.size() };
inline MotionInput motion_input { board };
