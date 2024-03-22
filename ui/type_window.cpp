#include "type_window.h"

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

        template<class T>
        [[nodiscard]] T generate_unique_window_type_id_as() noexcept
        {
            static T id{};
            return ++id;
        }

        template<class UniqueId>
        constexpr auto unqiue_name_max_size_v = 2u * sizeof(UniqueId) + 1u;

        template<class UniqueId, class Char>
        constexpr basic_zstring_view<Char> unique_name(UniqueId id, Char(&buffer)[unqiue_name_max_size_v<UniqueId>]) noexcept
        {
            constexpr auto back_index = narrow<ptrdiff_t>(sizeof(UniqueId));
            Char* pname{ buffer + back_index };
            *pname = {};

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

                *--pname = hexchars[id & 0xfu];
            }
            while (id >>= 4);

            return pname;
        }
    }

    struct type_window_parameters : WNDCLASSEXW
    {
        using base_type = WNDCLASSEXW;

        constexpr type_window_parameters() noexcept
            : base_type
            {
                .cbSize{ sizeof(base_type) },
                .style{ CS_DBLCLKS },
                .lpfnWndProc{ ui::window_procedure }
            }
        {}

        constexpr type_window_parameters(const type_window_parameters&) = default;

        constexpr type_window_parameters& operator = (const type_window_parameters&) = default;

        shared_resource<HBRUSH, gdi_object_deleter> background_brush{};
    };

    void window_type_resource_deleter::operator()(type_window_resource type) const noexcept
    {
        if (type)
        {
            D_ASSERT_OR_UNUSED(UnregisterClassW(type.handle, type.module));
        }
    }

    type_window_builder::type_window_builder() noexcept
    {
        std::construct_at(_p_impl());
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

    unique_type_window type_window_builder::build(wzstring_view name) noexcept
    {
        const auto p_impl = _p_impl();
        p_impl->lpszClassName = name.c_str();
        D_ASSERT(!is_null_or_empty(p_impl->lpszClassName));

        if (!p_impl->hInstance)
        {
            p_impl->hInstance = GetModuleHandleW(nullptr);
            D_ASSERT(p_impl->hInstance);
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
            MAKEINTATOMW(RegisterClassExW(as_const_pointer(p_impl))),
            p_impl->hInstance
        };
    }

    unique_type_window type_window_builder::build() noexcept
    {
        using unique_id_t = uint16_t;
        WCHAR buffer_for_unique_name[unqiue_name_max_size_v<unique_id_t>];
        return build(unique_name(generate_unique_window_type_id_as<unique_id_t>(), buffer_for_unique_name));
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
