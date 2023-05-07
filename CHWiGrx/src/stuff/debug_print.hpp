#pragma once

#ifdef _DEBUG

#include <iostream>

namespace details
{

template <typename T>
concept WCoutPrintable = requires(T arg) { std::wcout << arg; };

template <typename T>
void print_arg(T arg) noexcept
{
    if constexpr (WCoutPrintable<T>) {
        std::wcout << arg << L" ";
        if (std::wcout.bad()) {
            std::cout << " std::wcout on unrecoverable state... recover";
            std::wcout.clear();
        }
    }
    else {
        std::cout << arg << " ";
    }
}
}    // namespace details

#endif    // _DEBUG

template <typename... Ts>
void debug_print([[maybe_unused]] Ts... ts) noexcept
{
#ifdef _DEBUG
    ((details::print_arg(ts)), ...);
    std::cout << std::endl;
#else
    ((void)ts, ...);
#endif
}
