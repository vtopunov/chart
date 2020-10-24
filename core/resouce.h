#pragma once

#include <utility>

#include <core/view.h>
#include <core/null.h>
#include <core/intrusive_list.h>

template<class T>
using has_value_t = decltype(has_value(std::declval<T&>()));

template<class T>
constexpr bool has_check_v = is_detected_v<has_value_t, T>;

struct resource_construct_t
{};

inline constexpr resource_construct_t resource_construct{};

class resource_destroy_t
{
private:
    template <class T, class D>
    friend class unique_resource;

    template <class T, class D>
    friend class linked_resource;

    constexpr resource_destroy_t() noexcept = default;
};

struct resource_default_deleter
{
    template<class T>
    void operator () (T&& resource, resource_destroy_t) const noexcept
    {
        close(std::move(resource));
    }
};

template <class T, class D = resource_default_deleter>
class unique_resource
{
public:
    using resource_type = T;
    using view_type = view_t<resource_type>;
    using null_type = null_t<resource_type>;
    using deleter_type = D;
    static constexpr null_type null{};

    constexpr unique_resource() noexcept
        : resource_{ null }
    {}

    constexpr unique_resource(null_type) noexcept
        : unique_resource{}
    {}

    template<class... Args>
    constexpr unique_resource(resource_construct_t, Args&&... args) noexcept
        : resource_{ std::forward<Args>(args)... }
    {}

    constexpr unique_resource(unique_resource&& right) noexcept
        : resource_{ right.release() }
    {}

    constexpr unique_resource(const unique_resource&) noexcept = delete;

    ~unique_resource() noexcept
    {
        close_(std::move(resource_), close_tag_);
    }

    constexpr unique_resource& operator=(unique_resource&& right) noexcept
    {
        swap(right);
        return *this;
    }

    constexpr unique_resource& operator=(const unique_resource&) noexcept = delete;

    unique_resource& operator=(null_type) noexcept
    {
        reset();
        return *this;
    }

    constexpr void swap(unique_resource& right) noexcept
    {
        std::swap(resource_, right.resource_);
    }

    [[nodiscard]]
    constexpr const resource_type& resource() const noexcept
    {
        return resource_;
    }

    [[nodiscard]]
    constexpr decltype( auto )  operator ->() const noexcept
    {
        return &resource_;
    }

    template<
        bool dummy = true,
        class = std::enable_if_t<( has_check_v<resource_type> && dummy )>
    > [[nodiscard]]
        constexpr explicit operator bool() const noexcept
    {
        return has_value(resource_);
    }

    [[nodiscard]]
    constexpr operator view_type () const noexcept
    {
        return resource_;
    }

    [[nodiscard]]
    constexpr resource_type release() noexcept
    {
        return std::exchange(resource_, null);
    }

    void reset() noexcept
    {
        [[maybe_unused]]
        const unique_resource temp{ std::move(*this) };
    }

private:
    resource_type resource_;
    static constexpr deleter_type close_{};
    static constexpr resource_destroy_t close_tag_{};
};

template<class T, class D = resource_default_deleter>
class linked_resource
{
private:
    using self = linked_resource;

public:
    using resource_type = T;
    using view_type = view_t<resource_type>;
    using null_type = null_t<resource_type>;
    using deleter_type = D;
    using unique_resource_type = unique_resource<resource_type, deleter_type>;
    static constexpr null_type null{};

    constexpr linked_resource() noexcept
        : linked_resource{ resource_construct, null }
    {}

    constexpr linked_resource(null_type) noexcept
        : linked_resource{}
    {}

    constexpr linked_resource(unique_resource_type&& right) noexcept
        : linked_resource{ resource_construct, right.release() }
    {}


    linked_resource(linked_resource&& right) noexcept = delete;

    template<class... Args>
    constexpr linked_resource(resource_construct_t, Args&&... args) noexcept
        : resource_{ std::forward<Args>(args)... }
        , copies_{ self_linked() }
    {}

    constexpr linked_resource(const linked_resource& right) noexcept
        : resource_{ right.resource_ }
        , copies_{ linked_with(right) }
    {}

    ~linked_resource() noexcept
    {
        if ( has_copies() )
        {
            unlink();
        }
        else
        {
            close_(std::move(resource_), close_tag_);
        }
    }

    linked_resource& operator = (const linked_resource& right) noexcept
    {
        if ( this != std::addressof(right) )
        {
            deattach_and_reset(right.resource_);
            copies_ = linked_with(right);
        }

        return *this;
    }

    linked_resource& operator = (linked_resource&& right) noexcept = delete;

    linked_resource& operator = (unique_resource_type&& right) noexcept
    {
        deattach_and_reset(right.release());
        return *this;
    }

    linked_resource& operator = (null_type) noexcept
    {
        deattach_and_reset();
        return *this;
    }

    [[nodiscard]]
    constexpr const resource_type& resource() const noexcept
    {
        return resource_;
    }

    [[nodiscard]]
    constexpr decltype( auto ) operator ->() const noexcept
    {
        return &resource_;
    }

    template<
        bool dummy = true,
        class = std::enable_if_t<(has_check_v<resource_type> && dummy )>
    > [[nodiscard]]
        constexpr explicit operator bool() const noexcept
    {
        return has_value(resource_);
    }

    [[nodiscard]]
    constexpr operator view_type () const noexcept
    {
        return resource_;
    }

    template<class T>
    void deattach_and_reset(T&& new_resource) noexcept
    {
        if ( has_copies() )
        {
            unlink();
            resource_ = std::forward<T>(new_resource);
        }
        else
        {
            copies_ = self_linked();
            close_(std::exchange(resource_, std::forward<T>(new_resource)), close_tag_);
        }
    }

    void deattach_and_reset() noexcept
    {
        deattach_and_reset(null);
    }

    [[nodiscard]]
    constexpr resource_type deattach_and_release() noexcept
    {
        if ( has_copies() )
        {
            unlink();
        }
        else
        {
            copies_ = self_linked();
        }

        return std::exchange(resource_, null);
    }

private:
    [[nodiscard]]
    constexpr bool has_copies() const noexcept
    {
        return copies_.next != &copies_;
    }

    [[nodiscard]]
    constexpr intrusive_list_node self_linked() noexcept
    {
        return cyclic(&copies_);
    };

    [[nodiscard]]
    constexpr intrusive_list_node linked_with(const linked_resource& item) noexcept
    {
#pragma warning(push)
#pragma warning(disable : 26492) // Don't use const_cast

        return push(&copies_, const_cast<intrusive_list_node*>(&item.copies_));

#pragma warning(pop)
    }

    constexpr void unlink() const noexcept
    {
        pop(copies_);
    }

private:
    resource_type resource_;
    intrusive_list_node copies_;
    static constexpr deleter_type close_{};
    static constexpr resource_destroy_t close_tag_{};
};