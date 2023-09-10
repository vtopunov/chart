#pragma once

#include <random>

#include <debug/debug.h>

#include <gl/texture.h>


[[nodiscard]]
inline gl::texture2d pix8map_test_texture_generate(pxsize2d sizes) noexcept
{
    using pixmap_t = pix8map;

    pixmap_t tex_mem{ sizes };
    if (!tex_mem)
    {
        e_debug("out of memory");
        return {};
    }

    std::default_random_engine content_generator{};

    {
        const auto width = tex_mem.width();
        for (auto line_it : tex_mem)
        {
            for (const auto end = line_it + width; line_it < end; line_it += pixmap_t::alignment)
            {
                const auto value = content_generator();
                static_assert(pixmap_t::alignment == sizeof(value));
                memcpy(line_it, &value, sizeof(value));
            }
        }
    }

    auto texture = gl::create_texture2d(tex_mem);
    if (!texture)
    {
        e_debug("create texture error: {}", glGetError());
        return {};
    }

    return texture;
}
