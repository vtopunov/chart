#pragma once

#include <functional>

#include <core/defs.h>
#include <platform/windows/config.h>

namespace os_windows
{
    class event;

    struct window_view
    {
        HWND handle;
        size_t id;
    };

    constexpr window_view null_window_view{ nullptr, 0u };

    struct event_handler_view
    {
        window_view window;
        size_t id;

        constexpr bool is_root() const noexcept
        {
            return window.id == id;
        }
    };

    constexpr event_handler_view null_event_handler_view{ null_window_view, 0u };

    enum class event_result_options
    {
        accept,
        ignore
    };

    struct event_result
    {
        LRESULT result;
        event_result_options options;
    };

    constexpr event_result accept_event_result{ 0, event_result_options::accept };

    constexpr event_result ignore_event_result{ 0, event_result_options::ignore };

    constexpr event_result ignore_event_callback(const event&) noexcept
    {
        return ignore_event_result;
    }

    using event_callback_function = std::function<event_result (const event&)>;

    using event_callback_factory = std::function<event_callback_function (event_handler_view)>;

    template<class T>
    struct add_const_to_pointer
    {
        using type = T;
    };

    template<class T>
    struct add_const_to_pointer<T*>
    {
        using type = const T*;
    };

    template<class T>
    using add_const_to_pointer_t = typename add_const_to_pointer<T>::type;

    using const_window_handle_t = add_const_to_pointer_t<HWND>;
}