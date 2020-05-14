#pragma once

#include <cassert>

#ifdef NDEBUG

#define D_ASSERT(expression) ((void)0)

#else

#define D_ASSERT(expression) ((void)(\
            (!!(expression)) ||      \
            (__debugbreak(), false)  \
        ))

#endif