#pragma once

template<class T>
struct intrusive_list_node
{
    T* prev;
    T* next;
};

template<class T>
using intrusive_list_member_ptr = intrusive_list_node<T> T::*;

template<class T>
constexpr T* next(intrusive_list_member_ptr<T> list, const T* item) noexcept
{
    return (item->*list).next;
}

template<class T>
constexpr T* prev(intrusive_list_member_ptr<T> list, const T* item) noexcept
{
    return (item->*list).prev;
}

template<class T>
constexpr void set_next(intrusive_list_member_ptr<T> list, T* item, T* next) noexcept
{
    (item->*list).next = next;
}

template<class T>
constexpr void set_prev(intrusive_list_member_ptr<T> list, T* item, T* prev) noexcept
{
    (item->*list).prev = prev;
}

template<class T>
constexpr void pop(intrusive_list_member_ptr<T> list, intrusive_list_node<T> item) noexcept
{
    set_next(list, item.prev, item.next);
    set_prev(list, item.next, item.prev);
}

template<class T>
constexpr intrusive_list_node<T> push(intrusive_list_member_ptr<T> list, T* current, T* item) noexcept
{
    const intrusive_list_node<T> root{ item, next(list, item) };
    set_next(list, root.prev, current);
    set_prev(list, root.next, current);
    return root;
}
