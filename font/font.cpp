#include "font.h"

#include <core/debug.h>

#include <ft2build.h>
#include FT_FREETYPE_H  

#include <freetype/internal/ftobjs.h>

namespace font
{
    namespace
    {
        [[nodiscard]]
        const char* accept_errno(FT_Error errc) noexcept
        {
            if (!!errc)
            {
                const auto error_string = FT_Error_String(errc);
                output_error_string("FT error: {}:{}", errc, error_string);
                return error_string;
            }

            return nullptr;
        }

        struct glyph_metrics_wrapper
        {
            const FT_Glyph_Metrics& m;

            [[nodiscard]]
            constexpr ft_fixed_t y_bearing() const noexcept
            {
                return { -m.horiBearingY };
            }

            [[nodiscard]]
            constexpr ft_fixed_t x_bearing() const noexcept
            {
                return { m.horiBearingX };
            }

            [[nodiscard]]
            constexpr point2d<ft_fixed_t> bearing() const noexcept
            {
                return { x_bearing(), y_bearing() };
            }

            [[nodiscard]]
            constexpr ft_fixed_t width() const noexcept
            {
                return { m.horiAdvance };
            }
        };

        [[nodiscard]]
        constexpr glyph_metrics metrics(FT_GlyphSlot glyph) noexcept
        {
            const glyph_metrics_wrapper m{ glyph->metrics };

            const auto top = m.y_bearing();

            return
            {
                .width{ m.width() },
                .top{ top },
                .bottom{ top + glyph->bitmap.rows }
            };
        }
    }

    struct _library_descriptor : FT_LibraryRec_
    {};

    struct _face_descriptor : FT_FaceRec_
    {};

    
    void font_deleter::operator()(const font_resource& font) const noexcept
    {
        FT_Done_Face(font.face);
        FT_Done_FreeType(font.lib);
    }

    font_t create_font(const_buffer_view font_storage, px::size2d_t sizes) noexcept
    {
        font_t result;

        auto& ft = as_mutable(result.r());

        {
            FT_Library lib{ nullptr };
            const auto errc = FT_Init_FreeType(&lib);
            ft.lib = static_cast<library_descriptor_t>(lib);
            
            if (accept_errno(errc))
            {
                result.reset();
            }
        }

        if (ft.lib)
        {
            FT_Face face{ nullptr };

            const auto errc = FT_New_Memory_Face(
                ft.lib,
                font_storage.as_ptr<FT_Byte>(),
                narrow_cast<FT_Long>(font_storage.size()), 0,
                &face
            );

            ft.face = static_cast<face_descriptor_t>(face);

            if (accept_errno(errc))
            {
                result.reset();
            }
        }

        if (ft.face)
        {
            if (!font::sizes(ft.face, sizes))
            {
                result.reset();
            }
        }

        return result;
    }

    bool sizes(face_descriptor_t face, px::size2d_t sizes) noexcept
    {
        const auto errc = FT_Set_Pixel_Sizes
        (
            face,
            narrow_cast<FT_UInt>(sizes.width()),
            narrow_cast<FT_UInt>(sizes.height())
        );

        return !accept_errno(errc);
    }

    cursor draw_char(pix8span_t image, cursor cursor, face_descriptor_t face, charmax_t char_code) noexcept
    {
        if (const auto end_x = cursor::value_type::instance(image.width()); cursor.x() >= end_x)
        {
            return invalid_cursor;
        }

        if (!char_code)
        {
            return invalid_cursor;
        }

        if (accept_errno(FT_Load_Char(face, safe_numeric_cast<FT_ULong>(char_code), FT_LOAD_RENDER)))
        {
            return invalid_cursor;
        }
        
        const auto glyph = face->glyph;
        if (!glyph)
        {
            return invalid_cursor;
        }

        const glyph_metrics_wrapper m{ glyph->metrics };

        const auto advance_x = m.width();
        if (!is_positive(advance_x))
        {
            return invalid_cursor;
        }

        const auto& bitmap = glyph->bitmap;

        if (bitmap.width && bitmap.rows)
        {
            if (!bitmap.buffer)
            {
                return invalid_cursor;
            }

            auto buffer = as_const_pointer(bitmap.buffer);

            size2d sizes
            {
                narrow_cast<pxside_t>(bitmap.width),
                narrow_cast<pxside_t>(bitmap.rows)
            };

            const auto line_size = narrow_cast<size_t>(bitmap.pitch);

            auto position = cursor + m.bearing();

            if (is_negative(position.x()))
            {
                const auto buffer_offset = narrow_cast<pxside_t>(-position.x().discard_fraction());
                if (buffer_offset >= sizes.width())
                {
                    return invalid_cursor;
                }
                
                buffer += buffer_offset;
                sizes.ref_width() -= buffer_offset;
                position.ref_x() = {};
            }

            if (is_negative(position.y()))
            {
                const auto line_offset = narrow_cast<pxside_t>(-position.y().discard_fraction());
                if (line_offset >= sizes.height())
                {
                    return invalid_cursor;
                }

                buffer += ( line_size * safe_numeric_cast<size_t>(line_offset) );
                sizes.ref_height() -= line_offset;
                position.ref_y() = {};
            }

            using pix_t = std::remove_pointer_t<decltype(buffer)>;
            static_assert(sizeof(pix_t) == image.px_size);
            const pixspan<const pix_t, px::dynamic_alignment> glyph_image{ buffer, sizes, line_size };

            image.store(as_px_position(position), glyph_image);
        }

        cursor.ref_x() += advance_x;

        return cursor;
    }

    glyph_metrics metrics(face_descriptor_t face, charmax_t char_code) noexcept
    {
        constexpr glyph_metrics invalid_metrics{};

        if (!char_code)
        {
            return invalid_metrics;
        }

        if (accept_errno(FT_Load_Char(face, safe_numeric_cast<FT_ULong>(char_code), FT_LOAD_DEFAULT)))
        {
            return invalid_metrics;
        }

        const auto glyph = face->glyph;

        return (glyph) ? metrics(glyph) : invalid_metrics;
    }
}