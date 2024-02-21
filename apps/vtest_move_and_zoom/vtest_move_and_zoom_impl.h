#pragma once


namespace vtest_move_and_zoom
{
    [[nodiscard]]
    constexpr pxzrectangle zoom(pxzrectangle rc, lpxoff2d zoom) noexcept
    { 
        const auto position = md_trunc_cast<pxoff2d>(rc.position - zoom / 2);
        const auto sizes = md_trunc_cast<pxsize2d>(rc.sizes + zoom);

        const auto ok 
            = md_is_safe_narrowing_conversion<gl::vec2f>(position)
            && md_is_safe_narrowing_conversion<gl::vec2f>(sizes);

        if(ok)
        {
            return
            {
                .position{ position },
                .sizes{ sizes }
            };
        }

        return rc;
    }

    [[nodiscard]]
    inline pxzrectangle zoom_increase(pxzrectangle rc, double rot) noexcept
    {
        constexpr double mul{ 0.05 };
        const auto sign = 1 - 2 * std::signbit(rot);
        const auto step_mul = sign * pow(mul, abs(rot));
        return zoom(rc, md_trunc_cast<lpxoff2d>(rc.sizes * step_mul));
    }

    [[nodiscard]]
    constexpr pxzrectangle default_area(pxsize2d viewport) noexcept
    {
        return
        {
            .position{ md_narrow<pxoff2d>(viewport / 4u) },
            .sizes{ viewport / 2u }
        };
    }

    class shaders_lib
    {
    public:
        [[nodiscard]]
        bool initialize(pxsize2d viewport, gl::texture2d_resource texture) noexcept
        {
            const auto was_successful
                = lib || lib.build();

            if (was_successful)
            {
                lib.use();
                lib.frag.s_texture.store(texture);
                lib.frag.u_color.store(1.0f, 0.5f, 0.5f, 1.0f);
                lib.vert.u_viewport.store(viewport);
            }

            return was_successful;
        }

        template<class T>
        void draw(const rectangle<T>& rc) const noexcept
        {
            lib.use();
            lib.vert.u_position.store(rc.position);
            lib.vert.u_size.store(rc.sizes);
            lib.vert.a_frame.draw();
        }

    private:
        shader_library<vert::positioned_texture, frag::luminance8_texture_mix_color> lib{};
    };

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

        auto texture = gl::create_texture2d(view(tex_mem));
        if (!texture)
        {
            e_debug("create texture error: {}", glGetError());
            return {};
        }

        return texture;
    }
}
