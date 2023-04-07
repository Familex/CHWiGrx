#pragma once

#include "../stuff/macro.h"
#include "../stuff/strong_typedef.hpp"

#include <string>

struct Id_tag {};
using Id = strong_typedef<struct Id_tag, std::size_t>;

Id operator""_id(unsigned long long int id) noexcept {
    return Id{ static_cast<int>(id) };
}

FN as_string(const Id id) noexcept -> std::string {
    return std::to_string(id);
}
