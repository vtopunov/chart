#pragma once

#include <cstdint>

#include <type_traits>

using u32argb_t = uint32_t;
static_assert(sizeof(u32argb_t) == 4 && std::is_unsigned_v<u32argb_t>);

using u8tint_t = uint8_t;
static_assert(sizeof(u8tint_t) == 1 && std::is_unsigned_v<u8tint_t>);

