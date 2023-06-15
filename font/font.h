#pragma once

#include <core/string_char.h>
#include <core/resource.h>
#include <core/buffer_view.h>
#include <core/zstring_view.h>
#include <core/utf.h>

#include <px/pixspan.h>

#include <font/fixed.h>

extern "C" 
{
    typedef struct FT_FaceRec_* private_detail_font_face_descriptor_t;
}

namespace font
{
    using face_descriptor_t = private_detail_font_face_descriptor_t;

    using charmax_t = char32_t;
    using fixed_t = fixed<int64_t, 6_uz>;

    struct face_deleter
    {
        void operator () (face_descriptor_t face) const noexcept;
    };

    using face = unique_resource<face_descriptor_t, face_deleter>;

    [[nodiscard]]
    face create_face(const_buffer_view font_storage, pxsize2d sizes) noexcept;

    [[nodiscard]]
    inline face create_face(const_buffer_view font_storage, pxside_t size) noexcept
    {
        return create_face(font_storage, size2d{ 0_px, size });
    }

    bool sizes(face_descriptor_t face, pxsize2d sizes) noexcept;

    inline bool size(face_descriptor_t face, pxside_t px) noexcept
    {
        return sizes(face, size2d{ 0_px, px });
    }

    struct cursor : point2d<fixed_t>
    {
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
    };

    constexpr auto invalid_position = fill_to<point2d>(numeric_max_v<pxside_t>);
    constexpr auto invalid_cursor = cursor::instance(invalid_position);

    cursor draw_char(pix8span image, cursor cursor, face_descriptor_t face, charmax_t char_code) noexcept;

    template<class String>
    cursor draw_text(pix8span image, cursor cursor, face_descriptor_t face, const String& text) noexcept
    {
        const auto char_processor = [&cursor, image, face](charmax_t char_code) noexcept
        {
            cursor = draw_char(image, cursor, face, char_code);
        };

        decode_utf<sizeof(charmax_t), string_char_t<String>>(text, char_processor);

        return cursor;
    }

    template<class Px, class String>
    cursor draw_text(pix8span image, vec2<Px> position, face_descriptor_t face, const String& text) noexcept
    {
        return draw_text(image, cursor::instance(std::move(position)), face, text);
    }

    template<class X, class Y, class String>
    cursor draw_text(pix8span image, X x, Y y, face_descriptor_t face, const String& text) noexcept
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
            if (glyph.success_bit)
            {
                width = width + glyph.width;
                top = std::min(top, glyph.top);
                bottom = std::max(bottom, glyph.bottom);
            }

            success_bit = success_bit && glyph.success_bit;
        }

        constexpr explicit operator bool() const noexcept
        {
            return success_bit;
        }
    };

    [[nodiscard]]
    metrics char_metrics(face_descriptor_t face, charmax_t char_code) noexcept;

    template<class String>
    [[nodiscard]] metrics text_metrics(metrics tm, face_descriptor_t face, const String& text) noexcept
    {
        const auto processor = [&tm, face](charmax_t ch) noexcept
        {
            if (tm.success_bit)
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
        .top{ numeric_max_v<fixed_t::value_type> },
        .bottom{ numeric_min_v<fixed_t::value_type> },
        .success_bit{ true }
    };

    template<class String>
    [[nodiscard]] metrics text_metrics(face_descriptor_t face, const String& text) noexcept
    {
        return text_metrics(initial_metrics, face, text);
    }
}
