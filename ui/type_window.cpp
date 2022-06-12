#include "type_window.h"

#include <core/narrow.h>

#include <ui/window.h>

namespace ui
{
    extern LRESULT CALLBACK window_procedure(HWND handle, UINT message, WPARAM word_parameter, LPARAM long_parameter) noexcept;

    namespace
    {
        [[nodiscard]]
        constexpr LPCWSTR MAKEINTATOMW(ATOM atom) noexcept
        {
#pragma push_macro("LPTSTR")
#undef LPTSTR
#define LPTSTR LPCWSTR
            static_assert(std::is_same_v<decltype(MAKEINTATOM(atom)), LPCWSTR>);
            return MAKEINTATOM(atom);
#pragma pop_macro("LPTSTR")
        }

        [[nodiscard]]
        constexpr LPCWSTR idc_arrow_w() noexcept
        {
#pragma push_macro("MAKEINTRESOURCE")
#undef MAKEINTRESOURCE
#define MAKEINTRESOURCE MAKEINTRESOURCEW
            return IDC_ARROW;
#pragma pop_macro("MAKEINTRESOURCE")
        }

        [[nodiscard]]
        uint16_t generate_unique_ui16() noexcept
        {
            static uint16_t id{ 0u };
            return ++id;
        }
    }


    HBRUSH stock(stock_brush brush) noexcept
    {
        return static_cast<HBRUSH>(GetStockObject(to_underlying(brush)));
    }

    void window_type_resource_deleter::operator()(type_window_resource type) const noexcept
    {
        if (type)
        {
            D_ASSERT_WITH_SIDE_EFFECTS(UnregisterClassW(type.name_id, type.module));
        }
    }

    unique_type_window type_window_builder::build_as(wzstring_view name) noexcept
    {
        data_.cbSize = sizeof(data_);
        data_.lpszClassName = name.c_str();
        D_ASSERT(!is_null_or_empty(data_.lpszClassName));

        if (!data_.hInstance)
        {
            data_.hInstance = GetModuleHandleW(nullptr);
        }

        if (!data_.lpfnWndProc)
        {
            data_.lpfnWndProc = window_procedure;
        }

        if (!data_.hCursor)
        {
            data_.hCursor = LoadCursorW(nullptr, idc_arrow_w());
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
    
    unique_type_window type_window_builder::build() noexcept
    {
        constexpr auto n_unique_name = 5_uz;
        WCHAR unique_hexname[n_unique_name];

        auto unique_ui16 = generate_unique_ui16();

        auto pname = unique_hexname + (n_unique_name - 1_uz);
        *pname = L'\0';

        do
        {
            constexpr char hexchars[] = "0123456789abcdef";
            *--pname = hexchars[unique_ui16 & 0xf];;
        } while (unique_ui16 >>= 4);

        return build_as(pname);
    }
}
