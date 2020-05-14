#include "window_type.h"

#include <charconv>
#include <algorithm>
#include <array>

#include <core/underlying_cast.h>
#include <core/span.h>

namespace os_windows
{
    extern LRESULT CALLBACK window_procedure(HWND window_handle, UINT message, WPARAM word_parameter, LPARAM long_parameter) noexcept;

    namespace
    {
        uint32_t generate_unique_ui32() noexcept
        {
            static uint32_t id{ 0u };
            return ++id;
        }

        span<const char> to_chars(span<char> chars, uint32_t value, int base = 10) noexcept
        {
            const auto result = std::to_chars(chars.begin(), chars.end(), value, base);
            if ( to_underlying(result.ec) )
                return {};
            return chars.cspan().prefix(narrow_cast<size_t>( result.ptr - chars.cdata() ));
        }

        constexpr LPCWSTR make_in_atom(ATOM atom) noexcept
        {
            return (LPCWSTR) ( (ULONG_PTR) ( atom ) );
        }

        window_type_view register_type(WNDCLASSEXW data) noexcept
        {
            data.cbSize = sizeof(data);

            std::array<WCHAR, 5> name{};
            if ( is_null_or_empty(data.lpszClassName) )
            {
                {
                    std::array<char, name.size() - 1u> hexname{};
                    const auto id_string = to_chars(hexname, generate_unique_ui32(), 16);
                    D_ASSERT(id_string.size() > 0u && id_string.data() && id_string.front() != '\0');
                    std::copy(id_string.cbegin(), id_string.cend(), name.begin());
                }
                data.lpszClassName = name.data();
            }

            if ( !data.hInstance )
            {
                data.hInstance = GetModuleHandleW(nullptr);
            }

            if ( !data.lpfnWndProc )
            {
                data.lpfnWndProc = window_procedure;
            }

            return { data.hInstance, make_in_atom(RegisterClassExW(&data)) };
        }
    }

    bool close(private_handle_t, window_type_view type) noexcept
    {
        if ( valid(type) )
        {
            const auto ok
                = !!UnregisterClassW(type.name_id, type.module_address);
            D_ASSERT(ok);
            return ok;
        }

        return false;
    }


    window_type_factory& window_type_factory::background(stock_brush brush) noexcept
    {
        return background(static_cast<HBRUSH>( GetStockObject(to_underlying(brush)) ));
    }

    safe_window_type window_type_factory::create() const noexcept
    {
        return 
        {
            handle_construct,
            register_type(data_)
        };
    }
}
