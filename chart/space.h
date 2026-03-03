#pragma once

#include <widget/ex_context.h>
#include <widget/stretchable.h>

#include <chart/space_diagonal_cache.h>
#include <chart/grid_shader.h>


namespace chart
{
    namespace private_detail_subitems
    {
        template<class... Items>
        struct subitems;
    }

    using private_detail_subitems::subitems;


    struct space
    {
        stretchable_pxrectangle geometry{};
        space_diagonal_cache space_cache{};
        pxrectangle geometry_cache
        {
            .position{},
            .sizes{ no_sizes }
        };
        pxsizes sizes_cache{ no_sizes };


        template<class... Items>
        [[nodiscard]] constexpr subitems<Items...> operator () (Items&... items) noexcept;

        [[nodiscard]] event_result process(const mouse_wheel_event<>& e) noexcept;
        [[nodiscard]] event_result process(const gesture_event<>& e) noexcept;

        constexpr void clear_cache() noexcept
        {
            space_cache.clear();
            sizes_cache = no_sizes;
        }

        template<class E>
        [[nodiscard]] constexpr auto has_space(const E& e) const noexcept -> decltype((e.content().sizes(), true))
        {
            return space_cache && (sizes_cache == e.content().sizes());
        }
    };


    namespace private_detail_subitems
    {
        namespace private_detail_call_items
        {
            template<size_t... Indices, class Tuple, class Value>
            [[nodiscard]] constexpr auto reftuple_push_back(std::index_sequence<Indices...>, Tuple&& tuple, Value&& value) noexcept
            {
                return std::forward_as_tuple(std::move(std::get<Indices>(std::forward<Tuple>(tuple)))..., std::forward<Value>(value));
            }

            template<size_t Index, class... Results, class TupleItems, class... Args>
            [[nodiscard]] constexpr auto call_items_impl(
                [[maybe_unused]] std::tuple<Results&&...>&& result_tuple,
                [[maybe_unused]] TupleItems&& items,
                [[maybe_unused]] Args&&... args
            ) noexcept
            {
                if constexpr (Index < std::tuple_size_v<std::remove_cvref_t<TupleItems>>)
                {
                    auto&& item = std::get<Index>(std::forward<TupleItems>(items));
                    using result_t = decltype(::invoke_if_exist(std::move(item), std::forward<Args>(args)...));

                    constexpr auto call_is_void = std::is_void_v<result_t>;
                    constexpr auto call_is_not_exist = is_invalid_invoke_result_v<result_t>;

                    if constexpr (call_is_void || call_is_not_exist)
                    {
                        if constexpr (call_is_void)
                        {
                            ::invoke_if_exist(std::move(item), std::forward<Args>(args)...);
                        }

                        return call_items_impl<Index + 1u>(
                            std::move(result_tuple),
                            std::forward<TupleItems>(items),
                            std::forward<Args>(args)...
                        );
                    }
                    else
                    {
                        auto&& result = ::invoke_if_exist(std::move(item), std::forward<Args>(args)...);

                        return call_items_impl<Index + 1u>(
                            reftuple_push_back(std::make_index_sequence<sizeof...(Results)>{}, std::move(result_tuple), std::move(result)),
                            std::forward<TupleItems>(items),
                            std::forward<Args>(args)...
                        );
                    }
                }
                else
                {
                    return std::make_from_tuple<std::tuple<std::remove_cvref_t<Results>...>>(std::move(result_tuple));
                }
            }

            template<class Tuple, class... Args>
            constexpr auto call_items(Tuple&& items, Args&&... args) noexcept
            {
                std::tuple<> results{};

                return call_items_impl<0u>(
                    std::move(results),
                    std::forward<Tuple>(items),
                    std::forward<Args>(args)...
                );
            }
        }

        using private_detail_call_items::call_items;

        template<class Tuple, class Value>
        [[nodiscard]] constexpr std::remove_cvref_t<Value> calculate_items_value(const Tuple& items, Value&& value0) noexcept
        {
            std::remove_cvref_t<Value> value{ std::forward<Value>(value0) };
            call_items(items, value);
            return value;
        }

        namespace private_detail_call_items_for_tuple
        {
            template<size_t... IndicesTupleArg0, class Tuple, class TupleArg0, class... Args>
            constexpr void call_items_for_tuple_impl(
                std::index_sequence<IndicesTupleArg0...>,
                [[maybe_unused]] Tuple&& items,
                [[maybe_unused]] TupleArg0&& tuple_arg0,
                [[maybe_unused]] Args&&... args) noexcept
            {
                if constexpr (sizeof...(IndicesTupleArg0))
                {
                    (call_items(std::forward<Tuple>(items), std::get<IndicesTupleArg0>(std::forward<TupleArg0>(tuple_arg0)), std::forward<Args>(args)...), ...);
                }
            }

            template<class Tuple, class TupleArg0, class... Args>
            constexpr void call_items_for_tuple(Tuple&& items, TupleArg0&& tuple_arg0, Args&&... args) noexcept
            {
                call_items_for_tuple_impl(
                    std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<TupleArg0>>>{},
                    std::forward<Tuple>(items),
                    std::forward<TupleArg0>(tuple_arg0),
                    std::forward<Args>(args)...
                );
            }
        }

        using private_detail_call_items_for_tuple::call_items_for_tuple;


        namespace private_detail_update_space
        {
            template<class Tuple>
            [[nodiscard]] constexpr bool try_first_update_space(space_diagonal_cache& space, const Tuple& items, pxsizes sizes) noexcept
            {
                return space.try_first_update(calculate_items_value(items, space_diagonal_initializer), sizes);
            }

            template<class Tuple>
            [[nodiscard]] constexpr bool try_update_space(space_diagonal_cache& space, const Tuple& items, pxsizes sizes) noexcept
            {
                return space.has_value()
                    || try_first_update_space(space, items, sizes);
            }
        }

        using private_detail_update_space::try_update_space;


        template<class Resource, bool Has>
        struct item_context
        {
            using resource_type = Resource;
            static constexpr bool has = Has;
            using resource_pack = std::conditional_t<has, ttypes<resource_type>, ttypes<>>;

            template<class Tuple, std::enable_if_t<ttypes_size_v<Tuple>&& has, int> = 0>
            [[nodiscard]] static constexpr auto get(const Tuple& e_context) noexcept
                -> decltype(e_context.template get<resource_type>())
            {
                return e_context.template get<resource_type>();
            }
        };

        template<class... Args>
        using items_redraw_event_t = ttypes_repack_t<types_cat_t<typename Args::resource_pack...>, redraw_event>;


        template<class... Items>
        struct subitems
        {
            using items_ref_tuple = std::tuple<Items&...>;

            template<class... Args>
            using items_has_call = ttypes_has_invoke<items_ref_tuple, Args...>;

            template<class... Args>
            using items_has_cref_call = items_has_call<const Args&...>;

            using items_has_space = items_has_call<space_diagonal&>;
            using items_has_limpix_space = items_has_cref_call<lumpixspan, space_manipulation>;
            using items_has_any_limpix = items_has_call<any_overload, temp_byte_buffer&, any_overload>;

            static constexpr bool items_has_background_value = items_has_cref_call<shader_embed::colored_rectangle>::value;

            static constexpr bool items_has_lumtex_value = std::disjunction_v<
                items_has_call<const shader_embed::luminance_texture&>,
                items_has_call<const shader_embed::luminance_texture&, any_overload>
            >;

            static constexpr bool items_has_space_buffer_view_value = std::conjunction_v<
                items_has_space,
                std::disjunction<items_has_any_limpix, items_has_limpix_space>
            >;

            static constexpr bool items_has_grid_value = std::conjunction_v<
                items_has_space,
                items_has_call<any_overload, const shader::grid&>
            >;

            using background_context = item_context<const shader_embed::colored_rectangle, items_has_background_value >;
            using luminance_figure_context = item_context<const shader_embed::luminance_texture, items_has_lumtex_value >;
            using grid_context = item_context<const shader::grid, items_has_grid_value >;
            using buffer_context = item_context<temp_byte_buffer, items_has_space_buffer_view_value >;

            using redraw_event_type = items_redraw_event_t<
                background_context,
                luminance_figure_context,
                grid_context,
                buffer_context
            >;

            space& space_ref;
            const items_ref_tuple items;

            [[nodiscard]]
            constexpr event_result operator () (const ui::size_event&) const noexcept
            {
                return event_result::redraw;
            }

            [[nodiscard]]
            constexpr event_result operator () (const ui::mouse_double_click_event&) const noexcept
            {
                space_ref.clear_cache();
                return event_result::redraw;
            }

            template<class T>
            [[nodiscard]] auto operator () (const T& e) const noexcept -> decltype(space_ref.process(e))
            {
                return space_ref.process(e);
            }

            constexpr void operator () (redraw_event_type e) const noexcept
            {
                const auto window_sizes = e.content().sizes();
                const auto has_new_sizes = window_sizes != space_ref.sizes_cache;

                if (has_new_sizes)
                {
                    space_ref.sizes_cache = window_sizes;
                    space_ref.geometry_cache = calculate_items_value(items,
                        widget::stretchable_geometry(space_ref.geometry, window_sizes)
                    );
                }

                if constexpr (background_context::has)
                {
                    const auto& shdr = background_context::get(e);
                    shdr.use();
                    shdr.geometry(space_ref.geometry_cache);

                    call_items(items, shdr);
                }

                if constexpr (items_has_space::value)
                {
                    if (try_update_space(space_ref.space_cache, items, space_ref.geometry_cache.sizes)) [[likely]]
                    {
                        const space_manipulation sys
                        {
                            space_ref.space_cache.value(),
                            make_space_diagonal(space_ref.geometry_cache.sizes)
                        };

                        [[maybe_unused]]
                        const auto temp_items = call_items(items, sys);

                        if constexpr (buffer_context::has)
                        {
                            if (has_new_sizes)
                            {
                                auto& temp_buffer = buffer_context::get(e);

                                if constexpr (items_has_limpix_space::value)
                                {
                                    const auto image = temp_buffer.image(space_ref.geometry_cache.sizes);

                                    call_items(items, image, sys);
                                }

                                if constexpr (items_has_any_limpix::value)
                                {
                                    call_items_for_tuple(items, temp_items, temp_buffer, space_ref.geometry_cache);
                                }
                            }
                        }

                        if constexpr (grid_context::has)
                        {
                            const auto& shdr = grid_context::get(e);
                            shdr.use();
                            shdr.geometry(space_ref.geometry_cache);

                            call_items_for_tuple(items, temp_items, shdr);
                        }
                    }
                }

                if constexpr (luminance_figure_context::has)
                {
                    const auto& shdr = luminance_figure_context::get(e);
                    shdr.use();

                    call_items(items, shdr, space_ref.geometry_cache);

                    shdr.geometry(space_ref.geometry_cache);
                    call_items(items, shdr);
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
    }

    template<class... Items>
    [[nodiscard]] constexpr subitems<Items...> space::operator () (Items&... items) noexcept
    {
        return
        {
            .space_ref{ *this },
            .items{ items... }
        };
    }
}



