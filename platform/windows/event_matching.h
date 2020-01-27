#pragma once

#include <array>

#include <platform/windows/defs.h>
#include <platform/windows/event.h>

namespace os_windows
{
    template<class function_type>
    struct event_matching
    {
        function_type function_;

        constexpr event_result operator () (const event& e) noexcept
        {
            switch (e.style())
            {
                case event_style::close: return on_specialized_event<event_style::close>(e);
                case event_style::timer: return on_specialized_event<event_style::timer>(e);
                case event_style::mouse_move: return on_specialized_event<event_style::mouse_move>(e);
            }

            return on_event(e);
        }

        constexpr event_result on_event( const event& e ) noexcept
        {
            return match( e );
        }

        template<event_style style>
        constexpr event_result on_specialized_event( const event& e ) noexcept
        {
            return match( *e.as<style>() );
        }

        template<class specialized_event>
        constexpr auto match( const specialized_event& e ) noexcept -> decltype( this->function_( e ) )
        {
            return function_( e );
        }

        struct ignore
        {
            constexpr ignore( const event& ) noexcept
            {}
        };

        constexpr event_result match(ignore) const noexcept
        {
            return ignore_event_result;
        }
    };

    template<class function_type>
    event_matching<std::decay_t<function_type>> event_match(function_type&& function) noexcept
    {
        return { std::forward<function_type>(function) };
    }
}

