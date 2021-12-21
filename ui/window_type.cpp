#include "window_type.h"

#include <charconv>
#include <array>

#include <core/narrow_cast.h>

#include <ui/window.h>

namespace ui
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
    }

    error_code_t error_code() noexcept
    {
        return GetLastError();
    }

    HBRUSH stock(stock_brush brush) noexcept
    {
        return static_cast<HBRUSH>( GetStockObject(to_underlying(brush)) );
    }

    void window_type_resource_deleter::operator()(window_type_resource type) const noexcept
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

            constexpr explicit collector(WNDCLASSEXW& data) noexcept
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
            data_.hCursor = LoadCursorW(nullptr, IDC_ARROWW);
        }

        if (!data_.hbrBackground)
        {
            data_.hbrBackground = stock(stock_brush::white);
        }

        return
        {
            resource_construct,
            data_.hInstance,
            MAKEINTATOMW(RegisterClassExW(&data_))
        };
    }
}
