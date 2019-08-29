#pragma once

#include <string>

#include <platform/windows/window_type.h>
#include <platform/windows/event.h>

namespace os_windows
{
    class window
    {
    public:
        constexpr window() noexcept = default;

        window( safe_window_type type, event_handler_type event_handler, not_null<HWND> window_handle ) noexcept
            : window_type_{ std::move( type ) }
            , event_handler_{ std::move( event_handler ) }
            , window_handle_{ window_handle.get() }
        {
            assert( is_valid() );
        }


        constexpr bool is_valid() const noexcept
        {
            return window_type_ && window_handle_;
        }

        bool show( int cmd = SW_SHOW ) const noexcept;

        bool update() const noexcept;

        HRESULT process_event( event e ) const noexcept
        {
            assert( is_event_handler() );
            return event_handler_( e );
        }

        constexpr bool is_event_handler() const noexcept
        {
            return event_handler_ != nullptr;
        }

        void construct_weak( const void& ) noexcept;

        void replace_weak( const void&, const safe_handle<window>& new_window ) noexcept;

        void close( const void& ) noexcept;

    private:
        constexpr HWND release_window_handle() noexcept
        {
            const auto temp_window_handle = window_handle_;
            window_handle_ = nullptr;
            return temp_window_handle;
        }

        constexpr bool release_event_handler() noexcept
        {
            const bool is_event_handler = is_event_handler();
            event_handler_ = nullptr;
            return is_event_handler;
        }

    private:
        safe_window_type window_type_;
        event_handler_type event_handler_;
        HWND window_handle_ = nullptr;
    };

    using safe_window = safe_handle<window>;

    class window_info
    {
    public:
        constexpr window_info() noexcept = default;

        window_info& type( safe_window_type type ) noexcept
        {
            type_ = std::move( type );
            return *this;
        }

        window_info& title( std::wstring title ) noexcept
        {
            title_ = std::move( title );
            return *this;
        }

        window_info& event_handler( event_handler_type event_handler ) noexcept
        {
            event_handler_ = std::move( event_handler );
            return *this;
        }

        friend safe_window create_window( window_info info ) noexcept;

    private:
        safe_window_type type_;
        event_handler_type event_handler_;
        std::wstring title_;
        DWORD style_ = WS_OVERLAPPEDWINDOW;
        int x_ = CW_USEDEFAULT;
        int y_ = 0;
        int width_ = CW_USEDEFAULT;
        int height_ = 0;
    };

    safe_window create_window( window_info info ) noexcept;
}