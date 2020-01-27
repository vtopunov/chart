#pragma once

#include <cassert>

#define D_CHECK(expression) ((void)( \
            (!!(expression)) ||       \
            (__debugbreak(), false)   \
        ))

#ifdef NDEBUG

#define D_ASSERT(expression) ((void)0)

#else

#define D_ASSERT(expression) D_CHECK(expression)

#endif