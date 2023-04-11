#pragma once

#include "../stuff/macro.h"
#include "../stuff/parsing.hpp"
#include "../stuff/parse_meta.hpp"

#include <vector>
#include <string>

constexpr int HEIGHT{ 8 };  // this definitely shouldn't be here (FIXME)
constexpr int WIDTH{ 8 };   // this definitely shouldn't be here (FIXME)

/// <summary>
///   x-axis from top  to bottom (↓)
///   y-axis from left to right  (→)
/// </summary>
struct Pos {
    int x{ -1 };
    int y{ -1 };

    friend from_string<Pos>;
    friend as_string<Pos>;

    CTOR Pos() noexcept {};

    CTOR Pos(int x, int y) noexcept
        : x(x)
        , y(y) 
    {};

    constexpr auto operator <=> (const Pos& other) const noexcept = default;

    FN operator-(const Pos& right) const noexcept -> Pos
    {
        return Pos{ (x - right.x), (y - right.y) };
    }

    FN operator+(const Pos& right) const noexcept -> Pos
    {
        return Pos{ (x + right.x), (y + right.y) };
    }

    constexpr auto operator +=(const Pos& r) noexcept -> Pos&
    {
        this->x += r.x; this->y += r.y; return *this;
    }

    FN mul_x(int mx) const noexcept -> Pos
    {
        return Pos{ x * mx, y };
    }

    FN loop_add(Pos add, int max_x, int max_y) noexcept -> void
    {
        this->x += add.x; this->y += add.y;
        if (this->x >= max_x) this->x = 0;
        if (this->x < 0) this->x = max_x - 1;
        if (this->y >= max_y) this->y = 0;
        if (this->y < 0) this->y = max_y - 1;
    }

    constexpr auto change_axes() noexcept -> Pos&
    {
        std::swap(x, y); return *this;
    }

    FN in(const std::vector<Pos>& lst) const noexcept -> bool
    {
        return std::find(lst.cbegin(), lst.cend(), *this) != lst.cend();
    }
};

FN inline change_axes(const Pos& val) noexcept -> Pos {
    return Pos{ val.y, val.x };
}

template <>
struct from_string<Pos> {
    [[nodiscard]] inline constexpr auto
        operator()(const std::string_view str, const FromStringMeta& meta) const noexcept
        -> std::expected<ParseResult<Pos>, std::size_t>
    {
        const auto payload_sus = svtoi(str);
        if (!payload_sus) {
            return std::unexpected{ payload_sus.error() };
        };
        const auto y = payload_sus.value().value % meta.width;
        const auto x = (payload_sus.value().value - y) / meta.width;
        return { { Pos{ static_cast<int>(x), static_cast<int>(y) }, payload_sus.value().position } };
    }
};

template <>
struct as_string<Pos> {
    [[nodiscard]] inline auto
        operator()(const Pos& pos, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        const auto payload = std::to_string(pos.y + pos.x * WIDTH);
        return std::string(meta.max_pos_length - payload.length(), '0') + payload;
    }
};
