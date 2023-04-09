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

    FN friend swap(strong_typedef& a, strong_typedef& b) noexcept
    {
        using std::swap;
        swap(static_cast<T&>(a), static_cast<T&>(b));
    }
};

namespace strong_typedef_utils {

    template <typename Tag, typename T>
    FN underlying_type_impl(strong_typedef<Tag, T>) noexcept -> T;

    template <typename T>
    using underlying_type = decltype(underlying_type_impl(std::declval<T>()));

    template <class StrongTypedef>
    struct addition
    {
        using type = underlying_type<StrongTypedef>;

        constexpr friend StrongTypedef&
            operator+=(StrongTypedef& lhs, const StrongTypedef& rhs)
        {
            static_cast<type&>(lhs) += static_cast<const type&>(rhs);
            return lhs;
        }

        [[nodiscard]] constexpr friend StrongTypedef
            operator+(const StrongTypedef& lhs, const StrongTypedef& rhs)
        {
            return StrongTypedef(static_cast<const type&>(lhs)
                + static_cast<const type&>(rhs));
        }

        constexpr friend StrongTypedef
            operator++(StrongTypedef& lhs)
        {
            ++static_cast<type&>(lhs);
            return lhs;
        }

        constexpr friend StrongTypedef
            operator++(StrongTypedef& lhs, int)
        {
            auto tmp = lhs;
            ++static_cast<type&>(lhs);
            return tmp;
        }
    };

    template <class StrongTypedef>
    struct subtraction
    {
        using type = underlying_type<StrongTypedef>;

        constexpr friend StrongTypedef&
            operator-=(StrongTypedef& lhs, const StrongTypedef& rhs)
        {
            static_cast<type&>(lhs) -= static_cast<const type&>(rhs);
            return lhs;
        }

        [[nodiscard]] constexpr friend StrongTypedef
            operator-(const StrongTypedef& lhs, const StrongTypedef& rhs)
        {
            return StrongTypedef(static_cast<const type&>(lhs)
                - static_cast<const type&>(rhs));
        }

        constexpr friend StrongTypedef
            operator--(StrongTypedef& lhs)
        {
            --static_cast<type&>(lhs);
            return lhs;
        }

        constexpr friend StrongTypedef
            operator--(StrongTypedef& lhs, int)
        {
            auto tmp = lhs;
            --static_cast<type&>(lhs);
            return tmp;
        }
    };

}   // strong_typedef_utils
