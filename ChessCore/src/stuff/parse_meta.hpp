#pragma once

#include "../structs/id.hpp"
#include "../stuff/enums.hpp"
#include "../stuff/macro.h"

struct AsStringMeta
{
    Id min_id{ 0_id };
    std::size_t max_pos_length{ 2 };
    std::size_t version{ 2 };    // from 00 to 99

    CTOR AsStringMeta() noexcept = default;

    CTOR AsStringMeta(const Id min_id, const std::size_t max_pos_length, const std::size_t version) noexcept
      : min_id{ min_id }
      , max_pos_length{ max_pos_length }
      , version{ version }
    { }
};

struct FromStringMeta
{
    bool idw{};
    Color turn{};
    std::vector<Id> castlings{};
    std::size_t height{ 8 };
    std::size_t width{ 8 };
    std::size_t max_pos_length{ 2 };
    std::size_t version{ 2 };

    CTOR FromStringMeta() noexcept = default;

    CTOR FromStringMeta(
        const bool idw,
        const Color turn,
        const std::vector<Id>& castlings,
        const std::size_t height,
        const std::size_t width,
        const std::size_t max_pos_length,
        const std::size_t version
    ) noexcept
      : idw{ idw }
      , turn{ turn }
      , castlings{ castlings }
      , height{ height }
      , width{ width }
      , max_pos_length{ max_pos_length }
      , version{ version }
    { }
};
