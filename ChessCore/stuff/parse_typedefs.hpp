#pragma once

#include "macro.h"

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
[[nodiscard]] auto inline
    make_unexpected_parse(Args&&... args) -> std::unexpected<ParseErrorT>
{
    return std::unexpected<ParseErrorT>{ std::forward<Args>(args)... };
}

/// For usage name your parse error enum as ParseErrorType
#define UNEXPECTED_PARSE(...) \
    make_unexpected_parse<ParseError<ParseErrorType>>(__VA_ARGS__)

#pragma endregion   // Parse error

#pragma region from_string, as_string
/// implement operator() for this type with your type as template parameter
template <typename ResultType>
struct from_string;

/// implement operator() for this type with your type as template parameter
template <typename ResultType>
struct as_string;

#pragma endregion   // from_string, as_string
