#pragma once

#include <gl_core/texture.h>

#include <font/font.h>

#include <widget/temp_buffer.h>


namespace widget
{
    struct text_cache
    {
        static constexpr auto invalid_y = font::invalid_cursor.y();

    public:
        bool draw(temp_byte_buffer& buffer, font::face_resource face, std::u8string_view text, pxsizes sizes) noexcept;

        inline bool draw(temp_byte_buffer& buffer, font::face_resource face, std::u8string_view text) noexcept
        {
            return draw(buffer, face, text, fill_to<size2d>(numeric_max_v<npx_t>));
        }

        [[nodiscard]]
        constexpr gl::texture2d_resources texture() const noexcept
        {
            return texture_;
        }

        [[nodiscard]]
        constexpr font::point2fix center() const noexcept
        {
            return
            {
                font::fixed_t::instance(width(texture_)) / 2,
                y_
            };
        }

        constexpr explicit operator bool() const noexcept
        {
            return has_value();
        }

        [[nodiscard]]
        constexpr bool has_value() const noexcept
        {
            return !is_empty();
        }

        [[nodiscard]]
        constexpr bool is_empty() const noexcept
        {
            return invalid_y == y_;
        }

        void clear() noexcept
        {
            texture_.hide();
            y_ = invalid_y;
        }


    private:
        gl::texture2d texture_{};
        font::fixed_t y_{ invalid_y };
    };
}
