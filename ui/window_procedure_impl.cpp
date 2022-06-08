#include <ui/window.h>
#include <ui/event_processors_container.h>
#include <ui/event.h>

namespace ui
{
    LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM word_parameter, LPARAM long_parameter) noexcept
    {
        static_assert(std::is_same_v<event_result_t, LRESULT>);

        const event e
        {
            window,
            word_parameter,
            long_parameter,
            underlying_cast<event_style>( message )
        };

        switch ( message )
        {
            case WM_QUIT: break;

            case WM_DESTROY:
            {
                close(window);
            }
            break;

            default:
            {
                for ( const auto& processor : event_processors_global().lock() )
                {
                    if ( processor.window == window )
                    {
                        if ( const auto result = processor(e) )
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