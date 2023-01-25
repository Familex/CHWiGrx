#include "pch.h"
#include "CppUnitTest.h"
#include "../ChessCore/FigureBoard.h"

#include <algorithm>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft::VisualStudio::CppUnitTestFramework
{
    template <>
    std::wstring ToString<MainEvent>(const MainEvent& me) {
        return to_wstring(me);
    }
    template <>
    std::wstring ToString<FigureType>(const FigureType& ft) {
        return to_wstring(ft);
    }
}

namespace FigureBoardTesting
{

    Id operator""_id(unsigned long long int id) {
        return static_cast<Id>(id);
    }

    template<class C, class T>
    auto contains(const C& v, const T& x)
        -> decltype(end(v), true)
    {
        return end(v) != std::find(begin(v), end(v), x);
    }
    
    namespace Moves {

        const BoardRepr base{
            "1;0;0;B;R;2;0;1;B;H;3;0;2;B;B;4;0;3;B;Q;5;0;4;B;K;6;0;5;B;B;7;0;6;B;H;8;0;7;B;R;"
            "9;1;0;B;P;10;1;1;B;P;11;1;2;B;P;12;1;3;B;P;13;1;4;B;P;14;1;5;B;P;15;1;6;B;P;16;1;7;B;P;"

            "17;6;0;W;P;18;6;1;W;P;19;6;2;W;P;20;6;3;W;P;21;6;4;W;P;22;6;5;W;P;23;6;6;W;P;24;6;7;W;P;"
            "25;7;0;W;R;26;7;1;W;H;27;7;2;W;B;28;7;3;W;Q;29;7;4;W;K;30;7;5;W;B;31;7;6;W;H;32;7;7;W;R;"

            "[TW1;8;25;32;]<><>~"
        };

        const BoardRepr baseReversed{
            "1;0;0;W;R;2;0;1;W;H;3;0;2;W;B;4;0;3;W;Q;5;0;4;W;K;6;0;5;W;B;7;0;6;W;H;8;0;7;W;R;"
            "9;1;0;W;P;10;1;1;W;P;11;1;2;W;P;12;1;3;W;P;13;1;4;W;P;14;1;5;W;P;15;1;6;W;P;16;1;7;W;P;"

            "17;6;0;B;P;18;6;1;B;P;19;6;2;B;P;20;6;3;B;P;21;6;4;B;P;22;6;5;B;P;23;6;6;B;P;24;6;7;B;P;"
            "25;7;0;B;R;26;7;1;B;H;27;7;2;B;B;28;7;3;B;Q;29;7;4;B;K;30;7;5;B;B;31;7;6;B;H;32;7;7;B;R;"

            "[FW1;8;25;32;]<><>~"
        };

        const auto standard_provider = [] { return FigureType::Queen; };

        TEST_CLASS(PawnMoves)
        {
        public:
            
            TEST_METHOD(Basic)
            {
                FigureBoard board{ BoardRepr{base} };

                // white first moves
                for (Id id = 17_id; id <= 24_id; ++id) {
                    Assert::AreEqual(board.get_all_possible_moves(board.get_fig(id)).size(), 2ull);
                }

                // black first moves
                for (Id id = 9_id; id <= 16_id; ++id) {
                    Assert::AreEqual(board.get_all_possible_moves(board.get_fig(id)).size(), 2ull);
                }
            }	// TEST_METHOD(Basic)

            TEST_METHOD(BasicReversed)
            {
                FigureBoard board{ BoardRepr{baseReversed} };

                // white first moves
                for (Id id = 17_id; id <= 24_id; ++id) {
                    Assert::AreEqual(board.get_all_possible_moves(board.get_fig(id)).size(), 2ull);
                }

                // black first moves
                for (Id id = 9_id; id <= 16_id; ++id) {
                    Assert::AreEqual(board.get_all_possible_moves(board.get_fig(id)).size(), 2ull);
                }

            }	// TEST_METHOD(Basic)

            TEST_METHOD(Detailed)
            {
                FigureBoard board{ BoardRepr{base} };

                auto p_17 = board.get_fig(17_id);
                auto p_17_moves = board.get_all_possible_moves(p_17);
                Assert::AreEqual(p_17_moves.size(), 2ull);
                Assert::IsTrue(contains(p_17_moves, std::pair{ false, Pos{ 4, 0 } }));
                Assert::IsTrue(contains(p_17_moves, std::pair{ false, Pos{ 5, 0 } }));

                MoveMessage ms = std::get<MoveMessage>(board.move_check(p_17, Input{ p_17->get_pos(), Pos{4, 0} }));
                Assert::AreEqual(ms.main_ev, MainEvent::LMOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());
                
                auto p_18 = board.get_fig(18_id);
                auto p_18_moves = board.get_all_possible_moves(p_18);
                Assert::AreEqual(p_18_moves.size(), 2ull);
                Assert::IsTrue(contains(p_18_moves, std::pair{ false, Pos{ 4, 1 } }));
                Assert::IsTrue(contains(p_18_moves, std::pair{ false, Pos{ 5, 1 } }));
                
                ms = std::get<MoveMessage>(board.move_check(p_18, Input{ p_18->get_pos(), Pos{4, 1} }));
                Assert::AreEqual(ms.main_ev, MainEvent::LMOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());
                
                ms = std::get<MoveMessage>(board.move_check(p_18, Input{ p_18->get_pos(), Pos{5, 1} }));
                Assert::AreEqual(ms.main_ev, MainEvent::MOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());

            }  // TEST_METHOD(Detailed)
            
            TEST_METHOD(DetailedReversed)
            {
                FigureBoard board{ BoardRepr{baseReversed} };

                auto p_17 = board.get_fig(17_id);
                auto p_17_moves = board.get_all_possible_moves(p_17);
                Assert::AreEqual(p_17_moves.size(), 2ull);
                Assert::IsTrue(contains(p_17_moves, std::pair{ false, Pos{ 4, 0 } }));
                Assert::IsTrue(contains(p_17_moves, std::pair{ false, Pos{ 5, 0 } }));

                MoveMessage ms = std::get<MoveMessage>(board.move_check(p_17, Input{ p_17->get_pos(), Pos{4, 0} }));
                Assert::AreEqual(ms.main_ev, MainEvent::LMOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());

                auto p_18 = board.get_fig(18_id);
                auto p_18_moves = board.get_all_possible_moves(p_18);
                Assert::AreEqual(p_18_moves.size(), 2ull);
                Assert::IsTrue(contains(p_18_moves, std::pair{ false, Pos{ 4, 1 } }));
                Assert::IsTrue(contains(p_18_moves, std::pair{ false, Pos{ 5, 1 } }));

                ms = std::get<MoveMessage>(board.move_check(p_18, Input{ p_18->get_pos(), Pos{4, 1} }));
                Assert::AreEqual(ms.main_ev, MainEvent::LMOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());

                ms = std::get<MoveMessage>(board.move_check(p_18, Input{ p_18->get_pos(), Pos{5, 1} }));
                Assert::AreEqual(ms.main_ev, MainEvent::MOVE);
                Assert::IsTrue(ms.side_evs.empty());
                Assert::IsTrue(ms.to_eat.empty());
                Assert::IsTrue(ms.to_move.empty());
                Assert::IsTrue(ms.what_castling_breaks.empty());

            }  // TEST_METHOD(Detailed)

            TEST_METHOD(EnPassant)
            {
                FigureBoard board{ BoardRepr{
                    {new Figure(0_id, {3, 2}, Color::White, FigureType::Pawn),
                     new Figure(1_id, {1, 3}, Color::Black, FigureType::Pawn)},
                     Color::Black,
                     true
                } };
                
                auto black_pawn = board.get_fig(1_id);

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

                auto white_pawn = board.get_fig(0_id);

                auto moves = board.get_all_moves(white_pawn);
                Assert::IsTrue(
                    std::find_if(moves.begin(), moves.end(),
                        [](auto p) { return p.second == Pos{ 2, 3 }; }) 
                    != moves.end()
                );

                MoveMessage ms = 
                    std::get<MoveMessage>(board.move_check(white_pawn, Input{ white_pawn->get_pos(), Pos{2, 3} }));
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
                FigureBoard board{ BoardRepr{
                    {new Figure(0_id, {3, 2}, Color::Black, FigureType::Pawn),
                     new Figure(1_id, {1, 3}, Color::White, FigureType::Pawn)},
                     Color::White,
                     false
                } };

                auto white_pawn = board.get_fig(1_id);

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

                auto black_pawn = board.get_fig(0_id);

                auto moves = board.get_all_moves(black_pawn);
                Assert::IsTrue(
                    std::find_if(moves.begin(), moves.end(),
                        [](auto p) { return p.second == Pos{ 2, 3 }; })
                    != moves.end()
                );

                MoveMessage ms =
                    std::get<MoveMessage>(board.move_check(black_pawn, Input{ black_pawn->get_pos(), Pos{2, 3} }));
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
                FigureBoard board{ BoardRepr{
                    {new Figure(0_id, {1, 5}, Color::White, FigureType::Pawn)},
                     Color::White,
                     true
                } };

                auto white_pawn = board.get_fig(0_id);
                Assert::AreEqual(board.get_all_moves(white_pawn).size(), 1ull);
                
                auto ms = std::get<MoveMessage>(board.move_check(white_pawn, Input{ white_pawn->get_pos(), Pos{0, 5} }));
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
                FigureBoard board{ BoardRepr{
                    {new Figure(0_id, {1, 5}, Color::Black, FigureType::Pawn)},
                     Color::Black,
                     false
                } };

                auto black_pawn = board.get_fig(0_id);
                Assert::AreEqual(board.get_all_moves(black_pawn).size(), 1ull);

                MoveMessage ms = std::get<MoveMessage>(board.move_check(black_pawn, Input{ black_pawn->get_pos(), Pos{0, 5} }));
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
        };

    }	// namespace Moves
    
}    // namespace FigureBoardTesting
