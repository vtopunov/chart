#pragma once

#include <core/safe_handle.h>

#include <platform/windows/config.h>

namespace os_windows
{
    constexpr LPCWSTR make_in_atom( ATOM atom ) noexcept
    {
        return ( LPCWSTR) ( ( ULONG_PTR) ( atom ) );
    }

    constexpr ATOM invalid_atom{ INVALID_ATOM };
    constexpr ATOM max_int_atom{ MAXINTATOM };

    class atom
    {
    public:
        constexpr atom() noexcept = default;

        constexpr atom( ATOM atom ) noexcept
            : atom_{ atom }
        {
            assert( is_valid() );
        }

        constexpr LPCWSTR as_string_id() const noexcept
        {
            assert( is_valid() );
            return make_in_atom( atom_ );
        }

        std::wstring as_string() const noexcept
        {
            assert( is_valid() );
            constexpr size_t max_size_atom_buffer = 256;
            WCHAR buffer[max_size_atom_buffer];
            const auto size = narrow_cast<size_t>( GetAtomNameW( atom_, buffer, narrow_cast<int>( std::size(buffer) ) ) );
            assert( size );
            return { as_const_pointer( buffer ), size };
        }

        constexpr bool is_valid() const noexcept
        {
            return atom_ >= max_int_atom;
        }

    private:
        ATOM atom_{ invalid_atom };
    };

    class window_type
    {
    public:
        constexpr window_type() noexcept = default;

        constexpr window_type( HMODULE address, atom name ) noexcept
            : module_address_{ address.get() }
            , name_{ name }
        {
            assert( module_address_ );
            assert( name_.is_valid() );
        }

        void close() noexcept;

        constexpr HMODULE module_address() const noexcept
        {
            assert( is_valid() );
            return module_address_;
        }

        constexpr atom name() const noexcept
        {
            assert( is_valid() );
            return name_;
        }

        constexpr bool is_valid() const noexcept
        {
            return name_.is_valid();
        }

    private:
        constexpr atom release_name() noexcept
        {
            const auto temp = name_;
            name_ = {};
            return temp;
        }

        constexpr HMODULE release_module_address() noexcept
        {
            const auto temp = module_address_;
            module_address_ = {};
            return temp;
        }

    private:
        HMODULE module_address_{};
        atom name_{};
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

        friend safe_window_type register_window_type( window_type_info info ) noexcept;

    private:
        WNDCLASSEXW data_{};
    };

    safe_window_type register_window_type( window_type_info info ) noexcept;
}