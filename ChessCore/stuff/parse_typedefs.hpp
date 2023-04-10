#pragma once

#include "macro.h"

#include <cstddef>
#include <expected>

#pragma region Parse error

template <typename ErrorType>
struct ParseError {
    ErrorType type;
    std::size_t position;
};

template <typename ResultType>
struct ParseResult {
    ResultType value;
    std::size_t position;
};

template <typename ResultType, typename ErrorType>
using ParseEither = std::expected<ParseResult<ResultType>, ParseError<ErrorType>>;

/// Use this macro to return an parse error
#define UNEXPECTED_PARSE(parse_error_type, type, pos) \
    std::unexpected{ ParseError<parse_error_type>{ parse_error_type :: type, pos } }

#pragma endregion   // Parse error

#pragma region from_string, as_string
/// implement operator() for this type with your type as template parameter
template <typename ResultType>
struct from_string;

/// implement operator() for this type with your type as template parameter
template <typename ResultType>
struct as_string;

#pragma endregion   // from_string, as_string
