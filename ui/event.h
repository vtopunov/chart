#pragma once

#include <core/narrow.h>

#include <px/pxfwd.h>

#include <ui/event_fwd.h>

namespace ui
{
    using word_parameter_t = WPARAM;
    using long_parameter_t = LPARAM;

    enum class event_style : UINT
    {
        null = WM_NULL,
        size = WM_SIZE,
        paint = WM_PAINT,
        close = WM_CLOSE,
        mouse_move = WM_MOUSEMOVE,
        mouse_lbutton_double_click = WM_LBUTTONDBLCLK,
        user = WM_USER
    };

    template<event_style>
    class specialized_event;

    class event
    {
    public:
        constexpr event(
            window_resource window,
            word_parameter_t word_parameter,
            long_parameter_t long_parameter,
            event_style style
            ) noexcept
            : window_{ window }
            , word_parameter_{ word_parameter }
            , long_parameter_{ long_parameter }
            , style_{ style }
        {}

        [[nodiscard]]
        constexpr event_style style() const noexcept
        {
            return style_;
        }

        [[nodiscard]]
        constexpr window_resource window() const noexcept
        {
            return window_;
        }

        template<event_style style> [[nodiscard]]
        constexpr const specialized_event<style>& as() const noexcept
        {
            D_ASSERT(style == style_);
            return static_cast<const specialized_event<style>&>(*this);
        }

        event_result_t do_default_process() const noexcept;

    protected:
        [[nodiscard]]
        constexpr word_parameter_t word_parameter() const noexcept
        {
            return word_parameter_;
        }

        [[nodiscard]]
        constexpr long_parameter_t long_parameter() const noexcept
        {
            return long_parameter_;
        }

        [[nodiscard]]
        constexpr pxside_t x_long_parameter() const noexcept
        {
            return as_pxside( GET_X_LPARAM(long_parameter()) );
        }

        [[nodiscard]]
        constexpr pxside_t y_long_parameter() const noexcept
        {
            return as_pxside( GET_Y_LPARAM(long_parameter()) );
        }

        [[nodiscard]]
        constexpr px::vec2 vec_long_parameter() const noexcept
        {
            return { x_long_parameter(), y_long_parameter() };
        }

    private:
        window_resource window_;
        word_parameter_t word_parameter_;
        long_parameter_t long_parameter_;
        event_style style_;
    };

    template<event_style>
    class specialized_event : public event
    {};

    struct mouse_keys
    {
        enum e_mouse_keys : word_parameter_t
        {
            control = MK_CONTROL,
            lbutton = MK_LBUTTON,
            mbutton = MK_MBUTTON,
            rbutton = MK_RBUTTON,
            shift = MK_SHIFT,
            xbutton1 = MK_XBUTTON1,
            xbutton2 = MK_XBUTTON2
        };
        
        e_mouse_keys keys;

        [[nodiscard]]
        constexpr bool is(e_mouse_keys key) const noexcept
        {
            return (keys & key) == key;
        }

        [[nodiscard]]
        constexpr bool is_left() const noexcept
        {
            return is(lbutton);
        }

        [[nodiscard]]
        constexpr bool is_rigth() const noexcept
        {
            return is(rbutton);
        }

        [[nodiscard]]
        constexpr bool is_middle() const noexcept
        {
            return is(mbutton);
        }

        [[nodiscard]]
        constexpr bool is_pressed_shift() const noexcept
        {
            return is(shift);
        }

        [[nodiscard]]
        constexpr bool is_pressed_ctrl() const noexcept
        {
            return is(control);
        }
    };

    template<>
    class specialized_event<event_style::mouse_move> : public event
    {
    public:
        [[nodiscard]]
        constexpr pxside_t x() const noexcept
        {
            return x_long_parameter();
        }

        [[nodiscard]]
        constexpr pxside_t y() const noexcept
        {
            return y_long_parameter();
        }

        [[nodiscard]]
        constexpr px::point2d position() const noexcept
        {
            return { vec_long_parameter() };
        }

        [[nodiscard]]
        constexpr mouse_keys keys() const noexcept
        {
            return { safe_numeric_cast<mouse_keys::e_mouse_keys>( word_parameter() ) };
        }
    };

    template<>
    class specialized_event<event_style::size> : public event
    {
    public:
        [[nodiscard]]
        constexpr pxside_t width() const noexcept
        {
            return x_long_parameter();
        }

        [[nodiscard]]
        constexpr pxside_t height() const noexcept
        {
            return y_long_parameter();
        }

        [[nodiscard]]
        constexpr px::size2d sizes() const noexcept
        {
            return { vec_long_parameter() };
        }
    };

    using size_event = specialized_event<event_style::size>;
    using paint_event = specialized_event<event_style::paint>;
    using close_event = specialized_event<event_style::close>;
    using mouse_move_event = specialized_event<event_style::mouse_move>;
}