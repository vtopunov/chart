#pragma once

#include <cassert>

#undef assert

#ifdef NDEBUG

#undef _DEBUG
#undef DEBUG

#define assert(expression) ((void)0)

#else

#ifndef _DEBUG
#define _DEBUG 1
#endif

#ifndef DEBUG
#define DEBUG 1
#endif

#define assert(expression) ((void)(  \
            (!!(expression)) ||      \
            (__debugbreak(), false)  \
        ))


#endif