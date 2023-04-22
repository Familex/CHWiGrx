#include "pch.hpp"
#include "CppUnitTest.h"
#include "../ChessCore/ChessGame.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <array>

#ifdef REWRITE_ALL

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft::VisualStudio::CppUnitTestFramework
{
    
    template <>
    std::wstring ToString<FigureType>(const FigureType& ft) {
        return as_wstring(ft);
    }
}

namespace FigureBoardTesting
{

    using board_repr::BoardRepr;

    namespace {
        template <typename C>
        concept HasBeginEnd = requires(C c) {
            std::begin(c);
            std::end(c);
        };

        template <typename C, typename F>
            requires requires(C c, F f) {
            HasBeginEnd<C>;
            { f(c[0]) } -> std::same_as<bool>;
        }
        bool contains(const C& c, const F& f)
        {
            return std::find_if(std::begin(c), std::end(c), f) != std::end(c);
        }

        template<class C, class T>
            requires HasBeginEnd<C>
        && std::same_as<T, std::remove_reference_t<decltype(std::declval<C>()[0])>>
            bool contains(const C& v, const T& x)
        {
            return std::find(std::begin(v), std::end(v), x) != std::end(v);
        }

        const auto base{ BoardRepr::FromString(
                "1;0;0;B;R;2;0;1;B;H;3;0;2;B;B;4;0;3;B;Q;5;0;4;B;K;6;0;5;B;B;7;0;6;B;H;8;0;7;B;R;"
                "9;1;0;B;P;10;1;1;B;P;11;1;2;B;P;12;1;3;B;P;13;1;4;B;P;14;1;5;B;P;15;1;6;B;P;16;1;7;B;P;"

                "17;6;0;W;P;18;6;1;W;P;19;6;2;W;P;20;6;3;W;P;21;6;4;W;P;22;6;5;W;P;23;6;6;W;P;24;6;7;W;P;"
                "25;7;0;W;R;26;7;1;W;H;27;7;2;W;B;28;7;3;W;Q;29;7;4;W;K;30;7;5;W;B;31;7;6;W;H;32;7;7;W;R;"

                "[TW1;8;25;32;]<><>~"
        ).value() };

        const auto baseReversed{ BoardRepr::FromString(
            "1;0;0;W;R;2;0;1;W;H;3;0;2;W;B;4;0;3;W;Q;5;0;4;W;K;6;0;5;W;B;7;0;6;W;H;8;0;7;W;R;"
            "9;1;0;W;P;10;1;1;W;P;11;1;2;W;P;12;1;3;W;P;13;1;4;W;P;14;1;5;W;P;15;1;6;W;P;16;1;7;W;P;"

            "17;6;0;B;P;18;6;1;B;P;19;6;2;B;P;20;6;3;B;P;21;6;4;B;P;22;6;5;B;P;23;6;6;B;P;24;6;7;B;P;"
            "25;7;0;B;R;26;7;1;B;H;27;7;2;B;B;28;7;3;B;Q;29;7;4;B;K;30;7;5;B;B;31;7;6;B;H;32;7;7;B;R;"

            "[FW1;8;25;32;]<><>~"
        ).value() };

        const BoardRepr baseEmpty{ {}, Color::White, true };

        const auto standard_provider = [] { return FigureType::Queen; };

        bool move_check(const std::array<const char[9], 8>& table, 
                        const Color col,
                        const FigureType ft,
                        const bool idw) {
            /* ---- function constants ---- */
            const Color enemy_col = what_next(col);
            const FigureType placeholder_figure_type = FigureType::None;

            /* ---- table constants ---- */
            constexpr char move = 'm';
            constexpr char enemy_which_cant_be_eaten = 'e';
            constexpr char enemy_which_can_be_eaten = 'k';
            constexpr char figure = 'f';
            constexpr char figure_which_cant_be_eaten = 'F';

            /*
             {"________",
              "________",
              "________",
              "________",
              "________",
              "________",
              "________",
              "________"},
            */

            /* ---- table parse ---- */
            std::vector<Pos> moves;
            std::vector<Pos> eats;
            std::vector<Pos> enemies;
            std::vector<Pos> figures_which_cant_be_eaten;
            Pos figure_pos;
            
            for (int i = 0; i < HEIGHT; ++i) {
                std::string_view line{ table[i] };
                
                for (int j = 0; j < line.size(); ++j) {
                    if (line[j] == move) {
                        moves.emplace_back(i, j);
                    }
                    else if (line[j] == enemy_which_can_be_eaten) {
                        eats.emplace_back(i, j);
                        enemies.emplace_back(i, j);
                    }
                    else if (line[j] == enemy_which_cant_be_eaten) {
                        enemies.emplace_back(i, j);
                    }
                    else if (line[j] == figure) {
                        figure_pos = Pos{ i, j };
                    }
                    else if (line[j] == figure_which_cant_be_eaten) {
                        figures_which_cant_be_eaten.emplace_back(i, j);
                    }
                }
            }

            /* ---- board setup ---- */
            
            Id id_tmp{ 0_id };
            ChessGame b{ BoardRepr{ {}, col, idw } };
            
            // main figure
            b.place_fig(new Figure{ id_tmp++, figure_pos, col, ft });
            // enemies
            for (const auto& p : enemies) {
                b.place_fig(new Figure{ id_tmp++, p, enemy_col, placeholder_figure_type });
            }
            // figure which can't be eaten
            for (const auto& p : figures_which_cant_be_eaten) {
                b.place_fig(new Figure{ id_tmp++, p, col, placeholder_figure_type });
            }
            
            /* ---- move_check ---- */
            auto figure_moves = b.get_all_moves(b.get_fig(0_id).value());

            const auto is_move = [](std::pair<bool, Pos> p) { return !p.first; };
            const auto is_eat = [](std::pair<bool, Pos> p) { return p.first; };

            return std::all_of(figure_moves.begin(), figure_moves.end(), [&](const auto& p) {
                return (is_move(p) && contains(moves, p.second))
                    || (is_eat(p) && contains(eats, p.second)); }
            ) && moves.size() + eats.size() == figure_moves.size();
            // last condition is for checking if there are no extra moves
        }

        void move_check_assert(
            const std::array<const char[9], 8>& table,
            const Color col,
            const FigureType ft,
            const bool idw)
        {
            Assert::IsTrue(move_check(table, col, ft, idw));
        }

        void move_check_assert(const std::array<const char[9], 8>& table, 
                               const FigureType ft)
        {
            for (auto col : { Color::White, Color::Black })
            {
                for (auto idw : { false, true })
                {
                    move_check_assert(table, col, ft, idw);
                }
            }
        }

        bool is_legal_move(Input&& move, FigureType ft) {
            ChessGame b{ BoardRepr{baseEmpty} };
            b.place_fig(new Figure{ 0_id, move.from, Color::White, ft });
            auto moves = b.get_all_moves(b.get_fig(0_id).value());
            return std::find_if(moves.begin(), moves.end(),
                [&](auto p) { return p.second == move.target; }) != moves.end();
        }

        template <bool idw>
        std::size_t moves_count(Pos&& pos, FigureType ft) {
            ChessGame b{ BoardRepr{baseEmpty } };
            b.set_idw(idw);
            b.place_fig(new Figure{ 0_id, pos, Color::White, ft });
            return b.get_all_moves(b.get_fig(0_id).value()).size();
        }
        
    }   // detail namespace
    
    namespace Moves {

        /*
         *   0 1 2 3 4 5 6 7 ->
         * 0 _ _ _ _ _ _ _ _  y
         * 1 _ _ _ _ _ _ _ _
         * 2 _ _ _ _ _ _ _ _
         * 3 _ _ _ _ _ _ _ _
         * 4 _ _ _ _ _ _ _ _
         * 5 _ _ _ _ _ _ _ _
         * 6 _ _ _ _ _ _ _ _
         * 7 _ _ _ _ _ _ _ _
         * | 
         * v x
         */

        TEST_CLASS(PawnMoves)
        { public:
            TEST_METHOD(Basic)
            {
                for (auto [c, idw]
                    : std::initializer_list<std::pair<Color, bool>>{
                        { Color::White, true  },
                        { Color::Black, false } })
                {
                    move_check_assert(
                        {
                            "________",
                            "________",
                            "________",
                            "___kmk__", // 2 enemies and 1 move
                            "____f___", // current figure
                            "________",
                            "________",
                            "________"
                        },
                        c, FigureType::Pawn, idw
                    );

                    move_check_assert(
                        { "________",
                          "________",
                          "________",
                          "_e______",
                          "_me_____",
                          "kmke____",
                          "efee____",
                          "eee_____" },
                        c, FigureType::Pawn, idw
                    );

                    move_check_assert(
                        { "________",
                          "________",
                          "________",
                          "________",
                          "_e______",
                          "_mk_____",
                          "_f______",
                          "________" },
                        c, FigureType::Pawn, idw
                    );

                    move_check_assert(
                        { "________",
                          "________",
                          "________",
                          "________",
                          "__eee___",
                          "__kek___",
                          "__efe___",
                          "__eee___" },
                        c, FigureType::Pawn, idw
                    );

                    move_check_assert(
                        { "________",
                          "________",
                          "________",
                          "________",
                          "________",
                          "__FFF___",
                          "___f____",
                          "________" },
                        c, FigureType::Pawn, idw
                    );
                }
            }

            TEST_METHOD(LongMoves)
            {
                ChessGame board{ BoardRepr{base} };

                // white first moves
                for (Id id_ = 17_id; id_ <= 24_id; ++id_) {
                    Assert::AreEqual(board.get_all_possible_moves(board.get_fig(id_).value()).size(), 2ull);
                }

                // black first moves
                for (Id id_ = 9_id; id_ <= 16_id; ++id_) {
                    Assert::AreEqual(board.get_all_possible_moves(board.get_fig(id_).value()).size(), 2ull);
                }
            }	// TEST_METHOD(Basic)

            TEST_METHOD(LongMovesReversed)
            {
                ChessGame board{ BoardRepr{baseReversed} };

                // white first moves
                for (Id id_ = 17_id; id_ <= 24_id; ++id_) {
                    Assert::AreEqual(board.get_all_moves(board.get_fig(id_).value()).size(), 2ull);
                }

                // black first moves
                for (Id id_ = 9_id; id_ <= 16_id; ++id_) {
                    Assert::AreEqual(board.get_all_moves(board.get_fig(id_).value()).size(), 2ull);
                }

            }	// TEST_METHOD(Basic)

            TEST_METHOD(Detailed)
            {
                ChessGame board{ BoardRepr{base} };

                auto p_17 = board.get_fig_unsafe(17_id);
                auto p_17_moves = board.get_all_possible_moves(p_17);
                Assert::AreEqual(p_17_moves.size(), 2ull);
                Assert::IsTrue(contains(p_17_moves, std::pair{ false, Pos{ 4, 0 } }));
                Assert::IsTrue(contains(p_17_moves, std::pair{ false, Pos{ 5, 0 } }));

                MoveMessage ms = board.move_check(p_17, Input{ p_17->get_pos(), Pos{4, 0} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::LMOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());
                
                auto p_18 = board.get_fig_unsafe(18_id);
                auto p_18_moves = board.get_all_possible_moves(p_18);
                Assert::AreEqual(p_18_moves.size(), 2ull);
                Assert::IsTrue(contains(p_18_moves, std::pair{ false, Pos{ 4, 1 } }));
                Assert::IsTrue(contains(p_18_moves, std::pair{ false, Pos{ 5, 1 } }));
                
                ms = board.move_check(p_18, Input{ p_18->get_pos(), Pos{4, 1} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::LMOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());
                
                ms = board.move_check(p_18, Input{ p_18->get_pos(), Pos{5, 1} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::MOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());

            }  // TEST_METHOD(Detailed)
            
            TEST_METHOD(DetailedReversed)
            {
                ChessGame board{ BoardRepr{baseReversed} };

                auto p_17 = board.get_fig_unsafe(17_id);
                auto p_17_moves = board.get_all_possible_moves(p_17);
                Assert::AreEqual(p_17_moves.size(), 2ull);
                Assert::IsTrue(contains(p_17_moves, std::pair{ false, Pos{ 4, 0 } }));
                Assert::IsTrue(contains(p_17_moves, std::pair{ false, Pos{ 5, 0 } }));

                MoveMessage ms = board.move_check(p_17, Input{ p_17->get_pos(), Pos{4, 0} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::LMOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());

                auto p_18 = board.get_fig_unsafe(18_id);
                auto p_18_moves = board.get_all_possible_moves(p_18);
                Assert::AreEqual(p_18_moves.size(), 2ull);
                Assert::IsTrue(contains(p_18_moves, std::pair{ false, Pos{ 4, 1 } }));
                Assert::IsTrue(contains(p_18_moves, std::pair{ false, Pos{ 5, 1 } }));

                ms = board.move_check(p_18, Input{ p_18->get_pos(), Pos{4, 1} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::LMOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());

                ms = board.move_check(p_18, Input{ p_18->get_pos(), Pos{5, 1} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::MOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());

            }  // TEST_METHOD(Detailed)

            TEST_METHOD(EnPassant)
            {
                ChessGame board{ BoardRepr{
                    {new Figure(0_id, Pos{3, 2}, Color::White, FigureType::Pawn),
                     new Figure(1_id, Pos{1, 3}, Color::Black, FigureType::Pawn)},
                     Color::Black,
                     true
                } };
                
                auto black_pawn = board.get_fig_unsafe(1_id);

                {
                    auto [conducted, msg] =
                        board.provide_move(
                            black_pawn,
                            Input{ black_pawn->get_pos(), Pos{3, 3} },
                            Color::Black,
                            standard_provider
                        );
                    Assert::IsTrue(conducted);
                    board.set_last_move(msg);
                }

                auto white_pawn = board.get_fig_unsafe(0_id);

                auto moves = board.get_all_moves(white_pawn);
                Assert::IsTrue(
                    std::find_if(moves.begin(), moves.end(),
                        [](auto p) { return p.second == Pos{ 2, 3 }; }) 
                    != moves.end()
                );

                MoveMessage ms = board.move_check(white_pawn, Input{ white_pawn->get_pos(), Pos{2, 3} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::EN_PASSANT);
                Assert::IsFalse(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty() && ms.what_castling_breaks.empty());

                {
                    auto [conducted, mm] = board.provide_move(
                        white_pawn,
                        Input{ white_pawn->get_pos(), Pos{2, 3} },
                        Color::White,
                        standard_provider
                    );
                    Assert::IsTrue(conducted);
                    Assert::AreEqual(mm.who_went.get_id(), white_pawn->get_id());
                    Assert::AreEqual(mm.ms.main_ev, MainEvent::EN_PASSANT);
                    Assert::AreEqual(mm.ms.to_eat[0], black_pawn->get_id());
                    Assert::IsTrue(mm.ms.side_evs.empty());
                }
                
            }   // TEST_METHOD(EnPassant)

            TEST_METHOD(EnPassantReversed)
            {
                ChessGame board{ BoardRepr{
                    {new Figure(0_id, Pos{3, 2}, Color::Black, FigureType::Pawn),
                     new Figure(1_id, Pos{1, 3}, Color::White, FigureType::Pawn)},
                     Color::White,
                     false
                } };

                auto white_pawn = board.get_fig_unsafe(1_id);

                {
                    auto [conducted, msg] =
                        board.provide_move(
                            white_pawn,
                            Input{ white_pawn->get_pos(), Pos{3, 3} },
                            Color::Black,
                            standard_provider
                        );
                    Assert::IsTrue(conducted);
                    board.set_last_move(msg);
                }

                auto black_pawn = board.get_fig_unsafe(0_id);

                auto moves = board.get_all_moves(black_pawn);
                Assert::IsTrue(
                    std::find_if(moves.begin(), moves.end(),
                        [](auto p) { return p.second == Pos{ 2, 3 }; })
                    != moves.end()
                );

                MoveMessage ms = board.move_check(black_pawn, Input{ black_pawn->get_pos(), Pos{2, 3} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::EN_PASSANT);
                Assert::IsFalse(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty() && ms.what_castling_breaks.empty());

                {
                    auto [conducted, mm] = board.provide_move(
                        black_pawn,
                        Input{ black_pawn->get_pos(), Pos{2, 3} },
                        Color::White,
                        standard_provider
                    );
                    Assert::IsTrue(conducted);
                    Assert::AreEqual(mm.who_went.get_id(), black_pawn->get_id());
                    Assert::AreEqual(mm.ms.main_ev, MainEvent::EN_PASSANT);
                    Assert::AreEqual(mm.ms.to_eat[0], white_pawn->get_id());
                    Assert::IsTrue(mm.ms.side_evs.empty());
                }

            }   // TEST_METHOD(EnPassantReversed)
            
            TEST_METHOD(Promotion)
            {
                ChessGame board{ BoardRepr{
                    {new Figure(0_id, Pos{1, 5}, Color::White, FigureType::Pawn)},
                     Color::White,
                     true
                } };

                auto white_pawn = board.get_fig_unsafe(0_id);
                Assert::AreEqual(board.get_all_moves(white_pawn).size(), 1ull);
                
                MoveMessage ms = board.move_check(white_pawn, Input{ white_pawn->get_pos(), Pos{0, 5} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::MOVE);
                Assert::IsTrue(contains(ms.side_evs, SideEvent::PROMOTION));
                Assert::IsTrue(
                       ms.to_eat.empty()
                    && ms.to_move.empty()
                    && ms.what_castling_breaks.empty()
                );
                
                {
                    auto [conducted, msg] =
                        board.provide_move(
                            white_pawn,
                            Input{ white_pawn->get_pos(), Pos{0, 5} },
                            Color::White,
                            standard_provider
                        );
                    Assert::IsTrue(conducted);
                    Assert::AreEqual(msg.ms.main_ev, MainEvent::MOVE);
                    Assert::IsTrue(contains(msg.ms.side_evs, SideEvent::PROMOTION));
                    Assert::AreEqual(msg.who_went.get_id(), white_pawn->get_id());
                    Assert::AreEqual(white_pawn->get_type(), standard_provider());
                    Assert::IsTrue(msg.ms.to_move.empty());
                }

            }   // TEST_METHOD(Promotion)

            TEST_METHOD(PromotionReversed)
            {
                ChessGame board{ BoardRepr{
                    {new Figure(0_id, Pos{1, 5}, Color::Black, FigureType::Pawn)},
                     Color::Black,
                     false
                } };

                auto black_pawn = board.get_fig_unsafe(0_id);
                Assert::AreEqual(board.get_all_moves(black_pawn).size(), 1ull);

                MoveMessage ms = board.move_check(black_pawn, Input{ black_pawn->get_pos(), Pos{0, 5} }).value();
                Assert::AreEqual(ms.main_ev, MainEvent::MOVE);
                Assert::IsTrue(contains(ms.side_evs, SideEvent::PROMOTION));
                Assert::IsTrue(
                    ms.to_eat.empty()
                    && ms.to_move.empty()
                    && ms.what_castling_breaks.empty()
                );

                {
                    auto [conducted, msg] =
                        board.provide_move(
                            black_pawn,
                            Input{ black_pawn->get_pos(), Pos{0, 5} },
                            Color::White,
                            standard_provider
                        );
                    Assert::IsTrue(conducted);
                    Assert::AreEqual(msg.ms.main_ev, MainEvent::MOVE);
                    Assert::IsTrue(contains(msg.ms.side_evs, SideEvent::PROMOTION));
                    Assert::AreEqual(msg.who_went.get_id(), black_pawn->get_id());
                    Assert::AreEqual(black_pawn->get_type(), standard_provider());
                    Assert::IsTrue(msg.ms.to_move.empty());
                }

            }   // TEST_METHOD(PromotionReversed)

        };  // TEST_CLASS(PawnMoves)

        TEST_CLASS(KnightMoves)
        { public:
            TEST_METHOD(Basic)
            {
                const std::size_t moves[][8]{
                    {2, 3, 4, 4, 4, 4, 3, 2},
                    {3, 4, 6, 6, 6, 6, 4, 3},
                    {4, 6, 8, 8, 8, 8, 6, 4},
                    {4, 6, 8, 8, 8, 8, 6, 4},
                    {4, 6, 8, 8, 8, 8, 6, 4},
                    {4, 6, 8, 8, 8, 8, 6, 4},
                    {3, 4, 6, 6, 6, 6, 4, 3},
                    {2, 3, 4, 4, 4, 4, 3, 2}
                };
                
                for (int x = 0; x < 8; ++x)
                {
                    for (int y = 0; y < 8; ++y)
                    {
                        Assert::AreEqual(
                            moves_count<true>(Pos{ x, y }, FigureType::Knight), 
                            moves[x][y]
                        );
                        Assert::AreEqual(
                            moves_count<false>(Pos{ x, y }, FigureType::Knight),
                            moves[x][y]
                        );
                    }
                }
            }
            
            TEST_METHOD(EatMoves)
            {
                move_check_assert(
                    { "________",
                      "__F_F___",
                      "_F___F__",
                      "___f____",
                      "_F___F__",
                      "__F_F___",
                      "________",
                      "________" },
                    FigureType::Knight);

                move_check_assert(
                    { "________",
                      "________",
                      "_FmFmF__",
                      "_mFFFm__",
                      "_FFfFF__",
                      "_mFFFm__",
                      "_FmFmF__",
                      "________" },
                    FigureType::Knight);

                move_check_assert(
                    { "________",
                      "________",
                      "_FkFkF__",
                      "_kFFFk__",
                      "_FFfFF__",
                      "_kFFFk__",
                      "_FkFkF__",
                      "________" },
                    FigureType::Knight);
            }
            
        };  // TEST_CLASS(KnightMoves)

        TEST_CLASS(BishopMoves)
        { public:
            TEST_METHOD(Basic)
            {
                move_check_assert(
                    { "_______m",
                      "m_____m_",
                      "_m___m__",
                      "__m_m___",
                      "___f____",
                      "__m_m___",
                      "_m___m__",
                      "m_____m_" },
                    FigureType::Bishop
                );
            }
            
            TEST_METHOD(EnemyBlock)
            {
                move_check_assert(
                    { "________",
                      "k_______",
                      "_m___k__",
                      "__m_m___",
                      "___f____",
                      "__m_k___",
                      "_m______",
                      "k_______" },
                    FigureType::Bishop);

                move_check_assert(
                    { "___m____",
                      "____m___",
                      "_____m_m",
                      "______f_",
                      "_____m_m",
                      "____k___",
                      "________",
                      "________" },
                    FigureType::Bishop);
            }

            TEST_METHOD(OwnBlock)
            {
                move_check_assert(
                    { "________",
                      "F_____F_",
                      "_m___m__",
                      "__m_m___",
                      "___f____",
                      "__m_m___",
                      "_F___m__",
                      "______m_" },
                    FigureType::Bishop);

                move_check_assert(
                    { "f_______",
                      "_m______",
                      "__F_____",
                      "________",
                      "________",
                      "________",
                      "________",
                      "________" },
                    FigureType::Bishop);
            }
            
        };  // TEST_CLASS(BishopMoves)

        TEST_CLASS(RookMoves)
        { public:
            TEST_METHOD(Basic)
            {
                move_check_assert(
                    { "___m____",
                      "___m____",
                      "mmmfmmmm",
                      "___m____",
                      "___m____",
                      "___m____",
                      "___m____",
                      "___m____" },
                    FigureType::Rook);

                move_check_assert(
                    { "__m_____",
                      "__m_____",
                      "__m_____",
                      "__m_____",
                      "__m_____",
                      "__m_____",
                      "mmfmmmmm",
                      "__m_____" },
                    FigureType::Rook
                );
            }

            TEST_METHOD(EnemyBlock)
            {
                move_check_assert(
                    { "________",
                      "__k_____",
                      "__m_____",
                      "_kfmmk__",
                      "__m_____",
                      "__m_____",
                      "__k_____",
                      "________" },
                    FigureType::Rook);

                move_check_assert(
                    { "________",
                      "________",
                      "________",
                      "________",
                      "________",
                      "________",
                      "k_______",
                      "fmmmmmmk" },
                    FigureType::Rook);
            }

            TEST_METHOD(OwnBlock)
            {
                move_check_assert(
                    { "________",
                      "__F_____",
                      "__m_____",
                      "__m_____",
                      "FmfF____",
                      "__m_____",
                      "__F_____",
                      "________" },
                    FigureType::Rook);

                move_check_assert(
                    { "________",
                      "________",
                      "________",
                      "________",
                      "________",
                      "________",
                      "F_______",
                      "fmmmmmmF" },
                    FigureType::Rook);
            }

            // castling tests are in the KingMoves test class
        };  // TEST_CLASS(RookMoves)

        TEST_CLASS(QueenMoves)
        {
            TEST_METHOD(Basic)
            {
                move_check_assert(
                    { "_m_m_m__",
                      "__mmm___",
                      "mmmfmmmm",
                      "__mmm___",
                      "_m_m_m__",
                      "m__m__m_",
                      "___m___m",
                      "___m____" },
                    FigureType::Queen);

                move_check_assert(
                    { "___m____",
                      "___m____",
                      "___m___m",
                      "m__m__m_",
                      "_m_m_m__",
                      "__mmm___",
                      "mmmfmmmm",
                      "__mmm___" },
                    FigureType::Queen);

                move_check_assert(
                    { "mmmfmmmm",
                      "__mmm___",
                      "_m_m_m__",
                      "m__m__m_",
                      "___m___m",
                      "___m____",
                      "___m____",
                      "___m____" },
                    FigureType::Queen);

                move_check_assert(
                    { "___m____",
                      "___m____",
                      "___m____",
                      "___m___m",
                      "m__m__m_",
                      "_m_m_m__",
                      "__mmm___",
                      "mmmfmmmm" },
                    FigureType::Queen);
            }

            TEST_METHOD(EnemyBlock)
            {
                move_check_assert(
                    { "_m_k_e__",
                      "kmm__e__",
                      "mfmmmkee",
                      "kmm__e__",
                      "_m_k_e__",
                      "_m___e__",
                      "ekeeee__",
                      "_e______" },
                    FigureType::Queen);

                move_check_assert(
                    { "e______e",
                      "___e____",
                      "__k_____",
                      "___m___k",
                      "_e__m__m",
                      "_____m_m",
                      "___e__mm",
                      "__kmmmmf" },
                    FigureType::Queen);

                move_check_assert(
                    { "_e___kfk",
                      "_____mmm",
                      "___em_m_",
                      "___m__m_",
                      "__m___m_",
                      "_k____m_",
                      "e_____m_",
                      "______k_" },
                    FigureType::Queen);
            }
            
            TEST_METHOD(OwnBlock)
            {
                move_check_assert(
                    { "_m_F_e__",
                      "Fmm__e__",
                      "mfmmmFee",
                      "Fmm__e__",
                      "_m_F_e__",
                      "_m___e__",
                      "eFeeee__",
                      "_e______" },
                    FigureType::Queen);

                move_check_assert(
                    { "e______e",
                      "___e____",
                      "__F_____",
                      "___m___F",
                      "_e__m__m",
                      "_____m_m",
                      "___e__mm",
                      "__Fmmmmf" },
                    FigureType::Queen);

                move_check_assert(
                    { "_e___Ffk",
                      "_____mmm",
                      "___em_m_",
                      "___m__m_",
                      "__m___m_",
                      "_F____m_",
                      "e_____m_",
                      "______F_" },
                    FigureType::Queen);
            }
            
        };  // TEST_CLASS(QueenMoves)

        TEST_CLASS(KingMoves)
        { public:
            TEST_METHOD(Basic)
            {
                move_check_assert(
                    { "________",
                      "________",
                      "___mmm__",
                      "___mfm__",
                      "___mmm__",
                      "________",
                      "________",
                      "________" },
                    FigureType::King);

                move_check_assert(
                    { "________",
                      "________",
                      "mm______",
                      "fm______",
                      "mm______",
                      "________",
                      "________",
                      "________" },
                    FigureType::King);
            }

            TEST_METHOD(Eat)
            {
                move_check_assert(
                    { "________",
                      "________",
                      "___kkk__",
                      "___kfk__",
                      "___kkk__",
                      "________",
                      "________",
                      "________" },
                    FigureType::King);

                move_check_assert(
                    { "________",
                      "________",
                      "kk______",
                      "fk______",
                      "kk______",
                      "________",
                      "________",
                      "________" },
                    FigureType::King);
            }

            TEST_METHOD(SelfEat)
            {
                move_check_assert(
                    { "________",
                      "________",
                      "___FFF__",
                      "___FfF__",
                      "___FFF__",
                      "________",
                      "________",
                      "________" },
                    FigureType::King);

                move_check_assert(
                    { "________",
                      "________",
                      "FF______",
                      "fF______",
                      "FF______",
                      "________",
                      "________",
                      "________" },
                    FigureType::King);
            }

            TEST_METHOD(Castling)
            {
                for (const char* br
                    : { 
                        /* R___K__R */
                        "46;7;0;W;R;45;7;4;W;K;47;7;7;W;R;[TW46;47;]<><>~",
                        /* _R__K__R */
                        "46;7;1;W;R;45;7;4;W;K;47;7;7;W;R;[TW46;47;]<><>~",
                    })
                {
                    ChessGame b{ BoardRepr::FromString( br ).value() };

                    auto king = b.get_fig_unsafe(45_id);
                    auto moves = b.get_all_moves(king);

                    Assert::AreEqual(moves.size(), 7ull);

                    Assert::AreEqual(
                        std::count_if(moves.begin(), moves.end(), [&](const auto& p) {
                            MoveMessage mm = b.move_check(king, Input{ king->get_pos(), p.second }).value();
                            return mm.main_ev == MainEvent::CASTLING; }
                    ), 2ll);
                }
            }
            
        };  // TEST_CLASS(KingMoves)
        
    }	// namespace Moves
    
}    // namespace FigureBoardTesting


#endif  // REWRITE_ALL