#pragma once

#include <cstdint>
#include <type_traits>

template <typename T>
struct custom_make_unsigned_t
{
    static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>, "T must be an integral or floating-point type");
    using type = std::conditional_t<sizeof(T) == 1, std::uint8_t,
                                    std::conditional_t<sizeof(T) == 2, std::uint16_t, std::conditional_t<sizeof(T) == 4, std::uint32_t, std::conditional_t<sizeof(T) == 8, std::uint64_t, void>>>>;
};

template <typename T>
using make_uint = typename custom_make_unsigned_t<T>::type;
