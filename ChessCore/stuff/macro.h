#pragma once

#define FN [[nodiscard]] constexpr auto
#define CTOR [[nodiscard]] constexpr explicit
#define USING_BEG(...) [__VA_ARGS__]() {
#define USING_END } ()
/// optional unwrap (from https://stackoverflow.com/a/60746626)
#define IF_LET(name, expr)      \
    if (auto name##_sus = expr) \
        if (auto& name = *name##_sus; false) {} else
