#pragma once

#include <core/intrusive.h>
#include <core/view.h>
#include <core/null.h>


template <class T, class D>
class unique_resource
{
    using self_type = unique_resource<T, D>;

public:
    using resource_type = T;
    using view_type = view_t<resource_type>;
    using null_type = null_t<resource_type>;
    using deleter_type = D;
    static constexpr auto null = null_v<resource_type>;

    constexpr unique_resource() noexcept
        : resource_(null)
    {}

    constexpr unique_resource(null_type) noexcept
        : unique_resource{}
    {}

    template<class... Args, std::enable_if_t<std::negation_v<has_type_uncvref<self_type, Args...>>, int> = 0>
    constexpr explicit unique_resource(Args&&... args) noexcept
        : resource_{ std::forward<Args>(args)... }
    {}

    constexpr unique_resource(unique_resource&& right) noexcept
        : resource_{ right.release() }
    {}

    constexpr unique_resource(const unique_resource&) noexcept = delete;

    constexpr ~unique_resource() noexcept
    {
        constexpr deleter_type close{};
        close(std::move(resource_));
    }

    constexpr unique_resource& operator=(unique_resource&& right) noexcept
    {
        swap(right);
        return *this;
    }

    constexpr unique_resource& operator=(const unique_resource&) noexcept = delete;

    constexpr unique_resource& operator=(null_type) noexcept
    {
        reset();
        return *this;
    }

    constexpr void swap(unique_resource& right) noexcept
    {
        u_swap(resource_, right.resource_);
    }

    template<bool dummy = true, std::enable_if_t<(dummy) && is_nullable_v<resource_type>, int> = 0>
    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return has_value(r());
    }

    [[nodiscard]] constexpr operator view_type () const noexcept
    {
        return view(r());
    }

    template<bool dummy = true,
        std::enable_if_t<(dummy) && std::negation_v<is_same_uncvref<view_type, resource_type>>, int> = 0>
    [[nodiscard]] constexpr operator const resource_type& () const noexcept
    {
        return r();
    }

    [[nodiscard]]
    constexpr const resource_type& r() const noexcept
    {
        return resource_;
    }

    [[nodiscard]]
    constexpr resource_type release() noexcept
    {
        return release(null);
    }

    template<class U>
    [[nodiscard]] constexpr resource_type release(U&& new_resource) noexcept
    {
        resource_type resource{ std::move(resource_) };
        resource_ = std::forward<U>(new_resource);
        return resource;
    }

    constexpr void reset() noexcept
    {
        [[maybe_unused]]
        const unique_resource temp{ std::move(*this) };
    }

private:
    resource_type resource_;
};


template<class T, class D>
class shared_resource
{
    using self_type = shared_resource<T, D>;

public:
    using resource_type = T;
    using view_type = view_t<resource_type>;
    using null_type = null_t<resource_type>;
    using deleter_type = D;
    using unique_resource_type = unique_resource<resource_type, deleter_type>;
    static constexpr auto null = null_v<resource_type>;

    constexpr shared_resource() noexcept
        : resource_(null)
        , copies_{ make_intrusive_cyclic_node(std::addressof(copies_)) }
    {}

    constexpr shared_resource(null_type) noexcept
        : shared_resource{}
    {}

    constexpr shared_resource(unique_resource_type&& right) noexcept
        : shared_resource{ right.release() }
    {}

    shared_resource(shared_resource&& right) noexcept = delete;

    template<class... Args, std::enable_if_t<std::negation_v<has_type_uncvref<self_type, Args...>>, int> = 0>
    constexpr explicit shared_resource(Args&&... args) noexcept
        : resource_{ std::forward<Args>(args)... }
        , copies_{ make_intrusive_cyclic_node(std::addressof(copies_)) }
    {}

    constexpr shared_resource(const shared_resource& right) noexcept
        : resource_{ right.resource_ }
        , copies_
        {
            make_intrusive_front_node
            (
                std::addressof(copies_),
                right._p_mutable_copies()
            )
        }
    {}

    constexpr ~shared_resource() noexcept
    {
        [[maybe_unused]]
        const intrusive_owner temp{ copies_ };
        close_if_unique();
    }

    constexpr shared_resource& operator = (const shared_resource& right) noexcept
    {
        if (this != std::addressof(right))
        {
            deattach_and_reset(right.resource_);
            intrusive_write_front_node(std::addressof(copies_), right._p_mutable_copies());
        }

        return *this;
    }

    shared_resource& operator = (shared_resource&& right) noexcept = delete;

    constexpr shared_resource& operator = (unique_resource_type&& right) noexcept
    {
        deattach_and_reset(right.release());
        return *this;
    }

    shared_resource& operator = (null_type) noexcept
    {
        deattach_and_reset();
        return *this;
    }

    template<bool dummy = true, std::enable_if_t<(dummy) && is_nullable_v<resource_type>, int> = 0>
    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return has_value(r());
    }

    [[nodiscard]] constexpr operator view_type () const noexcept
    {
        return r();
    }

    template<bool dummy = true,
        std::enable_if_t<(dummy) && std::negation_v<is_same_uncvref<view_type, resource_type>>, int> = 0>
    [[nodiscard]] constexpr operator const resource_type& () const noexcept
    {
        return r();
    }

    [[nodiscard]]
    constexpr const resource_type& r() const noexcept
    {
        return resource_;
    }

    template<class U>
    constexpr void deattach_and_reset(U&& new_resource) noexcept
    {
        [[maybe_unused]]
        const intrusive_owner temp{ copies_ };
        close_if_unique();
        resource_ = std::forward<U>(new_resource);
    }

    constexpr void deattach_and_reset() noexcept
    {
        deattach_and_reset(null);
    }

private:
    [[nodiscard]]
    constexpr auto _p_mutable_copies() const noexcept
    {
        return as_mutable_pointer(std::addressof(copies_));
    }

    [[nodiscard]]
    constexpr bool is_unique() const noexcept
    {
        return intrusive_is_empty(std::addressof(copies_));
    }

    constexpr void close_if_unique() noexcept
    {
        if (is_unique())
        {
            constexpr deleter_type close{};
            close(std::move(resource_));
        }
    }

private:
    resource_type resource_;
    intrusive_node copies_;
};


template<class View>
struct default_resource
{
    using base_resource_type = default_resource<View>;
    using view_type = View;

    struct null_type
    {
        template<class N>
        constexpr operator N () const noexcept
        {
            static_assert(identical_derived_v<N, base_resource_type>);

            if constexpr (std::is_pointer_v<view_type>)
            {
                return { nullptr };
            }
            else
            {
                return { null_v<view_type> };
            }
        }
    };

    view_type handle;

    constexpr operator view_type() const noexcept
    {
        return handle;
    }

    constexpr explicit operator bool() const noexcept
    {
        return !!handle;
    }
};