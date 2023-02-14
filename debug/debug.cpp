#include "debug.h"

#include <os/log.h>

namespace private_detail_debug
{
#if defined(D_OS_WINDOWS)
    void output_debug_string(const char* string) noexcept
    {
        OutputDebugStringA(string);
    }

    void output_debug_string(const wchar_t* string) noexcept
    {
        OutputDebugStringW(string);
    }

#elif defined(D_OS_ANDROID)
    using prio_arg_t = int;

    template<class E>
    constexpr prio_arg_t as_prio_arg(E e) noexcept
    {
        return static_cast<prio_arg_t>(e);
    }

    constexpr bool prio_eq(log_priority tested, android_LogPriority standard) noexcept
    {
        return as_prio_arg(tested) == as_prio_arg(standard);
    }

    void priority_output_debug_string(log_priority priority, const char* string) noexcept
    {
        static_assert(std::is_same_v<std::underlying_type_t<log_priority>, std::underlying_type_t<android_LogPriority> >);

        static_assert( prio_eq( LOG_UNKNOWN , ANDROID_LOG_UNKNOWN ) );
        static_assert( prio_eq( LOG_DEFAULT , ANDROID_LOG_DEFAULT ) && LOG_DEFAULT > LOG_UNKNOWN );
        static_assert( prio_eq( LOG_VERBOSE , ANDROID_LOG_VERBOSE ) && LOG_VERBOSE > LOG_DEFAULT );
        static_assert( prio_eq( LOG_DEBUG   , ANDROID_LOG_DEBUG   ) && LOG_DEBUG   > LOG_VERBOSE );
        static_assert( prio_eq( LOG_INFO    , ANDROID_LOG_INFO    ) && LOG_INFO    > LOG_DEBUG   );
        static_assert( prio_eq( LOG_WARN    , ANDROID_LOG_WARN    ) && LOG_WARN    > LOG_INFO    );
        static_assert( prio_eq( LOG_ERROR   , ANDROID_LOG_ERROR   ) && LOG_ERROR   > LOG_WARN    );
        static_assert( prio_eq( LOG_FATAL   , ANDROID_LOG_FATAL   ) && LOG_FATAL   > LOG_ERROR   );
        static_assert( prio_eq( LOG_SILENT  , ANDROID_LOG_SILENT  ) && LOG_SILENT  > LOG_FATAL   );

        const auto prio = as_prio_arg(priority);
        
        {
            constexpr auto min_prio = as_prio_arg(ANDROID_LOG_UNKNOWN);
            static_assert(!min_prio);
            constexpr auto max_prio = as_prio_arg(ANDROID_LOG_SILENT);
            static_assert(max_prio > min_prio && max_prio <= std::numeric_limits<prio_arg_t>::max());
            D_ASSERT(prio >= min_prio && prio <= max_prio);
        }

        D_UNUSED(__android_log_write(prio, "org.chart", string));
    }

#endif

}
