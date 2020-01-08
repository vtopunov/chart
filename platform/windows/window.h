#pragma once

#include <string>

#include <platform/windows/window_type.h>
#include <platform/windows/window_view.h>

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
            return handle_ != nullptr;
        }

        bool show( int cmd = SW_SHOW ) const noexcept;

        bool update() const noexcept;

        void close() noexcept;

        constexpr window_view view() const noexcept
        {
            return { handle_ };
        }

    private:

    private:
        safe_window_type type_;
        HWND handle_{ nullptr };
    };

    using safe_window = safe_handle<window>;

    class window_info
    {
    public:
        window_info() noexcept = default;

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
}