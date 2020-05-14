#include <platform/windows/window.h>
#include <platform/windows/event_processors_container.h>
#include <platform/windows/event.h>

namespace os_windows
{
    extern void clear_global_state() noexcept;

    LRESULT CALLBACK window_procedure(HWND window_handle, UINT message, WPARAM word_parameter, LPARAM long_parameter) noexcept
    {
        switch ( message )
        {
            case WM_DESTROY:
            case WM_NCDESTROY:
            {
                native_window_system::close_childrens(window_handle);
                event_processors_container_global().erase(window_handle);
            }
            break;

            case WM_QUIT:
            {
                clear_global_state();
            }
            break;

            default:
            {
                const auto& map = const_event_processors_container_global();
                auto items = map.lower_bound(window_handle);

                if ( starts_with_key(items, window_handle) )
                {
                    const event current_event =
                    {
                        {
                            window_handle,
                            items.first->value.id()
                        },
                        word_parameter,
                        long_parameter,
                        underlying_cast<event_style>( message )
                    };

                    auto revision = map.revision();

                    do
                    {
                        if ( items.first->value.has_ready_state() )
                        {
                            const auto current_item_id = items.first->value.id();

                            item_event_processor::process_context context{ as_mutable(items.first->value) };

                            const auto result = context.do_process(current_event);
                            if ( revision == map.revision() )
                            {
                                context.move_to(as_mutable(items.first->value));
                                ++items.first;
                            }
                            else
                            {
                                revision = map.revision();

                                items = map.find({ current_event.window(), current_item_id });

                                if ( items.first != items.last )
                                {
                                    {
                                        const auto& cvalue_ref = items.first->value;
                                        if ( cvalue_ref.in_process() )
                                        {
                                            context.move_to(as_mutable(cvalue_ref));
                                        }
                                    }

                                    ++items.first;
                                }
                            }

                            if ( result )
                            {
                                return *result;
                            }
                        }
                        else
                        {
                            ++items.first;
                        }
                    }
                    while ( starts_with_key(items, window_handle) );
                }
            }
            break;
        }

        return DefWindowProcW(window_handle, message, word_parameter, long_parameter);
    }

    event_result_t event::do_default_process() const noexcept
    {
        return DefWindowProcW(window_.handle, to_underlying(style_), word_parameter_, long_parameter_);
    }
}