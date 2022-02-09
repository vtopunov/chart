#pragma once

#include <cassert>


#define D_UNUSED(expression) ((void)(expression))

#ifdef NDEBUG

#define D_ASSERT(expression) D_UNUSED(0)
#define D_ASSERT_WITH_SIDE_EFFECTS(expression) D_UNUSED(expression)

#else

#define D_ASSERT(expression) D_UNUSED((!!(expression)) || ((__debugbreak()), 0))
#define D_ASSERT_WITH_SIDE_EFFECTS(expression) D_ASSERT(expression)

#endif
