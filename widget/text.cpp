#include "text.h"

#include <core/buffer.h>

using namespace std::string_view_literals;


namespace widget
{
    namespace
    {
        [[nodiscard]] font::metrics initial_tm(font::face_resource face) noexcept
        {
            auto tm = font::text_metrics(face, u8"Ap"sv);
            tm.width = {};
            return tm;
        };
    }

    bool text_cache::draw(temp_byte_buffer& buffer, font::face_resource face, std::u8string_view text, pxsizes sizes) noexcept
    {
        if (is_empty() && !text.empty())
        {
            const auto tm = font::text_metrics(initial_tm(face), face, text);
            if (!tm) [[unlikely]]
            {
                return false;
            }

            const auto pixs = buffer.zimage
            (
                std::min(sizes.width(), ceil_to<npx_t>(tm.width)),
                std::min(sizes.height(), ceil_to<npx_t>(tm.bottom - tm.top))
            );

            const auto cursor = font::draw_text(pixs, 0_npx, -tm.top, face, text);
            if (!cursor) [[unlikely]]
            {
                return false;
            }

            if (!gl::update(texture_, pixs)) [[unlikely]]
            {
                return false;
            }

            const auto height = font::fixed_t::instance(pixs.height());
            y_ = std::clamp(cursor.cref_y(), {}, height) / 2;
        }

        return true;
    }
}