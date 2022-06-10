#pragma once

#include <core/clamp_cast.h>

#include <px/pxfwd.h>

#include <ui/event_fwd.h>

namespace ui
{
#ifdef D_OS_WINDOWS
    using word_parameter_t = size_t;
    using long_parameter_t = ptrdiff_t;
#endif

    class event
    {
    public:
        template<event_style style> 
        [[nodiscard]] constexpr const specialized_event<style>& as() const noexcept
        {
            D_ASSERT(style == style_);
            return static_cast<const specialized_event<style>&>(*this);
        }

        [[nodiscard]]
        constexpr event_style style() const noexcept
        {
            return style_;
        }

#if defined(D_OS_WINDOWS)
        constexpr event(
            window_handle_t window,
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
        constexpr window_handle_t window() const noexcept
        {
            return window_;
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
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
            return as_pxside(lo_cast<word_t>(static_cast<dword_t>(long_parameter())));
            D_WARNING_POP;
        }

        [[nodiscard]]
        constexpr pxside_t y_long_parameter() const noexcept
        {
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
            return as_pxside(hi_cast<word_t>(static_cast<dword_t>(long_parameter())));
            D_WARNING_POP;
        }

        [[nodiscard]]
        constexpr px::vec2 vec_long_parameter() const noexcept
        {
            return { x_long_parameter(), y_long_parameter() };
        }

#elif defined(D_OS_ANDROID)
        constexpr explicit event(event_style style) noexcept
            : style_{ style }
        {}

#endif

    private:
#ifdef D_OS_WINDOWS
        window_handle_t window_;
        word_parameter_t word_parameter_;
        long_parameter_t long_parameter_;
#endif

        event_style style_;
    };

    template<event_style>
    class specialized_event : public event
    {};

#if defined(D_OS_WINDOWS)
    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_enum_is_unscoped__prefer_enum_class);

    struct mouse_keys
    {
        enum e_mouse_keys : word_parameter_t
        {
            lbutton = 0x0001, // MK_LBUTTON,
            rbutton = 0x0002, // MK_RBUTTON,
            shift   = 0x0004, // MK_SHIFT,
            control = 0x0008, // MK_CONTROL,
            mbutton = 0x0010  // MK_MBUTTON
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

    D_WARNING_POP;

    class mouse_event : public event
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
            return { underlying_cast<mouse_keys::e_mouse_keys>(word_parameter()) };
        }
    };

    template<>
    class specialized_event<event_style::mouse_lbutton_down> : public mouse_event
    {};

    template<>
    class specialized_event<event_style::mouse_lbutton_up> : public mouse_event
    {};

    template<>
    class specialized_event<event_style::mouse_move> : public mouse_event
    {};

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

#elif defined(D_OS_ANDROID)
    struct input_event : public event
    {
        constexpr input_event(const AInputEvent* e) noexcept
            : event{ event_style::null }
        {
            D_ASSERT(e);
        }
    };

#endif
}