#pragma once

#include <core/buffer.h>
#include <core/buffer_view.h>

#include <widget/event.h>


namespace widget
{
    struct pix8_temp_buffer : buffer_t
    {
        bool operator () (viewport_size2d viewport) noexcept;

        constexpr operator buffer_view() const noexcept
        {
            return as_mutable(*this);
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn();
        }
    };

    struct content_sizes_cache : pxsize2d
    {
        constexpr content_sizes_cache() noexcept
            : pxsize2d{ 0_px, 0_px }
        {}

        constexpr void operator () (viewport_size2d viewport) noexcept
        {
            as_size2d(*this) = as_size2d(viewport);
        }

#ifdef D_OS_WINDOWS
        constexpr void operator () (const ui::size_event& e) noexcept
        {
            as_size2d(*this) = e.sizes();
        }
#endif

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn();
        }
    };


    template<class T>
    using unview_t = std::conditional_t<
        std::is_same_v<buffer_view, std::remove_cvref_t<T>>,
        pix8_temp_buffer,
        T
    >;

    template<class T>
    using unview_ref_t = std::add_lvalue_reference_t<unview_t<T>>;

    template<class T>
    using unview_cref_t = std::add_lvalue_reference_t<std::add_const_t<unview_t<T>>>;
}

using widget::content_sizes_cache;