#pragma once

#include <core/fmt.h>
#include <core/small_vector.h>

#include <utility/font_cache.h>

#include <shader/library.h>

#include <chart/space_manipulation.h>
#include <chart/periodic_position.h>


namespace chart
{
    namespace private_detail_value_labels
    {
        using texspan = span<char>;

        [[nodiscard]]
        inline std::string_view make_label_string_view(const texspan text, const double value) noexcept
        {   
            const auto data = std::data(text);
            const auto format_result = fmt::format_to_n(data, std::size(text), "{:.10g}", value);
            return std::string_view{ data, format_result.size };
        }

        struct buffer_interpret
        {
            const lumpixspan image;
            const texspan text;
        };

        [[nodiscard]]
        constexpr buffer_interpret make_buffer_interpret(basic_buffer_view<luminance_t> lumview, const luminance_pixspace space) noexcept
        {
            D_ASSERT(space.size() <= lumview.size());

            return
            {
                .image{ lumview.data(), space },
                .text{ make_span(interpret<char>(lumview)).subspan(space.size_bytes()) }
            };
        }
    }

    struct value_labels
    {
        static constexpr pxmargins margins
        {
            .left{ 75_npx },
            .top{ 8_npx },
            .right{ 8_npx },
            .bottom{ 30_npx }
        };

        static constexpr pxpoint frame_widths{ 3_npx, 3_npx };
        static constexpr size_t max_text_size{ 256u };

        font_cache::cached_face font{ font_cache::default_font() };
        point2d<gl::texture2d_owner> tex_axis{};

        constexpr void operator () (pxrectangle& geometry) const noexcept
        {
            constexpr auto margins_move = margins.left_top();
            constexpr auto margins_zoomout = margins_move + margins.right_bottom();

            geometry.position += margins_move;
            
            if (margins_zoomout.x() < geometry.sizes.width()) [[likely]]
            {
                geometry.sizes.ref_width() -= margins_zoomout.x();
            }

            if (margins_zoomout.y() < geometry.sizes.height()) [[likely]]
            {
                geometry.sizes.ref_height() -= margins_zoomout.y();
            }
        }

        [[nodiscard]]
        static constexpr point2d<const luminance_pixspace> space(const pxrectangle& geometry) noexcept
        {
            constexpr auto y_axis_width = margins.left - frame_widths.x();
            constexpr auto y_axis_ex_height = margins.top + std::min(margins.top, margins.bottom);
            constexpr auto x_axis_height = margins.bottom - frame_widths.y();
            return
            {
                luminance_pixspace{ geometry.width() + margins.right, x_axis_height },
                luminance_pixspace{ y_axis_width, geometry.height() + y_axis_ex_height}
            };
        }

        void operator () (const periodic_value_position& grid, temp_byte_buffer& buffer, const pxrectangle& geometry) noexcept
        {
            using namespace private_detail_value_labels;

            const auto space_axis = space(geometry);
            const auto axis_size_bytes = std::max({
                max_text_size,
                space_axis.cref_x().size_bytes(),
                space_axis.cref_y().size_bytes()
            });

            if (const auto lumview = buffer.try_get_reserve(axis_size_bytes + max_text_size)) [[likely]]
            {
                const auto tm0 = ::font::char_metrics(font, '0');
                D_ASSERT(tm0);

                {
                    const auto [image, textbuf] = make_buffer_interpret(lumview, space_axis.cref_x());
                    const auto x_px0 = grid.px.begin.x() + 2.0;
                    const auto y_px = 2 - tm0.top;

                    zero_memory(image);

                    ::font::fixed_t x_px_end{ numeric_min_v<::font::fixed_t::int_type> };
                    for (size_t i = 0; i < grid.count.x(); ++i)
                    {
                        const auto x = grid.value.begin.x() + i * grid.value.repeat.x();
                        const auto x_label_text = make_label_string_view(textbuf, x);
                        const auto x_label_tm = ::font::text_metrics(font, x_label_text);
                        const auto x_px = ::font::fixed_t::instance(x_px0 + i * grid.px.repeat.x()) - x_label_tm.width / 2;

                        if (x_px_end < x_px) [[likely]]
                        {
                            ::font::draw_text
                            (
                                image,
                                x_px,
                                y_px,
                                font,
                                x_label_text
                            );

                            x_px_end = x_px + x_label_tm.width;
                        }
                    }

                    D_CHECK(gl::update(tex_axis.ref_x(), image));
                }

                {                    
                    const auto [image, textbuf] = make_buffer_interpret(lumview, space_axis.cref_y());
                    const auto y_px0 = image.height() + 0.5 * rational_to_float(tm0.top) + 1.5 - grid.px.begin.y();

                    zero_memory(image);

                    for (size_t i = 0; i < grid.count.y(); ++i )
                    {
                        const auto y = grid.value.begin.y() + i * grid.value.repeat.y();
                        const auto y_label_text = make_label_string_view(textbuf, y);
                        const auto y_label_tm = ::font::text_metrics(font, y_label_text);
                        D_ASSERT(y_label_tm);

                        const auto x_px = image.width() - y_label_tm.width;
                        const auto y_px = y_px0 - i * grid.px.repeat.y();
                        
                        ::font::draw_text
                        (
                            image,
                            x_px,
                            y_px,
                            font, 
                            y_label_text
                         );
                    }

                    D_CHECK(gl::update(tex_axis.ref_y(), image));
                }
            }
        }

        void operator () (const shader_embed::luminance_texture& shdr, const pxrectangle& geometry) noexcept
        {
            const auto space_axis = space(geometry);

            shdr.color(::colors::black_f);

            {
                shdr.position(geometry.x0(), geometry.y1() + frame_widths.y());
                shdr.sizes(space_axis.cref_x().sizes());
                shdr.texture(tex_axis.cref_x());
                shdr.draw();
            }

            {
                shdr.position(geometry.p00() - margins.left_top());
                shdr.sizes(space_axis.cref_y().sizes());
                shdr.texture(tex_axis.cref_y());
                shdr.draw();
            }
        }
    };
}



