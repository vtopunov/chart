#pragma once

#include <core/buffer.h>
#include <core/buffer_view.h>

#include <widget/fwd.h>


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
}