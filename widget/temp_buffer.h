#pragma once

#include <core/buffer.h>
#include <core/buffer_view.h>

#include <px/pixspan.h>

#include <ui/event.h>

#include <widget/event.h>


namespace widget
{
    class temp_byte_buffer
    {
    public:
        constexpr temp_byte_buffer() noexcept = default;
        D_DISABLE_COPYMOVE_CA(temp_byte_buffer);

        [[nodiscard]]
        lumpixspan image(luminance_pixspace space) noexcept;

        [[nodiscard]]
        lumpixspan image(npx_t w, npx_t h) noexcept
        {
            return image(luminance_pixspace{ w, h });
        }

        [[nodiscard]]
        lumpixspan zimage(luminance_pixspace space) noexcept
        {
            const auto new_span = image(space);
            zero_memory(new_span);
            return new_span;
        }

        [[nodiscard]]
        lumpixspan zimage(npx_t w, npx_t h) noexcept
        {
            return zimage(luminance_pixspace{ w, h });
        }

        constexpr dummy apply(no_overload) const noexcept
        {
            return dummy_v;
        }

#ifdef D_OS_WINDOWS
        void operator () (const ui::size_event& e) noexcept;

#endif

    private:
        byte_buffer buffer_{};
    };
}