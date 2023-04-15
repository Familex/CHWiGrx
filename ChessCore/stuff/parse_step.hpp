#pragma once

#include "parsing.hpp"
#include "parse_meta.hpp"

#include <variant>

namespace parse_step {

    template <typename Result>
    concept FromStringUsingMeta = requires (const std::string_view sv, const FromStringMeta & meta) {
        from_string<Result>{}(sv, meta);
    };

    template <typename Result, typename Error = ParseErrorType>
    struct ParseStep {
        bool forward_error{ false };

        std::size_t extra_position{ 0 };    // extra moves curr_pos by this value
        std::size_t* curr_pos{ nullptr };   // not hold ownership
        std::optional<std::size_t> substr_position{ std::nullopt };
        std::optional<std::size_t> substr_max_length{ std::nullopt };
        Error error{ }; // Default error to throw
        Error unexpected_end_error{ };

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
            return from_string<Result>{}(sv.substr(get_substr_position(), get_substr_max_length()), meta);
        }

        FN apply_from_string(const std::string_view sv, const FromStringMeta& meta) noexcept
            -> ParseEither<Result, Error>
            requires (!FromStringUsingMeta<Result>)
        {
            return from_string<Result>{}(sv.substr(get_substr_position(), get_substr_max_length()));
        }

        FN operator() (const std::string_view sv,
            const FromStringMeta& meta) noexcept
            -> ParseEither<Result, Error>
        {
            if (sv.size() < get_curr_pos()) {
                return std::unexpected{
                    ParseError<Error> {
                        unexpected_end_error,
                        get_curr_pos()
                    }
                };
            }
            
            const auto value_sus =
                apply_from_string(
                    sv.substr(
                        get_substr_position(),
                        get_substr_max_length()),
                    meta
                );

            if (value_sus) {
                if (curr_pos) {
                    *curr_pos += value_sus->position + extra_position;
                }
                return value_sus;
            }
            else {
                return std::unexpected{
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

    template <typename Result, typename Error = ParseErrorType>
    struct ParseStepBuilder {
        ParseStep<Result, Error> parse_step{ };
        
        FN set_auto_substr( bool value ) && noexcept -> ParseStepBuilder {
            this->parse_step.auto_substr = value;
            return *this;
        }
        
        FN set_extra_position( std::size_t value ) && noexcept -> ParseStepBuilder {
            this->parse_step.extra_position = value;
            return *this;
        }
        
        FN set_curr_pos( std::size_t& value ) && noexcept -> ParseStepBuilder {
            this->parse_step.curr_pos = &value;
            return *this;
        }
        
        FN set_substr_max_length( std::size_t value ) && noexcept -> ParseStepBuilder {
            this->parse_step.substr_max_length = value;
            return *this;
        }
        
        FN set_error( Error value ) && noexcept -> ParseStepBuilder {
            this->parse_step.error = value;
            return *this;
        }

        FN set_unexpected_end_error(Error value) && noexcept -> ParseStepBuilder {
            this->parse_step.unexpected_end_error = value;
            return *this;
        }
        
        FN build() && noexcept -> ParseStep<Result, Error> {
            return this->parse_step;
        }
        
        FN operator() (const std::string_view sv,
                       const FromStringMeta& meta) && noexcept
           -> ParseEither<Result, Error> {
            this->parse_step();               
        }
    };
    
    template <typename ParseStepError>
    struct ParseStepException : std::exception {
        ParseStepError error;

        CTOR ParseStepException(ParseStepError error) noexcept
            : error{ error }
        { }
    };

    // throws ParseStepException<ParseStepError>
    template <typename ParseStepResult, typename ParseStepError>
    FN static make_step(const std::string_view sv,
        const FromStringMeta& meta,
        ParseStep<ParseStepResult, ParseStepError>&& parseStep)
        -> ParseStepResult
    {
        const auto& result_sus =
            std::forward<ParseStep<ParseStepResult, ParseStepError>>(parseStep)(sv, meta);

        if (result_sus) {
            return result_sus->value;
        }
        else {
            throw ParseStepException<ParseError<ParseStepError>>{ result_sus.error() };
        }
    }

    // idk how to implement execute_sequence without exceptions 😁
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
            // FORSE EXETUTION ORDER 😡
            return { { collector(
                make_step(sv, meta, std::forward<ParseStepBuilder<ParseStepResults, Error>>(steps).set_curr_pos(curr_pos).build())...
            ), curr_pos } };
        }

        catch (const ParseStepException<ParseError<Error>>& e) {
            return std::unexpected{ e.error };
        }
    }
}   // namespace parse_step