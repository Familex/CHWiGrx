#pragma once

#ifndef _DEBUG
#pragma message("cexau.h included here. Remove before release.")
#endif

#ifdef _MSC_VER

/// FIXME clang-format.

// count arguments
#define CEXAU_ARG_COUNT(...) CEXAU_INTERNAL_EXPAND_ARGS_PRIVATE(CEXAU_INTERNAL_ARGS_AUGMENTER(__VA_ARGS__))
#define CEXAU_INTERNAL_ARGS_AUGMENTER(...) unused, __VA_ARGS__
#define CEXAU_INTERNAL_EXPAND(x) x
#define CEXAU_INTERNAL_EXPAND_ARGS_PRIVATE(...)                                                                        \
    CEXAU_INTERNAL_EXPAND(CEXAU_INTERNAL_GET_ARG_COUNT_PRIVATE(                                                        \
        __VA_ARGS__,                                                                                                   \
        69,                                                                                                            \
        68,                                                                                                            \
        67,                                                                                                            \
        66,                                                                                                            \
        65,                                                                                                            \
        64,                                                                                                            \
        63,                                                                                                            \
        62,                                                                                                            \
        61,                                                                                                            \
        60,                                                                                                            \
        59,                                                                                                            \
        58,                                                                                                            \
        57,                                                                                                            \
        56,                                                                                                            \
        55,                                                                                                            \
        54,                                                                                                            \
        53,                                                                                                            \
        52,                                                                                                            \
        51,                                                                                                            \
        50,                                                                                                            \
        49,                                                                                                            \
        48,                                                                                                            \
        47,                                                                                                            \
        46,                                                                                                            \
        45,                                                                                                            \
        44,                                                                                                            \
        43,                                                                                                            \
        42,                                                                                                            \
        41,                                                                                                            \
        40,                                                                                                            \
        39,                                                                                                            \
        38,                                                                                                            \
        37,                                                                                                            \
        36,                                                                                                            \
        35,                                                                                                            \
        34,                                                                                                            \
        33,                                                                                                            \
        32,                                                                                                            \
        31,                                                                                                            \
        30,                                                                                                            \
        29,                                                                                                            \
        28,                                                                                                            \
        27,                                                                                                            \
        26,                                                                                                            \
        25,                                                                                                            \
        24,                                                                                                            \
        23,                                                                                                            \
        22,                                                                                                            \
        21,                                                                                                            \
        20,                                                                                                            \
        19,                                                                                                            \
        18,                                                                                                            \
        17,                                                                                                            \
        16,                                                                                                            \
        15,                                                                                                            \
        14,                                                                                                            \
        13,                                                                                                            \
        12,                                                                                                            \
        11,                                                                                                            \
        10,                                                                                                            \
        9,                                                                                                             \
        8,                                                                                                             \
        7,                                                                                                             \
        6,                                                                                                             \
        5,                                                                                                             \
        4,                                                                                                             \
        3,                                                                                                             \
        2,                                                                                                             \
        1,                                                                                                             \
        0                                                                                                              \
    ))
#define CEXAU_INTERNAL_GET_ARG_COUNT_PRIVATE(                                                                          \
    _1_,                                                                                                               \
    _2_,                                                                                                               \
    _3_,                                                                                                               \
    _4_,                                                                                                               \
    _5_,                                                                                                               \
    _6_,                                                                                                               \
    _7_,                                                                                                               \
    _8_,                                                                                                               \
    _9_,                                                                                                               \
    _10_,                                                                                                              \
    _11_,                                                                                                              \
    _12_,                                                                                                              \
    _13_,                                                                                                              \
    _14_,                                                                                                              \
    _15_,                                                                                                              \
    _16_,                                                                                                              \
    _17_,                                                                                                              \
    _18_,                                                                                                              \
    _19_,                                                                                                              \
    _20_,                                                                                                              \
    _21_,                                                                                                              \
    _22_,                                                                                                              \
    _23_,                                                                                                              \
    _24_,                                                                                                              \
    _25_,                                                                                                              \
    _26_,                                                                                                              \
    _27_,                                                                                                              \
    _28_,                                                                                                              \
    _29_,                                                                                                              \
    _30_,                                                                                                              \
    _31_,                                                                                                              \
    _32_,                                                                                                              \
    _33_,                                                                                                              \
    _34_,                                                                                                              \
    _35_,                                                                                                              \
    _36,                                                                                                               \
    _37,                                                                                                               \
    _38,                                                                                                               \
    _39,                                                                                                               \
    _40,                                                                                                               \
    _41,                                                                                                               \
    _42,                                                                                                               \
    _43,                                                                                                               \
    _44,                                                                                                               \
    _45,                                                                                                               \
    _46,                                                                                                               \
    _47,                                                                                                               \
    _48,                                                                                                               \
    _49,                                                                                                               \
    _50,                                                                                                               \
    _51,                                                                                                               \
    _52,                                                                                                               \
    _53,                                                                                                               \
    _54,                                                                                                               \
    _55,                                                                                                               \
    _56,                                                                                                               \
    _57,                                                                                                               \
    _58,                                                                                                               \
    _59,                                                                                                               \
    _60,                                                                                                               \
    _61,                                                                                                               \
    _62,                                                                                                               \
    _63,                                                                                                               \
    _64,                                                                                                               \
    _65,                                                                                                               \
    _66,                                                                                                               \
    _67,                                                                                                               \
    _68,                                                                                                               \
    _69,                                                                                                               \
    _70,                                                                                                               \
    count,                                                                                                             \
    ...                                                                                                                \
)                                                                                                                      \
    count

// utility (concatenation)
#define CEXAU_CONC(A, B) CEXAU_CONC_(A, B)
#define CEXAU_CONC_(A, B) A##B

#define CEXAU_GET_LAST_GET_ELEM_N(N, ...) CEXAU_CONC(CEXAU_GET_LAST_GET_ELEM_, N)(__VA_ARGS__)
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

#define CEXAU_GET_LAST(...)                                                                                            \
    CEXAU_GET_LAST_GET_ELEM_N(CEXAU_ARG_COUNT(__VA_ARGS__), _, __VA_ARGS__, , , , , , , , , , , )

#define CEXAU_MAP_LINES_EXCEPT_LAST_N(N, ...) CEXAU_CONC(CEXAU_MAP_LINES_EXCEPT_LAST_, N)(__VA_ARGS__)
#define CEXAU_MAP_LINES_EXCEPT_LAST_0(_0, ...)
#define CEXAU_MAP_LINES_EXCEPT_LAST_1(_0, _1, ...)
#define CEXAU_MAP_LINES_EXCEPT_LAST_2(_0, _1, _2, ...) _0;
#define CEXAU_MAP_LINES_EXCEPT_LAST_3(_0, _1, _2, _3, ...)                                                             \
    _0;                                                                                                                \
    _1;
#define CEXAU_MAP_LINES_EXCEPT_LAST_4(_0, _1, _2, _3, _4, ...)                                                         \
    _0;                                                                                                                \
    _1;                                                                                                                \
    _2;
#define CEXAU_MAP_LINES_EXCEPT_LAST_5(_0, _1, _2, _3, _4, _5, ...)                                                     \
    _0;                                                                                                                \
    _1;                                                                                                                \
    _2;                                                                                                                \
    _3;
#define CEXAU_MAP_LINES_EXCEPT_LAST_6(_0, _1, _2, _3, _4, _5, _6, ...)                                                 \
    _0;                                                                                                                \
    _1;                                                                                                                \
    _2;                                                                                                                \
    _3;                                                                                                                \
    _4;
#define CEXAU_MAP_LINES_EXCEPT_LAST_7(_0, _1, _2, _3, _4, _5, _6, _7, ...)                                             \
    _0;                                                                                                                \
    _1;                                                                                                                \
    _2;                                                                                                                \
    _3;                                                                                                                \
    _4;                                                                                                                \
    _5;
#define CEXAU_MAP_LINES_EXCEPT_LAST_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...)                                         \
    _0;                                                                                                                \
    _1;                                                                                                                \
    _2;                                                                                                                \
    _3;                                                                                                                \
    _4;                                                                                                                \
    _5;                                                                                                                \
    _6;
#define CEXAU_MAP_LINES_EXCEPT_LAST_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...)                                     \
    _0;                                                                                                                \
    _1;                                                                                                                \
    _2;                                                                                                                \
    _3;                                                                                                                \
    _4;                                                                                                                \
    _5;                                                                                                                \
    _6;                                                                                                                \
    _7;
#define CEXAU_MAP_LINES_EXCEPT_LAST_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...)                               \
    _0;                                                                                                                \
    _1;                                                                                                                \
    _2;                                                                                                                \
    _3;                                                                                                                \
    _4;                                                                                                                \
    _5;                                                                                                                \
    _6;                                                                                                                \
    _7;                                                                                                                \
    _8;

#define CEXAU_MAP_LINES_EXCEPT_LAST(...)                                                                               \
    CEXAU_MAP_LINES_EXCEPT_LAST_N(CEXAU_ARG_COUNT(__VA_ARGS__), __VA_ARGS__, , , , , , , , , , , )

/**
 * \brief for quick function tests using visual studio cpp interpreter
 * \example CEXAU test_val = CEXAUQKT(1 + 2);
 * \example
 * CEXAU test_val = CEXAUQKT( \n
 *     int a = 2, \n
 *     int b = 3, \n
 *     a + b
 * );
 */
#define CEXAU inline constexpr auto
#define CEXAU_CONSTEVAL_EXECUTE_ONE_LINE(return_line) ([&]() constexpr -> auto { return (return_line); })()
#define CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE(...)                                                                        \
    ([&]() constexpr -> auto {                                                                                         \
        CEXAU_MAP_LINES_EXCEPT_LAST(__VA_ARGS__)                                                                       \
        return CEXAU_GET_LAST(__VA_ARGS__);                                                                            \
    })()
/// macro overload
#define CEXAU_EXPAND(x) x
#define CEXAU_GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME
#define CEXAUQKT(...)                                                                                                                                                                                                                                                                                                                                                                                       \
    CEXAU_EXPAND(                                                                                                                                                                                                                                                                                                                                                                                           \
        CEXAU_GET_MACRO(__VA_ARGS__, CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, CEXAU_CONSTEVAL_EXECUTE_MULTI_LINE, CEXAU_CONSTEVAL_EXECUTE_ONE_LINE)( \
            __VA_ARGS__                                                                                                                                                                                                                                                                                                                                                                                     \
        )                                                                                                                                                                                                                                                                                                                                                                                                   \
    )

#endif    // _MSC_VER
