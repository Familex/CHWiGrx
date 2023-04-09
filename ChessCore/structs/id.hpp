#pragma once

#include "../stuff/macro.h"
#include "../stuff/strong_typedef.hpp"
#include "../stuff/parse_typedefs.hpp"

#include <cassert>
#include <string>
#include <format>
#include <algorithm>

using Id_type = unsigned long long int;
struct Id_tag { };
struct Id
    : strong_typedef<Id_tag, Id_type>
    , strong_typedef_utils::addition<Id>
    , strong_typedef_utils::subtraction<Id>
{
    using strong_typedef::strong_typedef;

    friend from_string<Id>;
    friend as_string<Id>;
};

FN operator""_id(Id_type id) noexcept -> Id {
    return Id{ id };
}

namespace std {
    template <>
    struct hash<Id> {
        FN operator()(const Id id) const noexcept -> size_t
        {
            return static_cast<Id_type>(id);
        }
    };
    
    template <>
    struct std::formatter<Id> : std::formatter<std::string> {
        [[nodiscard]] inline auto
            format(Id id, format_context& ctx) const noexcept
        {
            return formatter<std::string>::format(
                std::format("{}", static_cast<Id_type>(id)), ctx
            );
        }
    };
}

template <>
struct from_string<Id> {
    [[nodiscard]] inline auto
        operator()(const std::string_view sv, const FromStringMeta& meta) const noexcept
        -> std::optional<Id>
    {
        return svtoi(sv)
            .transform([](int i) {return Id{ static_cast<Id_type>(i) }; });
    }
};

template <>
struct as_string<Id> {
    [[nodiscard]] inline auto
        operator()(const Id id, const AsStringMeta& meta) const noexcept
        -> std::string
    {
        return std::to_string(id - meta.min_id);
    }
};
