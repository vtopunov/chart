#pragma once

#include <core/safe_handle.h>

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
            assert( module_address_ );
        }

        void close() noexcept;

        constexpr HMODULE module_address() const noexcept
        {
            assert( is_valid() );
            return module_address_;
        }

        constexpr LPCWSTR name_id() const noexcept
        {
            assert( is_valid() );
            return name_id_;
        }

        constexpr bool is_valid() const noexcept
        {
            return name_id_ != nullptr;
        }

    private:
        constexpr LPCWSTR release_name_id() noexcept
        {
            const auto temp = name_id_;
            name_id_ = nullptr;
            return temp;
        }

    private:
        HMODULE module_address_{};
        LPCWSTR name_id_{};
    };

    using safe_window_type = safe_handle<window_type>;

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
            assert( !name.empty() );
            name_ = std::move( name );
            data_.lpszClassName = name_.c_str();
            return *this;
        }

        constexpr window_type_info& module_address( HMODULE module_address )
        {
            assert( module_address );
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