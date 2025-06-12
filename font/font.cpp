#include "font.h"

#include <debug/debug.h>

namespace
{
    #include <ft2build.h>
    #include FT_FREETYPE_H
}


namespace font
{
    struct s_face_descriptor : FT_FaceRec_
    {};

    namespace
    {
        template<class T, size_t FractBits>
        [[nodiscard]] constexpr pxpoint as_pxposition(const point2d<fixed<T, FractBits>>& p) noexcept
        {
            return
            {
                trunc_to<npx_t>(p.x()),
                trunc_to<npx_t>(p.y())
            };
        }

        static_assert(as_pxposition(invalid_cursor) == invalid_position);

        using library_descriptor_t = FT_Library;

        const char* error_string(FT_Error errc) noexcept
        {
            const auto errs = FT_Error_String(errc);
            return (errs) ? errs : "";
        }

        void e_debug_ft(const char* string, FT_Error errc) noexcept
        {
            e_debug
            (
                "FT error: {}: {}:{}", 
                string, 
                static_cast<int>(errc), 
                error_string(errc)
            );
        }

        struct glyph_metrics_wrapper
        {
            const FT_Glyph_Metrics& m;

            [[nodiscard]]
            constexpr fixed_t y_bearing() const noexcept
            {
                return { -m.horiBearingY };
            }

            [[nodiscard]]
            constexpr fixed_t x_bearing() const noexcept
            {
                return { m.horiBearingX };
            }

            [[nodiscard]]
            constexpr point2d<fixed_t> bearing() const noexcept
            {
                return { x_bearing(), y_bearing() };
            }

            [[nodiscard]]
            constexpr fixed_t width() const noexcept
            {
                return { m.horiAdvance };
            }
        };

        class library
        {
        public:
            library() = delete;
            D_DISABLE_COPYMOVE_CA(library);

            enum class dtor_state
            {
                null,
                disabled = null,
                enabled
            };

            struct deref_type
            {
                void operator () (dtor_state state) const noexcept
                {
                    if (dtor_state::enabled == state)
                    {
                        D_ASSERT(ref_count);
                        if (!--ref_count)
                        {
                            done();
                        }
                    }
                }
            };

            struct library_ref : unique_resource<dtor_state, deref_type>
            {
                using unique_resource::unique_resource;

                operator library_descriptor_t () const noexcept
                {
                    return lib;
                }
            };

            [[nodiscard]] static library_ref instance() noexcept
            {
                static library_ref cached_ref{};

                as_mutable(cached_ref.r()) = dtor_state::enabled;

                library_ref ref{ dtor_state::enabled };

                if (lib)
                {
                    D_ASSERT(ref_count >= ref_count_initializer_with_cached_ref);
                    ++ref_count;
                }
                else
                {
                    library_descriptor_t temp_lib{ nullptr };
                    const auto errc = FT_Init_FreeType(&temp_lib);
                    const auto ok = (FT_Err_Ok == errc);

                    lib = ok ? temp_lib : nullptr;
                    ref_count = ok ? ref_count_initializer_with_cached_ref : ref_count_initializer_without_destroy;

                    if (!ok) [[unlikely]]
                    {
                        D_UNUSED(ref.release());
                        D_UNUSED(cached_ref.release());
                        e_debug_ft("FT_Init_FreeType", errc);
                    }
                }

                return ref;
            }

        private:
            static constexpr auto ref_count_initializer_with_cached_ref = 2_uz;
            static constexpr auto ref_count_initializer_without_destroy = 0_uz;

            static void done() noexcept
            {
                if (const auto errc = FT_Done_FreeType(std::exchange(lib, nullptr)); FT_Err_Ok != errc)
                {
                    e_debug_ft("FT_Init_FreeType", errc);
                }
            }

        private:
            inline static library_descriptor_t lib{ nullptr };
            inline static size_t ref_count{ ref_count_initializer_without_destroy };
        };

        struct ft_face_deleter
        {
            void operator()(FT_Face face) const noexcept
            {
                if (face)
                {
                    [[maybe_unused]]
                    const library::library_ref library_deref{ library::dtor_state::enabled };

                    FT_Done_Face(face);
                }
            }
        };
    }

    void face_deleter::operator()(face_descriptor_t face) const noexcept
    {
        constexpr ft_face_deleter ft_deleter{};
        ft_deleter(face);
    }

    face create_face(const_buffer_view font_storage, pxsizes sizes) noexcept
    {
        auto lib = library::instance();

        face result_face{};

        if (lib) [[likely]]
        {
            constexpr FT_Long face_index{ 0 };
            const auto ft_font_storage = font_storage.as_span<const FT_Byte>();
            unique_resource<FT_Face, ft_face_deleter> ft_face{};

            if (const auto errc
                = FT_New_Memory_Face
                (
                    lib,
                    ft_font_storage.data(),
                    narrow<FT_Long>(ft_font_storage.size()),
                    face_index,
                    std::addressof(as_mutable(ft_face.r()))
                ); errc != FT_Err_Ok) [[unlikely]]
            {
                ft_face.reset();
                e_debug_ft("FT_New_Memory_Face", errc);
            }

            result_face = face{ static_cast<face_descriptor_t>(ft_face.release()) };
        }

        if (result_face) [[likely]]
        {
            if (!font::sizes(result_face, sizes)) [[unlikely]]
            {
                result_face.reset();
            }
        }

        if (result_face) [[likely]]
        {
            D_UNUSED(lib.release());
        }

        return result_face;
    }

    bool sizes(face_descriptor_t face, pxsizes sizes) noexcept
    {
        if (const auto errc
            = FT_Set_Pixel_Sizes
            (
                face,
                narrow<FT_UInt>(sizes.width()),
                narrow<FT_UInt>(sizes.height())
            ); errc != FT_Err_Ok) [[unlikely]]
        {
            e_debug_ft("FT_Set_Pixel_Sizes", errc);
            return false;
        }

        return true;
    }

    cursor draw_char(lumpixspan image, cursor cursor, face_descriptor_t face, charmax_t char_code) noexcept
    {
        if (const auto end_x = cursor::value_type::instance(image.width()); cursor.x() >= end_x) [[unlikely]]
        {
            return invalid_cursor;
        }

        if (!char_code) [[unlikely]]
        {
            return invalid_cursor;
        }

        if (const auto errc 
            = FT_Load_Char
            (
                face, 
                numeric_cast<FT_ULong>(char_code), 
                FT_LOAD_RENDER
            ); FT_Err_Ok != errc) [[unlikely]]
        {
            e_debug_ft("FT_Load_Char FT_LOAD_RENDER", errc);
            return invalid_cursor;
        }

        const auto glyph = face->glyph;
        if (!glyph) [[unlikely]]
        {
            return invalid_cursor;
        }

        const glyph_metrics_wrapper m{ glyph->metrics };

        const auto advance_x = m.width();
        if (!is_positive(advance_x)) [[unlikely]]
        {
            return invalid_cursor;
        }

        const auto& bitmap = glyph->bitmap;

        if (bitmap.width && bitmap.rows) [[likely]]
        {
            if (!bitmap.buffer) [[unlikely]]
            {
                return invalid_cursor;
            }

            auto buffer = as_const_pointer(bitmap.buffer);

            size2d sizes
            {
                narrow<npx_t>(bitmap.width),
                narrow<npx_t>(bitmap.rows)
            };

            const auto line_size = narrow<size_t>(bitmap.pitch);

            auto position = cursor + m.bearing();

            if (is_negative(position.x()))
            {
                const auto buffer_offset = narrow<npx_t>(-position.x().discard_fraction());
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
                const auto line_offset = narrow<npx_t>(-position.y().discard_fraction());
                if (line_offset >= sizes.height())
                {
                    return invalid_cursor;
                }

                buffer += (line_size * numeric_cast<size_t>(line_offset));
                sizes.ref_height() -= line_offset;
                position.ref_y() = {};
            }

            using const_pix_t = std::remove_pointer_t<decltype(buffer)>;
            static_assert(std::is_const_v<const_pix_t> && sizeof(const_pix_t) == image.px_size);
            const pixspan<const_pix_t, px::dynamic_alignment> glyph_image{ buffer, sizes, line_size };

            image.store(as_pxposition(position), glyph_image);
        }

        cursor.ref_x() += advance_x;

        return cursor;
    }

    metrics char_metrics(face_descriptor_t face, charmax_t char_code) noexcept
    {
        constexpr metrics invalid_metrics{};

        if (!char_code) [[unlikely]]
        {
            return invalid_metrics;
        }

        if (const auto errc 
            = FT_Load_Char
            (
                face, 
                numeric_cast<FT_ULong>(char_code), 
                FT_LOAD_DEFAULT
            ); FT_Err_Ok != errc) [[unlikely]]
        {
            e_debug_ft("FT_Load_Char FT_LOAD_DEFAULT", errc);
            return invalid_metrics;
        }

        const auto glyph = face->glyph;
        if (!glyph) [[unlikely]]
        {
            return invalid_metrics;
        }

        const glyph_metrics_wrapper m{ glyph->metrics };

        const auto top = m.y_bearing();

        return
        {
            .width{ m.width() },
            .top{ top },
            .bottom{ top + glyph->bitmap.rows },
            .success_bit{ true }
        };
    }
}