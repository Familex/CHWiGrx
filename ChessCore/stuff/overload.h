#pragma once

#include <variant>

template <typename ...Ts> struct overload : Ts... { using Ts::operator()...; };
template <typename ...Ts> overload(Ts...) -> overload<Ts...>;

/// OVERLOAD macro for simple usage of overload pattern
/// <example>
/// std::variant<int, double> smth { 1.0 };
/// 
/// auto res = OVERLOAD(smth,
///     [] (int i) { return i * 2.0; },
///     [] (double d) { return d * 0.7; }
/// );
/// assert(res == 0.7);
/// </example
#define OVERLOAD(variants, ...) \
    std::visit(                 \
        overload{               \
            __VA_ARGS__         \
        },                      \
        variants                \
    )
