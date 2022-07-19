#pragma once

#include <core/clamp_cast.h>

#include <px/fwd.h>

#include <ui/event_fwd.h>

namespace ui
{
#ifdef D_OS_WINDOWS
    using word_parameter_t = size_t;
    using long_parameter_t = ptrdiff_t;
#endif

    struct idle_event {};


#if defined(D_OS_WINDOWS)
    class event
    {
    public:
        constexpr event
        (
            window_handle_t window,
            event_style style,
            word_parameter_t word_parameter,
            long_parameter_t long_parameter
         ) noexcept
            : long_parameter_{ long_parameter }
            , window_{ window }
            , word_parameter_{ word_parameter }
            , style_{ style }
        {}

        [[nodiscard]]
        constexpr window_handle_t window() const noexcept
        {
            return window_;
        }

        [[nodiscard]]
        constexpr event_style style() const noexcept
        {
            return style_;
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


    private:
        long_parameter_t long_parameter_;
        window_handle_t window_;
        word_parameter_t word_parameter_;
        event_style style_;
    };

#elif defined(D_OS_ANDROID)
    [[nodiscard]]
    constexpr event_style to_event_style(int32_t action) noexcept
    {
        return underlying_cast<event_style>(action);
    }

    class event
    {
    public:
        static constexpr int32_t action_mask{ 0xff };

        [[nodiscard]]
        constexpr event_style style() const noexcept
        {
            return to_event_style(action_ & action_mask);
        }

    protected:
        constexpr explicit event(int32_t action) noexcept
            : action_{ action }
        {}
        
        [[nodiscard]]
        constexpr int32_t action() const noexcept
        {
            return action_;
        }

    private:
        int32_t action_;
    };

#endif

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

#elif defined(D_OS_ANDROID)
    class mouse_event : public event
    {
    public:
        static constexpr int32_t p_index_mask{ 0xff00 };
        static constexpr int32_t p_index_shift{ 8 };
        
        [[nodiscard]]
        static mouse_event instance(const AInputEvent* input_e) noexcept;

        constexpr explicit operator bool() const noexcept
        {
            return nullptr != input_e_;
        }

        [[nodiscard]]
        constexpr size_t index() const noexcept
        {
            return narrow_cast<size_t>((event::action() & p_index_mask) >> p_index_shift);
        }

        [[nodiscard]]
        float x_by_index(size_t index) const noexcept;
        
        [[nodiscard]]
        float y_by_index(size_t index) const noexcept;

        struct cursor_position
        {
            const mouse_event& e;
            const size_t index;

            [[nodiscard]]
            float x() const noexcept
            {
                return e.x_by_index(index);
            }

            [[nodiscard]]
            float y() const noexcept
            {
                return e.y_by_index(index);
            }
        };

        [[nodiscard]]
        constexpr cursor_position position_by_index(size_t index) const noexcept
        {
            return { *this, index };
        }

        [[nodiscard]]
        size_t number_of_positions() const;
    
        [[nodiscard]]
        float x() const noexcept
        {
            return x_by_index(index());
        }

        [[nodiscard]]
        float y() const noexcept
        {
            return y_by_index(index());
        }

        [[nodiscard]]
        constexpr cursor_position position() const noexcept
        {
            return position_by_index(index());
        }

    private:
        constexpr mouse_event(int32_t action, const AInputEvent* input_e) noexcept
            : event{ action }
            , input_e_{ input_e }
        {}

    private:
        const AInputEvent* input_e_;
    };

#endif

    template<>
    class specialized_event<event_style::mouse_down> : public mouse_event
    {};

    template<>
    class specialized_event<event_style::mouse_up> : public mouse_event
    {};

    template<>
    class specialized_event<event_style::mouse_move> : public mouse_event
    {};

#if defined(D_OS_WINDOWS)
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
#endif


    template<event_style style> [[nodiscard]]
    constexpr const specialized_event<style>& event_specializing_for(const event& e) noexcept
    {
        D_ASSERT(style == e.style());
        return static_cast<const specialized_event<style>&>(e);
    }
}