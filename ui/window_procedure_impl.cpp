#include <ui/event.h>
#include <ui/window.h>
#include <ui/event_processors_container.h>

namespace ui
{
    namespace
    {
        constexpr event_style to_event_style(UINT message) noexcept
        {
            return underlying_cast<event_style>(message);
        }
    }


    LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM word_parameter, LPARAM long_parameter) noexcept
    {
        static_assert(std::is_same_v<word_parameter_t, WPARAM>);
        static_assert(std::is_same_v<long_parameter_t, LPARAM>);
        static_assert(std::is_same_v<event_result_t, LRESULT>);

        static_assert(event_style::null == to_event_style(WM_NULL));
        static_assert(event_style::size == to_event_style(WM_SIZE));
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

        const event e { window, to_event_style(message), word_parameter, long_parameter };

        switch (message)
        {
            case WM_QUIT:
                break;

            case WM_DESTROY:
            {
                close(window);
            }
            break;

            default:
            {
                for (const auto& processor : event_processors_global().lock())
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
        static_assert(std::is_same_v<event_result_t, LRESULT>);

        return DefWindowProcW(window_, to_underlying(style_), word_parameter_, long_parameter_);
    }
}