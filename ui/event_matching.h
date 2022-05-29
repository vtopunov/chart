#pragma once

#include <chrono>

#include <ui/event.h>

namespace ui
{
    using milliseconds_t = std::chrono::milliseconds;

    constexpr auto infinite = milliseconds_t{ INFINITE };
   

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


    template<class T>
    struct event_callback_instance
    {
        static std::optional<event_result_t> callback(void* data, const event& e) noexcept
        {
            const auto tdata = static_cast<T*>( data );

            switch ( e.style() )
            {
                case event_style::size:               return call_event(tdata, e.as<event_style::size>());
                case event_style::paint:              return call_event(tdata, e.as<event_style::paint>());
                case event_style::close:              return call_event(tdata, e.as<event_style::close>());
                case event_style::mouse_move:         return call_event(tdata, e.as<event_style::mouse_move>());
                case event_style::mouse_lbutton_down: return call_event(tdata, e.as<event_style::mouse_lbutton_down>());
                case event_style::mouse_lbutton_up:   return call_event(tdata, e.as<event_style::mouse_lbutton_up>());
            }

            return call_event(tdata, e);
        }
    };
}

