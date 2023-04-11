#pragma once

#include "macro.h"
#include "parsing.hpp"

#include <charconv>
#include <vector>
#include <expected>
#include <string>

using namespace std::string_literals;
using namespace std::string_view_literals;

template <class T>
FN inline operator +(std::vector<T> vec, T val) noexcept -> std::vector<T> {
    vec.push_back(val);
    return vec;
}

template <class T>
FN inline operator +(const std::vector<T>& l, const std::vector<T>& r) noexcept -> std::vector<T> {
    std::vector<T> tmp = l;
    tmp.insert(tmp.end(), r.begin(), r.end());
    return tmp;
}

FN inline
    split(const std::string_view str, const std::string_view delimiter) noexcept
    -> std::vector<std::string_view>
{
    std::vector<std::string_view> acc{};
    std::size_t current{}, previous{};
    while ((current = str.find(delimiter, previous)) != std::string_view::npos) {
        acc.push_back(str.substr(previous, current - previous));
        previous = current + delimiter.size();
    }
    const auto last = str.substr(previous);
    if (!last.empty()) {
        acc.push_back(last);
    }
    return acc;
}

/// Converts a string_view to an int if all string is convertable.
/// Returns next character position on success.
/// Returns invalid character position on error.
FN inline
    svtoi(const std::string_view s) noexcept
    -> std::expected<ParseResult<int>, std::size_t>
{
    int value{ };
    const auto res = std::from_chars(s.data(), s.data() + s.size(), value);
    const auto pos = static_cast<std::size_t>(res.ptr - s.data());
    if (res.ec == std::errc{}) {
        return { { value, pos } };
    }
    else {
        return std::unexpected{ pos };
    }
};
