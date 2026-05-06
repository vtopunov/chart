#pragma once

#include <core/rational.h>
#include <core/resource.h>
#include <core/zstring_view.h>
#include <core/utf.h>

#include <px/pixspan.h>


namespace font
{
    struct _private_face;

    using face_resource = _private_face*;

    using charmax_t = char32_t;
    using fixed_t = rational<int64_t, 64u>;
    static_assert(sizeof(fixed_t::int_type) > sizeof(npx_t));
    using point2fix = point2d<fixed_t>;

    struct face_deleter
    {
        void operator () (face_resource face) const noexcept;
    };

    using face = unique_resource<face_resource, face_deleter>;

    [[nodiscard]]
    face create_face(const_byte_buffer_view font_storage, pxsizes sizes) noexcept;

    [[nodiscard]]
    inline face create_face(const_byte_buffer_view font_storage, npx_t size) noexcept
    {
        return create_face(font_storage, size2d{ 0_npx, size });
    }

    bool sizes(face_resource face, pxsizes sizes) noexcept;

    inline bool size(face_resource face, npx_t px) noexcept
    {
        return sizes(face, size2d{ 0_npx, px });
    }

    struct cursor : point2fix
    {
        static constexpr auto invalid_npx = numeric_max_v<npx_t>;
        static constexpr auto invalid_npxf = fixed_t::instance(invalid_npx);
       
        template<class X, class Y>
        [[nodiscard]] static constexpr cursor instance(X x, Y y) noexcept
        {
            return
            {
                fixed_t::instance(std::move(x)),
                fixed_t::instance(std::move(y))
            };
        }

        template<class T>
        [[nodiscard]] static constexpr cursor instance(vec2<T> p) noexcept
        {
            return instance(std::move(p._0), std::move(p._1));
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            D_ASSERT(invalid_npxf >= cref_x());
            D_ASSERT(invalid_npxf >= cref_y());
            
            const auto is_invalid = (invalid_npxf == cref_x());
            D_ASSERT(is_invalid == (invalid_npxf == cref_y()));
            return !is_invalid;
        }
    };

    cursor draw_char(lumpixspan image, cursor cursor, face_resource face, charmax_t char_code) noexcept;

    template<class String>
    cursor draw_text(lumpixspan image, cursor cursor, face_resource face, const String& text) noexcept
    {
        const auto char_processor = [&cursor, image, face](charmax_t char_code) noexcept
        {
            cursor = draw_char(image, cursor, face, char_code);
        };

        decode_utf<sizeof(charmax_t), string_char_t<String>>(text, char_processor);

        return cursor;
    }

    template<class Px, class String>
    cursor draw_text(lumpixspan image, vec2<Px> position, face_resource face, const String& text) noexcept
    {
        return draw_text(image, cursor::instance(std::move(position)), face, text);
    }

    template<class X, class Y, class String>
    cursor draw_text(lumpixspan image, X x, Y y, face_resource face, const String& text) noexcept
    {
        return draw_text(image, cursor::instance(std::move(x), std::move(y)), face, text);
    }

    struct metrics
    {
        fixed_t width;
        fixed_t top;
        fixed_t bottom;
        bool success_bit;

        constexpr void add(const metrics& glyph) noexcept
        {
            D_ASSERT(success_bit);

            if (glyph.success_bit) [[likely]]
            {
                width = width + glyph.width;
                top = std::min(top, glyph.top);
                bottom = std::max(bottom, glyph.bottom);
            }

            success_bit = glyph.success_bit;
        }

        constexpr explicit operator bool() const noexcept
        {
            return success_bit;
        }
    };

    [[nodiscard]]
    metrics char_metrics(face_resource face, charmax_t char_code) noexcept;

    template<class String>
    [[nodiscard]] metrics text_metrics(metrics tm, face_resource face, const String& text) noexcept
    {
        const auto processor = [&tm, face](charmax_t ch) noexcept
        {
            if (tm.success_bit) [[likely]]
            {
                tm.add(char_metrics(face, ch));
            }
        };

        decode_utf<sizeof(charmax_t), string_char_t<String>>(text, processor);

        return tm;
    }

    constexpr metrics initial_metrics
    {
        .width{ 0 },
        .top{ numeric_max_v<> },
        .bottom{ numeric_lowest_v<> },
        .success_bit{ true }
    };

    template<class String>
    [[nodiscard]] metrics text_metrics(face_resource face, const String& text) noexcept
    {
        return text_metrics(initial_metrics, face, text);
    }
}
