#pragma once

#include <core/zstring_view.h>
#include <core/color.h>
#include <core/resource.h>

#include <px/pixmap.h>


struct spng_ctx;

namespace image
{
    using r8g8b8a8_color_pixmap = pixmap<rgba_color_t>;
    using const_r8g8b8a8_color_pixspan = pixspan<const rgba_color_t>;

    enum class png_errno
    {
        PNG_IO_ERROR = -2,
        PNG_IO_EOF = -1,
        PNG_OK = 0,
        PNG_INVAL,
        PNG_MEM,
        PNG_INT_OVERFLOW,
        PNG_SIGNATURE,
        PNG_WIDTH,
        PNG_HEIGHT,
        PNG_USER_WIDTH,
        PNG_USER_HEIGHT,
        PNG_BIT_DEPTH,
        PNG_COLOR_TYPE,
        PNG_COMPRESSION_METHOD,
        PNG_FILTER_METHOD,
        PNG_INTERLACE_METHOD,
        PNG_IHDR_SIZE,
        PNG_NOIHDR,
        PNG_CHUNK_POS,
        PNG_CHUNK_SIZE,
        PNG_CHUNK_CRC,
        PNG_CHUNK_TYPE,
        PNG_CHUNK_UNKNOWN_CRITICAL,
        PNG_DUP_PLTE,
        PNG_DUP_CHRM,
        PNG_DUP_GAMA,
        PNG_DUP_ICCP,
        PNG_DUP_SBIT,
        PNG_DUP_SRGB,
        PNG_DUP_BKGD,
        PNG_DUP_HIST,
        PNG_DUP_TRNS,
        PNG_DUP_PHYS,
        PNG_DUP_TIME,
        PNG_DUP_OFFS,
        PNG_DUP_EXIF,
        PNG_CHRM,
        PNG_PLTE_IDX,
        PNG_TRNS_COLOR_TYPE,
        PNG_TRNS_NO_PLTE,
        PNG_GAMA,
        PNG_ICCP_NAME,
        PNG_ICCP_COMPRESSION_METHOD,
        PNG_SBIT,
        PNG_SRGB,
        PNG_TEXT,
        PNG_TEXT_KEYWORD,
        PNG_ZTXT,
        PNG_ZTXT_COMPRESSION_METHOD,
        PNG_ITXT,
        PNG_ITXT_COMPRESSION_FLAG,
        PNG_ITXT_COMPRESSION_METHOD,
        PNG_ITXT_LANG_TAG,
        PNG_ITXT_TRANSLATED_KEY,
        PNG_BKGD_NO_PLTE,
        PNG_BKGD_PLTE_IDX,
        PNG_HIST_NO_PLTE,
        PNG_PHYS,
        PNG_SPLT_NAME,
        PNG_SPLT_DUP_NAME,
        PNG_SPLT_DEPTH,
        PNG_TIME,
        PNG_OFFS,
        PNG_EXIF,
        PNG_IDAT_TOO_SHORT,
        PNG_IDAT_STREAM,
        PNG_ZLIB,
        PNG_FILTER,
        PNG_BUFSIZE,
        PNG_IO,
        PNG_OF,
        PNG_BUF_SET,
        PNG_BADSTATE,
        PNG_FMT,
        PNG_FLAGS,
        PNG_CHUNKAVAIL,
        PNG_NCODE_ONLY,
        PNG_OI,
        PNG_NOPLTE,
        PNG_CHUNK_LIMITS,
        PNG_ZLIB_INIT,
        PNG_CHUNK_STDLEN,
        PNG_INTERNAL,
        PNG_CTXTYPE,
        PNG_NOSRC,
        PNG_NODST,
        PNG_OPSTATE,
        PNG_NOTFINAL,
        PNG_SIZE
    };

    [[nodiscard]]
    zstring_view png_error_string(png_errno e) noexcept;

    using png_resource = spng_ctx*;

    struct png_resource_deleter
    {
        void operator () (png_resource png) const noexcept;
    };

    using png_t = unique_resource<png_resource, png_resource_deleter>;

    enum class png_context_flags
    {
        DEFAULT = 0,
        IGNORE_ADLER32 = 1,
        ENCODER = 2
    };

    [[nodiscard]]
    png_t png_instance(png_context_flags flags = png_context_flags::DEFAULT) noexcept;

    png_errno png_set_buffer(png_resource png, const_buffer_view buffer) noexcept;

    enum class png_color_type : uint8_t
    {
        GRAYSCALE = 0,
        TRUECOLOR = 2,
        INDEXED = 3,
        GRAYSCALE_ALPHA = 4,
        TRUECOLOR_ALPHA = 6
    };

    class png_header
    {
    public:
        explicit png_header(png_resource png) noexcept;

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return errno_ == png_errno::PNG_OK;
        }

        [[nodiscard]]
        pxsize_t width() const noexcept;

        [[nodiscard]]
        pxsize_t height() const noexcept;

        [[nodiscard]]
        uint8_t bit_depth() const noexcept;

        [[nodiscard]]
        png_color_type color_type() const noexcept;

        [[nodiscard]]
        pxsize2d sizes() const noexcept
        {
            return { width(), height() };
        }

        [[nodiscard]]
        constexpr png_errno error_code() const noexcept
        {
            return errno_;
        }

    private:
        static constexpr auto png_header_len = 16_uz;
        static constexpr auto png_header_align = 8_uz;
        alignas(png_header_align) std::byte storage_[png_header_len]{};
        png_errno errno_{ png_errno::PNG_NOIHDR };
    };


    enum class png_format
    {
        RGBA8 = 1,
        RGBA16 = 2,
        RGB8 = 4,
        PNG = 256, // No conversion, host-endian 
        RAW = 512  // No conversion, big-endian
    };

    png_errno png_decoded_image_size(png_resource png, png_format format, size_t* size) noexcept;

    png_errno png_decode_image(png_resource png, png_format format, buffer_view out) noexcept;


    class r8g8b8a8_result : public const_r8g8b8a8_color_pixspan
    {
        using base_type = const_r8g8b8a8_color_pixspan;

    public:
        using base_type::space_type;
        using base_type::const_pointer;

        constexpr r8g8b8a8_result(const_pointer p, space_type space) noexcept
            : base_type{ p , space }
        {}

        constexpr r8g8b8a8_result(png_errno error_code) noexcept
            : errno_{ error_code }
        {}

        [[nodiscard]]
        constexpr png_errno error_code() const noexcept
        {
            return errno_;
        }

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return errno_ == png_errno::PNG_OK;
        }

    private:
        png_errno errno_{ png_errno::PNG_OK };
    };

    r8g8b8a8_result png_decode_to_r8g8b8a8(const_buffer_view image, buffer_t& temp) noexcept;

    class r8g8b8a8pixmap_result : public r8g8b8a8_color_pixmap
    {
        using base_type = r8g8b8a8_color_pixmap;

    public:
        using base_type::space_type;

        constexpr r8g8b8a8pixmap_result(buffer_t&& mem, const space_type& space) noexcept
            : base_type{ px::pixmap_construct, std::move(mem), space }
        {}

        constexpr r8g8b8a8pixmap_result(png_errno error_code) noexcept
            : errno_{ error_code }
        {}

        [[nodiscard]]
        constexpr png_errno error_code() const noexcept
        {
            return errno_;
        }

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return errno_ == png_errno::PNG_OK;
        }

    private:
        png_errno errno_{ png_errno::PNG_OK };
    };

    inline r8g8b8a8pixmap_result png_decode_to_r8g8b8a8(const_buffer_view image) noexcept
    {
        buffer_t temp{};

        const auto space = png_decode_to_r8g8b8a8(image, temp);
        if (!space)
        {
            return space.error_code();
        }

        return { std::move(temp), space };
    }
}