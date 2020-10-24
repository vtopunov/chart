#include <display/window.h>
#include <display/event_processors_container.h>
#include <display/event.h>

namespace display
{
    LRESULT CALLBACK window_procedure(HWND handle, UINT message, WPARAM word_parameter, LPARAM long_parameter) noexcept
    {
        const window_resource window{ handle };

        const event e
        {
            window,
            word_parameter,
            long_parameter,
            underlying_cast<event_style>( message )
        };

        switch ( message )
        {
            case WM_DESTROY:
            {
                close(window);
            }
            break;

            case WM_QUIT:
            {
                quit();
            }
            break;

            default:
            {
                for ( const auto& processor : event_processor::processors_container_global().lock() )
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
        return DefWindowProcW(window_.handle, to_underlying(style_), word_parameter_, long_parameter_);
    }
}