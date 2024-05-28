#pragma once

#include <core/buffer.h>
#include <core/buffer_view.h>

#include <widget/event.h>


namespace widget
{
    struct temp_buffer : byte_buffer
    {
        bool operator () (viewport_event<> e) noexcept;

        constexpr operator buffer_view() const noexcept
        {
            return as_mutable(*this);
        }

        constexpr noapply_t apply(no_overload) const noexcept
        {
            return noapply;
        }
    };
}