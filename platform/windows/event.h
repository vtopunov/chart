#pragma once

#include <functional>

#include <core/point.h>

#include <platform/windows/config.h>

namespace os_windows
{
    using event_type = UINT;

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

    enum class mouse_key
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
    class special_event<WM_MOUSEMOVE> : public special_event_base<WM_MOUSEMOVE>
    {
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

        constexpr mouse_key key() const noexcept
        {
            return narrow_cast<mouse_key>( word_parameter() );
        }
    };

    template<event_type special_type>
    constexpr const special_event<special_type>& event::as() const noexcept
    {
        return static_cast<const special_event<special_type>&>( *this );
    }

    using event_handler_type = std::function<LRESULT(event)>;

    LRESULT default_event_handler( event e ) noexcept;
}