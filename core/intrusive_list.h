#pragma once

struct intrusive_list_node 
{
    intrusive_list_node* prev;
    intrusive_list_node* next;
};

[[nodiscard]]
constexpr intrusive_list_node cyclic(intrusive_list_node* node) noexcept
{
    return { node, node };
}

constexpr void pop(intrusive_list_node item) noexcept
{
    item.prev->next = item.next;
    item.next->prev = item.prev;
}

[[nodiscard]]
constexpr intrusive_list_node push(intrusive_list_node* current, intrusive_list_node* item) noexcept
{
    const intrusive_list_node root{ item, item->next };
    root.prev->next = current;
    root.next->prev = current;
    return root;
}
