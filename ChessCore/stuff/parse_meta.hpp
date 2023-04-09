#pragma once

#include "../structs/id.hpp"
#include "../stuff/enums.hpp"

struct AsStringMeta {
    Id min_id{ 0_id };
    std::size_t max_pos_length{ 2 };
    std::size_t version{ 2 };    // from 00 to 99
};

struct FromStringMeta {
    bool idw{ };
    Color turn{ };
    std::vector<Id> castlings{ };
    std::size_t height{ 8 };
    std::size_t width{ 8 };
    std::size_t max_pos_length{ 2 };
    std::size_t version{ 2 };
};