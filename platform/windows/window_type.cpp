#include "window_type.h"

#include <charconv>
#include <algorithm>
#include <array>

#include <core/span.h>

namespace os_windows
{
    extern LRESULT CALLBACK window_procedure( HWND window_handle, UINT message, WPARAM word_parameter, LPARAM long_parameter ) noexcept;

    namespace
    {
        uint32_t generate_unique_ui32() noexcept
        {
            static uint32_t id{ 0u };
            return ++id;
        }

        span<const char> to_chars( span<char> chars, uint32_t value, int base = 10 ) noexcept
        {
            const auto result = std::to_chars( chars.begin(), chars.end(), value, base );
            if (result.ec == std::errc{})
                return {};
            return chars.cspan().left(narrow_cast<size_t>(result.ptr - chars.cdata()));
        }

        constexpr LPCWSTR make_in_atom( ATOM atom ) noexcept
        {
            return ( LPCWSTR) ( ( ULONG_PTR) ( atom ) );
        }

        bool unregister_class(const window_type type) noexcept
        {
            if (type.is_valid())
            {
                const auto ok 
                    = UnregisterClassW(type.name_id(), type.module_address()) != FALSE;
                D_ASSERT(ok);
                return ok;
            }

            return false;
        }
    }

    bool window_type::close() noexcept
    {
        return unregister_class(std::exchange(*this, {}));
    }

    safe_window_type register_window_type( window_type_info info ) noexcept
    {
        std::array<WCHAR, 5> name{};
        if ( !info.data_.lpszClassName )
        {
            {
                std::array<char, name.size() - 1u> hexname{};
                const auto id_string = to_chars( hexname, generate_unique_ui32(), 16 );
                D_ASSERT( !id_string.empty() && id_string.data() && id_string.front() != '\0' );
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

        return make_shared_handle<window_type>(
            info.data_.hInstance,
            make_in_atom((info.data_.hInstance) ? RegisterClassExW(&info.data_) : ATOM{ 0 })
        );
    }
}
