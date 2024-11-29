#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#pragma once

namespace rez
{

namespace internal
{

template<typename... args_t>
[[noreturn]]
inline auto throw_formatted(const std::format_string<args_t...> format, args_t&&... args) -> void
{
    throw std::runtime_error(std::format(format, std::forward<args_t>(args)...));
}

template <typename... args_t>
struct throw_ex final
{
    // so annoying to make std::source_location work with variadic arguments...
    // mark the constructor as 'noreturn' to prevent msvc from issuing warnings
    // for an unnamed temporary object
    [[noreturn]]
    throw_ex(
        const std::format_string<args_t...> format,
        args_t&&... args,
        const std::source_location& loc = std::source_location::current()
    )
    {
        throw_formatted(
            "{:s} - {:s}:{:d}",
            std::format(format, std::forward<args_t>(args)...),
            loc.file_name(),
            loc.line()
        );
    }
};

// workaround with deduction guide to use std::source_location with variadic arguments
template <typename... args_t>
throw_ex(const std::format_string<args_t...>, args_t&&...) -> throw_ex<args_t...>;

}

template <typename... args_t>
using throw_ex = internal::throw_ex<args_t...>;

template<typename... args_t>
inline auto log(
    const std::format_string<args_t...> format,
    args_t&&... args
) -> void
{
    std::cout << std::format(format, std::forward<args_t>(args)...);
}

inline auto pause() -> void
{
    log("Press the enter key to continue . . .");

    std::cin.clear();
    std::cin.get();
}

}

#define REZ_THROW(fmt, ...) rez::throw_ex(fmt, __VA_ARGS__)

#endif // !UTILITIES_HPP
