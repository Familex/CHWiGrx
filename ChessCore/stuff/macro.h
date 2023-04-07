#pragma once

#define FN [[nodiscard]] constexpr auto
#define CTOR [[nodiscard]] constexpr explicit
#define USING_BEG(...) [__VA_ARGS__]() {
#define USING_END } ()
