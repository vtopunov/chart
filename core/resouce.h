#pragma once

#include <core/null.h>
#include <core/view.h>
#include <core/intrusive_list.h>

struct resource_construct_t
{};

inline constexpr resource_construct_t resource_construct{};

template <class T, class D>
class unique_resource
{
public:
    using resource_type = T;
    using view_type = view_t<resource_type>;
    using null_type = null_t<resource_type>;
    using deleter_type = D;
    static constexpr auto null = null_v<std::remove_cvref_t<resource_type>>;

    constexpr unique_resource() noexcept
        : resource_(null)
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
        constexpr deleter_type close{};
        close(std::move(resource_));
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
        ::swap(resource_, right.resource_);
    }

    template<bool dummy = true, class = std::enable_if_t<(dummy) && is_nullable_v<resource_type>>>
    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return has_value(r());
    }

    template<bool dummy = true, class = std::enable_if_t<(dummy) && is_view_v<resource_type>>>
    [[nodiscard]] constexpr operator view_type () const noexcept
    {
        return view(r());
    }

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
        return std::exchange(resource_, null);
    }

    void reset() noexcept
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
private:
    using self = shared_resource;

public:
    using resource_type = T;
    using view_type = view_t<resource_type>;
    using null_type = null_t<resource_type>;
    using deleter_type = D;
    using unique_resource_type = unique_resource<resource_type, deleter_type>;
    static constexpr auto null = null_v<std::remove_cvref_t<resource_type>>;

    constexpr shared_resource() noexcept
        : resource_(null)
        , copies_{ self_linked() }
    {}

    constexpr shared_resource(null_type) noexcept
        : shared_resource{}
    {}

    constexpr shared_resource(unique_resource_type&& right) noexcept
        : shared_resource{ resource_construct, right.release() }
    {}

    shared_resource(shared_resource&& right) noexcept = delete;

    template<class... Args>
    constexpr shared_resource(resource_construct_t, Args&&... args) noexcept
        : resource_{ std::forward<Args>(args)... }
        , copies_{ self_linked() }
    {}

    constexpr shared_resource(const shared_resource& right) noexcept
        : resource_{ right.resource_ }
        , copies_{ linked_with(right) }
    {}

    ~shared_resource() noexcept
    {
        if (has_copies())
        {
            unlink();
        }
        else
        {
            close_(std::move(resource_));
        }
    }

    shared_resource& operator = (const shared_resource& right) noexcept
    {
        if (this != std::addressof(right))
        {
            deattach_and_reset(right.resource_);
            copies_ = linked_with(right);
        }

        return *this;
    }

    shared_resource& operator = (shared_resource&& right) noexcept = delete;

    shared_resource& operator = (unique_resource_type&& right) noexcept
    {
        deattach_and_reset(right.release());
        return *this;
    }

    shared_resource& operator = (null_type) noexcept
    {
        deattach_and_reset();
        return *this;
    }

    template<bool dummy = true, class = std::enable_if_t<(dummy) && is_nullable_v<resource_type>>>
    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return has_value(r());
    }

    template<bool dummy = true, class = std::enable_if_t<(dummy) && is_view_v<resource_type>>>
    [[nodiscard]] constexpr operator view_type () const noexcept
    {
        return r();
    }

    [[nodiscard]] constexpr operator const resource_type& () const noexcept
    {
        return r();
    }

    [[nodiscard]]
    constexpr const resource_type& r() const noexcept
    {
        return resource_;
    }

    template<class T>
    void deattach_and_reset(T&& new_resource) noexcept
    {
        if (has_copies())
        {
            unlink();
            resource_ = std::forward<T>(new_resource);
        }
        else
        {
            copies_ = self_linked();
            close_(std::exchange(resource_, std::forward<T>(new_resource)));
        }
    }

    void deattach_and_reset() noexcept
    {
        deattach_and_reset(null);
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
    constexpr intrusive_list_node linked_with(const shared_resource& item) noexcept
    {
D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_do_not_use_const_cast)
        return push(&copies_, const_cast<intrusive_list_node*>(&item.copies_));
D_WARNING_POP
    }

    constexpr void unlink() const noexcept
    {
        pop(copies_);
    }

private:
    resource_type resource_;
    intrusive_list_node copies_;
    static constexpr deleter_type close_{};
};