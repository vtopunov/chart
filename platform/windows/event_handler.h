#pragma once

#include <functional>

#include <core/safe_handle.h>

#include <platform/windows/window_view.h>
#include <platform/windows/event_handler_fwd.h>

namespace os_windows
{
    class event_dispatcher
    {
    public:
        constexpr event_dispatcher() noexcept = default;

        constexpr event_dispatcher( HWND window_handle, procedure_id_t procedure_id ) noexcept
            : window_handle_{ window_handle }
            , procedure_id_{ procedure_id }
        {}

        void close() noexcept;

        constexpr bool is_valid() const noexcept
        {
            return is_valid_procedure_id( procedure_id_ );
        }

    private:
        constexpr procedure_id_t release_procedure_id() noexcept
        {
            const auto temp = procedure_id_;
            procedure_id_ = invaid_procedure_id;
            return temp;
        }

    private:
        HWND window_handle_{ nullptr };
        procedure_id_t procedure_id_{ invaid_procedure_id };
    };

    using safe_event_dispatcher = safe_handle<event_dispatcher>;

    using event_handler_type = std::function<LRESULT(const event&)>;

    safe_event_dispatcher register_event_handler( window_view window, event_handler_type event_handler ) noexcept;
}