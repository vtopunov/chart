#include "window_type.h"

#include <charconv>
#include <algorithm>
#include <array>

#include <core/underlying_cast.h>

#include <display/window.h>

#include <bit>

namespace display
{
    extern LRESULT CALLBACK window_procedure(HWND handle, UINT message, WPARAM word_parameter, LPARAM long_parameter) noexcept;

    namespace
    {
        [[nodiscard]]
        uint32_t generate_unique_ui32() noexcept
        {
            static uint32_t id{ 0u };
            return ++id;
        }

        [[nodiscard]]
        constexpr LPCWSTR make_in_atom(ATOM atom) noexcept
        {
            return std::bit_cast<LPCWSTR>(static_cast<ULONG_PTR>(atom));
        }

        [[nodiscard]]
        HCURSOR load_cursor(HINSTANCE instance, LPCWSTR wstr) noexcept
        {
            return LoadCursorW(instance, wstr);
        }

        [[nodiscard]]
        HCURSOR load_cursor(HINSTANCE instance, LPCSTR wstr) noexcept
        {
            return LoadCursorA(instance, wstr);
        }
    }

    error_code_t last_error_code() noexcept
    {
        return GetLastError();
    }

    HBRUSH stock(stock_brush brush) noexcept
    {
        return static_cast<HBRUSH>( GetStockObject(to_underlying(brush)) );
    }

    void window_type_resource_deleter::operator()(window_type_resource type, resource_destroy_t) const noexcept
    {
        if (type)
        {
            D_ASSERT_WITH_SIDE_EFFECTS(UnregisterClassW(type.name_id, type.module_instance));
        }
    }

    unique_window_type_t window_type_factory::create() noexcept
    {
        const struct collector
        {
            WNDCLASSEXW& data_;

            constexpr collector(WNDCLASSEXW& data) noexcept
               : data_{ data }
            {}

            D_DISABLE_COPY_MOVE(collector);

            constexpr ~collector() noexcept
            {
                data_.lpszClassName = nullptr;
            }
        } collect{ data_ };

        data_.cbSize = sizeof(WNDCLASSEXW);

        if ( !data_.lpszClassName )
        {
            {
                char name8bit[sizeof(uint32_t)];

                const auto result = std::to_chars
                (
                    std::begin(name8bit),
                    std::end(name8bit),
                    generate_unique_ui32(),
                    16
                );

                D_ASSERT( result.ec == std::errc{} );

                name_.assign(std::data(name8bit), result.ptr);
            }

            data_.lpszClassName = name_.c_str();
        }

        if ( !data_.hInstance )
        {
            data_.hInstance = GetModuleHandleW(nullptr);
        }

        if ( !data_.lpfnWndProc )
        {
            data_.lpfnWndProc = window_procedure;
        }

        if ( !data_.hCursor )
        {
            data_.hCursor = load_cursor(nullptr, IDC_ARROW);
        }

        if (!data_.hbrBackground)
        {
            data_.hbrBackground = stock(stock_brush::white);
        }

        return
        {
            resource_construct,
            data_.hInstance,
            make_in_atom(RegisterClassExW(&data_))
        };
    }
}
