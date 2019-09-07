#pragma once

#include <platform/windows/event.h>
#include <platform/windows/window.h>

namespace os_windows
{
    using event_handler_type = std::function<LRESULT( event )>;

    LRESULT default_event_handler( event e ) noexcept;

    constexpr size_t invaid_procedure_id = std::numeric_limits<size_t>::max();

    class event_handler_handle
    {
    public:
        constexpr event_handler_handle() noexcept = default;

        event_handler_handle( HWND window_handle, size_t procedure_id ) noexcept
            : window_handle_{ window_handle }
            , procedure_id_{ procedure_id }
        {
            assert( window_handle_ );
        }

        void close() noexcept;

        constexpr bool is_valid() const noexcept
        {
            return procedure_id_ != invaid_procedure_id;
        }

    private:
        constexpr size_t release_procedure_id() noexcept
        {
            const auto temp = procedure_id_;
            procedure_id_ = invaid_procedure_id;
            return temp;
        }

    private:
        HWND window_handle_;
        size_t procedure_id_{ invaid_procedure_id };
    };

    using safe_event_handler_handle = safe_handle<event_handler_handle>;

    safe_event_handler_handle register_event_handler( window_view window, event_handler_type event_handler ) noexcept;
}