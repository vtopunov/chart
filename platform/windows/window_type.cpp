#include "window_type.h"

#include <charconv>

#include <core/span.h>

namespace os_windows
{
    extern LRESULT CALLBACK window_procedure( HWND window_handle, UINT message, WPARAM word_parameter, LPARAM long_parameter ) noexcept;

    namespace
    {
        uint32_t id_generate() noexcept
        {
            static uint32_t id = 0;
            return ++id;
        }

        span<char> to_chars( span<char> chars, uint32_t value, int base = 10 ) noexcept
        {
            const auto result = std::to_chars( chars.begin(), chars.end(), value, base );
            const auto size = ( result.ec == std::errc{} ) ? narrow_cast<size_t>( result.ptr - chars.data() ) : 0_z;
            return chars.left( size );
        }

        ATOM register_class( const WNDCLASSEXW& data ) noexcept
        {
            return ( data.hInstance ) ? RegisterClassExW( &data ) : ATOM{ 0 };
        }

        constexpr LPCWSTR make_in_atom( ATOM atom ) noexcept
        {
            return ( LPCWSTR) ( ( ULONG_PTR) ( atom ) );
        }
    }

    void window_type::close() noexcept
    {
        if (const auto name_id = release_name_id(); name_id)
        {
            const auto result = UnregisterClassW( name_id, module_address_ );
            result; assert( result );
        }
    }

    safe_window_type register_window_type( window_type_info info ) noexcept
    {
        std::array<WCHAR, 5> name{};
        if ( !info.data_.lpszClassName )
        {
            {
                std::array<char, name.size() - 1> hexname{};
                const auto id_string = to_chars( hexname, id_generate(), 16 );
                assert( !id_string.empty() && id_string.data() && id_string.front() != '\0' );
                std::copy( id_string.cbegin(), id_string.cend(), name.begin() );
            }
            info.data_.lpszClassName = name.data();
        }

        if ( !info.data_.hInstance )
        {
            info.data_.hInstance = GetModuleHandleW( nullptr );
        }

        if ( !info.data_.lpfnWndProc )
        {
            info.data_.lpfnWndProc = window_procedure;
        }

        return window_type
        {
            info.data_.hInstance,
            make_in_atom( register_class( info.data_ ) )
        };
    }
}
