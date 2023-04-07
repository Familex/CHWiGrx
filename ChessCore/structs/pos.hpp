#pragma once

#include "../stuff/macro.h"

#include <vector>
#include <string>

constexpr int HEIGHT{ 8 };  // this definitely shouldn't be here (FIXME)
constexpr int WIDTH{ 8 };

/// <summary>
///   x-axis from top  to bottom (↓)
///   y-axis from left to right  (→)
/// </summary>
struct Pos {
    int x{ -1 };
    int y{ -1 };

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

    [[nodiscard]] auto
        as_string() const noexcept -> std::string
    {
        static const auto MAX_SIZE{ std::to_string(HEIGHT * WIDTH).length() };
        const auto payload = std::to_string(y + x * WIDTH);
        return std::string(MAX_SIZE - payload.length(), '0') + payload;
    }
};

inline FN change_axes(const Pos& val) noexcept -> Pos {
    return Pos{ val.y, val.x };
}
