#pragma once

#include <variant>

template<typename... Ts>
struct Overload : Ts...
{
    using Ts::operator()...;
};
template<typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

/// VISIT macro for simple usage of Overload pattern
/// <example>
/// std::variant<int, double> smth { 1.0 };
///
/// auto res = VISIT(smth,
///     [] (int i) { return i * 2.0; },
///     [] (double d) { return d * 0.7; }
/// );
/// assert(res == 0.7);
/// </example
#define VISIT(variants, ...) std::visit(Overload { __VA_ARGS__ }, variants)
