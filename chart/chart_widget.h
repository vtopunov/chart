#pragma once

#include <utility/px.h>

#include <widget/stretchable.h>

#include <chart/event.h>
#include <chart/chart_line.h>
#include <chart/space_diagonal.h>


namespace chart
{
    using widget::stretchable_pxrectangle;

    template<class Tuple>
    struct chart_items_widget;


    template<class... Lines>
    [[nodiscard]] constexpr bool try_force_update_lines_space(space_diagonal_cache& cache, pxsize2d pxsizes, const Lines&... lines) noexcept
    {
        space_diagonal_t diagonal{ space_diagonal_initializer };
        (call_if_exist(lines, diagonal), ...);
        return cache.try_update(diagonal, pxsizes);
    }

    template<class... Lines>
    [[nodiscard]] constexpr bool try_update_lines_space(space_diagonal_cache& cache, pxsize2d pxsizes, const Lines&... lines) noexcept
    {
        if (cache.has_value())
        {
            return true;
        }

        return try_force_update_lines_space(cache, pxsizes, lines...);
    }


    struct chart_widget
    {
        static constexpr auto background_color = gl::colors::white_f;

        stretchable_pxrectangle geometry{};
        space_diagonal_cache lines_space_cache{};
        pxsize2d chart_space_cache{};

        template<class... Items>
        chart_items_widget<std::tuple<Items&...>> operator () (Items&... items) noexcept;

        event_result process(mouse_wheel_event_t e) noexcept;
        event_result process(mouse_move_event_t e) noexcept;

        template<class... Args>
        event_result process_with_items(mouse_double_click_event_t e, const Args&... args) noexcept
        {
            if (lines_space_cache)
            {
                if (const auto chart_sizes = stretchable_sizes(geometry, e);
                    chart_sizes.width() && chart_sizes.height() && (chart_sizes == chart_space_cache))
                {
                    if (try_force_update_lines_space(lines_space_cache, chart_sizes, args...))
                    {
                        chart_space_cache = {};
                        return event_result::redraw;
                    }
                }
            }

            return event_result::idle;
        }

        template<class... Args>
        void process_with_items(redraw_event_t e, Args&... args) noexcept
        {
            const auto chart_sizes = stretchable_sizes(geometry, e);

            const pxrectangle view_geometry
            {
                .position{ geometry.position },
                .sizes{ chart_sizes }
            };

            e.get<shader::colored_rectangle>()
                .use()
                .store(view_geometry)
                .store(background_color)
                .draw();

            if (chart_sizes != chart_space_cache)
            {
                chart_space_cache = chart_sizes;

                if (try_update_lines_space(lines_space_cache, chart_sizes, args...))
                {
                    const auto image = px::create_pix8span(e.get<buffer_view>(), chart_sizes);
                    const auto transformation = make_transformation
                    (
                        lines_space_cache.value(),
                        make_pix_space_diagonal(chart_sizes)
                    );

                    (call_if_exist(args, image, transformation), ...);
                }
            }

            {
                const auto& shdr = e.get<shader::gray_texture_mix_color>()
                    .use()
                    .store(view_geometry);

                (call_if_exist(args, shdr), ...);
            }
        }

        void clear_cache() noexcept
        {
            lines_space_cache.clear();
            chart_space_cache = {};
        }
    };

    template<class Tuple>
    struct chart_items_widget
    {
        chart_widget& widget;
        Tuple items;

        constexpr event_result operator () (const ui::size_event&) const noexcept
        {
            return event_result::redraw;
        }

        template<class T>
        auto operator () (const T& e) const noexcept -> decltype(widget.process(e))
        {
            return widget.process(e);
        }

        template<class T>
        auto operator () (const T& e) const noexcept -> decltype(widget.process_with_items(e))
        {
            return process_with_items(e, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
        }

        template<class T, size_t... I>
        auto process_with_items(const T& e, std::index_sequence<I...>) const noexcept
        {
            return widget.process_with_items(e, std::get<I>(items)...);
        }

        template<class Fn>
        decltype(auto) apply(Fn fn) noexcept
        {
            return fn
            (
                widget::ex_context_v<redraw_event_t>,
                widget::ex_context_v<mouse_move_event_t>
            );
        }
    };

    template<class... Items>
    chart_items_widget<std::tuple<Items&...>> chart_widget::operator () (Items&... items) noexcept
    {
        return
        {
            .widget{ *this },
            .items{ items... }
        };
    }
}

using chart::chart_line;
using chart::chart_widget;



