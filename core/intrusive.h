#pragma once

#include <initializer_list>

#include <core/type_traits.h>


template<class T>
using decl_intrusive_node_value_type_t = std::remove_pointer_t<decltype(as_pointer(std::declval<const T&>().intrusive_next_pnode))>;

template<class T>
using const_intrusive_node_for_t = const basic_intrusive_node<decl_intrusive_node_value_type_t<T>>;


template<class T>
struct basic_intrusive_node
{
    T* intrusive_prev_pnode;
    T* intrusive_next_pnode;

    [[nodiscard]]
    constexpr bool operator == (const basic_intrusive_node&) const noexcept = default;

    [[nodiscard]]
    constexpr bool operator != (const basic_intrusive_node&) const noexcept = default;
};

struct intrusive_node : basic_intrusive_node<intrusive_node>
{};


template<class T>
[[nodiscard]] constexpr T* as_intrusive_value_pnode(basic_intrusive_node<T>* const pnode) noexcept
{
    return static_cast<T*>(pnode);
}

template<class T>
[[nodiscard]] constexpr const T* as_intrusive_value_pnode(const basic_intrusive_node<T>* const pnode) noexcept
{
    return static_cast<const T*>(pnode);
}

template<class T>
[[nodiscard]] constexpr basic_intrusive_node<T> make_intrusive_cyclic_node(T* const pnode) noexcept
{
    return { pnode, pnode };
}

template<class T>
constexpr void intrusive_write_link(const basic_intrusive_node<T> item, const basic_intrusive_node<T> link) noexcept
{
    item.intrusive_prev_pnode->intrusive_next_pnode = link.intrusive_next_pnode;
    item.intrusive_next_pnode->intrusive_prev_pnode = link.intrusive_prev_pnode;
}

template<class T>
constexpr void intrusive_write_link(const basic_intrusive_node<T> item, T* const pnode) noexcept
{
    intrusive_write_link(item, make_intrusive_cyclic_node(pnode));
}

template<class T>
constexpr void intrusive_unlink(const basic_intrusive_node<T> item) noexcept
{
    intrusive_write_link(item, item);
}

template<class T>
constexpr void intrusive_reset_ref(basic_intrusive_node<T>& node) noexcept
{
    node = make_intrusive_cyclic_node(as_intrusive_value_pnode(std::addressof(node)));
}

template<class T>
[[nodiscard]] constexpr const basic_intrusive_node<T> intrusive_release_ref(basic_intrusive_node<T>& node) noexcept
{
    const auto old = node;
    intrusive_reset_ref(node);
    return old;
}

template<class T>
constexpr void intrusive_force_unlink_ref(basic_intrusive_node<T>& node) noexcept
{
    intrusive_unlink(intrusive_release_ref(node));
}

template<class T>
[[nodiscard]] constexpr const_intrusive_node_for_t<T> make_intrusive_node(T* const new_dependency, T* const prev_dependency, T* const next_dependency) noexcept
{
    const_intrusive_node_for_t<T> linked_dependency{ prev_dependency, next_dependency };
    intrusive_write_link(linked_dependency, new_dependency);
    return linked_dependency;
}

template<class T>
[[nodiscard]] constexpr const_intrusive_node_for_t<T> make_intrusive_front_node(T* const new_dependency, T* const linker) noexcept
{
    return make_intrusive_node(new_dependency, linker, linker->intrusive_next_pnode);
}

template<class T>
[[nodiscard]] constexpr const_intrusive_node_for_t<T> make_intrusive_back_node(T* const new_dependency, T* const linker) noexcept
{
    return make_intrusive_node(new_dependency, linker->intrusive_prev_pnode, linker);
}

template<class T>
constexpr void intrusive_write(basic_intrusive_node<T>*const pnode, const basic_intrusive_node<T> item) noexcept
{
    *pnode = item;
}

template<class T>
constexpr void intrusive_write_front_node(T* const new_dependency, T* const linker) noexcept
{
    intrusive_write(new_dependency, make_intrusive_front_node(new_dependency, linker));
}

template<class T>
constexpr void intrusive_write_back_node(T* const new_dependency, T* const linker) noexcept
{
    intrusive_write(new_dependency, make_intrusive_back_node(new_dependency, linker));
}

template<class T>
[[nodiscard]] constexpr bool intrusive_prev_pnode_is_this(const basic_intrusive_node<T>* const root) noexcept
{
    return root->intrusive_prev_pnode == root;
}

template<class T>
[[nodiscard]] constexpr bool intrusive_next_pnode_is_this(const basic_intrusive_node<T>* const root) noexcept
{
    return root->intrusive_next_pnode == root;
}

template<class T>
[[nodiscard]] constexpr bool intrusive_is_empty(const basic_intrusive_node<T>* const root) noexcept
{
    return intrusive_next_pnode_is_this(root);
}

template<class T>
class intrusive_owner
{
public:
    using node_type = basic_intrusive_node<T>;

    constexpr intrusive_owner() noexcept = delete;
    D_DISABLE_COPYMOVE_CA(intrusive_owner);

    constexpr intrusive_owner(node_type linked_node) noexcept
        : node_{ linked_node }
    {}

    constexpr ~intrusive_owner() noexcept
    {
        intrusive_unlink(node_);
    }

private:
    node_type node_;
};

template<class T>
intrusive_owner(basic_intrusive_node<T>) -> intrusive_owner<T>;


template<class T>
struct intrusive_node_object : basic_intrusive_node<T>
{
    using intrusive_node_type = basic_intrusive_node<T>;

    constexpr intrusive_node_object() noexcept
        : intrusive_node_type{ make_intrusive_cyclic_node(as_intrusive_value_pnode(this)) }
    {}

    D_DISABLE_COPYMOVE_CA(intrusive_node_object);

    constexpr ~intrusive_node_object() noexcept
    {
        intrusive_unlink(*this);
    }
};

namespace private_detail_intrusive_list
{
    template<class T>
    [[nodiscard]] constexpr bool is_self_cyclic_node_object_test(const basic_intrusive_node<T>* const) noexcept
    {
        return true;
    }

    template<class T>
    [[nodiscard]] constexpr bool is_self_cyclic_node_object_test(const intrusive_node_object<T>* const obj) noexcept
    {
        return intrusive_prev_pnode_is_this(obj)
            && intrusive_next_pnode_is_this(obj);
    }
}

template<class NodePointer>
struct intrusive_list_iterator
{
    static_assert(std::is_pointer_v<NodePointer>);

    using pointer = NodePointer;
    using value_type = std::remove_pointer_t<NodePointer>;
    using reference = std::add_lvalue_reference_t<value_type>;
    using const_pointer = add_const_pointer_t<pointer>;
    using const_iterator = intrusive_list_iterator<const_pointer>;

    pointer intrusive_current_pnode;

    constexpr intrusive_list_iterator& operator ++ () noexcept
    {
        intrusive_current_pnode = intrusive_current_pnode->intrusive_next_pnode;
        return *this;
    }

    constexpr intrusive_list_iterator& operator -- () noexcept
    {
        intrusive_current_pnode = intrusive_current_pnode->intrusive_prev_pnode;
        return *this;
    }

    constexpr intrusive_list_iterator operator ++ (int) noexcept
    {
        auto temp = *this;
        operator++();
        return temp;
    }

    constexpr intrusive_list_iterator operator -- (int) noexcept
    {
        auto temp = *this;
        operator--();
        return temp;
    }

    [[nodiscard]]
    constexpr reference operator * () const noexcept
    {
        return *intrusive_current_pnode;
    }

    constexpr pointer operator->() const noexcept
    {
        return intrusive_current_pnode;
    }

    template<class P, std::enable_if_t<std::conjunction_v<std::is_pointer<P>, std::is_convertible<NodePointer, P>>, int> = 0>
    constexpr operator intrusive_list_iterator<P>() const noexcept
    {
        return { .intrusive_current_pnode{ intrusive_current_pnode } };
    }
};

template<class L, class R>
[[nodiscard]] constexpr auto operator == (const intrusive_list_iterator<L>& left, const intrusive_list_iterator<R>& right) noexcept
-> decltype(left.intrusive_current_pnode == right.intrusive_current_pnode)
{
    return left.intrusive_current_pnode == right.intrusive_current_pnode;
}

template<class L, class R>
[[nodiscard]] constexpr auto operator != (const intrusive_list_iterator<L>& left, const intrusive_list_iterator<R>& right) noexcept
-> decltype(left.intrusive_current_pnode != right.intrusive_current_pnode)
{
    return left.intrusive_current_pnode != right.intrusive_current_pnode;
}

template<class T>
class intrusive_list
{
public:
    using value_type = T;
    using const_value_type = const value_type;
    using pointer = value_type*;
    using const_pointer = const_value_type*;
    using reference = std::add_lvalue_reference_t<value_type>;
    using const_reference = std::add_lvalue_reference_t<const_value_type>;
    using iterator = intrusive_list_iterator<pointer>;
    using const_iterator = intrusive_list_iterator<const_pointer>;
    using view_type = intrusive_list_ref<const_value_type>;

    constexpr intrusive_list() noexcept = default;

    D_DISABLE_COPYMOVE_CA(intrusive_list);

    constexpr intrusive_list(std::initializer_list<pointer> ilist) noexcept
        : intrusive_list{}
    {
        for (const auto value_ptr : ilist)
            attach_to_back(value_ptr);
    }

    constexpr void attach_to_back(pointer value_ptr) noexcept
    {
        {
            using private_detail_intrusive_list::is_self_cyclic_node_object_test;
            D_ASSERT(is_self_cyclic_node_object_test(value_ptr));
        }

       intrusive_write_back_node(value_ptr, _p_root());
    }

    constexpr void attach_to_front(pointer value_ptr) noexcept
    {
        {
            using private_detail_intrusive_list::is_self_cyclic_node_object_test;
            D_ASSERT(is_self_cyclic_node_object_test(value_ptr));
        }

        intrusive_write_front_node(value_ptr, _p_root());
    }

    [[nodiscard]]
    constexpr const_reference cfront() const noexcept
    {
        return _front();
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        return _back();
    }

    [[nodiscard]]
    constexpr const_reference front() const noexcept
    {
        return _front();
    }

    [[nodiscard]]
    constexpr const_reference back() const noexcept
    {
        return _back();
    }

    [[nodiscard]]
    constexpr reference front() noexcept
    {
        return _front();
    }

    [[nodiscard]]
    constexpr reference back() noexcept
    {
        return _back();
    }

    constexpr void pop_back() noexcept
    {
        ::intrusive_force_unlink_ref(_back());
    }

    constexpr void pop_front() noexcept
    {
        ::intrusive_force_unlink_ref(_front());
    }

    constexpr void clear() noexcept
    {
        if constexpr (std::is_base_of_v<intrusive_node_object<value_type>, value_type>)
        {
            auto it = begin();
            const auto it_end = end();

            while (it_end != it)
            {
                intrusive_reset_ref(*(it++));
            }
        }

        intrusive_reset_ref(root_);
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return { .intrusive_current_pnode{ root_.intrusive_next_pnode } };
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return { .intrusive_current_pnode{ _p_root() } };
    }

    [[nodiscard]]
    constexpr const_iterator begin() const noexcept
    {
        return cbegin();
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept
    {
        return cend();
    }

    [[nodiscard]]
    constexpr iterator begin() noexcept
    {
        return { .intrusive_current_pnode{ const_cast<pointer>(cbegin().intrusive_current_pnode) } };
    }

    [[nodiscard]]
    constexpr iterator end() noexcept
    {
        return { .intrusive_current_pnode{ const_cast<pointer>(cend().intrusive_current_pnode) } };
    }

    [[nodiscard]]
    constexpr bool is_empty() const noexcept
    {
        return intrusive_is_empty(_p_root());
    }

private:
    [[nodiscard]]
    constexpr reference _front() const noexcept
    {
        return *(root_.intrusive_next_pnode);
    }

    [[nodiscard]]
    constexpr reference _back() const noexcept
    {
        return *(root_.intrusive_prev_pnode);
    }

    [[nodiscard]]
    constexpr pointer _p_root() noexcept
    {
        return as_intrusive_value_pnode(std::addressof(root_));
    }

    [[nodiscard]]
    constexpr const_pointer _p_root() const noexcept
    {
        return as_intrusive_value_pnode(std::addressof(root_));
    }

    template<class>
    friend class intrusive_list_ref;

private:
    intrusive_node_object<value_type> root_{};
};

template<class T>
class intrusive_list_ref
{
public:
    using value_type = T;
    using mutable_value_type = std::remove_const_t<value_type>;
    using const_value_type = std::add_const_t<value_type>;
    using reference = std::add_lvalue_reference_t<value_type>;
    using const_reference = std::add_lvalue_reference_t<const_value_type>;
    using pointer = value_type*;
    using const_pointer = const_value_type*;
    using iterator = intrusive_list_iterator<pointer>;
    using const_iterator = intrusive_list_iterator<const_pointer>;
    using view_type = intrusive_list_ref<const_value_type>;

    D_DEFAULT_ALL_CA(intrusive_list_ref);

    template<class MutableT, std::enable_if_t<std::is_same_v<std::add_const_t<MutableT>, value_type>, int> = 0>
    constexpr intrusive_list_ref(const intrusive_list<MutableT>& list) noexcept
        : proot_{ list._p_root() }
    {}

    template<class ConstOrMutableT, std::enable_if_t<is_const_convertible_v<ConstOrMutableT, value_type>, int> = 0>
    constexpr intrusive_list_ref(intrusive_list<ConstOrMutableT>& list) noexcept
        : proot_{ list._p_root() }
    {}


    [[nodiscard]]
    constexpr reference front() const noexcept
    {
        return *(proot_->intrusive_next_pnode);
    }

    [[nodiscard]]
    constexpr reference back() const noexcept
    {
        return *(proot_->intrusive_prev_pnode);
    }

    [[nodiscard]]
    constexpr const_reference cfront() const noexcept
    {
        return front();
    }

    [[nodiscard]]
    constexpr const_reference cback() const noexcept
    {
        return back();
    }

    [[nodiscard]]
    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    [[nodiscard]]
    constexpr const_iterator cend() const noexcept
    {
        return end();
    }

    [[nodiscard]]
    constexpr iterator begin() const noexcept
    {
        return { .intrusive_current_pnode{ proot_->intrusive_next_pnode } };
    }

    [[nodiscard]]
    constexpr iterator end() const noexcept
    {
        return { .intrusive_current_pnode{ proot_ } };
    }

    [[nodiscard]]
    constexpr bool is_empty() const noexcept
    {
        return intrusive_is_empty(proot_);
    }

    [[nodiscard]]
    constexpr bool is_null() const noexcept
    {
        return !proot_;
    }

    [[nodiscard]]
    constexpr bool is_null_or_empty() const noexcept
    {
        return is_null() || is_empty();
    }

private:
    pointer proot_{ nullptr };
};