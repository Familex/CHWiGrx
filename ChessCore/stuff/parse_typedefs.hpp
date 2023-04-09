#pragma once

#include "macro.h"
#include "../structs/id.hpp"

#include <cstddef>
#include <expected>

#pragma region Parse error

template <typename ParseErrorType>
struct ParseError {
    ParseErrorType type;
    std::size_t position;
};

template <typename ResultType, typename ParseErrorT>
using ParseResult = std::expected<ResultType, ParseError<ParseErrorT>>;

template <typename ParseErrorT, typename ...Args>
FN inline make_unexpected_parse(Args&&... args) -> std::unexpected<ParseErrorT> {
    return std::unexpected<ParseErrorT>{ std::forward<Args>(args)... };
}

/// For usage name your parse error enum as ParseErrorType
#define UNEXPECTED_PARSE(...) \
    make_unexpected_parse<ParseError<ParseErrorType>>(__VA_ARGS__)

#pragma endregion   // Parse error

#pragma region Meta for as_string and from_string

struct AsStringMeta {
    Id min_id;
    std::size_t max_pos_length;
    std::size_t version;    // from 00 to 99
};

struct FromStringMeta {
    std::size_t max_pos_length;
};

#pragma endregion

#pragma region from_string, as_string
/// implement operator() for this type with your type as template parameter
template <typename ResultType>
struct from_string {
    [[nodiscard]] inline auto
#ifdef P1169R4
        static
#endif
        operator() (const std::string_view sv,
                    const FromStringMeta& meta) 
#ifndef P1169R4
        const
#endif
            noexcept = delete;
};

/// implement operator() for this type with your type as template parameter
template <typename ResultType>
struct as_string {
    [[nodiscard]] inline auto
#ifdef P1169R4
        static
#endif
        operator() (const ResultType result,
                    const AsStringMeta& meta)
#ifndef P1169R4
        const
#endif
        noexcept = delete;
};

#pragma endregion   // from_string, as_string
