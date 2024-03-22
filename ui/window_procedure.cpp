#include <os/os.h>

#include <ui/window_procedure.h>
#include <ui/event.h>
#include <ui/window.h>
#include <ui/event_processors_storage.h>


namespace ui
{
    namespace
    {
        constexpr event_style to_event_style(uint_t message) noexcept
        {
            return underlying_cast<event_style>(message);
        }

        template<WPARAM test_value>
        constexpr bool test_mouse_wheel_delta_v = (mouse_wheel_delta(test_value) == (GET_WHEEL_DELTA_WPARAM(test_value)));
    }

    event_result_t D_OS_APICALL window_procedure
    (
        window_handle_t window, 
        uint_t message, 
        word_parameter_t word_parameter, 
        long_parameter_t long_parameter
    ) noexcept
    {
        static_assert(event_style::null == to_event_style(WM_NULL));
        static_assert(event_style::size == to_event_style(WM_SIZE));
        static_assert(event_style::quit == to_event_style(WM_QUIT));
        static_assert(event_style::mouse_wheel == to_event_style(WM_MOUSEWHEEL));
        static_assert(event_style::mouse_move == to_event_style(WM_MOUSEMOVE));
        static_assert(event_style::mouse_down == to_event_style(WM_LBUTTONDOWN));
        static_assert(event_style::mouse_up == to_event_style(WM_LBUTTONUP));
        static_assert(event_style::mouse_double_click == to_event_style(WM_LBUTTONDBLCLK));

        static_assert(mouse_keys::lbutton == mouse_keys::instance(MK_LBUTTON));
        static_assert(mouse_keys::rbutton == mouse_keys::instance(MK_RBUTTON));
        static_assert(mouse_keys::shift == mouse_keys::instance(MK_SHIFT));
        static_assert(mouse_keys::control == mouse_keys::instance(MK_CONTROL));
        static_assert(mouse_keys::mbutton == mouse_keys::instance(MK_MBUTTON));
        static_assert(min_mouse_wheel_delta == WHEEL_DELTA);
        static_assert(std::is_same_v<ui::mouse_wheel_delta_t, decltype(GET_WHEEL_DELTA_WPARAM(std::declval<WPARAM>()))>);
        static_assert(test_mouse_wheel_delta_v<numeric_max_v<WPARAM>>);

        const event e { window, to_event_style(message), word_parameter, long_parameter };

        switch (message)
        {
            case WM_DESTROY:
            {
                close(window);
            }
            break;

            default: [[likely]]
            {
                for (const auto& processor : event_processors_global().lock()) [[likely]]
                {
                    if (processor.window == window)
                    {
                        if (const auto result = processor(e))
                        {
                            return *result;
                        }
                    }
                }
            }
            break;
        }

        return e.do_default_process();
    }

    event_result_t event::do_default_process() const noexcept
    {
        return DefWindowProcW(window_, to_underlying(style_), word_parameter_, long_parameter_);
    }
}