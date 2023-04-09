#pragma once

#include "macro.h"

#include <charconv>
#include <vector>
#include <optional>
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

[[nodiscard]] inline constexpr auto split(const std::string_view str, const std::string_view delimiter) noexcept -> std::vector<std::string_view> {
    std::vector<std::string_view> acc{};
    std::size_t current{}, previous{};
    while ((current = str.find(delimiter, previous)) != std::string_view::npos) {
        acc.push_back(str.substr(previous, current - previous));
        previous = current + delimiter.size();
    }
    acc.push_back(str.substr(previous, current - previous));
    return acc;
}

[[nodiscard]] inline constexpr auto svtoi(const std::string_view s) noexcept -> std::optional<int>
{
    if (int value; std::from_chars(s.data(), s.data() + s.size(), value).ec == std::errc{})
        return value;
    else
        return std::nullopt;
};
