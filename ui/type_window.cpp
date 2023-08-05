#include "type_window.h"

#include <core/narrow.h>
#include <os/os.h>

#include <ui/window.h>


namespace ui
{
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

        [[nodiscard]]
        HBRUSH stock(stock_brush brush) noexcept
        {
            static_assert(std::is_same_v<int, std::underlying_type_t<stock_brush>>);
            static_assert(WHITE_BRUSH  == to_underlying(stock_brush::white));
            static_assert(LTGRAY_BRUSH == to_underlying(stock_brush::light_gray));
            static_assert(GRAY_BRUSH   == to_underlying(stock_brush::gray));
            static_assert(DKGRAY_BRUSH == to_underlying(stock_brush::dark_gray));
            static_assert(BLACK_BRUSH  == to_underlying(stock_brush::black));
            static_assert(NULL_BRUSH   == to_underlying(stock_brush::null));

            return static_cast<HBRUSH>(GetStockObject(to_underlying(brush)));
        }
    }

    void window_type_resource_deleter::operator()(type_window_resource type) const noexcept
    {
        if (type)
        {
            static_assert(std::is_same_v<decltype(type.name_id), LPCWSTR>);
            D_ASSERT_OR_UNUSED(UnregisterClassW(type.name_id, type.module));
        }
    }

    unique_type_window type_window_builder::build_as(wzstring_view name) noexcept
    {
        const auto pdata = wndcls();
        pdata->cbSize = sizeof(*pdata);
        pdata->lpszClassName = name.c_str();
        D_ASSERT(!is_null_or_empty(pdata->lpszClassName));
        pdata->style |= CS_DBLCLKS;

        if (!pdata->hInstance)
        {
            pdata->hInstance = GetModuleHandleW(nullptr);
            D_ASSERT(pdata->hInstance);
        }

        if (!pdata->lpfnWndProc)
        {
            pdata->lpfnWndProc = ui::window_procedure;
        }

        if (!pdata->hCursor)
        {
            pdata->hCursor = LoadCursorW(nullptr, idc_arrow_w());
            D_ASSERT(pdata->hCursor);
        }

        if (!pdata->hbrBackground)
        {
            pdata->hbrBackground = stock(stock_brush::white);
            D_ASSERT(pdata->hbrBackground);
        }

        return
        {
            resource_construct,
            pdata->hInstance,
            MAKEINTATOMW(RegisterClassExW(pdata))
        };
    }

    unique_type_window type_window_builder::build() noexcept
    {
        constexpr auto n_unique_name = 5_uz;
        WCHAR unique_hexname[n_unique_name];

        auto pname = unique_hexname + (n_unique_name - 1_uz);
        *pname = L'\0';

        auto unique_ui16 = generate_unique_ui16();
        do
        {
            constexpr char hexchars[]
            {
                '0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'a', 'b',
                'c', 'd', 'e', 'f'
            };
            static_assert(16_uz == std::size(hexchars));

            *--pname = hexchars[unique_ui16 & 0xfu];
        }
        while (unique_ui16 >>= 4);

        return build_as(pname);
    }
    
    tagWNDCLASSEXW* type_window_builder::wndcls() noexcept
    {
        return as_mutable_pointer(cwndcls());
    }
    
    const tagWNDCLASSEXW* type_window_builder::cwndcls() const noexcept
    {
        static_assert(wndclass_len >= sizeof(tagWNDCLASSEXW));
        static_assert(wndclass_align >= alignof(tagWNDCLASSEXW));
        return reinterpret_cast<const tagWNDCLASSEXW*>(storage_);
    }


    type_window_builder& type_window_builder::style(uint_t style) noexcept
    {
        wndcls()->style = style;
        return *this;
    }

    type_window_builder& type_window_builder::module(module_handle_t module) noexcept
    {
        wndcls()->hInstance = module;
        return *this;
    }

    type_window_builder& type_window_builder::background(stock_brush brush) noexcept
    {
        wndcls()->hbrBackground = stock(brush);
        return *this;
    }

    type_window_builder& type_window_builder::window_procedure(wndproc_t proc) noexcept
    {
        wndcls()->lpfnWndProc = proc;
        return *this;
    }

    module_handle_t type_window_builder::module() const noexcept
    {
        return cwndcls()->hInstance;
    }
}
