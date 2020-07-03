#pragma once

#include <cassert>

#define D_UNUSED(expression) ((void)(expression))

#ifdef NDEBUG

#define D_ASSERT(expression) D_UNUSED(false)

#else

#define D_ASSERT(expression) D_UNUSED((!!(expression)) || ((__debugbreak()), false))

#endif
