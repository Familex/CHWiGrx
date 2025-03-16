#pragma once

#include "macro.h"

#include <utility>    // std::swap

template <class Tag, typename T>
class StrongTypedef
{
    T value_;

public:
    CTOR StrongTypedef()
      : value_()
    { }

    CTOR StrongTypedef(const T& value)
      : value_(value)
    { }

    CTOR StrongTypedef(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
      : value_(std::move(value))
    { }

    [[nodiscard]] explicit constexpr operator T&() noexcept { return value_; }

    [[nodiscard]] explicit constexpr operator const T&() const noexcept { return value_; }

    FN friend swap(StrongTypedef& a, StrongTypedef& b) noexcept
    {
        using std::swap;
        swap(static_cast<T&>(a), static_cast<T&>(b));
    }
};

namespace strong_typedef_utils
{
template <class StrongTypedef, class UnderlyingType>
struct Addition
{
    constexpr friend StrongTypedef& operator+=(StrongTypedef& lhs, const StrongTypedef& rhs)
    {
        static_cast<UnderlyingType&>(lhs) += static_cast<const UnderlyingType&>(rhs);
        return lhs;
    }

    [[nodiscard]] constexpr friend StrongTypedef operator+(const StrongTypedef& lhs, const StrongTypedef& rhs)
    {
        return StrongTypedef(static_cast<const UnderlyingType&>(lhs) + static_cast<const UnderlyingType&>(rhs));
    }

    constexpr friend StrongTypedef operator++(StrongTypedef& lhs)
    {
        ++static_cast<UnderlyingType&>(lhs);
        return lhs;
    }

    constexpr friend StrongTypedef operator++(StrongTypedef& lhs, int)
    {
        auto tmp = lhs;
        ++static_cast<UnderlyingType&>(lhs);
        return tmp;
    }
};

template <class StrongTypedef, class UnderlyingType>
struct Subtraction
{
    constexpr friend StrongTypedef& operator-=(StrongTypedef& lhs, const StrongTypedef& rhs)
    {
        static_cast<UnderlyingType&>(lhs) -= static_cast<const UnderlyingType&>(rhs);
        return lhs;
    }

    [[nodiscard]] constexpr friend StrongTypedef operator-(const StrongTypedef& lhs, const StrongTypedef& rhs)
    {
        return StrongTypedef(static_cast<const UnderlyingType&>(lhs) - static_cast<const UnderlyingType&>(rhs));
    }

    constexpr friend StrongTypedef operator--(StrongTypedef& lhs)
    {
        --static_cast<UnderlyingType&>(lhs);
        return lhs;
    }

    constexpr friend StrongTypedef operator--(StrongTypedef& lhs, int)
    {
        auto tmp = lhs;
        --static_cast<UnderlyingType&>(lhs);
        return tmp;
    }
};

}    // namespace strong_typedef_utils
