#pragma once

#include "macro.h"
#include "parsing.hpp"

#include <charconv>
#include <expected>
#include <ranges>
#include <string>
#include <vector>

using namespace std::string_literals;
using namespace std::string_view_literals;

template <class T>
FN inline operator+(std::vector<T> vec, T val) noexcept -> std::vector<T>
{
    vec.push_back(val);
    return vec;
}

template <class T>
FN inline
operator+(const std::vector<T>& l, const std::vector<T>& r) noexcept -> std::vector<T>
{
    std::vector<T> tmp = l;
    tmp.insert(tmp.end(), r.begin(), r.end());
    return tmp;
}

FN inline split(const std::string_view str, const std::string_view delimiter) noexcept
    -> std::vector<std::string_view>
{
    std::vector<std::string_view> acc{};
    std::size_t current{}, previous{};
    while ((current = str.find(delimiter, previous)) != std::string_view::npos) {
        acc.push_back(str.substr(previous, current - previous));
        previous = current + delimiter.size();
    }
    if (const auto last = str.substr(previous); !last.empty()) {
        acc.push_back(last);
    }
    return acc;
}

/**
 * \brief Converts a string_view to an int if all string is convertible.
 * \param s string_view to convert.
 * \return Invalid character position on error and next character position on success.
 */
FN inline svtoi(const std::string_view s) noexcept
    -> std::expected<ParseResult<int>, std::size_t>
{
    int value{};
    const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    const auto pos = static_cast<std::size_t>(ptr - s.data());
    if (ec == std::errc{}) {
        return { { value, pos } };
    }
    else {
        return std::unexpected{ pos };
    }
}

/**
 * \brief Inclusive std::views::iota without runtime error
 * \param a first bound
 * \param b second bound
 * \return (a..=b) if a <= b and (b..=a) otherwise
 */
template <typename T>
FN inline between(T a, T b) noexcept
{
    if (a > b) {
        return std::views::iota(b, a + 1);
    }
    return std::views::iota(a, b + 1);
}
