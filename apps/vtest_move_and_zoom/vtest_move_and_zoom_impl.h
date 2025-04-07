#pragma once


namespace vtest_move_and_zoom
{
    using figure_area = rectangle<pxoff_t>;

    template<class T>
    [[nodiscard]] constexpr bool apply_zoom(figure_area& rc, T&& zoom) noexcept
    {
        using vec2i_t = vec2<intmax_t>;  

        const auto vzoom = md_ceil_cast<vec2i_t>(std::forward<T>(zoom));
        const auto [vposition, vsizes] = md_numeric_cast<vec2<vec2i_t>>(rc.position, rc.sizes);

        const figure_area new_rc
        { 
            .position{ md_clamp_cast<pxoffs>(vposition - vzoom / 2) },
            .sizes{ md_clamp_cast<pxsizes>(vsizes + vzoom) }
        };

        return px::update_pxf(rc, new_rc);
    }

    [[nodiscard]]
    inline bool apply_nzoom(figure_area& rc, double rot) noexcept
    {
        constexpr auto mul = 0.05;
        const auto sign = 1 - 2 * std::signbit(rot);
        const auto step_mul = sign * pow(mul, abs(rot));
        return apply_zoom(rc, rc.sizes * step_mul);
    }

    [[nodiscard]]
    constexpr figure_area default_area(pxsizes viewport) noexcept
    {
        return
        {
            .position{ md_narrow<pxoffs>(viewport / 4u) },
            .sizes{ viewport / 2u }
        };
    }

    class shaders_lib
    {
    public:
        [[nodiscard]]
        bool initialize(pxsizes viewport, gl::texture2d_resource texture) noexcept
        {
            if (lib.load()) [[likely]]
            {
                lib.texture(texture);
                lib.color(1.0f, 0.5f, 0.5f, 1.0f);
                lib.viewport(viewport);
                return true;
            }

            return false;
        }

        template<class T>
        void draw(const rectangle<T>& rc) const noexcept
        {
            lib.use();
            lib.geometry(rc);
            lib.draw();
        }

    private:
        shader_embed::luminance_texture lib{};
    };

    [[nodiscard]]
    inline gl::texture2d make_test_lumpixmap_texture(pxsizes sizes) noexcept
    {
        using pixmap_t = lumpixmap;

        pixmap_t tex_mem{ sizes };
        if (!tex_mem)
        {
            e_debug("out of memory");
            return {};
        }

        {
            std::default_random_engine content_generator{};

            for (auto line : tex_mem)
            {
                const auto cend = line.cend();
                for (auto it = line.begin(); it < cend; it += pixmap_t::alignment)
                {
                    const auto value = content_generator();
                    static_assert(pixmap_t::alignment == sizeof(value));
                    memcpy(it, &value, sizeof(value));
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
}
