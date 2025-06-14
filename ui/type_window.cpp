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

        struct cache_type
        {
            shared_type_window type{};
            shared_resource<HBRUSH, gdi_object_deleter> background_brush{};
        };

        cache_type cache{};

        constexpr void clear_cache() noexcept
        {
            cache.background_brush.deattach_and_reset();
            cache.type.deattach_and_reset();
        }

        void rebuild(wzstring_view name) noexcept
        {
            lpszClassName = name.c_str();
            D_ASSERT(!is_null_or_zfront(lpszClassName));

            if (!hInstance)
            {
                hInstance = os::current_module();
                D_ASSERT(hInstance);
            }

            if (!hCursor)
            {
                hCursor = LoadCursorW(nullptr, idc_arrow_w());
                D_ASSERT(hCursor);
            }

            if (!hbrBackground)
            {
                hbrBackground = stock(stock_brush::white);
                D_ASSERT(hbrBackground);
            }

            cache.type = unique_resource<type_window_resource, window_type_resource_deleter>
            {
                MAKEINTATOMW(RegisterClassExW(as_const_pointer(this)))
            };
        }
    };

    void window_type_resource_deleter::operator()(type_window_resource type) const noexcept
    {
        if (type)
        {
            D_ASSERT_OR_UNUSED(UnregisterClassW(type.handle, os::current_module()));
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

    shared_type_window type_window_builder::build() noexcept
    {
        const auto p_impl = _p_impl();
        if (!(p_impl->cache.type))
        {
            using unique_id_t = uint16_t;
            WCHAR buffer_for_unique_name[unqiue_name_max_size_v<unique_id_t>];
            p_impl->rebuild(unique_name(generate_unique_window_type_id_as<unique_id_t>(), buffer_for_unique_name));
        }

        return p_impl->cache.type;
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
        const auto p_impl = _p_impl();
        p_impl->style = style;
        p_impl->clear_cache();
        return *this;
    }

    type_window_builder& type_window_builder::background(stock_brush brush) noexcept
    {
        const auto p_impl = _p_impl();
        p_impl->hbrBackground = stock(brush);
        p_impl->clear_cache();
        return *this;
    }

    type_window_builder& type_window_builder::background(unique_brush brush) noexcept
    {
        const auto p_impl = _p_impl();
        p_impl->hbrBackground = brush;
        p_impl->cache.background_brush = std::move(brush);
        p_impl->cache.type.deattach_and_reset();
        return *this;
    }

    type_window_builder& type_window_builder::window_procedure(wndproc_t proc) noexcept
    {
        const auto p_impl = _p_impl();
        p_impl->lpfnWndProc = proc;
        p_impl->clear_cache();
        return *this;
    }

    uint_t type_window_builder::style() const noexcept
    {
        return _c_p_impl()->style;
    }

    const_brush_handle_t type_window_builder::background() const noexcept
    {
        return _c_p_impl()->hbrBackground;
    }
}
