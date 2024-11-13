#pragma once

#include <utility/px.h>

#include <widget/ex_context.h>
#include <widget/stretchable.h>

#include <chart/space_diagonal_cache.h>
#include <chart/shader.h>
#include <chart/grid.h>


namespace chart
{
    namespace private_detail_call_items
    {
        template<size_t... Indices, class Tuple, class... Args>
        constexpr void void_ccall_items_impl(std::index_sequence<Indices...>, const Tuple& tuple_items, [[maybe_unused]] const Args&... args) noexcept
        {
            (std::invoke(std::get<Indices>(tuple_items), args...), ...);
        }

        template<size_t... Indices, class Tuple, class... Args>
        [[nodiscard]] constexpr auto ccall_items_impl(std::index_sequence<Indices...>, const Tuple& tuple_items, [[maybe_unused]] const Args&... args) noexcept
        {
            if constexpr (sizeof...(Indices))
            {
                return std::make_tuple(std::invoke(std::get<Indices>(tuple_items), args...)...);
            }
            else
            {
                return;
            }
        }

        template<class... Args>
        struct ccall_items_without_result_pred
        {
            template<class Fn>
            struct type : call_without_result_is_detected<const Fn&, const Args&...>
            {};
        };

        template<class... Args>
        struct ccall_items_with_result_pred
        {
            template<class Fn>
            struct type : call_with_result_is_detected<const Fn&, const Args&...>
            {};
        };

        template<class Tuple, class... Args>
        [[nodiscard]] constexpr auto ccall_items(const Tuple& tuple_items, const Args&... args) noexcept
        {
            using seq_t = std::make_index_sequence<std::tuple_size_v<Tuple>>;

            using void_seq_t = types_sequence_if_t<
                typename ccall_items_without_result_pred<Args...>::type,
                Tuple, seq_t
            >;

            using result_seq_t = types_sequence_if_t<
                typename ccall_items_with_result_pred<Args...>::type,
                Tuple, seq_t
            >;

            void_ccall_items_impl(void_seq_t{}, tuple_items, args...);
            return ccall_items_impl(result_seq_t{}, tuple_items, args...);
        }
    }

    using private_detail_call_items::ccall_items;


    template<class Tuple>
    [[nodiscard]] constexpr space_diagonal calculate_items_space(const Tuple& items) noexcept
    {
        space_diagonal diagonal{ space_diagonal_initializer };
        std::apply([&diagonal] (const auto&... items) noexcept { (call_if_exist(items, diagonal), ...); }, items);
        return diagonal;
    }

    template<class Tuple>
    [[nodiscard]] constexpr bool try_first_update_items_space(space_diagonal_cache& space, const Tuple& items, pxsize2d sizes) noexcept
    {
        return space.try_first_update(calculate_items_space(items), sizes);
    }

    template<class Tuple>
    [[nodiscard]] constexpr bool try_update_items_space(space_diagonal_cache& space, const Tuple& items, pxsize2d sizes) noexcept
    {
        return space.has_value() || try_first_update_items_space(space, items, sizes);
    }

    template<class Tuple, class... Args>
    constexpr void draw_items_to_cache(Tuple& items, const Args&... args) noexcept
    {
        std::apply([&args...] (auto&... items) noexcept { (call_if_exist(items, args...), ...); }, items);
    }

    template<class Tuple, class Shader, class Layouts>
    constexpr void draw_items_for_layouts(const Tuple& items, const Shader& shdr, const Layouts& tuple_layouts) noexcept
    {
        std::apply([&items, &shdr] (const auto&... layouts)  noexcept
        {
            (ccall_items(items, shdr, layouts), ...);
        }, tuple_layouts);
    }

    struct space
    {
        stretchable_pxrectangle geometry{};
        space_diagonal_cache items_space_cache{};
        pxsize2d pixspace_sizes_cache{ px::no_sizes };

        template<class... Items>
        constexpr subitems<std::tuple<Items&...>> operator () (Items&... items) noexcept;

        [[nodiscard]] event_result process(basic_mouse_double_click_event<>) noexcept;
        [[nodiscard]] event_result process(mouse_wheel_event<> e) noexcept;
        [[nodiscard]] event_result process(gesture_event<> e) noexcept;

        constexpr void clear_pixspace_cache() noexcept
        {
            pixspace_sizes_cache = px::no_sizes;
        }

        constexpr void clear_cache() noexcept
        {
            items_space_cache.clear();
            clear_pixspace_cache();
        }

        [[nodiscard]]
        constexpr bool pixspace_is_updated(pxsize2d pixspace_sizes_now) const noexcept
        {
            return pixspace_sizes_now.has_positive_square()
                && pixspace_sizes_cache == pixspace_sizes_now;
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
        using items_has_space_type = types_has_call<Tuple, space_diagonal&>;

        static constexpr auto items_has_background = types_has_call_v<Tuple, const shader::background_user&>;
        static constexpr auto items_has_grid = std::conjunction_v<
            items_has_space_type,
            types_has_call<Tuple, const shader::grid_user&, periodic_value_position>
        >;
        static constexpr auto items_has_lumpix_values = std::conjunction_v<
            items_has_space_type,
            types_has_call<Tuple, lumpixspan, space_manipulation>
        >;
        static constexpr auto items_has_luminance_figure = types_has_call_v<Tuple, const shader::luminance_figure_user&>;

        using redraw_event_type = redraw_event_for_tuple_t<
            types_push_back_if_t<
            types_push_back_if_t<
            types_push_back_if_t<
            types_push_back_if_t<std::tuple<>,
            items_has_background, shader::background>,
            items_has_grid, shader::grid>,
            items_has_luminance_figure, shader::luminance_figure>,
            items_has_lumpix_values, buffer_view>
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

        constexpr void operator () (redraw_event_type e) const noexcept
        {
            if (const auto space_sizes = stretchable_sizes(space_ref.geometry, e); space_sizes.has_positive_square()) [[likely]]
            {
                const pxrectangle geometry
                {
                    .position{ space_ref.geometry.position },
                    .sizes{ space_sizes }
                };

                if constexpr (items_has_background)
                {
                    const auto& shdr = e
                        .template get<shader::background>()
                        .use()
                        .geometry(geometry);

                    ccall_items(items, shdr);
                }

                {
                    if (try_update_items_space(space_ref.items_space_cache, items, space_sizes)) [[likely]]
                    {
                        const space_manipulation sys
                        {
                            space_ref.items_space_cache.value(),
                            make_pxspace_diagonal(space_sizes)
                        };

                        [[maybe_unused]]
                        const auto layouts = ccall_items(items, sys);

                        if (space_sizes != space_ref.pixspace_sizes_cache)
                        {
                            space_ref.pixspace_sizes_cache = space_sizes;

                            if constexpr (items_has_lumpix_values)
                            {
                                const auto image = px::create_lumpixspan(e.template get<buffer_view>(), space_sizes);
                                draw_items_to_cache(items, image, sys);
                            }
                        }

                        if constexpr (items_has_grid)
                        {
                            const auto& shdr = e
                                .template get<shader::grid>()
                                .use()
                                .geometry(geometry);

                            draw_items_for_layouts(items, shdr, layouts);
                        }
                    }
                }

                if constexpr (items_has_luminance_figure)
                {
                    const auto& shdr = e
                        .template get<shader::luminance_figure>()
                        .use()
                        .geometry(geometry);

                    ccall_items(items, shdr);
                }
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



