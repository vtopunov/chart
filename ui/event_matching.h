#pragma once

#include <ui/event.h>
#include <ui/event_timer.h>

namespace ui
{
    struct peek_event
    {};

    struct idle_event
    {};

    struct timer_event
    {
        event_timer_resource timer_id;
    };

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
    auto call_event(T* function, peek_event e) noexcept
        -> decltype( std::declval<T&>()( std::declval<peek_event>() ) )
    {
        return ( *function )( e );
    }

    template<class T>
    auto call_event(T* function, idle_event e) noexcept
        -> decltype( ( std::declval<T&>()( std::declval<idle_event>() ), 0 ) )
    {
        ( *function )( e );
        return 0;
    }

    template<class T>
    auto call_event(T* function, const MSG& msg) noexcept
        -> decltype( (std::declval<T&>()( std::declval<timer_event>() ), 0) )
    {
        if ( msg.message == WM_TIMER )
        {
            ( *function )( timer_event{ underlying_cast<event_timer_resource>( msg.wParam ) } );
        }

        return 0;
    }

    template<class T>
    constexpr std::nullopt_t call_event(T*, ignore_event<event>) noexcept
    {
        return std::nullopt;
    }

    template<class T>
    constexpr bool call_event(T*, ignore_event<peek_event>) noexcept
    {
        return false;
    }

    template<class T>
    constexpr void call_event(T*, ignore_event<idle_event>) noexcept
    {
    }

    template<class T>
    constexpr void call_event(T*, ignore_event<MSG>) noexcept
    {
    }

    template<class T>
    struct event_callback_instance
    {
        static std::optional<event_result_t> callback(void* data, const event& e) noexcept
        {
            const auto tdata = static_cast<T*>( data );

            switch ( e.style() )
            {
                case event_style::size:       return call_event(tdata, e.as<event_style::size>());
                case event_style::paint:      return call_event(tdata, e.as<event_style::paint>());
                case event_style::close:      return call_event(tdata, e.as<event_style::close>());
                case event_style::mouse_move: return call_event(tdata, e.as<event_style::mouse_move>());
            }

            return call_event(tdata, e);
        }
    };
}

