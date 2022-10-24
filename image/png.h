#pragma once

#include <core/zstring_view.h>
#include <core/color.h>
#include <core/resouce.h>

#include <px/pixmap.h>

struct spng_ctx;

namespace image
{
    using r8g8b8a8pixmap = pixmap<rgba_color32_t>;

    enum class png_errno
    {
        IO_ERROR = -2,
        IO_EOF = -1,
        OK = 0,
        INVAL,
        MEM,
        INT_OVERFLOW,
        SIGNATURE,
        WIDTH,
        HEIGHT,
        USER_WIDTH,
        USER_HEIGHT,
        BIT_DEPTH,
        COLOR_TYPE,
        COMPRESSION_METHOD,
        FILTER_METHOD,
        INTERLACE_METHOD,
        IHDR_SIZE,
        NOIHDR,
        CHUNK_POS,
        CHUNK_SIZE,
        CHUNK_CRC,
        CHUNK_TYPE,
        CHUNK_UNKNOWN_CRITICAL,
        DUP_PLTE,
        DUP_CHRM,
        DUP_GAMA,
        DUP_ICCP,
        DUP_SBIT,
        DUP_SRGB,
        DUP_BKGD,
        DUP_HIST,
        DUP_TRNS,
        DUP_PHYS,
        DUP_TIME,
        DUP_OFFS,
        DUP_EXIF,
        CHRM,
        PLTE_IDX,
        TRNS_COLOR_TYPE,
        TRNS_NO_PLTE,
        GAMA,
        ICCP_NAME,
        ICCP_COMPRESSION_METHOD,
        SBIT,
        SRGB,
        TEXT,
        TEXT_KEYWORD,
        ZTXT,
        ZTXT_COMPRESSION_METHOD,
        ITXT,
        ITXT_COMPRESSION_FLAG,
        ITXT_COMPRESSION_METHOD,
        ITXT_LANG_TAG,
        ITXT_TRANSLATED_KEY,
        BKGD_NO_PLTE,
        BKGD_PLTE_IDX,
        HIST_NO_PLTE,
        PHYS,
        SPLT_NAME,
        SPLT_DUP_NAME,
        SPLT_DEPTH,
        TIME,
        OFFS,
        EXIF,
        IDAT_TOO_SHORT,
        IDAT_STREAM,
        ZLIB,
        FILTER,
        BUFSIZE,
        IO,
        OF,
        BUF_SET,
        BADSTATE,
        FMT,
        FLAGS,
        CHUNKAVAIL,
        NCODE_ONLY,
        OI,
        NOPLTE,
        CHUNK_LIMITS,
        ZLIB_INIT,
        CHUNK_STDLEN,
        INTERNAL,
        CTXTYPE,
        NOSRC,
        NODST,
        OPSTATE,
        NOTFINAL,
        SIZE
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
            return errno_ == png_errno::OK;
        }

        [[nodiscard]]
        pxside_t width() const noexcept;

        [[nodiscard]]
        pxside_t height() const noexcept;

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
        png_errno errno_{ png_errno::NOIHDR };
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

    class pix32result : public pix32space
    {
        using base_type = pix32space;

    public:
        constexpr pix32result(const base_type& space) noexcept
            : base_type{ space }
        {}

        constexpr pix32result(png_errno error_code) noexcept
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
            return errno_ == png_errno::OK;
        }

    private:
        png_errno errno_{ png_errno::OK };
    };

    pix32result png_decode_to_r8g8b8a8(const_buffer_view image, buffer_t& temp) noexcept;

    class r8g8b8a8pixmap_result : public r8g8b8a8pixmap
    {
        using base_type = r8g8b8a8pixmap;

    public:
        constexpr r8g8b8a8pixmap_result(buffer_t& mem, const space_type& space) noexcept
            : base_type{ px::pixmap_construct, mem, space }
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
            return errno_ == png_errno::OK;
        }

    private:
        png_errno errno_{ png_errno::OK };
    };

    inline r8g8b8a8pixmap_result png_decode_to_r8g8b8a8(const_buffer_view image) noexcept
    {
        buffer_t temp;

        const auto space = png_decode_to_r8g8b8a8(image, temp);
        if (!space)
        {
            return space.error_code();
        }

        return { temp, space };
    }
}