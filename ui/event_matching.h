#pragma once

#include <chrono>

#include <ui/event.h>

namespace ui
{
    using milliseconds_t = std::chrono::milliseconds;

    constexpr auto infinite = milliseconds_t{ D_CONDITIONAL_OS_WINDOWS(0xffffffff, -1) };
   
    struct idle_event
    {};

    template<class T>
    struct ignore_event
    {
        constexpr ignore_event(const T&) noexcept
        {}
    };

    template<class T, event_style es>
    auto call_event(T* function, const specialized_event<es>& e) noexcept
        -> decltype( std::declval<T&>()( std::declval<specialized_event<es>>() ) )
    {
        return ( *function )( e );
    }

    template<class T>
    auto call_event(T* function, const event& e) noexcept
        -> decltype( std::declval<T&>()( std::declval<event>() ) )
    {
        return ( *function )( e );
    }

    template<class T>
    auto call_event(T* function, idle_event e) noexcept -> decltype( std::declval<T&>()( std::declval<idle_event>() ) )
    {
        return ( *function )( e );
    }


    template<class T>
    constexpr std::nullopt_t call_event(T*, ignore_event<event>) noexcept
    {
        return std::nullopt;
    }

    template<class T>
    constexpr milliseconds_t call_event(T*, ignore_event<idle_event>) noexcept
    {
        return infinite;
    }


    template<class TDataPtr>
    struct event_callback_instance
    {
        static event_result_opt_t callback(void* data, const event& e) noexcept
        {
            D_ASSERT(data);

#if defined(D_OS_WINDOWS)
            const auto tdata = static_cast<TDataPtr>( data );

            switch ( e.style() )
            {
                case event_style::size:               return call_event(tdata, e.as<event_style::size>());
                case event_style::close:              return call_event(tdata, e.as<event_style::close>());
                case event_style::mouse_move:         return call_event(tdata, e.as<event_style::mouse_move>());
                case event_style::mouse_lbutton_down: return call_event(tdata, e.as<event_style::mouse_lbutton_down>());
                case event_style::mouse_lbutton_up:   return call_event(tdata, e.as<event_style::mouse_lbutton_up>());
            }

            return call_event(tdata, e);

#elif defined(D_OS_ANDROID)
            return std::nullopt;

#endif
        }
    };
}

