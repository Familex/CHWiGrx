#pragma once

#include <variant>

template <typename... Ts>
struct Overload : Ts...
{
    using Ts::operator()...;
};

template <typename T, typename... Fs>
constexpr auto visit_one(T&& value, Fs&&... overloads) noexcept
{
    return std::visit(Overload{ std::forward<Fs>(overloads)... }, std::forward<T>(value));
}
