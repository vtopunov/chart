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
        mouse_move = WM_MOUSEMOVE,
        event_handler_registered = WM_USER,
        user
    };

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

        template<event_type special_type>
        constexpr const special_event<special_type>& as() const noexcept;

        friend LRESULT default_event_handler( event e ) noexcept;

    protected:
        constexpr event_type type() const noexcept
        {
            return type_;
        }

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
    class special_event_base : public event
    {
    public:
        explicit constexpr operator bool() const noexcept
        {
            return special_type == type();
        }
    };

    template<event_type special_type>
    class special_event : public special_event_base<special_type>
    {};

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
    class special_event<event_type::mouse_move> : public special_event_base<event_type::mouse_move>
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

    using mouse_move_event = special_event<event_type::mouse_move>;

    template<event_type special_type>
    constexpr const special_event<special_type>& event::as() const noexcept
    {
        return static_cast<const special_event<special_type>&>( *this );
    }
}