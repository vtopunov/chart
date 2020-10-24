#include "png.h"

#include <spng.h>

#include <core/underlying_cast.h>


namespace image
{
    static_assert(to_underlying(png_errno::IO_ERROR) == SPNG_IO_ERROR);
    static_assert(to_underlying(png_errno::OK) == SPNG_OK);
    static_assert(to_underlying(png_errno::NOTFINAL) == SPNG_ENOTFINAL);

    zstring_view png_error_string(png_errno e) noexcept
    {
        return spng_strerror(static_cast<spng_errno>(e));
    }


    void png_resource_deleter::operator()(png_resource png, resource_destroy_t) const noexcept
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


#pragma warning(push)
#pragma warning(disable : 26490) // don't use reinterpret_cast

    png_header::png_header(png_resource png) noexcept
        : storage_{}
        , errno_{ png_errno::NOIHDR }
    {
        static_assert(png_header_len >= sizeof(spng_ihdr));
        errno_ = underlying_cast<png_errno>(spng_get_ihdr(png, reinterpret_cast<spng_ihdr*>(&storage_)));
    }

    uint32_t png_header::width() const noexcept
    {
        static_assert(std::is_same_v<decltype(spng_ihdr::width), uint32_t>);
        return reinterpret_cast<const spng_ihdr&>(storage_).width;
    }

    uint32_t png_header::height() const noexcept
    {
        static_assert(std::is_same_v<decltype(spng_ihdr::height), uint32_t>);
        return reinterpret_cast<const spng_ihdr&>(storage_).height;
    }

    uint8_t png_header::bit_depth() const noexcept
    {
        static_assert(std::is_same_v<decltype(spng_ihdr::bit_depth), uint8_t>);
        return reinterpret_cast<const spng_ihdr&>(storage_).bit_depth;
    }

    png_color_type png_header::color_type() const noexcept
    {
        static_assert(std::is_same_v<decltype(spng_ihdr::color_type), std::underlying_type_t<png_color_type>>);
        static_assert(to_underlying(png_color_type::GRAYSCALE) == SPNG_COLOR_TYPE_GRAYSCALE);
        static_assert(to_underlying(png_color_type::TRUECOLOR) == SPNG_COLOR_TYPE_TRUECOLOR);
        static_assert(to_underlying(png_color_type::INDEXED) == SPNG_COLOR_TYPE_INDEXED);
        static_assert(to_underlying(png_color_type::GRAYSCALE_ALPHA) == SPNG_COLOR_TYPE_GRAYSCALE_ALPHA);
        static_assert(to_underlying(png_color_type::TRUECOLOR_ALPHA) == SPNG_COLOR_TYPE_TRUECOLOR_ALPHA);
        return underlying_cast<png_color_type>(reinterpret_cast<const spng_ihdr&>(storage_).color_type);
    }

#pragma warning(push) // don't use reinterpret_cast


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


}
