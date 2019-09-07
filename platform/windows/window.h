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

        window( safe_window_type type, HWND window_handle ) noexcept
            : type_{ std::move( type ) }
            , handle_{ window_handle }
        {}

        constexpr bool is_valid() const noexcept
        {
            return to_bool( handle_ );
        }

        bool show( int cmd = SW_SHOW ) const noexcept;

        bool update() const noexcept;

        void close() noexcept;

        constexpr HWND native_handle() const noexcept
        {
            return handle_;
        }

    private:
        constexpr HWND release_window_handle() noexcept
        {
            const auto temp_window_handle = handle_;
            handle_ = nullptr;
            return temp_window_handle;
        }

    private:
        safe_window_type type_;
        HWND handle_ = nullptr;
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

        friend safe_window create_window( window_info info ) noexcept;     

    private:
        safe_window_type type_;
        std::wstring title_;
        DWORD style_ = WS_OVERLAPPEDWINDOW;
        int x_ = CW_USEDEFAULT;
        int y_ = 0;
        int width_ = CW_USEDEFAULT;
        int height_ = 0;
    };

    safe_window create_window( window_info info ) noexcept;

    struct window_view
    {
        HWND handle;

        constexpr window_view( const safe_window& safe ) noexcept
            : handle{ safe->native_handle() }
        {}

        constexpr window_view( HWND handle ) noexcept
            : handle{ handle }
        {}

        explicit constexpr operator bool() const noexcept
        {
            return to_bool( handle );
        }
    };
}