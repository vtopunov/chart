#pragma once

#include <core/resouce.h>
#include <core/buffer_view.h>
#include <core/zstring_view.h>
#include <core/utf.h>

#include <px/pixspan.h>

#include <font/fixed.h>

namespace font
{
    using charmax_t = char32_t;
    using ft_fixed_t = fixed<int64_t, 6_uz>;

    struct _library_descriptor;
    struct _face_descriptor;

    using library_descriptor_t = _library_descriptor*;
    using face_descriptor_t = _face_descriptor*;

    struct font_resource
    {
        using view_type = face_descriptor_t;

        library_descriptor_t lib;
        face_descriptor_t face;

        [[nodiscard]]
        explicit constexpr operator bool() const noexcept
        {
            return !!face;
        }

        [[nodiscard]]
        constexpr operator face_descriptor_t () const noexcept
        {
            return face;
        }
    };

    struct font_deleter
    {
        void operator () (const font_resource& lib) const noexcept;
    };

    using font_t = unique_resource<font_resource, font_deleter>;

    [[nodiscard]]
    font_t create_font(const_buffer_view font_storage, px::size2d_t sizes) noexcept;

    [[nodiscard]]
    inline font_t create_font(const_buffer_view font_storage, pxside_t size) noexcept
    {
        return create_font(font_storage, size2d{ 0_px, size });
    }

    bool sizes(face_descriptor_t face, px::size2d_t sizes) noexcept;

    inline bool size(face_descriptor_t face, pxside_t px) noexcept
    {
        return sizes(face, size2d{ 0_px, px });
    }

    struct cursor : point2d<ft_fixed_t>
    {
        template<class T>
        [[nodiscard]] static constexpr cursor instance(T x, T y) noexcept
        {
            return
            {
                ft_fixed_t::instance(std::move(x)),
                ft_fixed_t::instance(std::move(y))
            };
        }

        template<class T>
        [[nodiscard]] static constexpr cursor instance(vec2<T> p) noexcept
        {
            return instance(std::move(p._0), std::move(p._1));
        }
    };

    template<class T, size_t FractBits>
    constexpr px::point2d_t as_px_position(const point2d<fixed<T, FractBits>>& p) noexcept
    {
        return
        {
            p.x().narrow_to<pxside_t>(),
            p.y().narrow_to<pxside_t>()
        };
    }


    inline constexpr point2d px_invalid_position{ fill_vec2(numeric_max_v<pxside_t>) };
    inline constexpr auto invalid_cursor = cursor::instance(px_invalid_position);
    static_assert(as_px_position(invalid_cursor) == px_invalid_position);

    cursor draw_char(pix8span_t image, cursor cursor, face_descriptor_t face, charmax_t char_code) noexcept;

    template<class T>
    cursor draw_text(pix8span_t image, cursor cursor, face_descriptor_t face, std::basic_string_view<T> text) noexcept
    {
        const auto char_processor = [&cursor, image, face](charmax_t char_code) noexcept
        {
            cursor = font::draw_char(image, cursor, face, char_code);
        };

        decode_utf<sizeof(charmax_t)>(text, char_processor);

        return cursor;
    }

    template<class Px, class Char>
    cursor draw_text(pix8span_t image, vec2<Px> position, face_descriptor_t face, std::basic_string_view<Char> text) noexcept
    {
        return draw_text(image, cursor::instance(std::move(position)), face, text);
    }

    template<class Px, class Char>
    cursor draw_text(pix8span_t image, Px x, Px y, face_descriptor_t face, std::basic_string_view<Char> text) noexcept
    {
        return draw_text(image, cursor::instance(std::move(x), std::move(y)), face, text);
    }

    struct glyph_metrics
    {
        ft_fixed_t width;
        ft_fixed_t top;
        ft_fixed_t bottom;

        [[nodiscard]]
        constexpr bool is_valid() const noexcept
        {
            return is_positive(width);
        }
    };

    [[nodiscard]]
    glyph_metrics metrics(face_descriptor_t face, charmax_t char_code) noexcept;

    struct text_metrics
    {
        ft_fixed_t width;
        ft_fixed_t top;
        ft_fixed_t bottom;
        bool success_bit;

        constexpr void add(const glyph_metrics& glyph) noexcept
        {
            const auto glyph_is_valid = glyph.is_valid();

            if (glyph_is_valid)
            {
                width = width + glyph.width;
                top = std::min(top, glyph.top);
                bottom = std::max(bottom, glyph.bottom);
            }

            success_bit = success_bit && glyph_is_valid;
        }

        constexpr explicit operator bool() const noexcept
        {
            return success_bit;
        }
    };

    template<class T>
    [[nodiscard]] text_metrics metrics(face_descriptor_t face, std::basic_string_view<T> text, text_metrics tm) noexcept
    {
        const auto processor = [&tm, face](charmax_t ch) noexcept
        {
            if (tm.success_bit)
            {
                tm.add(metrics(face, ch));
            }
        };

        decode_utf<sizeof(charmax_t)>(text, processor);

        return tm;
    }


    inline constexpr text_metrics initial_text_metrics
    {
        .width{ 0 },
        .top{ numeric_max_v<ft_fixed_t::value_type> },
        .bottom{ numeric_min_v<ft_fixed_t::value_type> },
        .success_bit{ true }
    };

    template<class T>
    [[nodiscard]] text_metrics metrics(face_descriptor_t face, std::basic_string_view<T> text) noexcept
    {
        return metrics(face, text, initial_text_metrics);
    }
}
