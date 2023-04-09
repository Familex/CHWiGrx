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
};

struct FromStringMeta {
    std::size_t max_pos_length;
};

#pragma endregion

#pragma region Stringify
/// All macro are used to implement Stringify static methods with one signature.

/// implement from_string and as_string for this struct
template <typename ResultType>
struct Stringify;

#define STRINGIFY_DECLARE_ERROR_TYPE \
    enum class ParseErrorType

#define STRINGIFY_DECLARE_BEGIN(ResultTypeT) \
    template <> \
    struct Stringify<ResultTypeT> { \
    using ResultType = ResultTypeT; \

#define STRINGIFY_DECLARE_END \
    };

#define STRINGIFY_DECLARE_FROM_STRING \
    [[nodiscard]] static inline auto \
    from_string(const std::string_view sv, \
                const FromStringMeta& meta) noexcept \
    -> ParseResult<ResultType, ParseErrorType>

#define STRINGIFY_DECLARE_AS_STRING \
    [[nodiscard]] static inline auto \
    as_string(const ResultType result, \
              const AsStringMeta& meta) noexcept \
    -> std::string

#pragma endregion   // Stringify
