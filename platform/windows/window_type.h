#pragma once

#include <string>

#include <core/shared_handle.h>
#include <platform/windows/config.h>

namespace os_windows
{
    class window_type
    {
    public:
        constexpr window_type() noexcept = default;

        constexpr window_type( HMODULE address, LPCWSTR name_id ) noexcept
            : module_address_{ address }
            , name_id_{ name_id }
        {
            D_ASSERT( module_address_ );
        }

        bool close() noexcept;

        constexpr HMODULE module_address() const noexcept
        {
            return module_address_;
        }

        constexpr LPCWSTR name_id() const noexcept
        {
            return name_id_;
        }

        constexpr bool is_valid() const noexcept
        {
            return name_id_ != nullptr;
        }

    private:
        HMODULE module_address_{nullptr};
        LPCWSTR name_id_{nullptr};
    };

    using safe_window_type = shared_handle<window_type>;

    class window_type_info
    {
    public:
        constexpr window_type_info() noexcept
        {
            data_.cbSize = sizeof( data_ );
            data_.style = CS_HREDRAW | CS_VREDRAW;
        }

        constexpr window_type_info& style( UINT style ) noexcept
        {
            data_.style = style;
            return *this;
        }

        window_type_info& name( std::wstring name ) noexcept
        {
            D_ASSERT( !name.empty() );
            name_ = std::move( name );
            data_.lpszClassName = name_.c_str();
            return *this;
        }

        constexpr window_type_info& module_address( HMODULE module_address )
        {
            D_ASSERT( module_address );
            data_.hInstance = module_address;
            return *this;
        }

        friend safe_window_type register_window_type( window_type_info info ) noexcept;

    private:
        WNDCLASSEXW data_{};
        std::wstring name_;
    };

    safe_window_type register_window_type( window_type_info info ) noexcept;
}