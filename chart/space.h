#pragma once

#include <utility/px.h>

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
        space_diagonal_cache items_space_cache{};
        pxsizes pixspace_sizes_cache{ px::no_sizes };

        template<class... Items>
        [[nodiscard]] constexpr subitems<Items...> operator () (Items&... items) noexcept;

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
        constexpr bool pixspace_is_updated(pxsizes pixspace_sizes_now) const noexcept
        {
            return pixspace_sizes_now.has_positive_square()
                && pixspace_sizes_cache == pixspace_sizes_now;
        }
    };


    namespace private_detail_subitems
    {
        namespace private_detail_call_items
        {
            template<size_t... Indices, class... Types, class Value>
            [[nodiscard]] constexpr auto reftuple_push_back(std::index_sequence<Indices...>, std::tuple<Types&&...>&& tuple, Value&& value) noexcept
            {
                return std::forward_as_tuple(std::forward<Types>(std::get<Indices>(std::move(tuple)))..., std::forward<Value>(value));
            }

            template<size_t Index, class... Results, class TupleItems, class... Args>
            [[nodiscard]] constexpr auto call_items_impl(
                [[maybe_unused]] std::tuple<Results&&...>&& results,
                [[maybe_unused]] TupleItems&& items,
                [[maybe_unused]] Args&&... args
            ) noexcept
            {
                if constexpr (Index < std::tuple_size_v<std::remove_cvref_t<TupleItems>>)
                {
                    using result_t = std::remove_cvref_t<decltype(call_if_exist(std::get<Index>(std::forward<TupleItems>(items)), std::forward<Args>(args)...))>;

                    constexpr auto call_is_void = std::is_void_v<result_t>;
                    constexpr auto call_is_not_exist = std::is_same_v<result_t, no_overload>;

                    if constexpr (call_is_void || call_is_not_exist)
                    {
                        if constexpr (call_is_void)
                        {
                            call_if_exist(
                                std::get<Index>(std::forward<TupleItems>(items)),
                                std::forward<Args>(args)...
                            );
                        }

                        return call_items_impl<Index + 1u>(
                            std::move(results),
                            std::forward<TupleItems>(items),
                            std::forward<Args>(args)...
                        );
                    }
                    else
                    {
                        auto&& result = call_if_exist(std::get<Index>(std::forward<TupleItems>(items)), std::forward<Args>(args)...);

                        return call_items_impl<Index + 1u>(
                            reftuple_push_back(std::make_index_sequence<sizeof...(Results)>{}, std::move(results), std::move(result)),
                            std::forward<TupleItems>(items),
                            std::forward<Args>(args)...
                        );
                    }
                }
                else
                {
                    return std::make_from_tuple<std::tuple<std::remove_cvref_t<Results>...>>(std::move(results));
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


        namespace private_detail_call_items_for_tuple
        {
            template<size_t... TupleIndicesArg0, class Tuple, class TupleArg0, class... Args>
            constexpr void call_items_for_tuple_impl(
                std::index_sequence<TupleIndicesArg0...>,
                [[maybe_unused]] Tuple&& items,
                [[maybe_unused]] TupleArg0&& tuple_arg0,
                [[maybe_unused]] Args&&... args) noexcept
            {
                if constexpr (sizeof...(TupleIndicesArg0))
                {
                    (call_items(std::forward<Tuple>(items), std::get<TupleIndicesArg0>(std::forward<TupleArg0>(tuple_arg0)), std::forward<Args>(args)...), ...);
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
            [[nodiscard]] constexpr space_diagonal calculate_space(const Tuple& items) noexcept
            {
                space_diagonal diagonal{ space_diagonal_initializer };
                call_items(items, diagonal);
                return diagonal;
            }

            template<class Tuple>
            [[nodiscard]] constexpr bool try_first_update_space(space_diagonal_cache& space, const Tuple& items, pxsizes sizes) noexcept
            {
                return space.try_first_update(calculate_space(items), sizes);
            }

            template<class Tuple>
            [[nodiscard]] constexpr bool try_update_space(space_diagonal_cache& space, const Tuple& items, pxsizes sizes) noexcept
            {
                return space.has_value() || try_first_update_space(space, items, sizes);
            }
        }

        using private_detail_update_space::try_update_space;


        template<class Resource, bool Has>
        struct item_context
        {
            using resource_type = Resource;
            static constexpr bool has = Has;
            using resource_pack = std::conditional_t<has, types_pack<resource_type>, types_pack<>>;

            template<class Tuple>
            [[nodiscard]] static constexpr auto get(const Tuple& e_context) noexcept -> decltype(e_context.template get<resource_type>())
            {
                return e_context.template get<resource_type>();
            }
        };

        template<class... Args>
        using items_redraw_event_t = repack_types_t<types_cat_t<typename Args::resource_pack...>, redraw_event>;


        template<class... Items>
        struct subitems
        {
            using items_ref_tuple = std::tuple<Items&...>;

            template<class... Args>
            using items_has_call = types_has_call<items_ref_tuple, Args...>;

            template<class... Args>
            using items_has_cref_call = items_has_call<const Args&...>;

            using items_has_space = items_has_call<space_diagonal&>;
            using items_has_limpix_space = items_has_cref_call<lumpixspan, space_manipulation>;
            using items_has_buffer = items_has_cref_call<any_overload, buffer_view>;

            using items_has_lumtex_space = std::conjunction<
                items_has_space,
                items_has_cref_call<any_overload, shader_embed::luminance_texture>
            >;

            static constexpr bool items_has_background_value = items_has_cref_call<shader_embed::colored_rectangle>::value;

            static constexpr bool items_has_lumtex_value = std::disjunction_v<
                items_has_cref_call<shader_embed::luminance_texture>,
                items_has_lumtex_space
            >;

            static constexpr bool items_has_space_buffer_view_value = std::conjunction_v<
                items_has_space,
                std::disjunction<items_has_buffer, items_has_limpix_space>
            >;

            static constexpr bool items_has_grid_value = std::conjunction_v<
                items_has_space,
                items_has_cref_call<any_overload, shader::grid>
            >;

            using background_context = item_context<shader_embed::colored_rectangle, items_has_background_value >;
            using luminance_figure_context = item_context<shader_embed::luminance_texture, items_has_lumtex_value >;
            using grid_context = item_context<shader::grid, items_has_grid_value >;
            using buffer_context = item_context<buffer_view, items_has_space_buffer_view_value >;

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

            template<class T>
            [[nodiscard]] auto operator () (const T& e) const noexcept -> decltype(space_ref.process(e))
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

                    if constexpr (background_context::has)
                    {
                        const auto& shdr = background_context::get(e);
                        shdr.use();
                        shdr.geometry(geometry);

                        call_items(items, shdr);
                    }

                    if constexpr (items_has_space::value)
                    {
                        if (try_update_space(space_ref.items_space_cache, items, space_sizes)) [[likely]]
                        {
                            const space_manipulation sys
                            {
                                space_ref.items_space_cache.value(),
                                make_pxspace_diagonal(space_sizes)
                            };

                            [[maybe_unused]]
                            const auto temp_items = call_items(items, sys);

                            if (space_sizes != space_ref.pixspace_sizes_cache)
                            {
                                space_ref.pixspace_sizes_cache = space_sizes;

                                if constexpr (buffer_context::has)
                                {
                                    const auto buffer = buffer_context::get(e);

                                    if constexpr (items_has_limpix_space::value)
                                    {
                                        const auto image = px::create_lumpixspan(buffer, space_sizes);

                                        call_items(items, image, sys);
                                    }

                                    if constexpr (items_has_buffer::value)
                                    {
                                        call_items_for_tuple(items, temp_items, buffer);
                                    }
                                }
                            }

                            if constexpr (grid_context::has)
                            {
                                const auto& shdr = grid_context::get(e);
                                shdr.use();
                                shdr.geometry(geometry);

                                call_items_for_tuple(items, temp_items, shdr);
                            }

                            if constexpr (luminance_figure_context::has)
                            {
                                const auto& shdr = luminance_figure_context::get(e);
                                shdr.use();

                                call_items_for_tuple(items, temp_items, shdr);
                            }
                        }
                    }

                    if constexpr (luminance_figure_context::has)
                    {
                        const auto& shdr = luminance_figure_context::get(e);
                        shdr.use();
                        shdr.geometry(geometry);

                        call_items(items, shdr);
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



