#include "text.h"

#include <core/buffer.h>

#include <utility/px.h>

using namespace std::string_view_literals;


namespace widget
{
    namespace
    {
        [[nodiscard]] font::metrics initial_tm(font::face_descriptor_t face) noexcept
        {
            auto tm = font::text_metrics(face, u8"Ap"sv);
            tm.width = {};
            return tm;
        };
    }

    bool text_cache::draw(buffer_view buffer, font::face_descriptor_t face, std::u8string_view text, pxsizes sizes) noexcept
    {
        if (is_empty() && !text.empty())
        {
            const auto tm = font::text_metrics(initial_tm(face), face, text);
            if (!tm) [[unlikely]]
            {
                return false;
            }

            const auto pixs = px::create_zeros_lumpixspan
            (
                buffer,
                std::min(sizes.width(), ceil_to<npx_t>(tm.width)),
                std::min(sizes.height(), ceil_to<npx_t>(tm.bottom - tm.top))
            );

            const auto y_cursor = font::draw_text(pixs, 0_npx, -tm.top, face, text).y();
            if (invalid_y == y_cursor) [[unlikely]]
            {
                return false;
            }

            if (!gl::update(texture_, pixs)) [[unlikely]]
            {
                return false;
            }

            const auto height = font::fixed_t::instance(pixs.height());
            y_ = std::clamp(y_cursor, {}, height) / 2;
        }

        return true;
    }
}