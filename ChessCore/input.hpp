#pragma once

#include "pos.hpp"
#include "macro.h"

#include <string>
#include <cassert>

struct Input {
    Pos from{ };
    Pos target{ };
    
    CTOR Input() noexcept = default;

    CTOR Input(const Pos& from, const Pos& target) noexcept
        : from{ from }
        , target{ target } {}

    CTOR Input(std::string str) {
        /* "from.first from.second target.first target.second" */
        str += " ";
        char delimiter = ' ';
        size_t delimiter_len = 1;
        size_t pos = 0;
        std::string token;
        std::vector<int> acc{};
        for (int i{}; i < 4; ++i) {
            if ((pos = str.find(delimiter)) != std::string::npos) {
                token = str.substr(0, pos);
                acc.push_back(std::stoi(token));
                str.erase(0, pos + delimiter_len);
            }
            else {
                assert(!"unexpected end of line");
            }
        }
        from.x = acc[0];
        from.y = acc[1];
        target.x = acc[2];
        target.y = acc[3];
    }
};