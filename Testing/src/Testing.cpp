#include "CppUnitTest.h"
#include "chess_game.hpp"
#include "pch.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft::VisualStudio::CppUnitTestFramework
{

template <>
std::wstring ToString<FigureType>(const FigureType& ft)
{
    const auto str = AsString<FigureType>{}(ft);
    return { str.begin(), str.end() };    // only to one-byte strings
}

template <>
std::wstring ToString<Pos>(const Pos& pos)
{
    const auto str = std::format("({}, {})", pos.x, pos.y);
    return { str.begin(), str.end() };
}

template <>
std::wstring ToString<std::vector<std::pair<Pos, Pos>>>(const std::vector<std::pair<Pos, Pos>>& vec)
{
    std::string str{ "[ " };
    for (const auto& [x, y] : vec) {
        str += std::format("(({}, {}), ({}, {})), ", x.x, x.y, y.x, y.y);
    }
    str.pop_back();
    str += "]";
    return { str.begin(), str.end() };
}

template <>
std::wstring ToString<std::pair<std::size_t, std::vector<std::pair<Pos, Pos>>>>(
    const std::pair<std::size_t, std::vector<std::pair<Pos, Pos>>>& val
)
{
    std::string str{ "[ " };
    for (const auto& [x, y] : val.second) {
        str += std::format("(({}, {}), ({}, {})), ", x.x, x.y, y.x, y.y);
    }
    str.pop_back();
    str += "]";
    const auto str2 = std::format("({}, {})", val.first, str);
    return { str2.begin(), str2.end() };
}

}    // namespace Microsoft::VisualStudio::CppUnitTestFramework

namespace figure_board
{

using board_repr::BoardRepr;

namespace
{
template <typename C>
concept HasBeginEnd = requires(C c) {
    std::begin(c);
    std::end(c);
};

template <typename C, typename F>
    requires requires(C c, F f) {
        HasBeginEnd<C>;
        {
            f(c[0])
        } -> std::same_as<bool>;
    }
bool contains(const C& c, const F& f)
{
    return std::find_if(std::begin(c), std::end(c), f) != std::end(c);
}

template <class C, class T>
    requires HasBeginEnd<C> && std::same_as<T, std::remove_reference_t<decltype(std::declval<C>()[0])>>
bool contains(const C& v, const T& x)
{
    return std::find(std::begin(v), std::end(v), x) != std::end(v);
}

const auto BASE{ FromString<BoardRepr>{}("02H8W8TWC0,7,25,32!"
                                         "0.00BR1.01BH2.02BB3.03BK4.04BQ5.05BB6.06BH7.07BR8.08BP9.09BP10.10BP11.11BP13."
                                         "12BP14.13BP15.14BP16.15BP17.48WP18.49WP19.50WP20.51WP21.52WP22.53WP23.54WP24."
                                         "55WP25.56WR26.57WH27.58WB28.59WK29.60WQ30.61WB31.62WH32.63WR<><>")
                     ->value };

const auto BARS_REVERSED{
    FromString<BoardRepr>{}("02H8W8FWC0,7,25,32!"
                            "0.00WR1.01WH2.02WB3.03WQ4.04WK5.05WB6.06WH7.07WR8.08WP9.09WP10.10WP11.11WP12."
                            "12WP13.13WP14.14WP15.15WP16.48BP17.49BP18.50BP19.51BP20.52BP21.53BP22.54BP23."
                            "55BP25.56BR26.57BH27.58BB28.59BQ29.60BK30.61BB31.62BH32.63BR<><>")
        ->value
};

const BoardRepr BASE_EMPTY{ {}, Color::White, true };

const auto STANDARD_PROVIDER = [] {
    return FigureType::Queen;
};

template <typename Range, typename Fn>
void any_fmap(Range&& range, Fn&& fn)
{
    for (const auto el : std::forward<Range>(range)) {
        if (std::forward<Fn>(fn)(el)) {
            Assert::IsTrue(true);
            return;
        }
    }
    Assert::Fail();
}

template <typename Range, typename Fn>
    requires std::ranges::range<std::ranges::range_value_t<Range>>
void all_fmap(Range&& range, Fn&& fn)
{
    for (const auto el : std::forward<Range>(range)) {
        if (!std::forward<Fn>(fn)(el)) {
            Assert::Fail();
            return;    // 😀
        }
    }
    Assert::IsTrue(true);
}

template <typename T>
void assert_eq(const T& actual, const T& expected)
{
    Assert::AreEqual(expected, actual);
}

template <typename T, typename U>
    requires requires(T t, U u) { t == u; }
void assert_eq(const T& actual, const U& expected)
{
    Assert::AreEqual(expected, actual);
}

bool move_check(const std::array<const char[9], 8>& table, const Color col, const FigureType ft, const bool idw)
{
    /* ---- function constants ---- */
    const auto enemy_col = what_next(col);
    // Must not be special (eg. King)
    constexpr auto placeholder_figure_type = FigureType::Knight;

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

    for (std::size_t i = 0; i < HEIGHT; ++i) {
        std::string_view line{ table[i] };

        for (std::size_t j = 0; j < line.size(); ++j) {
            int ii = static_cast<int>(i);
            int jj = static_cast<int>(j);

            if (line[j] == move) {
                moves.emplace_back(ii, jj);
            }
            else if (line[j] == enemy_which_can_be_eaten) {
                eats.emplace_back(ii, jj);
                enemies.emplace_back(ii, jj);
            }
            else if (line[j] == enemy_which_cant_be_eaten) {
                enemies.emplace_back(ii, jj);
            }
            else if (line[j] == figure) {
                figure_pos = Pos{ ii, jj };
            }
            else if (line[j] == figure_which_cant_be_eaten) {
                figures_which_cant_be_eaten.emplace_back(ii, jj);
            }
        }
    }

    /* ---- board setup ---- */

    IdType id_tmp{ 0 };
    ChessGame b{ BoardRepr{ {}, col, idw } };

    // main figure
    b.place_fig(new Figure{ Id{ id_tmp++ }, figure_pos, col, ft });
    // enemies
    for (const auto& p : enemies) {
        b.place_fig(new Figure{ Id{ id_tmp++ }, p, enemy_col, placeholder_figure_type });
    }
    // figure which can't be eaten
    for (const auto& p : figures_which_cant_be_eaten) {
        b.place_fig(new Figure{ Id{ id_tmp++ }, p, col, placeholder_figure_type });
    }

    /* ---- move_check ---- */
    auto figure_moves = b.get_all_moves(b.get_fig(0_id).value());

    const auto is_move = [](const std::pair<bool, Pos> p) {
        return !p.first;
    };
    const auto is_eat = [](const std::pair<bool, Pos> p) {
        return p.first;
    };

    return std::ranges::all_of(
               figure_moves,
               [&](const auto& p) {
                   return (is_move(p) && contains(moves, p.second)) || (is_eat(p) && contains(eats, p.second));
               }
           ) &&
           moves.size() + eats.size() == figure_moves.size();
    // last condition is for checking if there are no extra moves
}

void move_check_assert(const std::array<const char[9], 8>& table, const Color col, const FigureType ft, const bool idw)
{
    Assert::IsTrue(move_check(table, col, ft, idw));
}

void move_check_assert(const std::array<const char[9], 8>& table, const FigureType ft)
{
    for (const auto col : { Color::White, Color::Black }) {
        for (const auto idw : { false, true }) {
            move_check_assert(table, col, ft, idw);
        }
    }
}

bool is_legal_move(Input&& move, const FigureType ft)
{
    ChessGame b{ BoardRepr{ BASE_EMPTY } };
    b.place_fig(new Figure{ 0_id, move.from, Color::White, ft });
    auto moves = b.get_all_moves(b.get_fig(0_id).value());
    return std::ranges::find_if(moves, [&](auto p) { return p.second == move.target; }) != moves.end();
}

template <bool Idw>
std::size_t moves_count(Pos&& pos, const FigureType ft)
{
    ChessGame b{ BoardRepr{ BASE_EMPTY } };
    b.set_idw(Idw);
    b.place_fig(new Figure{ 0_id, pos, Color::White, ft });
    return b.get_all_moves(b.get_fig(0_id).value()).size();
}

}    // namespace

namespace moves
{

/*
 *   0 1 2 3 4 5 6 7 →
 * 0 _ _ _ _ _ _ _ _  y
 * 1 _ _ _ _ _ _ _ _
 * 2 _ _ _ _ _ _ _ _
 * 3 _ _ _ _ _ _ _ _
 * 4 _ _ _ _ _ _ _ _
 * 5 _ _ _ _ _ _ _ _
 * 6 _ _ _ _ _ _ _ _
 * 7 _ _ _ _ _ _ _ _
 * ↓
 *  x
 */

TEST_CLASS(PawnMoves)
{
public:
    TEST_METHOD(Basic)
    {
        for (auto [c, idw] :
             std::initializer_list<std::pair<Color, bool>>{ { Color::White, true }, { Color::Black, false } })
        {
            move_check_assert(
                { "________",
                  "________",
                  "________",
                  "___kmk__",    // 2 enemies and 1 move
                  "____f___",    // current figure
                  "________",
                  "________",
                  "________" },
                c,
                FigureType::Pawn,
                idw
            );

            move_check_assert(
                { "________",
                  "________",
                  "________",
                  "_e______",    // format
                  "_me_____",
                  "kmke____",
                  "efee____",
                  "eee_____" },
                c,
                FigureType::Pawn,
                idw
            );

            move_check_assert(
                { "________",
                  "________",
                  "________",
                  "________",    // format
                  "_e______",
                  "_mk_____",
                  "_f______",
                  "________" },
                c,
                FigureType::Pawn,
                idw
            );

            move_check_assert(
                { "________",
                  "________",
                  "________",
                  "________",    // format
                  "__eee___",
                  "__kek___",
                  "__efe___",
                  "__eee___" },
                c,
                FigureType::Pawn,
                idw
            );

            move_check_assert(
                { "________",
                  "________",
                  "________",
                  "________",    // format
                  "________",
                  "__FFF___",
                  "___f____",
                  "________" },
                c,
                FigureType::Pawn,
                idw
            );
        }    // namespace moves
    }        // namespace figure_board

    TEST_METHOD(EnPassant)
    {
        ChessGame board{ BoardRepr{ { new Figure{ 0_id, Pos{ 3, 5 }, Color::White, FigureType::Pawn },
                                      new Figure{ 1_id, Pos{ 1, 6 }, Color::Black, FigureType::Pawn } },
                                    Color::Black,
                                    true } };
        const auto white_pawn = board.get_fig_unsafe(0_id);
        const auto black_pawn = board.get_fig_unsafe(1_id);
        const auto black_pawn_long_move_sus =
            board.determine_move(black_pawn, Input{ black_pawn->get_pos(), Pos{ 3, 6 } }, STANDARD_PROVIDER());
        Assert::IsTrue(black_pawn_long_move_sus.has_value());
        Assert::IsTrue(board.provide_move(*black_pawn_long_move_sus));
        Assert::AreEqual(board.get_all_moves(white_pawn).size(), 2ull);
        const auto move =
            board.determine_move(white_pawn, Input{ white_pawn->get_pos(), Pos{ 2, 6 } }, STANDARD_PROVIDER());
        Assert::IsTrue(std::holds_alternative<mvmsg::EnPassant>(move->main_event));
    }    // TEST_METHOD(EnPassant)

    TEST_METHOD(EnPassantReversed)
    {
        ChessGame board{ BoardRepr{ { new Figure{ 1_id, Pos{ 3, 5 }, Color::Black, FigureType::Pawn },
                                      new Figure{ 0_id, Pos{ 1, 6 }, Color::White, FigureType::Pawn } },
                                    Color::White,
                                    false } };
        const auto white_pawn = board.get_fig_unsafe(0_id);
        const auto black_pawn = board.get_fig_unsafe(1_id);
        const auto white_pawn_long_move_sus =
            board.determine_move(white_pawn, Input{ white_pawn->get_pos(), Pos{ 3, 6 } }, STANDARD_PROVIDER());
        Assert::IsTrue(white_pawn_long_move_sus.has_value());
        Assert::IsTrue(board.provide_move(*white_pawn_long_move_sus));
        Assert::AreEqual(board.get_all_moves(black_pawn).size(), 2ull);
        const auto move =
            board.determine_move(black_pawn, Input{ black_pawn->get_pos(), Pos{ 2, 6 } }, STANDARD_PROVIDER());
        Assert::IsTrue(std::holds_alternative<mvmsg::EnPassant>(move->main_event));
    }    // TEST_METHOD(EnPassantReversed)

    TEST_METHOD(Promotion)
    {
        for (const auto figure_type : PROMOTION_FIGURES) {
            ChessGame board{ BoardRepr{
                { new Figure{ 0_id, Pos{ 1, 5 }, Color::White, FigureType::Pawn } }, Color::White, true } };
            const auto pawn = board.get_fig_unsafe(0_id);
            Assert::AreEqual(board.get_all_moves(pawn).size(), 1ull);
            const auto move = board.determine_move(pawn, Input{ pawn->get_pos(), Pos{ 0, 5 } }, figure_type);
            Assert::IsTrue(move.has_value());
            any_fmap(move->side_evs, [](mvmsg::SideEvent sv) { return std::holds_alternative<mvmsg::Promotion>(sv); });
        }
    }    // TEST_METHOD(Promotion)

    TEST_METHOD(PromotionReversed)
    {
        for (const auto figure_type : PROMOTION_FIGURES) {
            ChessGame board{ BoardRepr{
                { new Figure{ 0_id, Pos{ 1, 5 }, Color::Black, FigureType::Pawn } }, Color::Black, false } };
            const auto pawn = board.get_fig_unsafe(0_id);
            Assert::AreEqual(board.get_all_moves(pawn).size(), 1ull);
            const auto move = board.determine_move(pawn, Input{ pawn->get_pos(), Pos{ 0, 5 } }, figure_type);
            Assert::IsTrue(move.has_value());
            any_fmap(move->side_evs, [](mvmsg::SideEvent sv) { return std::holds_alternative<mvmsg::Promotion>(sv); });
        }
    }    // TEST_METHOD(PromotionReversed)
};       // TEST_CLASS(PawnMoves)

TEST_CLASS(KnightMoves)
{
public:
    TEST_METHOD(Basic)
    {
        const std::size_t moves[][8]{ { 2, 3, 4, 4, 4, 4, 3, 2 }, { 3, 4, 6, 6, 6, 6, 4, 3 },
                                      { 4, 6, 8, 8, 8, 8, 6, 4 }, { 4, 6, 8, 8, 8, 8, 6, 4 },
                                      { 4, 6, 8, 8, 8, 8, 6, 4 }, { 4, 6, 8, 8, 8, 8, 6, 4 },
                                      { 3, 4, 6, 6, 6, 6, 4, 3 }, { 2, 3, 4, 4, 4, 4, 3, 2 } };

        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                Assert::AreEqual(moves_count<true>(Pos{ x, y }, FigureType::Knight), moves[x][y]);
                Assert::AreEqual(moves_count<false>(Pos{ x, y }, FigureType::Knight), moves[x][y]);
            }
        }
    }

    TEST_METHOD(EatMoves)
    {
        move_check_assert(
            { "________",
              "__F_F___",
              "_F___F__",
              "___f____",    // format
              "_F___F__",
              "__F_F___",
              "________",
              "________" },
            FigureType::Knight
        );

        move_check_assert(
            { "________",
              "________",
              "_FmFmF__",
              "_mFFFm__",    // format
              "_FFfFF__",
              "_mFFFm__",
              "_FmFmF__",
              "________" },
            FigureType::Knight
        );

        move_check_assert(
            { "________",
              "________",
              "_FkFkF__",
              "_kFFFk__",    // format
              "_FFfFF__",
              "_kFFFk__",
              "_FkFkF__",
              "________" },
            FigureType::Knight
        );
    }
};    // TEST_CLASS(KnightMoves)

TEST_CLASS(BishopMoves)
{
public:
    TEST_METHOD(Basic)
    {
        move_check_assert(
            { "_______m",
              "m_____m_",
              "_m___m__",
              "__m_m___",    // format
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
              "__m_m___",    // format
              "___f____",
              "__m_k___",
              "_m______",
              "k_______" },
            FigureType::Bishop
        );

        move_check_assert(
            { "___m____",
              "____m___",
              "_____m_m",
              "______f_",    // format
              "_____m_m",
              "____k___",
              "________",
              "________" },
            FigureType::Bishop
        );
    }

    TEST_METHOD(OwnBlock)
    {
        move_check_assert(
            { "________",
              "F_____F_",
              "_m___m__",
              "__m_m___",    // format
              "___f____",
              "__m_m___",
              "_F___m__",
              "______m_" },
            FigureType::Bishop
        );

        move_check_assert(
            { "f_______",
              "_m______",
              "__F_____",
              "________",    // format
              "________",
              "________",
              "________",
              "________" },
            FigureType::Bishop
        );
    }
};    // TEST_CLASS(BishopMoves)

TEST_CLASS(RookMoves)
{
public:
    TEST_METHOD(Basic)
    {
        move_check_assert(
            { "___m____",
              "___m____",
              "mmmfmmmm",
              "___m____",    // format
              "___m____",
              "___m____",
              "___m____",
              "___m____" },
            FigureType::Rook
        );

        move_check_assert(
            { "__m_____",
              "__m_____",
              "__m_____",
              "__m_____",    // format
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
              "_kfmmk__",    // format
              "__m_____",
              "__m_____",
              "__k_____",
              "________" },
            FigureType::Rook
        );

        move_check_assert(
            { "________",
              "________",
              "________",
              "________",    // format
              "________",
              "________",
              "k_______",
              "fmmmmmmk" },
            FigureType::Rook
        );
    }

    TEST_METHOD(OwnBlock)
    {
        move_check_assert(
            { "________",
              "__F_____",
              "__m_____",
              "__m_____",    // format
              "FmfF____",
              "__m_____",
              "__F_____",
              "________" },
            FigureType::Rook
        );

        move_check_assert(
            { "________",
              "________",
              "________",
              "________",    // format
              "________",
              "________",
              "F_______",
              "fmmmmmmF" },
            FigureType::Rook
        );
    }

    // castling tests are in the KingMoves test class
};    // TEST_CLASS(RookMoves)

TEST_CLASS(QueenMoves)
{
    TEST_METHOD(Basic)
    {
        move_check_assert(
            { "_m_m_m__",
              "__mmm___",
              "mmmfmmmm",
              "__mmm___",    // format
              "_m_m_m__",
              "m__m__m_",
              "___m___m",
              "___m____" },
            FigureType::Queen
        );

        move_check_assert(
            { "___m____",
              "___m____",
              "___m___m",
              "m__m__m_",    // format
              "_m_m_m__",
              "__mmm___",
              "mmmfmmmm",
              "__mmm___" },
            FigureType::Queen
        );

        move_check_assert(
            { "mmmfmmmm",
              "__mmm___",
              "_m_m_m__",
              "m__m__m_",    // format
              "___m___m",
              "___m____",
              "___m____",
              "___m____" },
            FigureType::Queen
        );

        move_check_assert(
            { "___m____",
              "___m____",
              "___m____",
              "___m___m",    // format
              "m__m__m_",
              "_m_m_m__",
              "__mmm___",
              "mmmfmmmm" },
            FigureType::Queen
        );
    }

    TEST_METHOD(EnemyBlock)
    {
        move_check_assert(
            { "_m_k_e__",
              "kmm__e__",
              "mfmmmkee",
              "kmm__e__",    // format
              "_m_k_e__",
              "_m___e__",
              "ekeeee__",
              "_e______" },
            FigureType::Queen
        );

        move_check_assert(
            { "e______e",
              "___e____",
              "__k_____",
              "___m___k",    // format
              "_e__m__m",
              "_____m_m",
              "___e__mm",
              "__kmmmmf" },
            FigureType::Queen
        );

        move_check_assert(
            { "_e___kfk",
              "_____mmm",
              "___em_m_",
              "___m__m_",    // format
              "__m___m_",
              "_k____m_",
              "e_____m_",
              "______k_" },
            FigureType::Queen
        );
    }

    TEST_METHOD(OwnBlock)
    {
        move_check_assert(
            { "_m_F_e__",
              "Fmm__e__",
              "mfmmmFee",
              "Fmm__e__",    // format
              "_m_F_e__",
              "_m___e__",
              "eFeeee__",
              "_e______" },
            FigureType::Queen
        );

        move_check_assert(
            { "e______e",
              "___e____",
              "__F_____",
              "___m___F",    // format
              "_e__m__m",
              "_____m_m",
              "___e__mm",
              "__Fmmmmf" },
            FigureType::Queen
        );

        move_check_assert(
            { "_e___Ffk",
              "_____mmm",
              "___em_m_",
              "___m__m_",    // format
              "__m___m_",
              "_F____m_",
              "e_____m_",
              "______F_" },
            FigureType::Queen
        );
    }
};    // TEST_CLASS(QueenMoves)

TEST_CLASS(KingMoves)
{
public:
    TEST_METHOD(Basic)
    {
        move_check_assert(
            { "________",
              "________",
              "___mmm__",
              "___mfm__",    // format
              "___mmm__",
              "________",
              "________",
              "________" },
            FigureType::King
        );

        move_check_assert(
            { "________",
              "________",
              "mm______",
              "fm______",    // format
              "mm______",
              "________",
              "________",
              "________" },
            FigureType::King
        );
    }

    TEST_METHOD(Eat)
    {
        move_check_assert(
            { "________",
              "________",
              "___kkk__",
              "___kfk__",    // format
              "___kkk__",
              "________",
              "________",
              "________" },
            FigureType::King
        );

        move_check_assert(
            { "________",
              "________",
              "kk______",
              "fk______",    // format
              "kk______",
              "________",
              "________",
              "________" },
            FigureType::King
        );
    }

    TEST_METHOD(SelfEat)
    {
        move_check_assert(
            { "________",
              "________",
              "___FFF__",
              "___FfF__",    // format
              "___FFF__",
              "________",
              "________",
              "________" },
            FigureType::King
        );

        move_check_assert(
            { "________",
              "________",
              "FF______",
              "fF______",    // format
              "FF______",
              "________",
              "________",
              "________" },
            FigureType::King
        );
    }

    TEST_METHOD(Castling)
    {
        std::vector<std::pair<Pos, Pos>> fails;

        // include impossible in 960-chess cases ((k=0, r=1), (k=7, r=6))
        const auto k_iota = [](bool is_left) {
            if (is_left)
                return std::views::iota(1, 8);
            return std::views::iota(0, 7);
        };
        const auto r_iota = [](bool is_left, int k) {
            if (is_left)
                return std::views::iota(0, k);
            return std::views::iota(k + 1, 8);
        };
        for (const auto idw : { false, true }) {
            const auto row = idw ? 7 : 0;
            for (const auto is_left : { false, true }) {
                for (const auto k : k_iota(is_left)) {
                    for (const auto r : r_iota(is_left, k)) {
                        const auto kp = Pos{ row, k };
                        const auto rp = Pos{ row, r };
                        ChessGame board{ BoardRepr{ { new Figure{ 0_id, kp, Color::White, FigureType::King },
                                                      new Figure{ 1_id, rp, Color::White, FigureType::Rook } },
                                                    Color::White,
                                                    idw } };

                        const auto c = Pos{ row, board.get_column_by_name('c') };
                        const auto d = Pos{ row, board.get_column_by_name('d') };
                        const auto g = Pos{ row, board.get_column_by_name('g') };
                        const auto f = Pos{ row, board.get_column_by_name('f') };

                        const auto ke = is_left ^ !idw ? c : g;
                        const auto re = is_left ^ !idw ? d : f;

                        const auto king = board.get_fig_unsafe(0_id);
                        const auto rook = board.get_fig_unsafe(1_id);
                        auto move =
                            board.provide_move(king, Input{ king->get_pos(), ke }, Color::White, STANDARD_PROVIDER);
                        // If king moves to one tile (and there is no rook), then user must move rook
                        if (std::abs(kp.y - ke.y) == 1 && rp != ke || !move) {
                            board.undo_move();
                            move = board.provide_move(rook, Input{ rp, re }, Color::White, STANDARD_PROVIDER);
                        }
                        if (!move || king->get_pos() != ke || rook->get_pos() != re) {
                            fails.emplace_back(kp, rp);
                        }
                    }
                }
            }
        }
        assert_eq(std::pair{ fails.size(), fails }, { 0, {} });
    }    // TEST_METHOD(Castling)

};    // TEST_CLASS(KingMoves)

}    // namespace moves

TEST_CLASS(Utils)
{
    TEST_METHOD(GetColumnByName)
    {
        {
            ChessGame board{ BoardRepr{ {}, Color::White, true } };
            Assert::AreEqual(6, board.get_column_by_name('g'));
            Assert::AreEqual(6, board.get_column_by_name('G'));
            Assert::AreEqual(5, board.get_column_by_name('f'));
            Assert::AreEqual(5, board.get_column_by_name('F'));
            Assert::AreEqual(2, board.get_column_by_name('c'));
            Assert::AreEqual(2, board.get_column_by_name('C'));
            Assert::AreEqual(3, board.get_column_by_name('d'));
            Assert::AreEqual(3, board.get_column_by_name('D'));
        }
        {
            ChessGame board{ BoardRepr{ {}, Color::White, false } };
            Assert::AreEqual(1, board.get_column_by_name('g'));
            Assert::AreEqual(1, board.get_column_by_name('G'));
            Assert::AreEqual(2, board.get_column_by_name('f'));
            Assert::AreEqual(2, board.get_column_by_name('F'));
            Assert::AreEqual(5, board.get_column_by_name('c'));
            Assert::AreEqual(5, board.get_column_by_name('C'));
            Assert::AreEqual(4, board.get_column_by_name('d'));
            Assert::AreEqual(4, board.get_column_by_name('D'));
        }
    }    // TEST_METHOD(GetColumnByName)
};       // TEST_CLASS(Utils)

}    // namespace figure_board
