#pragma once

#include <cassert>

#define D_UNUSED(expression) ((void)(expression))

#ifdef NDEBUG

#define D_IS_DEBUG 0
#define D_ONLY_DEBUG(A)
#define D_ASSERT(expression) D_UNUSED(0)
#define D_ASSERT_OR_UNUSED(expression) D_UNUSED(expression)


#else


#define D_IS_DEBUG 1
#define D_ONLY_DEBUG(A) A

#ifdef _MSC_VER
#define D_ASSERT(expression) D_UNUSED((!!(expression)) || ((__debugbreak()), 0))
#else
#define D_ASSERT(expression) assert(expression)
#endif

#define D_ASSERT_OR_UNUSED(expression) D_ASSERT(expression)


#endif
