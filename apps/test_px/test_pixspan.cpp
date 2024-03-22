#include <px/pixspan.h>


void test_pixspan() noexcept
{
    constexpr pxsize2d image_sizes{ 9_npx, 9_npx };

    static_assert(4_uz == px::default_alignment);
    constexpr auto line_size = size_align<px::default_alignment>(image_sizes.width());
    static_assert(line_size == px::aligned_width<1u, px::default_alignment>(image_sizes.width()));

    static constexpr pix8_t image[image_sizes.height() * line_size]
    {
        0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00,
        0xff, 0xcc, 0xcc,  0xcc, 0xcc, 0xcc,  0xcc, 0xcc, 0xff,  0x00, 0x00, 0x00,
        0xff, 0xcc, 0x99,  0x99, 0x99, 0x99,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,

        0xff, 0xcc, 0x99,  0x66, 0x66, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
        0xff, 0xcc, 0x99,  0x66, 0x33, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
        0xff, 0xcc, 0x99,  0x66, 0x66, 0x66,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,

        0xff, 0xcc, 0x99,  0x99, 0x99, 0x99,  0x99, 0xcc, 0xff,  0x00, 0x00, 0x00,
        0xff, 0xcc, 0xcc,  0xcc, 0xcc, 0xcc,  0xcc, 0xcc, 0xff,  0x00, 0x00, 0x00,
        0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0xff, 0xff, 0xff,  0x00, 0x00, 0x00
    };

    constexpr const_pix8span image_span{ std::data(image), image_sizes };
    static_assert(std::data(image) == image_span.data());
    static_assert(std::data(image) == std::data(image_span.line()));
    static_assert(image_sizes.width() == std::size(image_span.line()));
    static_assert(line_size == std::size(image_span.lines()));
    static_assert(image_span.line() == *image_span.begin());
    static_assert(std::end(image) == image_span.end());
    static_assert(line_size == image_span.line_size());

    {
        uint8_t temp_image[std::size(image)]{};
        pix8span temp_image_span{ std::data(temp_image), image_sizes };
        D_ASSERT(image_sizes == temp_image_span.store(image_span));
        D_ASSERT(!memcmp(temp_image, image, sizeof(image)));
    }

    {
        uint8_t temp_image[std::size(image) - line_size]{};
        constexpr auto cropped_height = image_sizes.height() - 1_uz;
        pix8span temp_image_span{ std::data(temp_image), image_sizes.width(), cropped_height };
        const auto cropped_size = temp_image_span.store(image_span);
        D_ASSERT(image_sizes.width() == cropped_size.width());
        D_ASSERT(cropped_height == cropped_size.height());
        D_ASSERT(!memcmp(temp_image, image, sizeof(temp_image)));
    }

    {
        uint8_t temp_image[std::size(image) + line_size]{};
        pix8span temp_image_span{ std::data(temp_image), image_sizes.width(), image_sizes.height() + 1_npx };
        D_ASSERT(image_sizes == temp_image_span.store(0_npx, 1_npx, image_span));
        D_ASSERT(!memcmp(temp_image + line_size, image, sizeof(image)));
    }

    {
        uint8_t temp_image[std::size(image)]{};
        pix8span temp_image_span{ std::data(temp_image), image_sizes };

        {
            const_pix8span const_temp_image_span{ temp_image_span };
            D_ASSERT(std::data(const_temp_image_span) == std::data(temp_image_span));
            D_ASSERT(space(const_temp_image_span) == space(temp_image_span));
            const_temp_image_span = {};
            D_ASSERT(std::data(const_temp_image_span) != std::data(temp_image_span));
            D_ASSERT(space(const_temp_image_span) != space(temp_image_span));
            const_temp_image_span = temp_image_span;
            D_ASSERT(std::data(const_temp_image_span) == std::data(temp_image_span));
            D_ASSERT(space(const_temp_image_span) == space(temp_image_span));
        }

        {
            pixspan<pix8span::pixel_type, px::dynamic_alignment> unalign_temp_image_span{ temp_image_span };
            D_ASSERT(std::data(unalign_temp_image_span) == std::data(temp_image_span));
            D_ASSERT(unalign_temp_image_span.sizes() == temp_image_span.sizes());
            D_ASSERT(unalign_temp_image_span.line_size() == temp_image_span.line_size());
            unalign_temp_image_span = {};
            D_ASSERT(std::data(unalign_temp_image_span) != std::data(temp_image_span));
            D_ASSERT(unalign_temp_image_span.sizes() != temp_image_span.sizes());
            D_ASSERT(unalign_temp_image_span.line_size() != temp_image_span.line_size());
            unalign_temp_image_span = temp_image_span;
            D_ASSERT(std::data(unalign_temp_image_span) == std::data(temp_image_span));
            D_ASSERT(unalign_temp_image_span.sizes() == temp_image_span.sizes());
            D_ASSERT(unalign_temp_image_span.line_size() == temp_image_span.line_size());
        }
    }
}

