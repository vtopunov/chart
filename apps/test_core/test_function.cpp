#include <array>
#include <random>
#include <vector>

#include <mimalloc.h>

#include <core/memory.h>


namespace debug_memory
{
    namespace
    {
        struct debug_memory_type
        {
            size_t id;
            size_t size;
            std::byte mem[2u * nbyte_arch];
        };

        struct debug_memory_span_type
        {
            const debug_memory_type* mem;
            size_t id;
            size_t size;
        };

        std::vector<debug_memory_span_type> v_mem;

        size_t check(span<const debug_memory_span_type> vmem) noexcept
        {
            size_t n_alloc{ 0 };

            {
                const auto cend = vmem.cend();
                for (auto it0 = vmem.cbegin(); it0 != cend; ++it0)
                {
                    const debug_memory_span_type sp0{ *it0 };
                    if (sp0.mem)
                    {
                        ++n_alloc;
                        D_ASSERT(sp0.id == sp0.mem->id);
                        D_ASSERT(sp0.size == sp0.mem->size);

                        for (auto it1 = std::next(it0); it1 != cend; ++it1)
                        {
                            const debug_memory_span_type sp1{ *it1 };

                            if (sp1.mem)
                            {
                                D_ASSERT(sp0.mem != sp1.mem);
                                D_ASSERT((sp0.mem->id) != (sp1.mem->id));
                                D_ASSERT(is_newmem<void>(sp0.mem, sp1.mem, std::max(sp0.size, sp1.size)));
                            }
                        }
                    }
                }
            }

            return n_alloc;
        }

        void chech_all_free() noexcept
        {
            D_ASSERT(0u == check(v_mem));
        }

        [[nodiscard]] void* debug_nosmall_mi_malloc(size_t size) noexcept
        {
            D_ASSERT(nbyte_arch < size);
            const auto debug_size = sizeof(debug_memory_type) + size;
            const auto p_debug_mem = static_cast<debug_memory_type*>(::mi_malloc(debug_size));
            D_ASSERT(p_debug_mem);
            {
                static size_t id{ 0 };
                p_debug_mem->id = ++id;
            }
            p_debug_mem->size = debug_size;
            v_mem.emplace_back(debug_memory_span_type{ p_debug_mem, p_debug_mem->id, debug_size });
            D_ASSERT(0u < check(v_mem));
            return std::data(p_debug_mem->mem);
        }

        void debug_mi_free(void* mem) noexcept
        {
            D_ASSERT(mem);
            const debug_memory_type* p_debug_mem{ nullptr };
            const auto bmem = static_cast<const std::byte*>(mem);
            for (auto& sp : v_mem)
            {
                if (sp.mem)
                {
                    if (bmem == std::data(sp.mem->mem))
                    {
                        p_debug_mem = std::exchange(sp.mem, nullptr);
                        break;
                    }
                }
            }
            D_ASSERT(p_debug_mem);
            ::mi_free(const_cast<void*>(static_cast<const void*>(p_debug_mem)));
        }
    }
}

#define mi_malloc(size) debug_memory::debug_nosmall_mi_malloc(size)
#define mi_free(mem) debug_memory::debug_mi_free(mem)

#include <core/unique_function.h>
#include <core/view.h>


namespace
{
    struct default_constructible_test_type
    {
        static constexpr int default_value{ 33 };

        int value;

        constexpr explicit default_constructible_test_type() noexcept
            : value{ default_value }
        {}
    };

    struct no_default_constructible_test_type
    {
        int value;

        no_default_constructible_test_type() = delete;
    };


    void set_errno33() noexcept
    {
        errno = 33;
    }

    double my_sin(double x) noexcept
    {
        return std::sin(x);
    }


    template<template<class> class Function>
    void test_function_view_for() noexcept
    {
        {
            Function<int(int) noexcept> test;
            D_ASSERT(!test);
            D_ASSERT(0 == test(33));
        }

        {
            const errno_holder hold_errno{};
            Function<void()> test = set_errno33;
            D_ASSERT(33 != errno);
            D_ASSERT(test);
            test();
            D_ASSERT(33 == errno);
        }

        {
            Function<void()> test{};
            D_ASSERT(!test);
            D_ASSERT(33 != errno);
            test();
            D_ASSERT(33 != errno);

            {
                const errno_holder hold_errno{};
                test = set_errno33;
                D_ASSERT(test);
                D_ASSERT(33 != errno);
                test();
                D_ASSERT(33 == errno);
            }

            test.reset();
            D_ASSERT(!test);
            D_ASSERT(33 != errno);
            test();
            D_ASSERT(33 != errno);
        }

        {
            const errno_holder hold_errno{};

            const Function<void(int)> test = [] (int value) noexcept
            {
                errno = value;
            };

            D_ASSERT(test);
            test(33);
            D_ASSERT(33 == errno);
        }

        {
            const Function<double(double)> test = my_sin;
            D_ASSERT(test);
            D_ASSERT(is_eqfp(test(0.333), std::sin(0.333)));
        }

        {
            int value{ 0 };

            const Function<void(int&)> test = [] (int& value_ref) noexcept
            {
                value_ref = 33;
            };

            D_ASSERT(test);
            test(value);
            D_ASSERT(33 == value);
        }

        {
            int value{ 0 };

            const Function<void(int)> test = [&value] (int new_value) noexcept
            {
                value = new_value;
            };

            D_ASSERT(test);
            test(33);
            D_ASSERT(33 == value);
        }

        {
            int value0{ 0 };
            int value1{ 0 };

            Function<void(int)> test0 = [&value0] (int new_value) noexcept
            {
                value0 = new_value;

            };

            Function<void(int)> test1 = [&value1] (int new_value) noexcept
            {
                value1 = new_value;
            };

            const auto cviewtest0 = ::view(std::as_const(test0));
            Function<void(int)> test2(cviewtest0);
            Function<void(int)> test3(std::move(::view(test1)));

            D_ASSERT((0 == value0) && (0 == value1));
            test0(32); D_ASSERT((32 == value0) && (0 == value1));
            test1(33); D_ASSERT((32 == value0) && (33 == value1));
            test2(34); D_ASSERT((34 == value0) && (33 == value1));
            test3(35); D_ASSERT((34 == value0) && (35 == value1));
            std::swap(test2, test3);
            test2(36); D_ASSERT((34 == value0) && (36 == value1));
            test3(37); D_ASSERT((37 == value0) && (36 == value1));

            {
                test0 = std::move(test2);
                test0(38); D_ASSERT((37 == value0) && (38 == value1));
                test1(39); D_ASSERT((37 == value0) && (39 == value1));
            }

            {
                test1 = cviewtest0;
                test0(40); D_ASSERT((37 == value0) && (40 == value1));
                test1(41); D_ASSERT((41 == value0) && (40 == value1));
            }
        }

        {
            int value{ 0 };

            const Function<int& ()> test = [&value] () noexcept -> int&
            {
                return value;
            };

            D_ASSERT(test);
            D_ASSERT(std::addressof(value) == std::addressof(test()));
            test() = 33;
            D_ASSERT(33 == value);

            Function<int& ()> temp_iref_test{};
            D_ASSERT(!temp_iref_test);
            D_ASSERT(0 == temp_iref_test());

            Function<double& ()> temp_fref_test{};
            D_ASSERT(!temp_fref_test);
            D_ASSERT(0.0 == temp_fref_test());

            Function<const default_constructible_test_type& ()> temp_cref_test{};
            static_assert(0 != default_constructible_test_type::default_value);
            D_ASSERT(default_constructible_test_type::default_value == temp_cref_test().value);
            
            Function<default_constructible_test_type& ()> temp_ref_test{};
            D_ASSERT(default_constructible_test_type::default_value == temp_ref_test().value);

            Function<no_default_constructible_test_type& ()> temp_nodef_ref_test{};
            D_ASSERT(0 == temp_nodef_ref_test().value);
        }

        {
            std::array<double, 1234u> big{};
            std::mt19937_64 random_engine_64{ std::random_device{}() };
            const Function<size_t()> test_big = [&big, &random_engine_64] () noexcept
            {
                const auto index = std::uniform_int_distribution<size_t>{ 0u, std::size(big) - 1u }(random_engine_64);
                const auto value = std::uniform_real_distribution<>{}(random_engine_64);
                big[index] = value;
                return index;
            };

            D_ASSERT(0.0 == big.front());
            D_ASSERT(0.0 == big.back());
            D_ASSERT(0.0 != big[test_big()]);
            D_ASSERT(0.0 != big[test_big()]);
            D_ASSERT(0.0 != big[test_big()]);
        }
    }

    namespace private_detai_test_unique_function
    {
        class trace_impl
        {
        public:
            enum class action_type
            {
                null,
                default_constructor,
                move_constructor,
                move_operator,
                invoke,
                destructor
            };

            struct action
            {
                const void* left;
                const void* right;
                action_type type;
            };

            constexpr void default_constructor(const void* self) noexcept
            {
                actions_.emplace_back(action{ self, self, action_type::default_constructor });
            }

            constexpr void move_constructor(const void* left, const void* right) noexcept
            {
                actions_.emplace_back(action{ left, right, action_type::move_constructor });
            }

            constexpr void move_operator(const void* left, const void* right) noexcept
            {
                actions_.emplace_back(action{ left, right, action_type::move_operator });
            }

            constexpr void invoke(const void* self) noexcept
            {
                actions_.emplace_back(action{ self, last_right_for(self), action_type::invoke } );
            }

            constexpr void destructor(const void* self) noexcept
            {
                actions_.emplace_back(action{ self, last_right_for(self), action_type::destructor });
            }

            [[nodiscard]] constexpr const void* last_right_for(const void* left) const noexcept
            {
                if (actions_.size())
                {
                    const auto rend = actions_.rend();
                    const auto result = std::find_if
                    (
                        actions_.rbegin(),
                        actions_.rend(),
                        [left] (const action& act) noexcept { return left == act.left; }
                    );
                    D_ASSERT(rend != result);
                    return result->right;
                }
                else
                {
                    D_ASSERT(pop_set_.left == left);
                    return pop_set_.right;
                }
            }

            struct pop_set
            {
                const void* left;
                const void* right;
                size_t last_n_alloc;
                bool is_big_obj;
            };

            struct pop_trace
            {
            public:
                D_DISABLE_ALL_CA(pop_trace);

                constexpr ~pop_trace() noexcept
                {
                    if (p_actions_)
                    {
                        no();
                    }
                    else
                    {
                        D_ASSERT(nullptr == set_);
                    }
                }

                template<class T>
                static constexpr bool is_smart_function_v = std::is_same_v<nullfunction_t, null_t<T> >;

                template<class T>
                static const void* address_of_object(const T& fn_value) noexcept
                {
                    static_assert(!is_address_v<T>);

                    if constexpr (is_smart_function_v<T>)
                    {
                        return fn_value.data();
                    }
                    else
                    {
                        return std::addressof(fn_value);
                    }
                }

                constexpr pop_trace(pop_set* set, std::vector<action>* p_actions)
                    : set_{ set }
                    , p_actions_{ p_actions }
                {
                    D_ASSERT(set_);
                    D_ASSERT(p_actions_);
                }

                constexpr pop_trace pop(action_type act) noexcept
                {
                    D_ASSERT(p_actions_);
                    D_ASSERT(p_actions_->size());
                    const auto pbegin = p_actions_->cbegin();
                    D_ASSERT(pbegin->left == set_->left);
                    D_ASSERT(pbegin->right == set_->right);
                    D_ASSERT(pbegin->type == act);
                    p_actions_->erase(pbegin);
                    return _release();
                }

                [[nodiscard]] constexpr pop_trace p_le(const void* left_p) noexcept
                {
                    D_ASSERT(set_);
                    set_->left = left_p;
                    return _release();
                }

                [[nodiscard]] constexpr pop_trace p_ri(const void* right_p) noexcept
                {
                    D_ASSERT(set_);
                    set_->right = right_p;
                    return _release();
                }

                [[nodiscard]] constexpr pop_trace p_obj(const void* obj) noexcept
                {
                    D_ASSERT(set_);
                    set_->left = obj;
                    set_->right = obj;
                    return _release();
                }

                template<class T>
                [[nodiscard]] constexpr pop_trace le(const T& left_value) noexcept
                {
                    return p_le(address_of_object(left_value));
                }

                template<class T>
                [[nodiscard]] constexpr pop_trace ri(const T& right_value) noexcept
                {
                    return p_ri(address_of_object(right_value));
                }

                template<class T>
                [[nodiscard]] constexpr pop_trace obj(const T& value) noexcept
                {
                    static_assert(!is_smart_function_v<T>);
                    set_->is_big_obj = (nbyte_arch < (sizeof(T)));
                    return p_obj(address_of_object(value));
                }

                constexpr void no() const noexcept
                {
                    D_ASSERT(set_);
                    D_ASSERT(p_actions_);
                    D_ASSERT(0u == (p_actions_->size()));
                }

                constexpr pop_trace ctor0() noexcept
                {
                    return pop(action_type::default_constructor);
                }

                constexpr pop_trace dtor() noexcept
                {
                    return pop(action_type::destructor);
                }

                constexpr pop_trace mv() noexcept
                {
                    return pop(action_type::move_constructor);
                }

                constexpr pop_trace umv() noexcept
                {
                    D_ASSERT(set_);
                    
                    if (set_->is_big_obj)
                    {
                        no();
                        return _release();
                    }
                    else
                    {
                        return mv();
                    }
                }

                constexpr pop_trace invoke() noexcept
                {
                    return pop(action_type::invoke);
                }

            private:
                constexpr pop_trace _release() noexcept
                {
                    return pop_trace
                    {
                        std::exchange(set_, nullptr),
                        std::exchange(p_actions_, nullptr)
                    };
                }

            private:
                pop_set* set_;
                std::vector<action>* p_actions_;
            };

            [[nodiscard]]
            constexpr pop_trace pop() noexcept
            {
                return pop_trace
                {
                    std::addressof(pop_set_),
                    std::addressof(actions_)
                };
            }

            constexpr void no() const noexcept
            {
                D_ASSERT(actions_.empty());
            }

            [[nodiscard]]
            constexpr bool is_big_obj() const noexcept
            {
                return pop_set_.is_big_obj;
            }

        private:
            std::vector<action> actions_{};

            pop_set pop_set_
            { 
                .left{nullptr}, 
                .right{nullptr},
                .last_n_alloc{0u},
                .is_big_obj{false}
            };
        };

        static trace_impl trace{};

        template<class R>
        constexpr function_view<R()> make_r{};

        template<class Mem, class R>
        struct test_function
        {
            Mem mem{};

            template<class... Args>
            constexpr R operator () (Args&&...) const noexcept
            {
                trace.invoke(this);
                return make_r<R>();
            }

            constexpr test_function() noexcept
            {
                trace.default_constructor(this);
            }

            constexpr ~test_function() noexcept
            {
                trace.destructor(this);
            }

            D_DISABLE_COPY_CA(test_function);

            constexpr test_function(test_function&& right) noexcept
            {
                trace.move_constructor(this, std::addressof(right));
            }

            constexpr test_function& operator = (test_function&& right) noexcept
            {
                trace.move_operator(this, std::addressof(right));
                return *this;
            }
        };
    }

    template<class Data, class R>
    void test_unique_function() noexcept
    {
        using namespace private_detai_test_unique_function;
        using test_function_t = test_function<Data, R>;
        using signature_t = R();
        using unique_function_t = unique_function<signature_t>;
        using function_view_t = function_view<signature_t>;
        static_assert(sizeof(test_function_t) == sizeof(Data));

        {
            void* p_my_fn = nullptr;

            {
                test_function_t my_fn;
                p_my_fn = &my_fn;
                trace.pop().obj(my_fn).ctor0();
                D_ASSERT((nbyte_arch < sizeof(Data)) == trace.is_big_obj());

                {
                    unique_function_t t_function = std::move(my_fn);
                    D_ASSERT(t_function);
                    trace.pop().le(t_function).ri(my_fn).mv();
                    t_function();
                    trace.pop().invoke();

                    unique_function_t u_function = std::move(t_function);
                    D_ASSERT(!t_function && u_function);
                    trace.pop().le(u_function).ri(t_function).umv();
                    t_function();
                    trace.no();
                    u_function();
                    trace.pop().invoke();

                    t_function = std::move(t_function);
                    u_function = std::move(u_function);
                    trace.no();

                    t_function = std::move(u_function);
                    trace.pop().le(t_function).ri(u_function).umv();
                    u_function = std::move(t_function);
                    trace.pop().le(u_function).ri(t_function).umv();

                    unique_function_t u_function_cp;
                    D_ASSERT(!u_function_cp);
                    trace.no();
                    u_function_cp = std::move(u_function);
                    D_ASSERT(u_function_cp && !u_function);
                    trace.pop().le(u_function_cp).ri(u_function).umv();
                    u_function_cp();
                    trace.pop().invoke();

                    t_function = std::move(u_function);
                    u_function = std::move(t_function);
                    trace.no();
                }
                trace.pop().dtor();

                {
                    unique_function_t empty_function = std::move(my_fn);
                    trace.pop().le(empty_function).ri(my_fn).mv();
                    unique_function_t no_empty_function = std::move(empty_function);
                    trace.pop().le(no_empty_function).ri(empty_function).umv();

                    D_ASSERT(!empty_function);
                    empty_function();
                    unique_function_t u_empty_function = std::move(empty_function);
                    D_ASSERT(!u_empty_function);
                    u_empty_function();
                    trace.no();

                    no_empty_function();
                    trace.pop().invoke();
                }
                trace.pop().dtor();

                {
                    unique_function_t t_function;
                    trace.no();
                    t_function = std::move(my_fn);
                    D_ASSERT(t_function);
                    trace.pop().le(t_function).ri(my_fn).mv();

                    {
                        unique_function_t unique_as_view{ std::move(std::move(t_function).view()) };
                        D_ASSERT(unique_as_view && t_function);
                        trace.no();
                        unique_as_view();
                        trace.pop().invoke();
                        t_function();
                        trace.pop().invoke();
                        unique_as_view = std::move(std::move(t_function).view());
                        D_ASSERT(unique_as_view && t_function);
                        trace.no();
                        unique_as_view();
                        trace.pop().invoke();
                        t_function();
                        trace.pop().invoke();
                    }
                    trace.no();
                }
                trace.pop().dtor();

                {
                    unique_function_t t_function = std::move(my_fn);
                    trace.pop().le(t_function).ri(my_fn).mv();
                    {
                        function_view_t view_from_unique{ std::move(t_function) };
                        D_ASSERT(view_from_unique && t_function);
                        trace.no();
                        view_from_unique();
                        trace.pop().invoke();
                        t_function();
                        trace.pop().invoke();
                    }
                    trace.no();
                }
                trace.pop().dtor();

                {
                    void* p_my_fn2 = nullptr;
                    {
                        test_function_t my_fn2;
                        p_my_fn2 = std::addressof(my_fn2);
                        trace.pop().obj(my_fn2).ctor0();

                        {
                            unique_function_t t_function = std::move(my_fn);
                            trace.pop().le(t_function).ri(my_fn).mv();
                            unique_function_t t_function2 = std::move(my_fn2);
                            trace.pop().le(t_function2).ri(my_fn2).mv();
                            trace.pop().le(t_function).no();
                            t_function = std::move(t_function2);
                            D_ASSERT(!t_function2 || t_function);
                            trace.pop().dtor().le(t_function).ri(t_function2).umv();
                            t_function = std::move(t_function2);
                            trace.pop().dtor();
                        }

                        {
                            {
                                unique_function_t t_function = std::move(my_fn);
                                trace.pop().le(t_function).ri(my_fn).mv();
                                unique_function_t t_function2 = std::move(my_fn2);
                                trace.pop().le(t_function2).ri(my_fn2).mv();
                                t_function2 = std::move(std::move(t_function).view());
                                D_ASSERT(t_function2 || !t_function);
                                trace.pop().dtor();
                                trace.pop().le(t_function).no();
                                t_function2();
                                trace.pop().invoke();
                            }
                            trace.pop().dtor();
                        }

                        {
                            {
                                unique_function_t t_function = std::move(my_fn);
                                trace.pop().le(t_function).ri(my_fn).mv();
                                t_function = std::move(my_fn2);
                                trace.pop().dtor().le(t_function).ri(my_fn2).mv();
                            }
                            trace.pop().dtor();
                        }

                        trace.pop().obj(my_fn2).no();
                    }
                    trace.pop().p_obj(p_my_fn2).dtor();
                }
                trace.pop().obj(my_fn).no();
            }
            trace.pop().p_obj(p_my_fn).dtor();
        }

        trace.no();
        ::debug_memory::chech_all_free();
    }


    namespace private_detail_test_invoke_if_exist
    {
        struct BA {};
        struct B : BA {};

        struct A
        {
            mutable int value{ 1 };

            void operator () (B) const
            {
                D_ASSERT(!errno && value);
                ++value;
            }

            void operator () (BA)
            {
                D_ASSERT(!errno && value);
                ++value;
            }
        };
    }

    void test_invoke_if_exist() noexcept
    {
        using namespace private_detail_test_invoke_if_exist;

        A lol;
        //const A clol;
        //std::move_only_function<void(B)> fn = lol; fn(B{}); // error // lol clol std::move(lol)
        //std::function<void(B)> fn2 = lol; fn2(B{}); // error // lol clol std::move(lol)
        D_ASSERT(1 == lol.value);
        ::function_view<void(B)> fn3 = lol; fn3(B{});
        D_ASSERT(2 == lol.value);
        ::unique_function<void(B)> fn4 = std::move(lol); fn4(B{});
        D_ASSERT(2 == lol.value);
        {
            const auto p_lol = static_cast<const A*>(fn4.data());
            D_ASSERT(3 == p_lol->value);
        }

        ::invoke_if_exist(A{}, B{});
        static_assert(::is_invocable_v<A, B>);
        static_assert(!::is_invocable_v<A, A>);
        static_assert(!std::is_invocable_v<A, B>);
        static_assert(std::is_invocable_v<const A, B>);

        {
            A lol2;
            std::as_const(lol2)(B{});
            D_ASSERT(2 == lol2.value);
        }

        {
            const A lol3;
            lol3(B{});
            D_ASSERT(2 == lol3.value);
        }
    }

    template<class Signature>
    using basic_unique_function_for_t = private_detail_function::private_detail_decl_function::decl_function_t<
        private_detail_function::private_detail_unique_function::basic_unique_function, 
        Signature
    >;
}


void test_function() noexcept
{
    test_function_view_for<function_view>();
    test_function_view_for<unique_function>();
    test_function_view_for<basic_unique_function_for_t>();
    test_unique_function<std::byte, void>();
    test_unique_function<void*, void>();
    test_unique_function<std::array<void*, 123u>, void>();
    test_invoke_if_exist();
}

