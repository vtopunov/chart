#pragma once

#include <core/point.h>
#include <core/underlying_cast.h>
#include <core/flags.h>

#include <platform/windows/defs.h>

namespace os_windows
{
    enum class event_style : UINT
    {
        null = WM_NULL,
        size = WM_SIZE,
        paint = WM_PAINT,
        close = WM_CLOSE,
        timer = WM_TIMER,
        mouse_move = WM_MOUSEMOVE,
        user = WM_USER
    };

    template<event_style>
    class specialized_event;

    class event
    {
    public:
        constexpr event(window_view window, WPARAM word_parameter, LPARAM long_parameter, event_style style) noexcept
            : window_{ window }
            , word_parameter_{ word_parameter }
            , long_parameter_{ long_parameter }
            , style_{ style }
        {}

        constexpr event_style style() const noexcept
        {
            return style_;
        }

        constexpr window_view window() const noexcept
        {
            return window_;
        }

        template<event_style style>
        class specialized_event_pointer_wrapper
        {
        public:
            using const_pointer_base = const event*;
            using value_type = specialized_event<style>;
            using const_pointer = const value_type*;
            using const_reference = const value_type&;

            constexpr specialized_event_pointer_wrapper(const_pointer_base pointer) noexcept
                : pointer_{ pointer }
            {}

            explicit constexpr operator bool() const noexcept
            {
                return specialization_is_correct();
            }

            constexpr const_pointer operator ->() const noexcept
            {
                return get();
            }

            constexpr const_reference operator *() const noexcept
            {
                return *get();
            }

            constexpr bool specialization_is_correct() const noexcept
            {
                return pointer_->style() == style;
            }

            constexpr const_pointer get() const noexcept
            {
                D_ASSERT(specialization_is_correct());

#pragma warning(push)
#pragma warning(disable : 26491) // Don't use static_cast downcasts
                return static_cast<const_pointer>(pointer_);
#pragma warning(pop)
            }

        private:
            const_pointer_base pointer_;
        };

        template<event_style style>
        constexpr specialized_event_pointer_wrapper<style> as() const noexcept
        {
            return this;
        }

        event_result_t do_default_process() const noexcept;

        constexpr auto operator<=>(const event&) const noexcept = default;

    protected:
        constexpr WPARAM word_parameter() const noexcept
        {
            return word_parameter_;
        }

        constexpr LPARAM long_parameter() const noexcept
        {
            return long_parameter_;
        }

        constexpr pixel_t x_long_parameter() const noexcept
        {
            return narrow_cast<pixel_t>(GET_X_LPARAM(long_parameter()));
        }

        constexpr pixel_t y_long_parameter() const noexcept
        {
            return narrow_cast<pixel_t>(GET_Y_LPARAM(long_parameter()));
        }

    private:
        window_view window_;
        WPARAM word_parameter_;
        LPARAM long_parameter_;
        event_style style_;
    };

    template<event_style>
    class specialized_event : public event
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
    class specialized_event<event_style::mouse_move> : public event
    {
    public:
        constexpr pixel_t x() const noexcept
        {
            return x_long_parameter();
        }

        constexpr pixel_t y() const noexcept
        {
            return y_long_parameter();
        }

        constexpr point_t position() const noexcept
        {
            return { x(), y() };
        }

        constexpr flags<mouse_key> key() const noexcept
        {
            return { underlying_cast<mouse_key>(word_parameter()) };
        }
    };

    template<>
    class specialized_event<event_style::timer> : public event
    {
    public:
        constexpr size_t id() const noexcept
        {
#pragma warning(push)
#pragma warning(disable : 26472) // Don't use a static_cast for arithmetic conversions
            return static_cast<size_t>(word_parameter()); // WPARAM may be less than zero
#pragma warning(pop)
        }
    };

    template<>
    class specialized_event<event_style::size> : public event
    {
    public:
        constexpr pixel_t width() const noexcept
        {
            return x_long_parameter();
        }

        constexpr pixel_t height() const noexcept
        {
            return y_long_parameter();
        }

        constexpr rect_size_t size() const noexcept
        {
            return { width(), height() };
        }
    };

    using size_event = specialized_event<event_style::size>;
    using paint_event = specialized_event<event_style::paint>;
    using close_event = specialized_event<event_style::close>;
    using timer_event = specialized_event<event_style::timer>;
    using mouse_move_event = specialized_event<event_style::mouse_move>;
}