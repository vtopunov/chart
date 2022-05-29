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

    constexpr bool prio_eq(debug_priority tested, android_LogPriority standard) noexcept
    {
        return as_prio_arg(tested) == as_prio_arg(standard);
    }

    void priority_output_debug_string(debug_priority priority, const char* string) noexcept
    {
        static_assert(std::is_same_v<std::underlying_type_t<debug_priority>, std::underlying_type_t<android_LogPriority> >);

        static_assert( prio_eq( UNKNOWN , ANDROID_LOG_UNKNOWN ) );
        static_assert( prio_eq( DEFAULT , ANDROID_LOG_DEFAULT ) && DEFAULT > UNKNOWN );
        static_assert( prio_eq( VERBOSE , ANDROID_LOG_VERBOSE ) && VERBOSE > DEFAULT );
        static_assert( prio_eq( DEBUG   , ANDROID_LOG_DEBUG   ) && DEBUG   > VERBOSE );
        static_assert( prio_eq( INFO    , ANDROID_LOG_INFO    ) && INFO    > DEBUG   );
        static_assert( prio_eq( WARN    , ANDROID_LOG_WARN    ) && WARN    > INFO    );
        static_assert( prio_eq( ERROR   , ANDROID_LOG_ERROR   ) && ERROR   > WARN    );
        static_assert( prio_eq( FATAL   , ANDROID_LOG_FATAL   ) && FATAL   > ERROR   );
        static_assert( prio_eq( SILENT  , ANDROID_LOG_SILENT  ) && SILENT  > FATAL   );

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
