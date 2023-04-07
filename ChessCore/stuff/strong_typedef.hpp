#pragma once

#include "macro.h"

#include <utility>  // std::swap

template <class Tag, typename T>
class strong_typedef
{
    T value_;
public:
    CTOR strong_typedef()
        : value_()
    { }

    CTOR strong_typedef(const T& value)
        : value_(value)
    { }

    CTOR strong_typedef(T&& value)
            noexcept(std::is_nothrow_move_constructible<T>::value)
        : value_(std::move(value))
    { }

    [[nodiscard]] constexpr
        operator T& () noexcept
    {
        return value_;
    }

    [[nodiscard]] constexpr
        operator const T& () const noexcept
    {
        return value_;
    }

    friend FN swap(strong_typedef& a, strong_typedef& b) noexcept
    {
        using std::swap;
        swap(static_cast<T&>(a), static_cast<T&>(b));
    }
};