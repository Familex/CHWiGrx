#pragma once

#include "parsing.hpp"
#include "parse_meta.hpp"

#include <variant>

namespace parse_step {

    template <typename Result>
    concept FromStringUsingMeta = requires (const std::string_view sv, const FromStringMeta & meta) {
        FromString<Result>{}(sv, meta);
    };

    template <typename Result, typename Error = ParseErrorType>
    struct ParseStep {
        using ParseStepParser = ParseEither<Result, Error>(*)( ParseStep const* const, const std::string_view, const FromStringMeta& );

        bool forward_error{ false };

        std::size_t extra_position{ 0 };    // extra moves curr_pos by this value
        std::size_t* curr_pos{ nullptr };   // not hold ownership
        std::optional<std::size_t> substr_position{ std::nullopt };
        std::optional<std::size_t> substr_max_length{ std::nullopt };
        Error error{ }; // Default error to throw
        Error on_abrupt_halt{ };
        std::optional<ParseStepParser> parser;

        FN get_substr_position() const noexcept -> std::size_t {
            return substr_position ? *substr_position : get_curr_pos();
        }

        FN get_substr_max_length() const noexcept -> std::size_t {
            return substr_max_length ? *substr_max_length : std::string_view::npos;
        }

        FN get_curr_pos() const noexcept -> std::size_t {
            return curr_pos ? *curr_pos : 0ull;
        }

        FN apply_from_string(const std::string_view sv, const FromStringMeta& meta) noexcept
            -> ParseEither<Result, Error>
            requires FromStringUsingMeta<Result>
        {
            return FromString<Result>{}(sv, meta);
        }

        FN apply_from_string(const std::string_view sv, const FromStringMeta& meta) noexcept
            -> ParseEither<Result, Error>
            requires (!FromStringUsingMeta<Result>)
        {
            return FromString<Result>{}(sv);
        }

        FN get_local_sv(const std::string_view sv) const noexcept -> std::string_view
        {
            if (sv.size() < get_substr_position()) {
                return sv.substr(0, 0);
            }
            return sv.substr(get_substr_position(), get_substr_max_length());
        } 

        FN operator() (const std::string_view sv, const FromStringMeta& meta) noexcept
            -> ParseEither<Result, Error>
        {
            if (sv.size() < get_curr_pos()) {
                return std::unexpected{
                    ParseError<Error> {
                        on_abrupt_halt,
                        get_curr_pos()
                    }
                };
            }

            if (auto value_sus =
                parser
                    ? parser.value()(this, sv, meta)
                    : apply_from_string(
                        get_local_sv(sv),
                        meta
                    ))
            {
                if (curr_pos) {
                    *curr_pos += value_sus->position + extra_position;
                }
                return value_sus;
            }
            else {
                return std::unexpected {
                    ParseError<Error> {
                        forward_error
                            ? value_sus.error().type
                            : error,
                        get_curr_pos()
                            + value_sus.error().position
                    }
                };
            }
        }
    };

    // Builder for ParseStep
    template <typename Result, typename Error = ParseErrorType>
    class ParseStepBuilder {
        ParseStep<Result, Error> parse_step_{ };

    public:
        constexpr ParseStepBuilder& operator=(const ParseStepBuilder&) noexcept = delete;
        constexpr ParseStepBuilder(const ParseStepBuilder&) noexcept = delete;

        CTOR ParseStepBuilder(ParseStepBuilder&&) noexcept = default;
        [[nodiscard]] constexpr ParseStepBuilder& operator=(ParseStepBuilder&&) noexcept = default;
        CTOR ParseStepBuilder() noexcept = default;
        ~ParseStepBuilder() noexcept = default;

        // Returns ParseStep object
        FN build() && noexcept -> ParseStep<Result, Error> {
            return this->parse_step_;
        }

        // Execute own ParseStep instead of build
        FN operator() (const std::string_view sv,
            const FromStringMeta& meta) && noexcept
            -> ParseEither<Result, Error> {
            return this->parse_step_(sv, meta);
        }

        // Parser what will be used to parse ( default: FromString<Result>{})
        FN parser(typename ParseStep<Result, Error>::ParseStepParser value) && noexcept -> ParseStepBuilder&& {
            this->parse_step_.parser = value;
            return std::move(*this);
        }

        // Value what adds to curr_pos after parse
        FN extra( std::size_t value ) && noexcept -> ParseStepBuilder&& {
            this->parse_step_.extra_position = value;
            return std::move(*this);
        }
        
        // Position to start parsing from (default: 0)
        // Also uses by other steps in sequence
        FN bind_curr_pos( std::size_t& value ) && noexcept -> ParseStepBuilder&& {
            this->parse_step_.curr_pos = &value;
            return std::move(*this);
        }
        
        // Max length of substring to parse
        FN max_length( std::size_t value ) && noexcept -> ParseStepBuilder&& {
            this->parse_step_.substr_max_length = value;
            return std::move(*this);
        }
        
        // Error what throws on parse error
        FN error( Error value ) && noexcept -> ParseStepBuilder&& {
            this->parse_step_.error = value;
            return std::move(*this);
        }

        // Error what throws on unexpected end of parse
        FN on_abrupt_halt( Error value ) && noexcept -> ParseStepBuilder&& {
            this->parse_step_.on_abrupt_halt = value;
            return std::move(*this);
        }

        // On turn on forward_error, error from parse step will be forwarded (error will be ignored)
        FN forward_error( bool value ) && noexcept {
            this->parse_step_.forward_error = value;
            return *this;
        }
    };
    
    template <typename ParseStepError>
    struct ParseStepException final : std::exception {
        ParseStepError error;

        CTOR ParseStepException(ParseStepError error) noexcept
            : error{ error }
        { }
    };

    // throws ParseStepException<ParseStepError>
    // idk how to implement execute_sequence without exceptions 😁
    template <typename ParseStepResult, typename ParseStepError>
    FN static make_step(const std::string_view sv,
        const FromStringMeta& meta,
        ParseStep<ParseStepResult, ParseStepError>&& parse_step)
        -> ParseStepResult
    {
        if (const auto& result_sus =
            std::forward<ParseStep<ParseStepResult, ParseStepError>>(parse_step)(sv, meta)) {
            return result_sus->value;
        }
        else {
            throw ParseStepException<ParseError<ParseStepError>>{ result_sus.error() };
        }
    }

    // Execute sequence of ParseSteps, and return result of collector
    // Order of execution is from left to right
    template <
        typename Collector,
        typename Error = ParseErrorType,
        typename ...ParseStepResults >
    FN execute_sequence(
        const std::size_t start_pos,
        const std::string_view sv,
        const FromStringMeta& meta,
        const Collector& collector,
        ParseStepBuilder<ParseStepResults, Error>&& ...steps) noexcept
        -> ParseEither<
                decltype( collector(
                    make_step(sv, meta, std::forward<ParseStepBuilder<ParseStepResults, Error>>(steps).build())...
                ) ),
                Error
           >
    {
        try {
            std::size_t curr_pos{ start_pos };
            // FORCE EXECUTION ORDER 😡
            std::tuple step_results{
                make_step(sv, meta, std::forward<ParseStepBuilder<ParseStepResults, Error>>(steps).bind_curr_pos(curr_pos).build())...
            };
            return { {
                std::apply(
                    [&collector](auto&& ...args) {
                        return collector( std::move(args)... );
                    },
                    std::move( step_results )
                ),
                curr_pos
            } };
        }
        catch (const ParseStepException<ParseError<Error>>& e) {
            return std::unexpected{ e.error };
        }
    }

    // Generates a constructor function for a type_
    // that takes a variadic number of arguments
    template <typename Result>
    FN gen_constructor() noexcept {
        return []<typename... Args>(Args&& ...args) noexcept {
            return Result{ std::forward<Args>(args)... };
        };
    }

}   // namespace parse_step_
