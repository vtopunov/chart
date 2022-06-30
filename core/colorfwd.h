#pragma once

#include <cstdint>

using u32argb_t = uint32_t;
static_assert(sizeof(u32argb_t) == 4u);

using u8tint_t = uint8_t;
static_assert(sizeof(u8tint_t) == 1u);

template<class T>
struct rgba_color;
