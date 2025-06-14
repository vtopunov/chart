#include <core/intrusive.h>
#include <core/static_vector.h>

#include <random>


namespace
{
    constexpr auto invalid_index = numeric_max_v<size_t>;

    struct i_node : basic_intrusive_node<i_node>
    {
        using node_type = basic_intrusive_node<i_node>;

        size_t index;

        [[nodiscard]]
        constexpr bool operator == (const i_node&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const i_node&) const noexcept = default;
    };

    [[nodiscard]]
    constexpr const i_node* iter_ptr(const i_node* ptr) noexcept
    {
        return ptr;
    }

    [[nodiscard]]
    constexpr const i_node* iter_ptr(std::reverse_iterator<const i_node*> it) noexcept
    {
        return &(*it);
    }

    void test_intrusive_list_finish(const i_node* const root, const i_node* const end, const i_node* const it) noexcept
    {
        D_UNUSED(root);
        D_ASSERT(end == it);
    }

    void test_intrusive_list_finish(const i_node* const root, const i_node* const end, const std::reverse_iterator<const i_node*> it) noexcept
    {
        D_UNUSED(end);
        D_ASSERT(root == iter_ptr(it));
    }

    template<class It>
    void test_intrusive_list(const i_node* const root, const i_node* const end, It array_it) noexcept
    {
        D_ASSERT(root < end);

        for (auto it = root->intrusive_next_pnode; it != root; it = it->intrusive_next_pnode, ++array_it)
        {
            const auto array_it_ptr = iter_ptr(array_it);
            D_ASSERT(array_it_ptr > root);
            D_ASSERT(array_it_ptr < end);
            D_ASSERT(it == array_it_ptr);
        }

        test_intrusive_list_finish(root, end, array_it);
    }

    template<class It>
    void test_intrusive_leaky_list(const i_node* const root, const i_node* const end, It array_it) noexcept
    {
        D_ASSERT(root < end);

        for (auto it = root->intrusive_next_pnode; it != root; it = it->intrusive_next_pnode, ++array_it)
        {
            while (true)
            {
                {
                    const auto array_it_ptr = iter_ptr(array_it);
                    D_ASSERT(array_it_ptr > root);
                    D_ASSERT(array_it_ptr < end);
                    if (invalid_index != array_it_ptr->index)
                    {
                        break;
                    }
                }

                ++array_it;
            }

            D_ASSERT(it == iter_ptr(array_it));
        }

        while (invalid_index == array_it->index)
        {
            {
                const auto array_it_ptr = iter_ptr(array_it);
                D_ASSERT(array_it_ptr > root);
                D_ASSERT(array_it_ptr < end);
            }
            ++array_it;
        }

        test_intrusive_list_finish(root, end, array_it);
    }

    template<size_t N>
    constexpr void fill_nodes_array(i_node(&node_array)[N]) noexcept
    {
        for (size_t i = 0; i < N; ++i)
        {
            node_array[i].index = i;
        }
    }

    template<class LT, class VT>
    void test_intrusive_ref_impl(const intrusive_list<LT>& list, intrusive_list_ref<VT> view) noexcept
    {
        constexpr auto eq_node = []<class L, class R>(basic_intrusive_node<L> l, basic_intrusive_node<R> r) noexcept
        {
            return l.intrusive_prev_pnode == r.intrusive_prev_pnode
                && l.intrusive_next_pnode == r.intrusive_next_pnode;
        };

        D_ASSERT(list.begin() == view.cbegin());
        D_ASSERT(list.end() == view.cend());
        D_ASSERT(list.cbegin() == view.cbegin());
        D_ASSERT(list.cend() == view.cend());
        D_ASSERT(view.begin() == view.cbegin());
        D_ASSERT(view.end() == view.cend());
        D_ASSERT(eq_node(view.front(), view.cfront()));
        D_ASSERT(eq_node(view.back(), view.cback()));
        D_ASSERT(eq_node(list.front(), view.cfront()));
        D_ASSERT(eq_node(list.back(), view.cback()));
        D_ASSERT(eq_node(list.cfront(), view.cfront()));
        D_ASSERT(eq_node(list.cback(), view.cback()));
        D_ASSERT(eq_node(view.cfront(), *(view.cbegin())));
        D_ASSERT(eq_node(view.cback(), *(--view.cend())));

        {
            auto l_it = list.cbegin();
            auto v_it = view.cbegin();

            for (auto& node_ref : view)
            {
                D_ASSERT(l_it != list.cend());
                D_ASSERT(v_it != view.cend());
                D_ASSERT(l_it == v_it);
                D_ASSERT(eq_node(*v_it, node_ref));
                D_ASSERT(eq_node(*l_it, node_ref));
                ++v_it;
                ++l_it;
            }

            D_ASSERT(l_it == list.cend());
            D_ASSERT(v_it == view.cend());
        }

        {
            auto l_it = list.cbegin();
            auto v_it = view.cbegin();

            for (auto& node_ref : list)
            {
                D_ASSERT(l_it != list.cend());
                D_ASSERT(v_it != view.cend());
                D_ASSERT(l_it == v_it);
                D_ASSERT(eq_node(*v_it, node_ref));
                D_ASSERT(eq_node(*l_it, node_ref));
                ++v_it;
                ++l_it;
            }

            D_ASSERT(l_it == list.cend());
            D_ASSERT(v_it == view.cend());
        }
    }

    void test_intrusive_ref(const intrusive_list<i_node>& list, intrusive_list_ref<i_node> view) noexcept
    {
        test_intrusive_ref_impl(list, view);
    }

    void test_intrusive_const_ref(const intrusive_list<i_node>& list, intrusive_list_ref<const i_node> view) noexcept
    {
        test_intrusive_ref_impl(list, view);
    }

    void test_intrusive_const_ref_select(const intrusive_list<i_node>& list, intrusive_list_ref<const i_node> view) noexcept
    {
        test_intrusive_const_ref(list, view);
    }

    void test_intrusive_const_ref_select(const intrusive_list<i_node>&, intrusive_list_ref<i_node>) noexcept
    {
        D_ASSERT(!errno);
        D_ASSERT(errno);
    }
}


void test_intrusive() noexcept
{
    static_assert(std::is_trivial_v<intrusive_node> && std::is_standard_layout_v<intrusive_node>);
    static_assert(std::is_same_v<decl_view_type_t<intrusive_list<i_node>>, intrusive_list<i_node>::view_type>);
    static_assert(std::is_same_v<decl_view_type_t<intrusive_list_ref<i_node>>, intrusive_list<i_node>::view_type>);
    static_assert(std::is_same_v<decl_view_type_t<intrusive_list_ref<i_node>>, intrusive_list_ref<i_node>::view_type>);

    {
        i_node nodes[10]{};
        D_ASSERT(!nodes->intrusive_prev_pnode);
        D_ASSERT(!nodes->intrusive_next_pnode);
        intrusive_reset_ref(*nodes);;
        D_ASSERT(nodes->intrusive_prev_pnode == nodes);
        D_ASSERT(nodes->intrusive_next_pnode == nodes);


        const auto make_list = [&nodes] (auto make_node) noexcept
        {
            for (auto it = std::next(std::begin(nodes)); it < std::end(nodes); ++it)
            {
                intrusive_write(it, make_node(it, nodes));
            }
        };

        const auto make_front_link_list = [&make_list] () noexcept
        {
            constexpr auto make_node = [] (auto link, auto root) noexcept
            {
                return make_intrusive_front_node(link, root);
            };

            make_list(make_node);
        };

        const auto make_back_link_list = [&make_list] () noexcept
        {
            constexpr auto make_node = [] (auto link, auto root) noexcept
            {
                return make_intrusive_back_node(link, root);
            };

            make_list(make_node);
        };


        fill_nodes_array(nodes);

        for (size_t i_try = 0; i_try < 3u; ++i_try)
        {
            make_front_link_list();

            for (auto cend = std::cend(nodes); cend != nodes; intrusive_unlink(*--cend))
            {
                test_intrusive_list(std::cbegin(nodes), cend, std::reverse_iterator{ cend });
            }
        }

        for (size_t i_try = 0; i_try < 3u; ++i_try)
        {
            make_back_link_list();

            for (auto cend = std::cend(nodes); cend != nodes; intrusive_unlink(*--cend))
            {
                test_intrusive_list(std::cbegin(nodes), cend, std::next(std::cbegin(nodes)));
            }
        }


        {
            std::mt19937_64 random_engine{ std::random_device{}() };
            static_vector<i_node*, std::size(nodes)> p_nodes{};

            const auto fill_p_nodes = [&p_nodes, &nodes] () noexcept
            {
                p_nodes.clear();
                for (auto p_node = std::next(std::begin(nodes)); p_node != std::cend(nodes); ++p_node)
                {
                    p_nodes.emplace_back(p_node);
                }
            };

            const auto random_erase = [&random_engine, &p_nodes] () noexcept
            {
                std::uniform_int_distribution<size_t> index_distribution{ 0u, p_nodes.size() - 1u };
                const auto random_index = index_distribution(random_engine);
                const auto p_node = p_nodes[random_index];
                intrusive_unlink(*p_node);
                p_node->index = invalid_index;
                p_nodes.erase(std::next(p_nodes.cbegin(), random_index));
            };

            for (size_t i_try = 0; i_try < 5u; ++i_try)
            {
                fill_nodes_array(nodes);
                make_front_link_list();

                {
                    fill_p_nodes();

                    while (p_nodes.size())
                    {
                        test_intrusive_leaky_list(std::cbegin(nodes), std::cend(nodes), std::crbegin(nodes));

                        random_erase();
                    }
                }
            }

            for (size_t i_try = 0; i_try < 5u; ++i_try)
            {
                fill_nodes_array(nodes);
                make_back_link_list();

                {
                    fill_p_nodes();

                    while (p_nodes.size())
                    {
                        test_intrusive_leaky_list(std::cbegin(nodes), std::cend(nodes), std::next(std::cbegin(nodes)));

                        random_erase();
                    }
                }
            }
        }
    }

    using i_node_intrusive_list_t = intrusive_list<i_node>;

    {
        i_node nodes[5];
        fill_nodes_array(nodes);

        i_node_intrusive_list_t nodes_list{ std::data(nodes), std::next(nodes, 1), std::next(nodes, 2) };

        const auto test_nodes_list = [&nodes, &nodes_list] (size_t i0, size_t count) noexcept
        {
            D_ASSERT(!count == nodes_list.is_empty());

            const auto test_list = [&nodes, i0, count] (auto& list) noexcept
            {
                size_t i = i0;
                for (auto& node : list)
                {
                    D_ASSERT(node == nodes[i]);
                    ++i;
                }
                D_ASSERT((i0 + count) == i);
            };

            test_list(nodes_list);
            test_list(std::as_const(nodes_list));
            test_intrusive_ref(nodes_list, nodes_list);
            test_intrusive_const_ref(nodes_list, nodes_list);
            test_intrusive_const_ref_select(nodes_list, std::as_const(nodes_list));
            D_UNUSED(static_cast<void (*) (const intrusive_list<i_node>&, intrusive_list_ref<i_node>)>(test_intrusive_const_ref_select));
        };

        test_nodes_list(0, 3);

        nodes_list.attach_to_back(std::next(nodes, 3)); test_nodes_list(0, 4);
        intrusive_unlink(nodes[0]); test_nodes_list(1, 3);

        nodes_list.attach_to_back(std::next(nodes, 4)); test_nodes_list(1, 4);
        intrusive_unlink(nodes[1]); test_nodes_list(2, 3);

        nodes_list.attach_to_front(std::next(nodes, 1)); test_nodes_list(1, 4);
        intrusive_unlink(nodes[4]); test_nodes_list(1, 3);

        nodes_list.attach_to_front(std::next(nodes, 0)); test_nodes_list(0, 4);
        intrusive_unlink(nodes[3]); test_nodes_list(0, 3);
    }


    using pop_t = void (i_node_intrusive_list_t::*)() noexcept;

    const auto test_pop = [] (pop_t pop) noexcept
    {
        i_node nodes[5];
        fill_nodes_array(nodes);
        i_node_intrusive_list_t nodes_list{ std::data(nodes), std::next(nodes, 1), std::next(nodes, 2), std::next(nodes, 3), std::next(nodes, 4) };
        size_t i_pop = 0;
        while (!nodes_list.is_empty())
        {
            (nodes_list.*pop)();
            ++i_pop;
        }
        D_ASSERT(5 == i_pop);
    };

    constexpr auto pop_back_ref = &i_node_intrusive_list_t::pop_back;
    constexpr auto pop_front_ref = &i_node_intrusive_list_t::pop_front;

    test_pop(pop_back_ref);
    test_pop(pop_front_ref);
}