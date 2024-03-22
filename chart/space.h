#pragma once

#include <ui/manipulator.h>

#include <utility/px.h>

#include <widget/ex_context.h>
#include <widget/stretchable.h>

#include <chart/space_diagonal.h>
#include <chart/shader.h>
#include <chart/grid_shader.h>


namespace chart
{
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
    [[nodiscard]] constexpr bool try_force_update_items_space(space_diagonal_cache& space, const Tuple& items, pxsize2d sizes) noexcept
    {
        return space.try_update(calculate_items_space(items), sizes);
    }

    template<class Tuple>
    [[nodiscard]] constexpr bool try_update_items_space(space_diagonal_cache& space, const Tuple& items, pxsize2d sizes) noexcept
    {
        return space.has_value() || try_force_update_items_space(space, items, sizes);
    }

    template<class Tuple, class... Args>
    constexpr void draw_items_to_cache(const Tuple& items, const Args... args) noexcept
    {
        std::apply([args...] (auto&... items) noexcept { (call_if_exist(items, args...), ...); }, items);
    }

    template<class Tuple, class Shader, class... Args>
    constexpr void draw_items(const Tuple& items, const Shader& shdr, const Args... args) noexcept
    {
        std::apply([&shdr, args...] (const auto&... items) noexcept { (call_if_exist(items, shdr, args...), ...); }, items);
    }

    struct space
    {
        stretchable_pxrectangle geometry{};
        space_diagonal_cache items_space_cache{};
        pxsize2d space_cache{};

        template<class... Items>
        constexpr subitems<std::tuple<Items&...>> operator () (Items&... items) noexcept;

        [[nodiscard]] event_result process(mouse_wheel_event<> e) noexcept;
        [[nodiscard]] event_result process(gesture_event<> e) noexcept;

        constexpr void clear_cache() noexcept
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

        static constexpr auto items_has_background = tuple_has_call_v<Tuple, const shader::background_user&>;
        static constexpr auto items_has_grid = std::conjunction_v<
            items_has_space_type,
            tuple_has_call<Tuple, const shader::grid_user&, space_diagonal_t, transformation_t>
        >;
        static constexpr auto items_has_pix8transformation = std::conjunction_v<
            items_has_space_type,
            tuple_has_call<Tuple, pix8span, transformation_t>
        >;
        static constexpr auto items_has_pix8figure = tuple_has_call_v<Tuple, const shader::pix8_figure_user&>;

        using redraw_event_type = redraw_event_for_tuple_t<
            tuple_push_back_if_t<
            tuple_push_back_if_t<
            tuple_push_back_if_t<
            tuple_push_back_if_t<std::tuple<>,
            items_has_background, shader::background>,
            items_has_grid, shader::grid>,
            items_has_pix8figure, shader::pix8_figure>,
            items_has_pix8transformation, buffer_view>
        >;

        space& space_ref;
        const Tuple items;

        [[nodiscard]]
        constexpr event_result operator () (const ui::size_event&) const noexcept
        {
            return event_result::redraw;
        }

        template<class T>
        auto operator () (const T& e) const noexcept -> decltype(space_ref.process(e))
        {
            return space_ref.process(e);
        }

        [[nodiscard]]
        constexpr event_result operator () (mouse_double_click_event<> e) const noexcept
        {
            if (space_ref.items_space_cache)
            {
                if (const auto space_sizes = stretchable_sizes(space_ref.geometry, e);
                    space_sizes.width() && space_sizes.height() && (space_sizes == space_ref.space_cache))
                {
                    if (try_force_update_items_space(space_ref.items_space_cache, items, space_sizes))
                    {
                        space_ref.space_cache = {};
                        return event_result::redraw;
                    }
                }
            }

            return event_result::idle;
        }

        constexpr void operator () (redraw_event_type e) const noexcept
        {
            const auto space_sizes = stretchable_sizes(space_ref.geometry, e);

            const pxrectangle geometry
            {
                .position{ space_ref.geometry.position },
                .sizes{ space_sizes }
            };

            if constexpr (items_has_background)
            {
                const auto& shdr = e
                    .template get<shader::background>()
                    .use().template as<shader::background_user>()
                    .geometry(geometry);

                draw_items(items, shdr);
            }

            if (try_update_items_space(space_ref.items_space_cache, items, space_sizes)) [[likely]]
            {
                const auto items_space = space_ref.items_space_cache.value();

                const auto value2px = make_transformation
                (
                    items_space,
                    make_pix_space_diagonal(space_sizes)
                );

                if (space_sizes != space_ref.space_cache)
                {
                    space_ref.space_cache = space_sizes;

                    if constexpr (items_has_pix8transformation)
                    {
                        const auto image = px::create_pix8span(e.template get<buffer_view>(), space_sizes);
                        draw_items_to_cache(items, image, value2px);
                    }
                }

                if constexpr (items_has_grid)
                {
                    const auto& shdr = e
                        .template get<shader::grid>()
                        .use()
                        .geometry(geometry);

                    draw_items(items, shdr, items_space, value2px);
                }
            }

                if constexpr (items_has_pix8figure)
                {
                    const auto& shdr = e
                        .template get<shader::pix8_figure>()
                        .use().template as<shader::pix8_figure_user>()
                        .geometry(geometry);

                    draw_items(items, shdr);
                }
        }

        template<class Fn>
        constexpr decltype(auto) apply(Fn&& fn) const noexcept
        {
            return widget::ex_context_v<
                redraw_event_type,
                gesture_event<>
            >(std::forward<Fn>(fn));
        }
    };

    template<class... Items>
    [[nodiscard]] constexpr subitems<std::tuple<Items&...>> space::operator () (Items&... items) noexcept
    {
        return
        {
            .space_ref{ *this },
            .items{ items... }
        };
    }
}



