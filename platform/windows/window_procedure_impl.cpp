#include <platform/windows/event_handler_container.h>
#include <platform/windows/event.h>

namespace os_windows
{
    LRESULT CALLBACK window_procedure(HWND window_handle, UINT message, WPARAM word_parameter, LPARAM long_parameter) noexcept
    {
        switch (message)
        {
            case WM_DESTROY:
            case WM_NCDESTROY:
            {
                event_handler_container_global().erase(window_handle);
            }
            break;

            case WM_QUIT:
            {
                event_handler_container_global().clear();
            }
            break;

            default:
            {
                const auto& map = const_event_handler_container_global();
                auto items = map.lower_bound(window_handle);

                if (starts_with_key(items, window_handle))
                {
                    const event current_event =
                    {
                        {
                            window_handle,
                            items.first->value.id()
                        },
                        word_parameter,
                        long_parameter,
                        underlying_cast<event_style>(message)
                    };

                    auto revision = map.revision();

                    do
                    {
                        const auto current_item_id = items.first->value.id();

                        auto current_item = std::exchange
                        (
                            const_cast<event_handler_item&>(items.first->value),
                            event_handler_item
                            {
                                current_item_id,
                                ignore_event_callback,
                                event_callback_state::in_process
                            }
                        );

                        const auto result = current_item.do_process_event(current_event);
                        if (revision == map.revision())
                        {
                            const_cast<event_handler_item&>(items.first->value) = std::move(current_item);
                            ++items.first;
                        }
                        else
                        {
                            revision = map.revision();

                            items = map.find
                            (
                                event_handler_view
                                {
                                    current_event.window(),
                                    current_item_id
                                }
                            );

                            if (items.first != items.last)
                            {
                                {
                                    const auto& cvalue_ref = items.first->value;
                                    if (cvalue_ref.callback_state() == event_callback_state::in_process)
                                    {
                                        const_cast<event_handler_item&>(cvalue_ref) = std::move(current_item);
                                    }
                                }

                                ++items.first;
                            }
                        }

                        if (result.options == event_result_options::accept)
                        {
                            return result.result;
                        }
                    }
                    while (starts_with_key(items, window_handle));
                }
            }
            break;
        }

        return DefWindowProcW(window_handle, message, word_parameter, long_parameter);
    }
}