#pragma once

#include <mimalloc.h>

#include <core/tuple.h>
#include <core/memory.h>
#include <core/function_view.h>


D_WARNING_PUSH;
D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized);

namespace private_detail_function
{
    namespace private_detail_unique_memory
    {
        using unique_memory_vtbl_destructor_t = const noexcept_function_pointer_t<void, void*>;
        using unique_memory_vtbl_move_constructor_t = const noexcept_function_pointer_t<void*, void*, void*>;

        struct unique_memory_vtbl
        {
            const unique_memory_vtbl_destructor_t destructor;
            const unique_memory_vtbl_move_constructor_t move_constructor;
        };

        template<class Vtbl>
        struct default_memory_vtbl
        {
            const Vtbl no_memory;
            const Vtbl trivial_small;
            const Vtbl trivial_alloc;
        };

        constexpr auto no_destructor = [] (void*) noexcept
        {};

        constexpr auto no_move_constructor = [] (void*, void* right) noexcept
        {
            return right;
        };

        constexpr auto trivial_small_move_constructor = [] (void* small, void* right) noexcept
        {
            D_ASSERT(is_newmem(small, right, nbyte_arch));
            memcpy(small, right, nbyte_arch);
            return small;
        };

        

        constexpr default_memory_vtbl<unique_memory_vtbl> default_unique_memory_vtbl
        {
            .no_memory
            {
                no_destructor,
                no_move_constructor
            },
            .trivial_small
            {
                default_unique_memory_vtbl.no_memory.destructor,
                trivial_small_move_constructor
            },
            .trivial_alloc
            {
                [](void* left) noexcept
                {
                    D_ASSERT(nullptr != left);
                    ::mi_free(left);
                },
                default_unique_memory_vtbl.no_memory.move_constructor
            }
        };

        using unique_memory_pvtbl_t = const unique_memory_vtbl*;

        constexpr default_memory_vtbl<unique_memory_pvtbl_t> default_unique_memory_pvtbl
        {
            .no_memory{ &default_unique_memory_vtbl.no_memory },
            .trivial_small{ &default_unique_memory_vtbl.trivial_small },
            .trivial_alloc{ &default_unique_memory_vtbl.trivial_alloc }
        };

        struct no_memory_destructor_value_type
        {
            static constexpr unique_memory_vtbl_destructor_t value
            {
                default_unique_memory_vtbl.no_memory.destructor
            };
        };

        template<class T>
        struct small_unique_memory_destructor_value_type
        {
            static constexpr auto value = [] (void* left) noexcept
            {
                ::destroy_at(static_cast<const T*>(left));
            };
        };

        struct trivial_small_unique_memory_move_constructor_value_type
        {
            static constexpr unique_memory_vtbl_move_constructor_t value
            {
                default_unique_memory_vtbl.trivial_small.move_constructor
            };
        };

        template<class T>
        struct small_unique_move_constructor_value_type
        {
            static constexpr auto value = [] (void* small, void* right) noexcept
            {
                ::move_construct_at(static_cast<T*>(small), std::move(*static_cast<T*>(right)));
                return small;
            };
        };

        template<class T>
        constexpr unique_memory_vtbl small_unique_memory_vtbl_v
        {
            conditional_op_or_t
            <
                std::negation_v<std::is_trivially_destructible<T>>,
                no_memory_destructor_value_type,
                small_unique_memory_destructor_value_type,
                T
            >::value,
            conditional_op_or_t
            <
                std::negation_v<std::is_trivially_move_constructible<T>>,
                trivial_small_unique_memory_move_constructor_value_type,
                small_unique_move_constructor_value_type,
                T
            >::value
        };

        template<class T>
        constexpr unique_memory_pvtbl_t small_unique_memory_vtbl_p{ &small_unique_memory_vtbl_v<T> };

        template<class T>
        constexpr unique_memory_vtbl alloc_unique_memory_vtbl_v
        {
            [](void* left) noexcept
            {
                ::destroy_at(static_cast<const T*>(left));
                ::mi_free(left);
            },
            default_unique_memory_vtbl.no_memory.move_constructor
        };

        template<class T>
        constexpr unique_memory_pvtbl_t alloc_unique_memory_vtbl_p{ &alloc_unique_memory_vtbl_v<T> };

        namespace unique_memory_public_namespace
        {
            using private_detail_unique_memory::unique_memory_pvtbl_t;
            using private_detail_unique_memory::default_unique_memory_pvtbl;
            using private_detail_unique_memory::alloc_unique_memory_vtbl_p;
            using private_detail_unique_memory::small_unique_memory_vtbl_p;
        }
    }

    namespace private_detail_unique_function
    {
        using namespace private_detail_unique_memory::unique_memory_public_namespace;
        using private_detail_basic_function_view::basic_function_view;
        using private_detail_decl_function::decl_function_t;

        template<class Fn>
        using is_no_memory_fn = std::disjunction<
            is_address<Fn>,
            has_no_unique_address<Fn>
        >;
       
        template<class R, class... Args>
        class basic_unique_function
        {
        public:
            using null_type = nullfunction_t;
            using view_type = basic_function_view<R, Args...>;
            using invoke_pointer_type = typename view_type::invoke_pointer_type;

            template<class Fn>
            using invoke_to = typename view_type::template invoke_to<Fn>;

            template<class Fn>
            static constexpr invoke_pointer_type fn_invoke_value = invoke_to<Fn>::invoke_r;

            template<class Fn>
            using is_compatible_fn_for_view = typename view_type::template is_compatible_fn<Fn>;

            template<class Fn>
            using is_compatible_fn = std::conjunction<
                std::disjunction<
                    std::conjunction<
                        std::negation<std::is_lvalue_reference<Fn>>,
                        std::is_move_constructible<std::remove_reference_t<Fn>>
                    >,
                    is_no_memory_fn<Fn>
                >,
                is_compatible_fn_for_view<Fn>
            >;

            template<class Fn>
            static constexpr bool is_compatible_fn_value = is_compatible_fn<Fn>::value;

            constexpr basic_unique_function() noexcept
                : view_{}
                , pvtbl_{ default_unique_memory_pvtbl.no_memory }
            {}

            constexpr basic_unique_function(null_type) noexcept
                : basic_unique_function{}
            {}

            D_DISABLE_COPY_CA(basic_unique_function);

            constexpr basic_unique_function(view_type view) noexcept
                : view_{ view }
                , pvtbl_{ default_unique_memory_pvtbl.no_memory }
            {}

            constexpr basic_unique_function(basic_unique_function&& right) noexcept
                : basic_unique_function{}
            {
                _u_move_construct(std::move(right));
            }

            template<class Fn, std::enable_if_t<is_compatible_fn_value<Fn>, int> = 0>
            constexpr basic_unique_function(Fn&& fn_value) noexcept
                : basic_unique_function{}
            {
                _t_move_construct(std::forward<Fn>(fn_value));
            }

            constexpr ~basic_unique_function() noexcept
            {
                pvtbl_->destructor(view_.data());
            }

            constexpr basic_unique_function& operator = (null_type) noexcept
            {
                reset();
                return *this;
            }

            constexpr basic_unique_function& operator = (view_type right) noexcept
            {
                view(right);
                return *this;
            }

            constexpr basic_unique_function& operator = (basic_unique_function&& right) noexcept
            {
                if (this != std::addressof(right)) [[likely]]
                {
                    reset();
                    _u_move_construct(std::move(right));
                }

                return *this;
            }

            template<class Fn>
            constexpr auto operator = (Fn&& fn_value) noexcept -> decltype(this->_t_move(std::forward<Fn>(fn_value)), *this)
            {
                this->_t_move(std::forward<Fn>(fn_value));
                return *this;
            }

            constexpr R operator () (Args... args) const noexcept
            {
                return view_(std::forward<Args>(args)...);
            }

            [[nodiscard]] constexpr explicit operator bool() const noexcept
            {
                return !!view_;
            }

            [[nodiscard]] constexpr view_type view() const noexcept
            {
                return view_;
            }

            constexpr void view(view_type new_view) noexcept
            {
                _reset(new_view);
            }

            constexpr void reset() noexcept
            {
                _reset(nullfunction);
            }

            [[nodiscard]] constexpr invoke_pointer_type invoke_address() const noexcept
            {
                return view_.invoke_address();
            }

            [[nodiscard]] constexpr void* data() const noexcept
            {
                return view_.data();
            }

        protected:
            template<class Fn>
            constexpr std::enable_if_t<is_compatible_fn_value<Fn>> _t_move(Fn&& fn_value) noexcept
            {
                reset();
                _t_move_construct(std::forward<Fn>(fn_value));
            }

        private:
            template<class View>
            constexpr void _reset(View&& new_view) noexcept
            {
                const auto [temp_view, temp_pvtbl] = _release(std::forward<View>(new_view));
                temp_pvtbl->destructor(temp_view.data());
            }

            constexpr void _u_move_construct(basic_unique_function&& right) noexcept
            {
                const auto [right_view, right_pvtbl] = right._release();

                view_ = view_type
                {
                    right_view.invoke_address(),
                    right_pvtbl->move_constructor(small_, right_view.data())
                };

                pvtbl_ = right_pvtbl;
            }

            template<class Fn>
            constexpr void _t_move_construct(Fn&& fn_value) noexcept
            {
                using decay_fn_t = std::decay_t<Fn>;

                if constexpr (is_no_memory_fn<decay_fn_t>::value)
                {
                    static_assert(std::is_trivially_destructible_v<Fn>);
                    static_assert(std::is_trivially_move_constructible_v<Fn>);
                    view_ = std::forward<Fn>(fn_value);
                }
                else
                {
                    constexpr auto invoke_address_value = fn_invoke_value<Fn>;
                    constexpr bool has_alloc = (nbyte_arch < sizeof(decay_fn_t));

                    if constexpr (has_alloc)
                    {
                        {
                            const auto mem = mi_malloc(sizeof(decay_fn_t));
                            if (!mem) [[unlikely]]
                            {
                                return;
                            }

                            view_.data(mem);
                            pvtbl_ = default_unique_memory_pvtbl.trivial_alloc;
                            ::move_construct_at(static_cast<decay_fn_t*>(mem), std::forward<Fn>(fn_value));
                        }

                        view_.invoke_address(invoke_address_value);

                        if constexpr (!std::is_trivially_destructible_v<decay_fn_t>)
                        {
                            pvtbl_ = alloc_unique_memory_vtbl_p<decay_fn_t>;
                        }
                    }
                    else
                    {
                        {
                            void* const p_small{ small_ };
                            ::move_construct_at
                            (
                                static_cast<decay_fn_t*>(p_small),
                                std::forward<Fn>(fn_value)
                            );
                            view_ = view_type{ invoke_address_value, p_small };
                        }

                        constexpr bool is_trivial_small = std::conjunction_v<
                            std::is_trivially_destructible<decay_fn_t>,
                            std::is_trivially_move_constructible<decay_fn_t>
                        >;

                        if constexpr (is_trivial_small)
                        {
                            pvtbl_ = default_unique_memory_pvtbl.trivial_small;
                        }
                        else
                        {
                            pvtbl_ = small_unique_memory_vtbl_p<decay_fn_t>;
                        }
                    }
                }
            }

            template<class View>
            constexpr const tuple<const view_type, const unique_memory_pvtbl_t> _release(View&& new_view) noexcept
            {
                return
                {
                    std::exchange(view_, std::forward<View>(new_view)),
                    std::exchange(pvtbl_, default_unique_memory_pvtbl.no_memory)
                };
            }

            constexpr const auto _release() noexcept
            {
                return _release(nullfunction);
            }

        private:
            static_assert((2u * nbyte_arch) == sizeof(view_type));
            view_type view_;
            std::byte small_[nbyte_arch];
            unique_memory_pvtbl_t pvtbl_;
        };

        template<class Signature>
        struct unique_function : decl_function_t<basic_unique_function, Signature>
        {
            using basic_unique_function_type = decl_function_t<basic_unique_function, Signature>;
            using typename basic_unique_function_type::null_type;
            using typename basic_unique_function_type::view_type;
            using basic_unique_function_type::basic_unique_function;

            constexpr unique_function(unique_function&&) noexcept = default;

            constexpr unique_function& operator = (null_type) noexcept
            {
                basic_unique_function_type::reset();
                return *this;
            }

            constexpr unique_function& operator = (view_type new_view) noexcept
            {
                basic_unique_function_type::view(new_view);
                return *this;
            }

            constexpr unique_function& operator=(unique_function&&) noexcept = default;

            template<class Fn>
            constexpr auto operator = (Fn&& fn_value) noexcept -> decltype(this->_t_move(std::forward<Fn>(fn_value)), *this)
            {
                this->_t_move(std::forward<Fn>(fn_value));
                return *this;
            }
        };

        static_assert(small_size_mini == sizeof(unique_function<void()>));
        static_assert((4u * nbyte_arch) == small_size_mini);
    }
}

using private_detail_function::private_detail_unique_function::unique_function;

D_WARNING_POP;