#pragma once

#include <ui/fwd.h>


namespace ui
{
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
        constexpr word_parameter_t _word_parameter() const noexcept
        {
            return word_parameter_;
        }

        [[nodiscard]]
        constexpr long_parameter_t _long_parameter() const noexcept
        {
            return long_parameter_;
        }

        using _coordinate_value_type = npx_t;
        static_assert(is_safe_numeric_conversion_v<_coordinate_value_type, word_t>);

        [[nodiscard]]
        constexpr _coordinate_value_type _x_coordinate() const noexcept
        {
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
            return lo_cast<word_t>(static_cast<dword_t>(_long_parameter()));
            D_WARNING_POP;
        }

        [[nodiscard]]
        constexpr _coordinate_value_type _y_coordinate() const noexcept
        {
            D_WARNING_PUSH;
            D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
            return hi_cast<word_t>(static_cast<dword_t>(_long_parameter()));
            D_WARNING_POP;
        }

        [[nodiscard]]
        constexpr _coordinate_value_type _x_coordinate(size_t index) const noexcept
        {
            D_ASSERT(index < _size()); D_UNUSED(index);
            return _x_coordinate();
        }

        [[nodiscard]]
        constexpr _coordinate_value_type _y_coordinate(size_t index) const noexcept
        {
            D_ASSERT(index < _size()); D_UNUSED(index);
            return _y_coordinate();
        }

        [[nodiscard]]
        constexpr size_t _index() const noexcept
        {
            return 0_uz;
        }

        [[nodiscard]]
        constexpr size_t _size() const noexcept
        {
            return 1_uz;
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
        static constexpr int32_t invalid_action{ -1 };
        static constexpr int32_t action_mask{ 0xff };
        static constexpr int32_t p_index_mask{ 0xff00 };
        static constexpr int32_t p_index_shift{ 8 };

        constexpr event() noexcept = default;

        event(const AInputEvent* input_e) noexcept;

        constexpr explicit operator bool() const noexcept
        {
            return invalid_action != action_;
        }

        [[nodiscard]]
        constexpr event_style style() const noexcept
        {
            return to_event_style(action_ & action_mask);
        }

    protected:
        [[nodiscard]]
        constexpr int32_t action() const noexcept
        {
            return action_;
        }

        using _coordinate_value_type = float;

        [[nodiscard]]
        _coordinate_value_type _x_coordinate() const noexcept
        {
            return _x_coordinate(_index());
        }

        [[nodiscard]]
        _coordinate_value_type _y_coordinate() const noexcept
        {
            return _y_coordinate(_index());
        }

        [[nodiscard]]
        _coordinate_value_type _x_coordinate(size_t index) const noexcept;

        [[nodiscard]]
        _coordinate_value_type _y_coordinate(size_t index) const noexcept;

        [[nodiscard]]
        constexpr size_t _index() const noexcept
        {
            return static_cast<size_t>((action_ & p_index_mask) >> p_index_shift);
        }

        [[nodiscard]]
        size_t _size() const noexcept;

    private:
        const AInputEvent* input_e_{ nullptr };
        int32_t action_{ invalid_action };
    };
#endif

    class pointer_event : public event
    {
    public:
        using value_type = event::_coordinate_value_type;
        using point2d_type = point2d<value_type>;

        [[nodiscard]]
        D_ONLY_OS_WINDOWS(constexpr) value_type x() const noexcept
        {
            return _x_coordinate();
        }

        [[nodiscard]]
        D_ONLY_OS_WINDOWS(constexpr) value_type y() const noexcept
        {
            return _y_coordinate();
        }

        [[nodiscard]]
        D_ONLY_OS_WINDOWS(constexpr) value_type x(size_t index) const noexcept
        {
            return _x_coordinate(index);
        }

        [[nodiscard]]
        D_ONLY_OS_WINDOWS(constexpr) value_type y(size_t index) const noexcept
        {
            return _y_coordinate(index);
        }

        [[nodiscard]]
        D_ONLY_OS_WINDOWS(constexpr) point2d_type pointer(size_t index) const noexcept
        {
            return { x(index), y(index) };
        }

        [[nodiscard]]
        D_ONLY_OS_WINDOWS(constexpr) point2d_type pointer() const noexcept
        {
            return { x(), y() };
        }

        [[nodiscard]]
        constexpr size_t index() const noexcept
        {
            return _index();
        }

        [[nodiscard]]
        D_ONLY_OS_WINDOWS(constexpr) size_t size() const noexcept
        {
            return _size();
        }
    };

    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_enum_is_unscoped__prefer_enum_class);

    struct mouse_keys
    {
        enum e_mouse_keys : word_t
        {
            lbutton = 0x0001, // MK_LBUTTON,
            rbutton = 0x0002, // MK_RBUTTON,
            shift = 0x0004, // MK_SHIFT,
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

        [[nodiscard]]
        constexpr bool operator == (const mouse_keys&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const mouse_keys&) const noexcept = default;

        [[nodiscard]]
        static constexpr mouse_keys instance(word_parameter_t word_parameter) noexcept
        {
            return { .keys{ underlying_cast<mouse_keys::e_mouse_keys>(lo_cast<word_t>(word_parameter)) } };
        }
    };

    [[nodiscard]]
    constexpr bool operator == (mouse_keys left, mouse_keys::e_mouse_keys right) noexcept
    {
        return left.keys == right;
    }

    [[nodiscard]]
    constexpr bool operator == (mouse_keys::e_mouse_keys left, mouse_keys right) noexcept
    {
        return left == right.keys;
    }

    [[nodiscard]]
    constexpr bool operator != (mouse_keys left, mouse_keys::e_mouse_keys right) noexcept
    {
        return left.keys != right;
    }

    [[nodiscard]]
    constexpr bool operator != (mouse_keys::e_mouse_keys left, mouse_keys right) noexcept
    {
        return left != right.keys;
    }

    constexpr mouse_keys touchpad_dummy_mouse_keys{ mouse_keys::lbutton };

    class mouse_event : public pointer_event
    {
    public:
        [[nodiscard]] constexpr mouse_keys keys() const noexcept
        {
            return D_CONDITIONAL_OS_WINDOWS(mouse_keys::instance(_word_parameter()), touchpad_dummy_mouse_keys);
        }
    };

    D_WARNING_POP; // W_enum_is_unscoped__prefer_enum_class


    template<event_style>
    class specialized_event : public event
    {};

    template<>
    class specialized_event<event_style::mouse_down> : public mouse_event
    {};

    template<>
    class specialized_event<event_style::mouse_up> : public mouse_event
    {};

    template<>
    class specialized_event<event_style::mouse_move> : public mouse_event
    {};


    using mouse_wheel_delta_t = short;
    constexpr mouse_wheel_delta_t min_mouse_wheel_delta{ 120 };

    [[nodiscard]]
    constexpr mouse_wheel_delta_t mouse_wheel_delta(word_parameter_t word_parametr) noexcept
    {
        return static_cast<mouse_wheel_delta_t>(hi_cast<word_t>(word_parametr));
    }


#ifdef D_OS_WINDOWS
    template<>
    class specialized_event<event_style::mouse_wheel> : public mouse_event
#else
    class mouse_wheel_event : public mouse_event
#endif
    {
    public:
        [[nodiscard]]
        constexpr mouse_wheel_delta_t delta() const noexcept
        {
            return D_CONDITIONAL_OS_WINDOWS(mouse_wheel_delta(_word_parameter()), 0);
        }

        [[nodiscard]]
        constexpr double rot() const noexcept
        {
            return numeric_cast<double>(delta()) / min_mouse_wheel_delta;
        }
    };

#ifdef D_OS_WINDOWS
    template<>
    class specialized_event<event_style::mouse_double_click> : public mouse_event {};
#else
    class mouse_double_click_event : public mouse_event {};
#endif


#ifdef D_OS_WINDOWS
    template<>
    class specialized_event<event_style::size> : public event
#else
    class size_event : public event
#endif
    {
    public:
        using event::event;
        using value_type = event::_coordinate_value_type;
        using size2d_type = size2d<value_type>;

        [[nodiscard]]
        constexpr value_type width() const noexcept
        {
            return _x_coordinate();
        }

        [[nodiscard]]
        constexpr value_type height() const noexcept
        {
            return _y_coordinate();
        }

        [[nodiscard]]
        constexpr size2d_type sizes() const noexcept
        {
            return { width(), height() };
        }
    };

    template<event_style style>
    [[nodiscard]] constexpr const specialized_event<style>& event_for(const event& e) noexcept
    {
        D_ASSERT(style == e.style());
        return identical_derived_cast<const specialized_event<style>&>(e);
    }


    [[nodiscard]]
    constexpr cmd_event_style to_cmd_event_style(int32_t cmd) noexcept
    {
        return underlying_cast<cmd_event_style>(cmd);
    }

    class cmd_event
    {
    public:
        constexpr cmd_event(cmd_event_style style) noexcept
            : style_{ style }
        {}

        [[nodiscard]]
        constexpr cmd_event_style style() const noexcept
        {
            return style_;
        }

    private:
        cmd_event_style style_;
    };

    template<cmd_event_style>
    class specialized_cmd_event : public cmd_event
    {};

    template<cmd_event_style style>
    [[nodiscard]] constexpr const specialized_cmd_event<style>& cmd_event_for(const cmd_event& e) noexcept
    {
        D_ASSERT(style == e.style());
        return static_cast<const specialized_cmd_event<style>&>(e);
    }

}