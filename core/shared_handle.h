#pragma once

#include <core/intrusive_list.h>
#include <core/assert.h>

#include <core/unique_handle.h>

template<class T>
class shared_handle
{
public:
    using handle_type = T;
    using view_type = private_handle::view_type_t<handle_type>;

    constexpr shared_handle(handle_type right = {}) noexcept
        : handle_{ std::move(right) }
        , copies_{ this, this }
    {}

    constexpr shared_handle(unique_handle<handle_type> right) noexcept
        : shared_handle{ right.release() }
    {}

    constexpr shared_handle(const shared_handle& right) noexcept
        : handle_{ right.handle_ }
        , copies_{ copies_impl_.push(this, const_cast<shared_handle*>(&right)) }
    {}

    ~shared_handle() noexcept
    {
        if (is_unique())
        {
            handle_.close();
        }
        else
        {
            copies_impl_.pop(this);

            if constexpr (private_handle::has_replace_owwer_v<handle_type>)
            {
                handle_.replace_owwer(copies_.prev);
            }
        }
    }

    shared_handle& operator = (const shared_handle& right) noexcept
    {
        if (this != &right)
        {
            if (is_unique())
            {
                handle_.close();
            }
            else
            {
                copies_impl_.pop(this);
            }

            handle_ = right.handle_;
            copies_ = copies_impl_.push(this, const_cast<shared_handle*>(&right));
        }

        return *this;
    }

    constexpr const handle_type* operator ->() const noexcept
    {
        D_ASSERT(is_valid());
        return &handle_;
    }

    explicit constexpr operator bool() const noexcept
    {
        return is_valid();
    }

    constexpr operator view_type () const noexcept
    {
        return private_handle::view(handle_);
    }

    constexpr bool is_valid() const noexcept
    {
        return private_handle::is_valid(handle_);
    }

    constexpr bool is_unique() const noexcept
    {
        return copies_impl_.is_unique(this);
    }

    void force_close_all_copies() noexcept
    {
        unique_handle<handle_type> handle{ std::exchange(handle_, {}) };

        for (auto next = copies_.next; next != this; next = next->copies_.next )
        {
            next->handle_ = {};
        }
    }

private:
    handle_type handle_;
    intrusive_node<shared_handle> copies_;

    static constexpr intrusive_list_impl<shared_handle> copies_impl_{ &shared_handle::copies_ };
};

template <class T, class... Types>
constexpr shared_handle<T> make_shared_handle(Types&&... args) noexcept
{
    return T{ std::forward<Types>(args)... };
}
