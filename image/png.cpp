#include "png.h"

#include <spng.h>

namespace image
{
    static_assert(to_underlying(png_errno::PNG_IO_ERROR) == SPNG_IO_ERROR);
    static_assert(to_underlying(png_errno::PNG_OK) == SPNG_OK);
    static_assert(to_underlying(png_errno::PNG_NOTFINAL) == SPNG_ENOTFINAL);

    namespace
    {
        const spng_ihdr* as_spng_ihdr(const std::byte* storage) noexcept
        {
            return reinterpret_cast<const spng_ihdr*>(storage);
        }

        spng_ihdr* as_spng_ihdr(std::byte* storage) noexcept
        {
            return reinterpret_cast<spng_ihdr*>(storage);
        }
    }

    zstring_view png_error_string(png_errno e) noexcept
    {
        if (e == png_errno::PNG_SIZE)
        {
            return "invalid image size"_zsv;
        }

        return spng_strerror(static_cast<spng_errno>(e));
    }


    void png_resource_deleter::operator()(png_resource png) const noexcept
    {
        spng_ctx_free(png);
    }

    png_t png_instance(png_context_flags flags) noexcept
    {
        static_assert(to_underlying(png_context_flags::IGNORE_ADLER32) == SPNG_CTX_IGNORE_ADLER32);
        static_assert(to_underlying(png_context_flags::ENCODER) == SPNG_CTX_ENCODER);

        return
        {
            resource_construct,
            spng_ctx_new(to_underlying(flags))
        };
    }

    png_errno png_set_buffer(png_resource png, const_buffer_view buffer) noexcept
    {
        return underlying_cast<png_errno>(spng_set_png_buffer(png, buffer.data(), buffer.size()));
    }


    D_WARNING_PUSH;
    D_WARNING_DISABLE_MSVC(W_do_not_use_reinterpret_cast);

    png_header::png_header(png_resource png) noexcept
    {
        static_assert(png_header_len >= sizeof(spng_ihdr));
        static_assert(png_header_align >= alignof(spng_ihdr));
        errno_ = underlying_cast<png_errno>(spng_get_ihdr(png, as_spng_ihdr(storage_)));
    }

    pxside_t png_header::width() const noexcept
    {
        return narrow_cast<pxside_t>(as_spng_ihdr(storage_)->width);
    }

    pxside_t png_header::height() const noexcept
    {
        return narrow_cast<pxside_t>(as_spng_ihdr(storage_)->height);
    }

    uint8_t png_header::bit_depth() const noexcept
    {
        static_assert(std::is_same_v<decltype(spng_ihdr::bit_depth), uint8_t>);
        return as_spng_ihdr(storage_)->bit_depth;
    }

    png_color_type png_header::color_type() const noexcept
    {
        static_assert(std::is_same_v<decltype(spng_ihdr::color_type), std::underlying_type_t<png_color_type>>);
        static_assert(to_underlying(png_color_type::GRAYSCALE) == SPNG_COLOR_TYPE_GRAYSCALE);
        static_assert(to_underlying(png_color_type::TRUECOLOR) == SPNG_COLOR_TYPE_TRUECOLOR);
        static_assert(to_underlying(png_color_type::INDEXED) == SPNG_COLOR_TYPE_INDEXED);
        static_assert(to_underlying(png_color_type::GRAYSCALE_ALPHA) == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA);
        static_assert(to_underlying(png_color_type::TRUECOLOR_ALPHA) == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA);
        return underlying_cast<png_color_type>(as_spng_ihdr(storage_)->color_type);
    }

    D_WARNING_POP;


    static_assert(to_underlying(png_format::RGBA8) == spng_format::SPNG_FMT_RGBA8);
    static_assert(to_underlying(png_format::RGBA16) == spng_format::SPNG_FMT_RGBA16);
    static_assert(to_underlying(png_format::RGB8) == spng_format::SPNG_FMT_RGB8);
    static_assert(to_underlying(png_format::PNG) == spng_format::SPNG_FMT_PNG);
    static_assert(to_underlying(png_format::RAW) == spng_format::SPNG_FMT_RAW);

    png_errno png_decoded_image_size(png_resource png, png_format format, size_t* size) noexcept
    {
        return underlying_cast<png_errno>(spng_decoded_image_size(png, to_underlying(format), size));
    }

    png_errno png_decode_image(png_resource png, png_format format, buffer_view out) noexcept
    {
        return underlying_cast<png_errno>(spng_decode_image(png, out.data(), out.size(), to_underlying(format), 0));
    }

    pix32result png_decode_to_r8g8b8a8(const_buffer_view image, buffer_t& temp) noexcept
    {
        constexpr auto png_format = png_format::RGBA8;

        if (!image || !image.size())
        {
            return png_errno::PNG_SIZE;
        }

        const auto png = png_instance();
        if (!png)
        {
            return png_errno::PNG_MEM;
        }

        png_errno errc{ png_errno::PNG_OK };

        const auto accept_errc = [&errc](png_errno new_errc) noexcept
        {
            errc = new_errc;
            return png_errno::PNG_OK != new_errc;
        };

        if (accept_errc(png_set_buffer(png, image)))
        {
            return errc;
        }

        const png_header png_header{ png };
        if (accept_errc(png_header.error_code()))
        {
            return errc;
        }

        size_t size = 0;
        if (accept_errc(png_decoded_image_size(png, png_format, &size)))
        {
            return errc;
        }

        if (!size)
        {
            return png_errno::PNG_SIZE;
        }

        const pix32space space{ png_header.sizes() };
        if (space.size_bytes() != size)
        {
            return png_errno::PNG_SIZE;
        }

        if (!temp.try_reserve(size))
        {
            return png_errno::PNG_MEM;
        }

        if (accept_errc(png_decode_image(png, png_format, temp)))
        {
            return errc;
        }

        return space;
    }
}
