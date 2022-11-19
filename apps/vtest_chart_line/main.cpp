#include <numbers>

#include <core/small_vector.h>
#include <core/lerp.h>

#include <debug/debug.h>

#include <px/algorithm.h>

#include <egl_ui/run.h>

#include <file/file_mmap.h>

#include <utility/shaders_library.h>

namespace
{
    constexpr pxsize2d frame_sizes{ 30_px, 30_px };

    double_t sinc(double_t x) noexcept
    {
        constexpr double_t near_zero_eps{ 0.0004 }; // (eps*120)^(1/4)

        if (abs(x) > near_zero_eps)
        {
            return sin(x) / x;
        }
        else
        {
            return 1 - x * x / 6;
        }
    }

    using f64point2d = point2d<double_t>;

    bool sinc_vector_initialize(small_vector<f64point2d>& v) noexcept
    {
        constexpr auto size = 300_uz;

        if (!v.try_reserve(size))
        {
            e_debug("out of memory");
            return false;
        }

        constexpr auto abscissa_max = 5 * std::numbers::pi_v<double_t>;
        constexpr num_range abscissa_range{ -abscissa_max, abscissa_max };
        constexpr num_range index_range{ 0_uz, size - 1_uz };
        constexpr auto abscissa = lerp(index_range, abscissa_range);

        for (size_t i = index_range._0; i <= index_range._1; ++i)
        {
            const auto x = abscissa(i);
            v.emplace_back(point2d{ x, sinc(x) });
        }

        return true;
    };

    using f64range = num_range<double_t>;

    using f64point2drange = point2d<f64range>;

    constexpr f64point2drange calculate_values_range(span<const f64point2d> line) noexcept
    {
        constexpr num_range invalid_range
        {
            numeric_max_v<double_t>,
            numeric_min_v<double_t>
        };

        constexpr point2d invalid_point_range
        {
            fill_vec2(invalid_range)
        };

        f64point2drange values_range{ invalid_point_range };
        for (const auto& pt : line)
        {
            if (pt._0 < values_range._0._0)
            {
                values_range._0._0 = pt._0;
            }

            if (pt._0 > values_range._0._1)
            {
                values_range._0._1 = pt._0;
            }

            if (pt._1 < values_range._1._0)
            {
                values_range._1._0 = pt._1;
            }

            if (pt._1 > values_range._1._1)
            {
                values_range._1._1 = pt._1;
            }
        }

        return values_range;
    }

    constexpr f64point2drange calculate_pix_range(pxsize2d sizes) noexcept
    {
        return
        {
            f64range{ 0.0, sizes.width() - 1.0 },
            f64range{ 0.0, sizes.height() - 1.0 },
        };
    }

    struct coordinate_transformation : vec2<polynomial2<double_t>>
    {
        template<class Pt>
        constexpr f64point2d operator () (const Pt& pt) const noexcept
        {
            return { _0(pt._0), _1(pt._1) };
        }
    };

    constexpr coordinate_transformation calculate_coordinate_transformation(f64point2drange from, f64point2drange to) noexcept
    {
        return
        {
            lerp(from.cref_x(), to.cref_x()),
            lerp(inverse(from.cref_y()), to.cref_y())
        };
    }


    template<class Transform>
    constexpr void draw_polyline(const pix8span image, span<const f64point2d> values, Transform value2px) noexcept
    {
        auto p0 = value2px(values[0]);

        for (const auto& p : values.subspan(1u))
        {
            const auto p1 = value2px(p);
            px::draw_antialiasing_line(image, p0.x(), p0.y(), p1.x(), p1.y());
            p0 = p1;
        }
    }

    constexpr void draw_chart_line(const pix8span image, span<const f64point2d> values) noexcept
    {
        draw_polyline
        (
            image,
            values,
            calculate_coordinate_transformation
            (
                calculate_values_range(values),
                calculate_pix_range(image.sizes())
            )
        );
    }

    class main_processor
    {
    public:
        [[nodiscard]]
        bool initialize(os::module_handle_t app) noexcept
        {
            if (!sinc_vector_initialize(values_))
            {
                return false;
            }

            egl_ = create_egl_window(app);
            if (!egl_)
            {
                return false;
            }

            if (!background_shaders_.build())
            {
                return false;
            }

            background_shaders_.use();
            background_shaders_.frag.u_color.store(gl::colors::white_f);
            background_shaders_.vert.u_viewport.store(sizes(egl_));
            background_shaders_.vert.u_position.store(frame_sizes);

            if (!tex_shaders_.build())
            {
                return false;
            }

            tex_shaders_.use();
            tex_shaders_.frag.u_color.store(gl::colors::blue_f);
            tex_shaders_.vert.u_viewport.store(sizes(egl_));
            tex_shaders_.vert.u_position.store(frame_sizes);

            return true;
        }

        ui::milliseconds_t operator() (ui::idle_event) noexcept
        {
            if (chart_lines_rendering(ui::sizes(app_window(egl_)) - 2 * frame_sizes))
            {
                const egl_painting_owner painting_lock{ egl_ };
                gl::clear(gl::colors::gray_f);

                background_shaders_.use();
                background_shaders_.vert.u_size.store(sizes(texture_));
                background_shaders_.vert.a_frame.draw();

                tex_shaders_.use();
                tex_shaders_.frag.s_texture.store(texture_);
                tex_shaders_.vert.u_size.store(sizes(texture_));
                tex_shaders_.vert.a_frame.draw();
            }

            return ui::infinite;
        }

        int run()
        {
            return egl_ui::run(egl_, *this);
        }

    private:
        bool chart_lines_rendering(const pxsize2d sizes) noexcept
        {
            if (texture_)
            {
                return true;
            }

            if (image_)
            {
                zero_memory(image_);
            }
            else
            {
                image_ = pix8map{ sizes };
                if (!image_)
                {
                    e_debug("out of memory");
                    return false;
                }
            }

            draw_chart_line(image_, values_);

            texture_ = gl::create_texture2d(image_);
            if (!texture_)
            {
                e_debug("create texture error: {}\n", glGetError());
                return false;
            }

            return true;
        }

    private:
        small_vector<f64point2d> values_{};
        egl_window egl_{};
        shaders_library<vert::positioned_texture, frag::gray_texture_mix_color> tex_shaders_{};
        shaders_library<vert::positioned_rectangle, frag::default_color> background_shaders_{};
        gl::texture2d texture_{};
        pix8map image_{};
    };
}


int app_main(os::module_handle_t app) noexcept
{
    main_processor processor;

    if (!processor.initialize(app))
    {
        e_debug("create window error: ui error: {}, egl error: {}",
            ui::error_code(), eglGetError());
        return EXIT_FAILURE;
    }

    return processor.run();
}




