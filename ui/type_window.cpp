#include "type_window.h"

#include <os/os.h>

#include <ui/window.h>


namespace ui
{
    void gdi_object_deleter::operator()(gdi_object_handle_t o) const noexcept
    {
        if (o)
        {
            D_ASSERT_OR_UNUSED(DeleteObject(o));
        }
    }

    unique_brush create_brush(rgba_color32_t color) noexcept
    {
        return
        {
            resource_construct,
            CreateSolidBrush(RGB(color.r, color.g, color.b))
        };
    }

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

    struct type_window_parameters : WNDCLASSEXW
    {
        shared_resource<HBRUSH, gdi_object_deleter> background_brush{};
    };

    void window_type_resource_deleter::operator()(type_window_resource type) const noexcept
    {
        if (type)
        {
            static_assert(std::is_same_v<decltype(type.name_id), LPCWSTR>);
            D_ASSERT_OR_UNUSED(UnregisterClassW(type.name_id, type.module));
        }
    }

    type_window_builder::type_window_builder() noexcept
    {
        using base_type = WNDCLASSEXW;

        const auto p_impl = _p_impl();
        static_assert(std::is_base_of_v<base_type, std::remove_cvref_t<decltype(*p_impl)>>);

        std::construct_at(p_impl);
        p_impl->cbSize = sizeof(base_type);
        p_impl->style = CS_DBLCLKS;
    }

    type_window_builder::type_window_builder(const type_window_builder& builder) noexcept
    {
        std::construct_at(_p_impl(), *builder._c_p_impl());
    }

    type_window_builder::~type_window_builder() noexcept
    {
        std::destroy_at(_p_impl());
    }

    type_window_builder& type_window_builder::operator=(const type_window_builder& builder) noexcept
    {
        *_p_impl() = *builder._c_p_impl();
        return *this;
    }

    unique_type_window type_window_builder::build_as(wzstring_view name) noexcept
    {
        const auto p_impl = _p_impl();
        p_impl->lpszClassName = name.c_str();
        D_ASSERT(!is_null_or_empty(p_impl->lpszClassName));

        if (!p_impl->hInstance)
        {
            p_impl->hInstance = GetModuleHandleW(nullptr);
            D_ASSERT(p_impl->hInstance);
        }

        if (!p_impl->lpfnWndProc)
        {
            p_impl->lpfnWndProc = ui::window_procedure;
        }

        if (!p_impl->hCursor)
        {
            p_impl->hCursor = LoadCursorW(nullptr, idc_arrow_w());
            D_ASSERT(p_impl->hCursor);
        }

        if (!p_impl->hbrBackground)
        {
            p_impl->hbrBackground = stock(stock_brush::white);
            D_ASSERT(p_impl->hbrBackground);
        }

        return
        {
            resource_construct,
            p_impl->hInstance,
            MAKEINTATOMW(RegisterClassExW(p_impl))
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
    
    type_window_parameters* type_window_builder::_p_impl() noexcept
    {
        return as_mutable_pointer(_c_p_impl());
    }
    
    const type_window_parameters* type_window_builder::_c_p_impl() const noexcept
    {
        static_assert(storage_size >= sizeof(type_window_parameters));
        static_assert(storage_align >= alignof(type_window_parameters));
        return reinterpret_cast<const type_window_parameters*>(storage_);
    }

    type_window_builder& type_window_builder::style(uint_t style) noexcept
    {
        _p_impl()->style = style;
        return *this;
    }

    type_window_builder& type_window_builder::module(module_handle_t module) noexcept
    {
        _p_impl()->hInstance = module;
        return *this;
    }

    type_window_builder& type_window_builder::background(stock_brush brush) noexcept
    {
        _p_impl()->hbrBackground = stock(brush);
        return *this;
    }

    type_window_builder& type_window_builder::background(unique_brush brush) noexcept
    {
        const auto p_impl = _p_impl();
        p_impl->hbrBackground = brush;
        p_impl->background_brush = std::move(brush);
        return *this;
    }

    type_window_builder& type_window_builder::window_procedure(wndproc_t proc) noexcept
    {
        _p_impl()->lpfnWndProc = proc;
        return *this;
    }

    uint_t type_window_builder::style() const noexcept
    {
        return _c_p_impl()->style;
    }

    module_handle_t type_window_builder::module() const noexcept
    {
        return _c_p_impl()->hInstance;
    }

    const_brush_handle_t type_window_builder::background() const noexcept
    {
        return _c_p_impl()->hbrBackground;
    }
}
