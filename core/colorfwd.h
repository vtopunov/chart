#pragma once

#include <cstdint>

#include <type_traits>

#include <core/size_type.h>

using u32argb_t = uint32_t;
static_assert(sizeof(u32argb_t) == 4_uz && std::is_unsigned_v<u32argb_t>);

using u8tint_t = uint8_t;
static_assert(sizeof(u8tint_t) == 1_uz && std::is_unsigned_v<u8tint_t>);

