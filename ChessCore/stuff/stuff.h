#pragma once

#include <new>
#include <map>
#include <set>
#include <list>
#include <tuple>
#include <format>
#include <vector>
#include <compare>
#include <cassert>
#include <optional>
#include <functional>

template <class T>
[[nodiscard]] std::vector<T> operator +(std::vector<T> vec, T val) {
    vec.push_back(val);
    return vec;
}

template <class T>
[[nodiscard]] std::vector<T> operator +(const std::vector<T>& l, const std::vector<T>& r) {
    auto tmp = l;
    tmp.insert(tmp.end(), r.begin(), r.end());
    return tmp;
}

[[nodiscard]] constexpr auto split(const std::string_view str, const std::string_view delimiter) noexcept -> std::vector<std::string_view> {
    std::vector<std::string_view> acc{};
    std::size_t current{}, previous{};
    while ((current = str.find(delimiter, previous)) != std::string_view::npos) {
        acc.push_back(str.substr(previous, current - previous));
        previous = current + delimiter.size();
    }
    acc.push_back(str.substr(previous, current - previous));
    return acc;
}

[[nodiscard]] constexpr auto svtoi(const std::string_view s) noexcept -> std::optional<int>
{
    if (int value; std::from_chars(s.data(), s.data() + s.size(), value).ec == std::errc{})
        return value;
    else
        return std::nullopt;
};

#pragma region CEXAU

// count arguments
#ifdef _MSC_VER
#    define CEXAU_ARG_COUNT(...) \
         CEXAU_INTERNAL_EXPAND_ARGS_PRIVATE( \
             CEXAU_INTERNAL_ARGS_AUGMENTER(__VA_ARGS__) \
         )
#    define CEXAU_INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#    define CEXAU_INTERNAL_EXPAND(x) x
#    define CEXAU_INTERNAL_EXPAND_ARGS_PRIVATE(...) \
         CEXAU_INTERNAL_EXPAND( \
             CEXAU_INTERNAL_GET_ARG_COUNT_PRIVATE( \
                 __VA_ARGS__, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 \
             ) \
         )
#    define CEXAU_INTERNAL_GET_ARG_COUNT_PRIVATE( \
         _1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, _10_, _11_, _12_, _13_, _14_, _15_, _16_, _17_, _18_, _19_, _20_, _21_, _22_, _23_, _24_, _25_, _26_, _27_, _28_, _29_, _30_, _31_, _32_, _33_, _34_, _35_, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, \
         count, ... \
     ) count
#else
#warning "CEXAU_ARG_COUNT is not implemented for this compiler"
#endif

// utility (concatenation)
#define CEXAU_CONC(A, B) CEXAU_CONC_(A, B)
#define CEXAU_CONC_(A, B) A##B

#define CEXAU_GET_LAST_GET_ELEM_N(N, ...) \
    CEXAU_CONC(CEXAU_GET_LAST_GET_ELEM_, N)(__VA_ARGS__)
#define CEXAU_GET_LAST_GET_ELEM_0(_0, ...) _0
#define CEXAU_GET_LAST_GET_ELEM_1(_0, _1, ...) _1
#define CEXAU_GET_LAST_GET_ELEM_2(_0, _1, _2, ...) _2
#define CEXAU_GET_LAST_GET_ELEM_3(_0, _1, _2, _3, ...) _3
#define CEXAU_GET_LAST_GET_ELEM_4(_0, _1, _2, _3, _4, ...) _4
#define CEXAU_GET_LAST_GET_ELEM_5(_0, _1, _2, _3, _4, _5, ...) _5
#define CEXAU_GET_LAST_GET_ELEM_6(_0, _1, _2, _3, _4, _5, _6, ...) _6
#define CEXAU_GET_LAST_GET_ELEM_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _7
#define CEXAU_GET_LAST_GET_ELEM_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define CEXAU_GET_LAST_GET_ELEM_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define CEXAU_GET_LAST_GET_ELEM_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _10

#define CEXAU_GET_LAST(...) \
    CEXAU_GET_LAST_GET_ELEM_N(CEXAU_ARG_COUNT(__VA_ARGS__), _, __VA_ARGS__ ,,,,,,,,,,,)

#define CEXAU_MAP_LINES_EXCEPT_LAST_N(N, ...) \
    CEXAU_CONC(CEXAU_MAP_LINES_EXCEPT_LAST_, N)(__VA_ARGS__)
#define  CEXAU_MAP_LINES_EXCEPT_LAST_0(_0, ...)
#define  CEXAU_MAP_LINES_EXCEPT_LAST_1(_0, _1, ...)                                     
#define  CEXAU_MAP_LINES_EXCEPT_LAST_2(_0, _1, _2, ...) _0;
#define  CEXAU_MAP_LINES_EXCEPT_LAST_3(_0, _1, _2, _3, ...) _0; _1;
#define  CEXAU_MAP_LINES_EXCEPT_LAST_4(_0, _1, _2, _3, _4, ...) _0; _1; _2;
#define  CEXAU_MAP_LINES_EXCEPT_LAST_5(_0, _1, _2, _3, _4, _5, ...) _0; _1; _2; _3;
#define  CEXAU_MAP_LINES_EXCEPT_LAST_6(_0, _1, _2, _3, _4, _5, _6, ...) _0; _1; _2; _3; _4;
#define  CEXAU_MAP_LINES_EXCEPT_LAST_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _0; _1; _2; _3; _4; _5;
#define  CEXAU_MAP_LINES_EXCEPT_LAST_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _0; _1; _2; _3; _4; _5; _6;
#define  CEXAU_MAP_LINES_EXCEPT_LAST_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _0; _1; _2; _3; _4; _5; _6; _7;
#define CEXAU_MAP_LINES_EXCEPT_LAST_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _0; _1; _2; _3; _4; _5; _6; _7; _8;

#define CEXAU_MAP_LINES_EXCEPT_LAST(...) \
    CEXAU_MAP_LINES_EXCEPT_LAST_N(CEXAU_ARG_COUNT(__VA_ARGS__), __VA_ARGS__ ,,,,,,,,,,,)

/// <summary> 
///     for quick function tests using visual studio cpp interpreter
/// </summary>
/// <example> 
///     CEXAU test_val = CEXAUQKT(1 + 2);
/// </example>
/// <example>
///     CEXAU test_val = CEXAUQKT(
///         int a = 2,
///         int b = 3,
///         a + b
///     );
/// </example>
#define CEXAU inline constexpr auto
#define CEXAU_CONSTEVAL_EXECUTE_ONE_LINE(return_line) \
    ([&]{ return (return_line); })()
#define CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE(...)  \
    ([&] {                                       \
        CEXAU_MAP_LINES_EXCEPT_LAST(__VA_ARGS__) \
        return CEXAU_GET_LAST(__VA_ARGS__);      \
    })()
/// macro overload
#define CEXAU_EXPAND(x) x
#define CEXAU_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME
#define CEXAUQKT(...)                           \
    CEXAU_EXPAND(                               \
        CEXAU_GET_MACRO(                        \
            __VA_ARGS__,                        \
            CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, \
            CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, \
            CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, \
            CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, \
            CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, \
            CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, \
            CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, \
            CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, \
            CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, \
            CEXAU_CONSTEVAL_EXECUTE_ONE_LINE    \
        )(__VA_ARGS__)                          \
    )

#pragma endregion

#define FN [[nodiscard]] constexpr auto
#define CTOR [[nodiscard]] constexpr explicit

constexpr int EN_PASSANT_INDENT = 4;

enum class GameEndType {
    Checkmate,
    FiftyRule,
    Stalemate, 
    InsufficientMaterial, 
    MoveRepeat,
    NotGameEnd
};

typedef int Id;
const Id ERR_ID{ -1 };
const int HEIGHT{ 8 };  // this definitely shouldn't be here (todo)
const int WIDTH{ 8 };

struct Pos {
    int x{ -1 };
    int y{ -1 };
    
    constexpr Pos() noexcept
        {};
    
    constexpr Pos(int x, int y) noexcept
        : x(x)
        , y(y) {};
    
    constexpr auto operator <=> (const Pos& other) const noexcept = default;
    
    [[nodiscard]] constexpr Pos operator-(const Pos& right) const noexcept
        { return { (x - right.x), (y - right.y) }; }
    
    [[nodiscard]] constexpr Pos operator+(const Pos& right) const noexcept
        { return { (x + right.x), (y + right.y) }; }
    
    constexpr Pos& operator +=(const Pos& r) noexcept
        { this->x += r.x; this->y += r.y; return *this; }
    
    [[nodiscard]] constexpr Pos mul_x(int mx) const noexcept
        { return { x * mx, y }; }
    
    constexpr void loop_add(Pos add, int max_x, int max_y) noexcept {
        this->x += add.x; this->y += add.y;
        if (this->x >= max_x) this->x = 0;
        if (this->x < 0) this->x = max_x - 1;
        if (this->y >= max_y) this->y = 0;
        if (this->y < 0) this->y = max_y - 1;
    }
    
    constexpr Pos& change_axes() noexcept
        { std::swap(x, y); return *this; }
    
    constexpr bool in(const std::vector<Pos>& lst) const noexcept 
        { return std::find(lst.cbegin(), lst.cend(), *this) != lst.cend(); }
    
};

[[nodiscard]] constexpr inline Pos change_axes(const Pos& val) noexcept {
    return { val.y, val.x };
}

enum class Color { 
    None, 
    Black, 
    White 
};

[[nodiscard]] constexpr Color char_to_col(const char ch) noexcept 
{
    switch (ch) {
        case 'W': case 'w': return Color::White;
        case 'B': case 'b': return Color::Black;
        case 'N': case 'n': return Color::None;
    }
    assert(!"char_to_col: invalid character");
    return Color::None;
}

[[nodiscard]] constexpr char col_to_char(const Color col) noexcept 
{
    switch (col) {
        case Color::Black: return 'B';
        case Color::White: return 'W';
        case Color::None: return 'N';
    }
    assert(!"col_to_char: invalid color");
    return 'E';
}

[[nodiscard]] constexpr Color what_next(const Color col) noexcept
{
    switch (col) {
        case Color::Black:
            return Color::White;
        case Color::White:
            return Color::Black;
    }
    assert(!"what_next: invalid color");
    return Color::None; // Perhaps there is a better way
}

inline const std::set<Color> PLAYABLE_COLORS { 
    Color::White, 
    Color::Black,
};

enum class FigureType { 
    None, 
    Pawn, 
    Knight, 
    Rook, 
    Bishop, 
    Queen, 
    King,
};

[[nodiscard]] constexpr FigureType char_to_figure_type(const char ch) noexcept
{
    switch (ch)
    {
        case 'K': case 'k': return FigureType::King;
        case 'H': case 'h': return FigureType::Knight;
        case 'P': case 'p': return FigureType::Pawn;
        case 'B': case 'b': return FigureType::Bishop;
        case 'Q': case 'q': return FigureType::Queen;
        case 'R': case 'r': return FigureType::Rook;
        case 'N': case 'n': return FigureType::None;
    }
    assert(!"char_to_figure_type: invalid character");
    return FigureType::None;
}

[[nodiscard]] constexpr char figure_type_to_char(const FigureType ft) noexcept
{
    switch (ft) {
        case FigureType::Pawn: return 'P';
        case FigureType::Rook: return 'R';
        case FigureType::Knight: return 'H';
        case FigureType::Bishop: return 'B';
        case FigureType::Queen: return 'Q';
        case FigureType::King: return 'K';
        case FigureType::None: return 'N';
    }
    assert(!"figure_type_to_char: invalid figure type");
    return 'E';
}

inline const std::set<FigureType> PLAYABLE_FIGURES {
    FigureType::Pawn, 
    FigureType::Knight, 
    FigureType::Rook, 
    FigureType::Bishop, 
    FigureType::Queen, 
    FigureType::King,
};

inline const std::set<FigureType> PROMOTION_FIGURES {
    FigureType::Knight, 
    FigureType::Rook, 
    FigureType::Bishop, 
    FigureType::Queen,
};

struct Input {
    Pos from{ };
    Pos target{ };
    
    [[nodiscard]] Input(std::string);
    [[nodiscard]] constexpr Input() noexcept = default;
        
    [[nodiscard]] constexpr Input(const Pos& from, const Pos& target) noexcept
        : from{ from }
        , target{ target } {}
};

enum class ErrorEvent { INVALID_MOVE, UNDER_CHECK, CHECK_IN_THAT_TILE, UNFORESEEN};
enum class MainEvent { E, EAT, MOVE, LMOVE, CASTLING, EN_PASSANT };
enum class SideEvent { E, CHECK, PROMOTION, CASTLING_BREAK };

[[nodiscard]] std::string to_string(SideEvent side_event) noexcept;
[[nodiscard]] std::wstring to_wstring(SideEvent side_event) noexcept;
[[nodiscard]] std::string to_string(MainEvent main_event) noexcept;
[[nodiscard]] std::wstring to_wstring(MainEvent main_event) noexcept;
[[nodiscard]] std::string to_string(FigureType) noexcept;
[[nodiscard]] std::wstring to_wstring(FigureType) noexcept;

struct MoveMessage {
    MainEvent main_ev{ MainEvent::E };
    std::vector<SideEvent> side_evs;
    std::vector<Id> to_eat;
    std::vector<std::pair<Id, Input>> to_move;
    std::vector<Id> what_castling_breaks;
};
