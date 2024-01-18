#include <algorithm>
#include <array>
#include <functional>
#include <chrono>

#include <core/resource.h>


namespace
{
    struct skip_op
    {
        template<class T>
        void operator () (T&&) const noexcept
        {}
    };

    template<class resource_type, class D>
    struct unsafe_resource
    {
        resource_type* operator -> () noexcept
        {
            return &h;
        }

        resource_type h;
        intrusive_node c;
    };

    template<class T, class D>
    [[nodiscard]] unsafe_resource<T, D>* unsafe(const shared_resource<T, D>* safe) noexcept
    {
        using safe_type = shared_resource<T, D>;
        using unsafe_type = unsafe_resource<T, D>;

        static_assert(sizeof(safe_type) == sizeof(unsafe_type));
        static_assert(alignof(safe_type) == alignof(unsafe_type));
        return (unsafe_type*)safe;
    }

    constexpr struct
    {
        template<class T, class D>
        [[nodiscard]] const intrusive_node* operator () (const shared_resource<T, D>* safe) const noexcept
        {
            return std::addressof(unsafe(safe)->c);
        }
    } node{};

    constexpr struct
    {
        template<class T, class D>
        [[nodiscard]] const intrusive_node* operator ()(const shared_resource<T, D>* safe) const noexcept
        {
            return node(safe)->intrusive_prev_pnode;
        }
    } prev{};

    constexpr struct
    {
        template<class T, class D>
        [[nodiscard]] const intrusive_node* operator ()(const shared_resource<T, D>* safe) const noexcept
        {
            return node(safe)->intrusive_next_pnode;
        }
    } next{};

    struct tested_resource
    {
        tested_resource() noexcept = default;

        tested_resource(int right) noexcept
            : value{ right }
        {}

        constexpr explicit operator bool() const noexcept
        {
            return value;
        }

        int value = 0;
        std::function<void(const tested_resource&)> check_close;
    };

    struct tested_resource_deleter
    {
        void operator () (const tested_resource& resource) const noexcept
        {
            if (resource.check_close)
            {
                resource.check_close(resource);
            }
        }
    };

    using tested_unique = unique_resource<tested_resource, tested_resource_deleter>;
    using tested_shared = shared_resource<tested_resource, tested_resource_deleter>;


    template<class T, class D, size_t N, class Node>
    [[nodiscard]] constexpr bool is_unqiue_nodes(const std::array<const shared_resource<T, D>*, N>& array_p, Node node) noexcept
    {
        std::array<const intrusive_node*, N> nodes{};
        std::transform(array_p.cbegin(), array_p.cend(), nodes.begin(), node);
        std::sort(nodes.begin(), nodes.end());

        const auto nodes_cend = nodes.cend();
        return nodes_cend == std::adjacent_find(nodes.cbegin(), nodes_cend);
    }

    template<class... Args>
    void test_shaded(int value, const Args&... args) noexcept
    {
        const std::array<const tested_shared*, sizeof...(args)> args_ptr_array{ std::addressof(args)... };
        D_ASSERT(is_unqiue_nodes(args_ptr_array, node));
        D_ASSERT(is_unqiue_nodes(args_ptr_array, prev));
        D_ASSERT(is_unqiue_nodes(args_ptr_array, next));

        constexpr auto n_arg = std::size(args_ptr_array);

        constexpr auto prev_i = [] (size_t i) noexcept
        {
            constexpr auto back_i = n_arg - 1u;
            return (i + back_i) % n_arg;
        };

        constexpr auto next_i = [] (size_t i) noexcept
        {
            return (i + 1u) % n_arg;
        };

        const auto test_node = [&args_ptr_array] (const intrusive_node* tested_node, size_t node_i) noexcept
        {
            D_ASSERT(tested_node == node(args_ptr_array[node_i]));
        };

        for (ptrdiff_t i = 0; i < n_arg; ++i)
        {
            const auto current_p = args_ptr_array[i];
            test_node(prev(current_p), prev_i(i));
            test_node(next(current_p), next_i(i));
            D_ASSERT(current_p->r().value == value);
        }
    }

    static_assert(std::is_same_v<null_t<tested_unique>, null_t<tested_resource>>);
    static_assert(std::is_same_v<null_t<tested_shared>, null_t<tested_resource>>);

    struct verifiable_resource
    {
        mutable bool is_valid;

        constexpr explicit operator bool() const noexcept
        {
            return is_valid;
        }
    };

    static_assert(std::is_trivially_copyable_v<verifiable_resource>);
    static_assert(std::is_trivially_copyable_v<skip_op>);

    using verifiable_unique = unique_resource<verifiable_resource, skip_op>;
    using verifiable_linked = shared_resource<verifiable_resource, skip_op>;

    static_assert(std::is_move_constructible_v<verifiable_unique>&& std::is_move_assignable_v<verifiable_unique>);
    static_assert(!std::is_copy_constructible_v<verifiable_unique> && !std::is_copy_assignable_v<verifiable_unique>);
    static_assert(!std::is_trivially_move_assignable_v<verifiable_unique> && !std::is_trivially_move_constructible_v<verifiable_unique>);

    static_assert(!std::is_same_v<null_t<tested_resource>, null_t<verifiable_resource>>);
    static_assert(std::is_same_v<null_t<verifiable_unique>, null_t<verifiable_resource>>);
    static_assert(std::is_same_v<null_t<verifiable_linked>, null_t<verifiable_resource>>);
    static_assert(std::is_same_v<resource_type_t<tested_unique>, tested_resource>);
    static_assert(std::is_same_v<resource_type_t<tested_shared>, tested_resource>);
    static_assert(std::is_same_v<resource_type_t<const tested_unique>, tested_resource>);
    static_assert(std::is_same_v<resource_type_t<const tested_shared>, tested_resource>);
}


void test_resource() noexcept
{
    constexpr struct
    {
        void operator () (const tested_shared& h1, int value) const noexcept
        {
            test_shaded(value, h1);
        }

        void operator () (const tested_shared& h1, const tested_shared& h2, int value) const noexcept
        {
            test_shaded(value, h1, h2);
        }

        void operator () (const tested_shared& h1, const tested_shared& h2, const tested_shared& h3, int value) const noexcept
        {
            test_shaded(value, h1, h2, h3);
        };

        void operator () (const tested_shared& h1, const tested_shared& h2, const tested_shared& h3, const tested_shared& h4, int value) const noexcept
        {
            test_shaded(value, h1, h2, h3, h4);
        };
    } check{};

    {
        {
            verifiable_unique safe;
            D_ASSERT(!safe.r().is_valid);
            D_ASSERT(!safe);
            safe.r().is_valid = true;
            D_ASSERT(safe);
            safe.r().is_valid = false;
            D_ASSERT(!safe);
        }

        {
            verifiable_linked safe;
            D_ASSERT(!safe.r().is_valid);
            D_ASSERT(!safe);
            safe.r().is_valid = true;
            D_ASSERT(safe);
            safe.r().is_valid = false;
            D_ASSERT(!safe);
        }
    }

    tested_shared h1{ resource_construct, 1 };
    check(h1, 1);

    { // self assignment
        h1 = h1;
        check(h1, 1);
    }

    {   // smart resource closure
        int closed_value = 0;
        {
            tested_shared h2{ resource_construct, 2 };
            as_mutable(h2.r()).check_close = [&closed_value] (const tested_resource& closing_handle) noexcept
            {
                D_ASSERT(closed_value != 2 && closing_handle.value == 2);
                closed_value = closing_handle.value;
            };
        }
        D_ASSERT(closed_value == 2);
    }

    { // copy constructor
        tested_shared h2{ h1 };
        check(h1, h2, 1);
    }
    check(h1, 1);

    { // assignment initialization 
        int h2_closed_value = 0;
        tested_shared h2{ resource_construct, 2 };
        as_mutable(h2.r()).check_close = [&h2_closed_value] (const tested_resource& closing_handle) noexcept
        {
            D_ASSERT(h2_closed_value != 2 && closing_handle.value == 2);
            h2_closed_value = closing_handle.value;
        };
        h2 = h1;
        check(h1, h2, 1);
        D_ASSERT(h2_closed_value == 2);
    }
    check(h1, 1);

    {   // assignment
        tested_shared h2{ resource_construct, 2 };

        int h1_closed_value = 0;
        as_mutable(h1.r()).check_close = [&h1_closed_value] (const tested_resource& closing_handle) noexcept
        {
            D_ASSERT(h1_closed_value != 1 && closing_handle.value == 1);
            h1_closed_value = closing_handle.value;
        };

        D_ASSERT(h1.r().value == 1);
        h1 = h2;
        check(h1, h2, 2);
        D_ASSERT(h1_closed_value == 1);
    }
    check(h1, 2);
    as_mutable(h1.r()).value = 1;

    {   // cyclic assignment
        tested_shared h2{ h1 };
        check(h1, h2, 1);
        h1 = h2;
        check(h1, h2, 1);
        h2 = h1;
        check(h1, h2, 1);
    }
    check(h1, 1);

    {   // cyclic assignment (ref count > 2)
        tested_shared h2{ h1 };
        tested_shared h3{ h2 };
        check(h1, h2, h3, 1);

        h1 = h2;
        check(h1, h3, h2, 1);
        h2 = h1;
        check(h1, h2, h3, 1);

        h2 = h3;
        check(h1, h3, h2, 1);
        h3 = h2;
        check(h1, h2, h3, 1);

        h3 = h1;
        check(h1, h3, h2, 1);
        h1 = h3;
        check(h1, h2, h3, 1);
    }
    check(h1, 1);

    {   // cyclic assignment (ref count > 3)
        tested_shared h2{ h1 };
        tested_shared h3{ h2 };
        tested_shared h4{ h3 };
        check(h1, h2, h3, h4, 1); 

        h1 = h2;
        check(h2, h1, h3, h4, 1);
        h2 = h1;
        check(h1, h2, h3, h4, 1);

        h1 = h3;
        check(h3, h1, h4, h2, 1);
        h4 = h3;
        check(h1, h2, h3, h4, 1);

        h1 = h4;
        check(h1, h2, h3, h4, 1);

        h2 = h1;
        check(h1, h2, h3, h4, 1);

        h2 = h3;
        check(h3, h2, h4, h1, 1);
        h3 = h2;
        check(h1, h2, h3, h4, 1);

        h2 = h4;
        check(h4, h2, h1, h3, 1);
        h1 = h4;
        check(h1, h2, h3, h4, 1);

        h3 = h1;
        check(h1, h3, h2, h4, 1);
        h2 = h1;
        check(h1, h2, h3, h4, 1);

        h3 = h2;
        check(h1, h2, h3, h4, 1);

        h3 = h4;
        check(h4, h3, h1, h2, 1);
        h4 = h3;
        check(h1, h2, h3, h4, 1);

        h4 = h1;
        check(h1, h4, h2, h3, 1);
        h1 = h4;
        check(h1, h2, h3, h4, 1);

        h4 = h2;
        check(h2, h4, h3, h1, 1);
        h3 = h2;
        check(h1, h2, h3, h4, 1);

        h4 = h3;
        check(h1, h2, h3, h4, 1);
    }
    check(h1, 1);

    {   // assignment (ref count >= 2)
        int h2_closed_value = 0;
        {
            tested_shared ch1{ h1 };
            check(h1, ch1, 1);

            tested_shared h2{ resource_construct, 2 };
            tested_shared ch2{ h2 };
            check(h2, ch2, 2);

            h2 = h1;
            check(ch1, h1, h2, 1);
            check(ch2, 2);

            ch1 = ch2;
            check(ch1, ch2, 2);
            check(h1, h2, 1);

            h2 = ch2;
            check(ch1, ch2, h2, 2);
            check(h1, 1);

            ch1 = h1;
            check(h2, ch2, 2);
            check(h1, ch1, 1);

            as_mutable(h2.r()).check_close = [&h2_closed_value] (const tested_resource& closing_handle) noexcept
            {
                D_ASSERT(h2_closed_value != 2 && closing_handle.value == 2);
                h2_closed_value = closing_handle.value;
            };
        }
        D_ASSERT(h2_closed_value == 2);
    }

    static struct h1_check_dtor_type
    {
        int h1_closed_value = 0;

        void operator () (const tested_resource& closing_handle) noexcept
        {
            D_ASSERT(h1_closed_value != 1 && closing_handle.value == 1);
            h1_closed_value = closing_handle.value;
        }

        ~h1_check_dtor_type() noexcept
        {
            D_ASSERT(h1_closed_value == 1);
        }
    } h1_check_dtor;

    check(h1, 1);

    as_mutable(h1.r()).check_close = std::ref(h1_check_dtor);
}
