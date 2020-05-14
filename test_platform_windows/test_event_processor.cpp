#include <platform/windows/event.h>
#include <platform/windows/event_processor.h>
#include <platform/windows/event_processors_container.h>

namespace os_windows
{
    extern LRESULT CALLBACK window_procedure(HWND, UINT, WPARAM, LPARAM);
}

using namespace os_windows;

LRESULT emit_event(const event& e) noexcept
{
    struct unpacked_event : public event
    {
        constexpr WPARAM word_parameter() const noexcept
        {
            return event::word_parameter();
        }

        constexpr LPARAM long_parameter() const noexcept
        {
            return event::long_parameter();
        }
    };

    const auto& event_parameters = static_cast<const unpacked_event&>( e );

    return window_procedure
    (
        e.window().handle,
        to_underlying(e.style()),
        event_parameters.word_parameter(),
        event_parameters.long_parameter()
    );
}

constexpr event null_event { null_window, 0u, 0L, event_style::null };

struct event_callback
{
    std::optional<event_result_t> current_result{ std::nullopt };
    mutable event last_event { null_event };
    mutable size_t counter{ 0u };

    constexpr std::optional<event_result_t> operator () (const event& e) const noexcept
    {
        ++counter;
        last_event = e;
        return current_result;
    }
};


bool test_event(const event& e, const event_callback& callback) noexcept
{
    D_ASSERT(e != null_event);

    const auto counter = callback.counter + 1u;

    callback.last_event = null_event;
    const auto result = emit_event(e);

    return counter == callback.counter
        && result == callback.current_result.value_or(result)
        && e == callback.last_event;
}

void test_event_processor() noexcept
{
    std::remove_pointer_t<window_handle_t> window_handle_impl{};
    constexpr window_view window{ &window_handle_impl, 1u };
    event_processors_container_global().insert_root(window.handle);

    event_callback callback{ 0x1234 };

    {
        constexpr event e { window, 0xfac4, 0xdeadbeef, event_style::size };

        const auto processor_owner = attach_event_processor(window, std::cref(callback));
        D_ASSERT(processor_owner);
        D_ASSERT(test_event(e, callback));
    }
}