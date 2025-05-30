#pragma once

#include "../parse/parse_error.hpp"
#include "../stuff/macro.h"
#include "../stuff/strong_typedef.hpp"
#include "../stuff/stuff.hpp"

#include <algorithm>
#include <format>
#include <string>

using IdType = unsigned long long int;
struct IdTag
{ };
struct Id
  : StrongTypedef<IdTag, IdType>
  , strong_typedef_utils::Addition<Id, IdType>
  , strong_typedef_utils::Subtraction<Id, IdType>
{
    using StrongTypedef::StrongTypedef;

    friend FromString<Id>;
    friend AsString<Id>;

    FN operator<=>(const Id& id) const noexcept
    {
        return static_cast<IdType>(*this) <=> static_cast<IdType>(id);
    }
    FN operator==(const Id& id) const noexcept
    {
        return static_cast<IdType>(*this) == static_cast<IdType>(id);
    }
};

FN operator""_id(const IdType id) noexcept -> Id { return Id{ id }; }

namespace std
{
template <>
struct hash<Id>
{
    FN operator()(const Id& id) const noexcept -> size_t { return static_cast<IdType>(id); }
};

template <>
struct formatter<Id> : formatter<string>
{
    [[nodiscard]] inline auto format(Id id, format_context& ctx) const noexcept
    {
        // FIXME ::std:: thing looks hacky
        return formatter<string>::format(::std::format<IdType>("{}", static_cast<IdType>(id)), ctx);
    }
};
}    // namespace std

template <>
struct FromString<Id>
{
    FN operator()(const std::string_view sv) const noexcept -> ParseEither<Id, ParseErrorType>
    {
        if (auto res = svtoi(sv)) {
            return { { Id{ static_cast<IdType>(res->value) }, res->position } };
        }
        else {
            return PARSE_STEP_UNEXPECTED(ParseErrorType, Id_Invalid, res.error());
        }
    }
};

template <>
struct AsString<Id>
{
    [[nodiscard]] auto operator()(const Id& id, const Id& min_id) const noexcept -> std::string
    {
        return std::to_string(static_cast<IdType>(id - min_id));
    }
};
