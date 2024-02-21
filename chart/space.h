#pragma once

#include <ui/manipulator.h>

#include <utility/px.h>

#include <widget/ex_context.h>
#include <widget/stretchable.h>

#include <chart/event.h>
#include <chart/polyline.h>
#include <chart/space_diagonal.h>


namespace chart
{
    using widget::stretchable_pxrectangle;

    template<class Tuple>
    struct subitems;

    template<class Tuple>
    [[nodiscard]] constexpr space_diagonal_t calculate_items_space(const Tuple& items) noexcept
    {
        space_diagonal_t diagonal{ space_diagonal_initializer };
        std::apply([&diagonal] (const auto&... items) noexcept { (call_if_exist(items, diagonal), ...); }, items);
        return diagonal;
    }

    template<class Tuple>
    void draw_items_to_cache(const pix8span pixs, const transformation_t value2px, const Tuple& items) noexcept
    {
        std::apply([pixs, value2px] (auto&... items) noexcept { (call_if_exist(items, pixs, value2px), ...); }, items);
    }

    template<class Shader, class Tuple>
    void draw_items(const Shader& shdr, const Tuple& items) noexcept
    {
        std::apply([&shdr] (const auto&... items) noexcept { (call_if_exist(items, shdr), ...); }, items);
    }

    struct space
    {
        stretchable_pxrectangle geometry{};
        space_diagonal_cache items_space_cache{};
        pxsize2d space_cache{};

        template<class... Items>
        subitems<std::tuple<Items&...>> operator () (Items&... items) noexcept;

        event_result process(mouse_wheel_event e) noexcept;
        event_result process(gesture_event e) noexcept;

        void clear_cache() noexcept
        {
            items_space_cache.clear();
            space_cache = {};
        }
    };

    template<class Tuple>
    struct redraw_event_for_tuple;

    template<class... Types>
    struct redraw_event_for_tuple<std::tuple<Types...>>
    {
        using type = redraw_event<Types...>;
    };

    template<class Tuple>
    using redraw_event_for_tuple_t = typename redraw_event_for_tuple<Tuple>::type;

    template<class Tuple>
    struct subitems
    {
        using items_has_space_type = tuple_has_call<Tuple, space_diagonal_t&>;

        static constexpr auto items_has_background = tuple_has_call_v<Tuple, const shader::background_shader_user&>;
        static constexpr auto items_has_space = items_has_space_type::value;
        static constexpr auto items_has_pix8transformation = std::conjunction_v<items_has_space_type, tuple_has_call<Tuple, pix8span, transformation_t>>;
        static constexpr auto items_has_pix8figure = tuple_has_call_v<Tuple, const shader::pix8_figure_shader_user>;
        
        using draw_context_tuple_tuple = tuple_push_back_if_t<tuple_push_back_if_t<tuple_push_back_if_t<std::tuple<>,
            items_has_background, shader::background_shader>,
            items_has_pix8figure, shader::pix8_figure_shader>,
            items_has_pix8transformation, buffer_view
        > ;
        using redraw_event = redraw_event_for_tuple_t<draw_context_tuple_tuple>;

        space& space_ref;
        const Tuple items;

        [[nodiscard]] constexpr bool try_update_items_space(pxsize2d space_sizes) const noexcept
        {
            if (space_ref.items_space_cache.has_value())
            {
                return true;
            }

            return try_foce_update_items_space(space_sizes);
        }

        [[nodiscard]] constexpr bool try_foce_update_items_space(pxsize2d space_sizes) const noexcept
        {
            return space_ref.items_space_cache.try_update(calculate_items_space(items), space_sizes);
        }

        constexpr event_result operator () (const ui::size_event&) const noexcept
        {
            return event_result::redraw;
        }

        template<class T>
        auto operator () (const T& e) const noexcept -> decltype(space_ref.process(e))
        {
            return space_ref.process(e);
        }

        constexpr event_result operator () (mouse_double_click_event e) const noexcept
        {
            if (space_ref.items_space_cache)
            {
                if (const auto space_sizes = stretchable_sizes(space_ref.geometry, e);
                    space_sizes.width() && space_sizes.height() && (space_sizes == space_ref.space_cache))
                {
                    if (try_foce_update_items_space(space_sizes))
                    {
                        space_ref.space_cache = {};
                        return event_result::redraw;
                    }
                }
            }

            return event_result::idle;
        }

        void operator () (redraw_event e) const noexcept
        {
            const auto space_sizes = stretchable_sizes(space_ref.geometry, e);

            const pxrectangle current_geometry
            {
                .position{ space_ref.geometry.position },
                .sizes{ space_sizes }
            };

            if (space_sizes != space_ref.space_cache)
            {
                space_ref.space_cache = space_sizes;

                if constexpr (items_has_pix8transformation)
                {
                    if (try_update_items_space(space_sizes))
                    {
                        const auto image = px::create_pix8span(e.get<buffer_view>(), space_sizes);
                        const auto value2px = make_transformation
                        (
                            space_ref.items_space_cache.value(),
                            make_pix_space_diagonal(space_sizes)
                        );
                        draw_items_to_cache(image, value2px, items);
                    }
                }
            }

            if constexpr (items_has_background)
            {
                const auto& shdr = e.get<shader::background_shader>()
                    .use().as<shader::background_shader_user>()
                    .store(current_geometry);

                draw_items(shdr, items);
            }

            if constexpr (items_has_pix8figure)
            {
                const auto& shdr = e
                    .get<shader::pix8_figure_shader>()
                    .use().as<shader::pix8_figure_shader_user>()
                    .store(current_geometry);

                draw_items(shdr, items);
            }
        }

        template<class Fn>
        constexpr decltype(auto) apply(Fn&& fn) const noexcept
        {
            return widget::ex_context_v<
                redraw_event,
                gesture_event
            >(std::forward<Fn>(fn));
        }
    };

    template<class... Items>
    subitems<std::tuple<Items&...>> space::operator () (Items&... items) noexcept
    {
        return
        {
            .space_ref{ *this },
            .items{ items... }
        };
    }
}



