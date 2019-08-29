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
    }

    void window_type::close() noexcept
    {
        if ( is_valid() )
        {
            const auto result = UnregisterClassW( release_name().as_string_id(), release_module_address() );
            result; assert( result != FALSE );
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
            if ( !info.data_.hInstance )
            {
                return {}
            }
        }

        if ( !info.data_.lpfnWndProc )
        {
            info.data_.lpfnWndProc = window_procedure;
        }

        const auto class_name_atom = RegisterClassExW( &( info.data_ ) );
        if ( !class_name_atom )
        {
            return {};
        }

        return window_type
        {
            not_null{ info.data_.hInstance },
            atom{ class_name_atom }
        };
    }
}
