#pragma once

#include <functional>

#include <core/point.h>
#include <core/underlying_cast.h>
#include <core/flags.h>

#include <platform/windows/config.h>

namespace os_windows
{
    enum class event_type : UINT
    {
        null = WM_NULL,
        close = WM_CLOSE,
        timer = WM_TIMER,
        mouse_move = WM_MOUSEMOVE,
        out_of_os
    };

    template <event_type type>
    using event_type_constant = std::integral_constant<event_type, type>;

    template<event_type special_type>
    class special_event;

    class event
    {
    public:
        constexpr event( HWND window_handle, event_type type, WPARAM word_parameter, LPARAM long_parameter ) noexcept
            : window_handle_{ window_handle }
            , long_parameter_{ long_parameter }
            , word_parameter_{ word_parameter }
            , type_{ type }
        {}

        friend LRESULT default_event_handler( const event& e ) noexcept;

        constexpr event_type type() const noexcept
        {
            return type_;
        }

        constexpr HWND window_handle() const noexcept
        {
            return window_handle_;
        }

        constexpr bool is( event_type checked_type ) const noexcept
        {
            return checked_type == type_;
        }

        template<event_type special_type>
        constexpr const special_event<special_type>& as() const noexcept;

    protected:
        constexpr WPARAM word_parameter() const noexcept
        {
            return word_parameter_;
        }

        constexpr LPARAM long_parameter() const noexcept
        {
            return long_parameter_;
        }

    private:
        HWND window_handle_;
        LPARAM long_parameter_;
        WPARAM word_parameter_;
        event_type type_;
    };


    template<event_type special_type>
    class special_event : public event
    {};

    template<event_type special_type>
    constexpr const special_event<special_type>& event::as() const noexcept
    {
        assert( is( special_type ) );
        return static_cast<const special_event<special_type>&>( *this );
    }

    enum class mouse_key : WPARAM
    {
        control = MK_CONTROL,
        lbutton = MK_LBUTTON,
        mbutton = MK_MBUTTON,
        rbutton = MK_RBUTTON,
        shift = MK_SHIFT,
        xbutton1 = MK_XBUTTON1,
        xbutton2 = MK_XBUTTON2
    };

    template<>
    class special_event<event_type::mouse_move> : public event
    {
    public:
        constexpr int x() const noexcept
        {
            return GET_X_LPARAM( long_parameter() );
        }

        constexpr int y() const noexcept
        {
            return GET_Y_LPARAM( long_parameter() );
        }

        constexpr point<int> position() const noexcept
        {
            return { x(), y() };
        }

        constexpr flags<mouse_key> key() const noexcept
        {
            return { underlying_cast<mouse_key>( word_parameter() ) };
        }
    };

    using timer_event = special_event<event_type::timer>;
    using mouse_move_event = special_event<event_type::mouse_move>;
    using close_event = special_event<event_type::close>;
}