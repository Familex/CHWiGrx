#pragma once

#include "../structs/id.hpp"

struct AsStringMeta {
    Id min_id{ 0_id };
    std::size_t max_pos_length{ 2 };
    std::size_t version{ 2 };    // from 00 to 99
};

struct FromStringMeta {
    std::size_t max_pos_length;
};