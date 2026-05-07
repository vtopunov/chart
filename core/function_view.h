#pragma once

#include <core/types_algorithm.h>


D_WARNING_PUSH;
D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized);

namespace private_detail_function
{
    namespace private_detail_fn_to_pvoid
    {
        template<class Fn>
        [[nodiscard]] constexpr std::enable_if_t<std::negation_v<has_no_unique_address<std::remove_reference_t<Fn>>>, void*> fn_to_pvoid(Fn&& fn) noexcept
        {
            if constexpr (is_address_v<Fn>)
            {
                return const_cast<void*>((const void*)(fn));
            }
            else
            {
                return const_cast<void*>(static_cast<const void*>(std::addressof(fn)));
            }
        }

        template<class Fn>
        [[nodiscard]] constexpr std::enable_if_t<has_no_unique_address_v<std::remove_reference_t<Fn>>, uninitialized_t> fn_to_pvoid(Fn&&) noexcept
        {
            static_assert(std::is_trivially_default_constructible_v<Fn>);
            return uninitialized_v;
        }
    }

    namespace private_detail_pvoid_to_fn
    {
        template<class Fn>
        [[nodiscard]] constexpr std::enable_if_t<std::conjunction_v<
            std::negation<has_no_unique_address<Fn>>,
            is_address<Fn>
        >, Fn> pvoid_to_fn(void* data) noexcept
        {
            return Fn(data);
        }

        template<class Fn>
        [[nodiscard]] constexpr std::enable_if_t<std::conjunction_v<
            std::negation<has_no_unique_address<Fn>>,
            std::negation<is_address<Fn>>
        >, std::add_lvalue_reference_t<std::remove_reference_t<Fn>> >
            pvoid_to_fn(void* data) noexcept
        {
            return *static_cast<std::remove_reference_t<Fn>*>(data);
        }

        template<class Fn>
        [[nodiscard]] constexpr std::enable_if_t<has_no_unique_address_v<Fn>, Fn> pvoid_to_fn(void*) noexcept
        {
            return {};
        }
    }

    namespace private_detail_invoke_traits
    {
        namespace private_detail_make_nothing_result
        {
            template<size_t N>
            constexpr std::byte zero_byte_array_v[N]{};

            template<size_t N>
            constexpr const void* const p_memzero_v{ zero_byte_array_v<N> };

            template<class T>
            constexpr T instance_for_null_v = null_v<T>;

            template<class R>
            constexpr std::enable_if_t<std::conjunction_v<
                std::negation<std::is_void<R>>, std::is_reference<R>, std::negation<is_null_constructible<R> >
            >, R> make_nothing_result(ttypes<R>) noexcept
            {
                using decay_type = std::remove_cvref_t<R>;

                constexpr auto aligned_sizeof_decay_R = size_align<nbyte_arch>(sizeof(decay_type));
                return const_cast<R>(*static_cast<const decay_type*>(p_memzero_v<aligned_sizeof_decay_R>));
            }

            template<class R>
            constexpr std::enable_if_t<std::conjunction_v<
                std::negation<std::is_void<R>>, std::is_reference<R>, is_null_constructible<R>
            >, R> make_nothing_result(ttypes<R>) noexcept
            {
                using decay_type = std::remove_cvref_t<R>;
                return const_cast<R>(instance_for_null_v<decay_type>);
            }

            template<class R>
            constexpr std::enable_if_t<std::conjunction_v<
                std::negation<std::is_void<R>>, std::negation<std::is_reference<R>>
            >, R> make_nothing_result(ttypes<R>) noexcept
            {
                return instance_for_null<R>();
            }

            template<class R>
            constexpr std::enable_if_t<std::is_void_v<R>> make_nothing_result(ttypes<R>) noexcept
            {
                return;
            }
        }

        template<class R, class... Args>
        struct invoke_traits
        {
            using invoke_pointer_type = add_noexcept_t<function_pointer_t<R, void*, Args...>>;

            struct invoke_nothing
            {
                static constexpr R invoke_r(void*, Args...) noexcept
                {
                    return private_detail_make_nothing_result::make_nothing_result(ttypes_v<R>);
                }
            };

            template<class Fn>
            struct impl_invoke_to : std::true_type
            {
                static constexpr R invoke_r(void* data, Args... args) noexcept
                {
                    return invoke_if_exist_r<R>(
                        private_detail_pvoid_to_fn::pvoid_to_fn<Fn>(data),
                        std::forward<Args>(args)...
                    );
                }
            };

            template<class Fn>
            using invoke_to = conditional_op_or_t<
                is_invocable_r_v<Fn, R, Args...>,
                std::false_type,
                impl_invoke_to,
                Fn
            >;
        };
    }

    namespace private_detail_basic_function_view
    {
        namespace private_detail_compatible_view
        {
            template<class T>
            using decl_view_method_t = std::remove_const_t<decltype(std::declval<T>().view())>;

            template<class View, class Fn>
            constexpr bool is_compatible_view0_v = std::conjunction_v<
                std::negation<std::is_same<View, Fn>>,
                is_detected_exact<View, decl_view_method_t, Fn>
            >;

            template<class View, class Fn>
            constexpr bool is_compatible_view_v = is_compatible_view0_v<std::remove_const_t<View>, std::decay_t<Fn> >;

            template<class View, class Fn>
            using is_uncompatible_view0 = std::conjunction<
                std::negation<std::is_same<View, Fn>>,
                std::negation<is_detected<decl_view_type_t, Fn> >
            >;

            template<class View, class Fn>
            using is_uncompatible_view = is_uncompatible_view0<std::remove_const_t<View>, std::decay_t<Fn> >;
        }

        using private_detail_compatible_view::is_compatible_view_v;
        using private_detail_compatible_view::is_uncompatible_view;
        using private_detail_fn_to_pvoid::fn_to_pvoid;


        template<class R, class... Args>
        class basic_function_view
        {
        public:
            using null_type = nullfunction_t;
            using view_type = basic_function_view<R, Args...>;
            using invoke_traits_type = private_detail_invoke_traits::invoke_traits<R, Args...>;
            using invoke_pointer_type = typename invoke_traits_type::invoke_pointer_type;
            using invoke_nothing_function_traits_type = typename invoke_traits_type::invoke_nothing;
            static constexpr auto invoke_nothing_value = invoke_nothing_function_traits_type::invoke_r;

            template<class Fn>
            using invoke_to = typename invoke_traits_type::template invoke_to<std::decay_t<Fn> >;

            template<class Fn>
            static constexpr invoke_pointer_type fn_invoke_value = invoke_to<Fn>::invoke_r;

            template<class Fn>
            static constexpr bool is_compatible_view_value = is_compatible_view_v<view_type, Fn>;

            template<class Fn>
            using is_compatible_fn = std::conjunction<
                is_uncompatible_view<view_type, Fn>,
                invoke_to<Fn>
            >;

            template<class Fn>
            static constexpr bool is_compatible_fn_value = is_compatible_fn<Fn>::value;

            constexpr basic_function_view() noexcept
                : invoke_{ invoke_nothing_value }
            {}

            constexpr basic_function_view(null_type) noexcept
                : basic_function_view{}
            {}

            D_DEFAULT_COPYMOVE_CA(basic_function_view);

            constexpr basic_function_view(invoke_pointer_type new_invoke, uninitialized_t) noexcept
                : invoke_{ new_invoke }
            {}

            constexpr basic_function_view(invoke_pointer_type new_invoke, void* new_data) noexcept
                : invoke_{ new_invoke }
                , data_{ new_data }
            {}

            template<class Fn, std::enable_if_t<is_compatible_view_value<Fn>, int> = 0>
            constexpr basic_function_view(const Fn& fn_value) noexcept
                : basic_function_view{ fn_value.view() }
            {}

            template<class Fn, std::enable_if_t<is_compatible_fn_value<Fn>, int> = 0>
            constexpr basic_function_view(Fn&& fn_value) noexcept
                : basic_function_view
                {
                    fn_invoke_value<Fn>,
                    fn_to_pvoid(std::forward<Fn>(fn_value))
                }
            {}

            constexpr basic_function_view& operator = (null_type) noexcept
            {
                reset();
                return *this;
            }

            template<class Fn>
            constexpr std::enable_if_t<is_compatible_view_value<Fn>, basic_function_view&> operator = (const Fn& fn_value) noexcept
            {
                *this = view(fn_value);
                return *this;
            }

            template<class Fn>
            constexpr std::enable_if_t<is_compatible_fn_value<Fn>, basic_function_view&> operator = (Fn&& fn_value) noexcept
            {
                invoke_ = fn_invoke_value<Fn>;
                data(fn_to_pvoid(std::forward<Fn>(fn_value)));
                return *this;
            }

            constexpr R operator () (Args... args) const noexcept
            {
                return invoke_(data_, std::forward<Args>(args)...);
            }

            [[nodiscard]] constexpr bool has_value() const noexcept
            {
                return invoke_nothing_value != invoke_;
            }

            [[nodiscard]] constexpr explicit operator bool() const noexcept
            {
                return has_value();
            }

            constexpr void reset() noexcept
            {
                invoke_ = invoke_nothing_value;
            }

            [[nodiscard]] constexpr invoke_pointer_type invoke_address() const noexcept
            {
                return invoke_;
            }

            constexpr void invoke_address(invoke_pointer_type new_invoke) noexcept
            {
                invoke_ = new_invoke;
            }

            [[nodiscard]] constexpr void* data() const noexcept
            {
                return data_;
            }

            constexpr void data(void* new_data) noexcept
            {
                data_ = new_data;
            }

            constexpr void data(uninitialized_t) noexcept
            {}

        private:
            invoke_pointer_type invoke_;
            union { void* data_; };
        };
    }

    namespace private_detail_decl_function
    {
        template<class Arg>
        using decay_invoke_argument_t = select_op_t<
            std::is_reference_v<Arg>,
            std::remove_cv_t,
            std::remove_volatile_t,
            Arg
        >;

        template<template <class...> class FunctionTuple, class Signature>
        using decl_function_t = ttypes_repack_t<
            ttypes_transform_t<decay_invoke_argument_t, ttypes_function_t<Signature>>,
            FunctionTuple
        >;
    }

    namespace private_detail_decl_function_view
    {
        template<class Signature>
        using function_view = private_detail_decl_function::decl_function_t<
            private_detail_basic_function_view::basic_function_view,
            Signature
        >;
    }
}

using private_detail_function::private_detail_decl_function_view::function_view;


D_WARNING_POP;