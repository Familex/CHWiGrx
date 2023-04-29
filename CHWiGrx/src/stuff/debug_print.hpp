#pragma once

#ifdef _DEBUG
#include <iostream>
#endif    // _DEBUG

template<typename... Ts>
void debug_print([[maybe_unused]] Ts... args) noexcept
{
#ifdef _DEBUG
    ((std::cout << args << " "), ...);
    std::cout << std::endl;
#else
    ((void)args, ...);
#endif    // _DEBUG
}
