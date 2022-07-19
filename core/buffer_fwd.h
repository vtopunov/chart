#pragma once

#include <cstddef>

template<class T>
class buffer;

static_assert(1u == sizeof(std::byte));
using buffer_t = buffer<std::byte>;