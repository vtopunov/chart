#pragma once

#include <tuple>

#include <core/types_algorithm.h>

#include <shader/export.h>


namespace shader_common
{
    template<class Lib>
    using decl_exports_pack_t = typename Lib::exports_pack_type;

    template<class T>
    using decl_exports_t = typename T::exports;

    template<class Export, class Lib>
    using decl_interface_t = typename Export::template interface<Lib>;


    template<class T>
    using is_uniform = std::is_base_of<shader_export::uniform_base, T>;

    template<class T>
    constexpr bool is_uniform_v = is_uniform<T>::value;

    static_assert(is_uniform_v<shader_export::sampler>);

    template<class T>
    using is_sampler = std::is_base_of<shader_export::sampler, T>;

    template<class T>
    using has_indexed_location = std::conjunction<std::negation<is_sampler<T>>, is_uniform<T>>;

    template<class T>
    using is_not_uniform = std::negation<is_uniform<T>>;

    template<class T>
    constexpr bool is_sampler_v = is_sampler<T>::value;

    template<class T>
    constexpr bool has_indexed_location_v = has_indexed_location<T>::value;

    template<class T>
    constexpr bool is_not_uniform_v = is_not_uniform<T>::value;


    template<class Exports>
    using export_pack_variables_t = types_sol_t<ttypes_transform_t<subtypes_t, Exports> >;

    template<template <class> class Pred, class Exports>
    constexpr size_t count_of_for_v = ttypes_count_if_v<Pred, export_pack_variables_t<Exports>>;

    template<class Exports>
    constexpr size_t count_of_locations_v = count_of_for_v<shader_common::has_indexed_location, Exports>;


    namespace private_detail_shader_common
    {
        namespace private_detail_for_each_export
        {
            constexpr struct forward_as_tuple_function
            {
                template<class... Args>
                constexpr auto operator () (Args&&... args) const noexcept
                {
                    return std::forward_as_tuple(std::forward<Args>(args)...);
                }
            } forward_as_tuple_function_v;

            template<class ExportsPack, size_t... Indices>
            [[nodiscard]] constexpr auto make_exports_tuple(std::index_sequence<Indices...>) noexcept
            {
                return std::tuple_cat(ttypes_element_t<Indices, ExportsPack>::apply(forward_as_tuple_function_v)...);
            }

            template<class ExportsPack>
            [[nodiscard]] constexpr auto make_exports_tuple() noexcept
            {
                return make_exports_tuple<ExportsPack>(std::make_index_sequence<ttypes_size_v<ExportsPack>>{});
            }

            template<class ExportsPack>
            constexpr auto exports_tuple_v = make_exports_tuple<ExportsPack>();

            template<class Tuple, class Fn, size_t... Indices>
            constexpr void tuple_for_each(Tuple&& tuple, Fn&& fn, std::index_sequence<Indices...>) noexcept
            {
                (fn(std::get<Indices>(std::forward<Tuple>(tuple))), ...);
            }

            template<class Tuple, class Fn, size_t... Indices>
            constexpr void tuple_for_each(Tuple&& tuple, Fn&& fn) noexcept
            {
                tuple_for_each
                (
                    std::forward<Tuple>(tuple), 
                    std::forward<Fn>(fn), 
                    std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple> > >{}
                );
            }

            template<class ExportsPack, class Fn>
            constexpr void for_each_export(Fn&& fn) noexcept
            {
                tuple_for_each
                (
                    exports_tuple_v<ExportsPack>, 
                    std::forward<Fn>(fn)
                );
            }
        }
    }

    using private_detail_shader_common::private_detail_for_each_export::for_each_export;


    namespace private_detail_shader_common
    {
        namespace private_detail_interface_conversion
        {
            struct empty_base_sample_type
            {
                std::byte dummy;
            };

            template<class Interface>
            struct is_empty_base_helper
            {
                struct empty_base_test_type : Interface
                {
                    std::byte dummy;
                };

                static constexpr bool value
                    = (sizeof(empty_base_test_type) == sizeof(empty_base_sample_type))
                    && (alignof(empty_base_test_type) == alignof(empty_base_sample_type));
            };

            template<class T>
            using is_empty_base = std::bool_constant<is_empty_base_helper<T>::value>;

            template<class T>
            constexpr bool is_empty_base_v = is_empty_base<T>::value;

            template<class Lib, class Interface>
            [[nodiscard]] constexpr const Lib& interface_to_library(const Interface& lib_interface) noexcept
            {
                static_assert(is_unqualified_class_v<Lib>);
                static_assert(is_unqualified_class_v<Interface>);
                static_assert(is_empty_base_v<Interface>);
                static_assert(!is_empty_base_v<Lib>);
                static_assert(std::is_base_of_v<Interface, Lib>);
                static_assert(sizeof(Lib) >= sizeof(Interface));
                static_assert(1_uz == ttypes_size_v<Interface>);
                static_assert(std::is_same_v<Lib, ttypes_element_t<0_uz, Interface> >);
                return static_cast<const Lib&>(lib_interface);
            }
        }
    }

    using private_detail_shader_common::private_detail_interface_conversion::interface_to_library;


    namespace private_detail_shader_common
    {
        namespace private_detail_index_location
        {
            template<template <class> class Pred, class ExportsPack>
            constexpr auto uniforms_v = [] () noexcept
            {
                std::array<const shader_export::uniform_base*, count_of_for_v<Pred, ExportsPack> > result{};
                for_each_export<ExportsPack>([data = result.data()] <class T> (const T & u) mutable noexcept
                {
                    if constexpr (Pred<T>::value)
                    {
                        *data = std::addressof(static_cast<const shader_export::uniform_base&>(u));
                        ++data;
                    }
                });

                return result;
            } ();

            template<template <class> class Pred, class Lib>
            [[nodiscard]] constexpr size_t index_location_for(const shader_export::uniform_base& u) noexcept
            {
                static_assert(is_unqualified_class_v<Lib>);
                using exports_pack_t = decl_exports_pack_t<Lib>;
                constexpr auto uniform_span_v = make_span(uniforms_v<Pred, exports_pack_t>);

                const auto index = find_n(uniform_span_v, std::addressof(u));
                D_ASSERT_OR_ASSUME(index < uniform_span_v.size());
                return index;
            }

            template<class Lib>
            [[nodiscard]] constexpr size_t index_location(const shader_export::uniform_base& u) noexcept
            {
                return index_location_for<shader_common::has_indexed_location, Lib>(u);
            }

            template<class Lib>
            [[nodiscard]] constexpr size_t simpler_number(const shader_export::sampler& sampler) noexcept
            {
                return index_location_for<shader_common::is_sampler, Lib>(sampler);
            }
        }
    }

    using private_detail_shader_common::private_detail_index_location::index_location;
    using private_detail_shader_common::private_detail_index_location::simpler_number;

    template<class Lib>
    [[nodiscard]] constexpr GLenum texture_number(const shader_export::sampler& sampler) noexcept
    {
        return narrow<GLenum>(GL_TEXTURE0 + simpler_number<Lib>(sampler));
    }


    template<class Lib>
    struct import_engine
    {
        template<class Uni>
        struct location_index
        {
            size_t index;

            template<class Interface, class... Types>
            [[nodiscard]] constexpr auto operator () (const Interface& lib_interface, const Types&... values) const noexcept -> decltype(Uni::store(gl::uniform_location::invalid, values...))
            {
                return Uni::store
                (
                    interface_to_library<Lib>(lib_interface).uniform_location(index), 
                    values...
                );
            }
        };

        template<class Uni>
        [[nodiscard]] constexpr location_index<Uni> operator () (const Uni& u) const noexcept
        {
            return { index_location<Lib>(u) };
        }
    };


    constexpr void store_sizes_if_support(no_overload, no_overload) noexcept
    {}

    template<class Lib, class Source>
    constexpr auto store_sizes_if_support(const Lib& lib, const Source& source) noexcept -> decltype(lib.sizes(sizes(source)))
    {
        return lib.sizes(sizes(source));
    }
}
