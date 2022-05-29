#pragma once

#if defined(_WIN32)
#define D_OS_WINDOWS 1
#elif defined(__ANDROID__)
#define D_OS_ANDROID 1
#else
#define D_OS_UNKNOWN 1
#endif
