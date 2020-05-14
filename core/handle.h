#pragma once

#include <utility>

#include <core/intrusive_list.h>
#include <core/member_detector.h>

template<class T>
using has_view_t = typename T::view_type;

template <class T>
using view_t = detected_or_t<T, has_view_t, T>;

template<class T>
constexpr view_t<T> to_view(const T& value) noexcept
{
    return value;
}

template<class T>
using has_bool_op_t = decltype( !!std::declval<T>() );

template<class T>
using has_bool_op_view_t = decltype( !!to_view(std::declval<T>()) );

template<class T>
using has_exist_t = decltype( exist(to_view(std::declval<T>())) );

template<class T>
constexpr bool has_bool_op_v = is_detected_v<has_bool_op_t, T>;

template<class T>
constexpr bool has_bool_op_view_v = is_detected_v<has_bool_op_view_t, T>;

template<class T>
constexpr bool has_exist_v = is_detected_v<has_exist_t, T>;

template<class T>
constexpr bool to_bool(const T& value) noexcept
{
    if constexpr ( has_bool_op_v<T> )
    {
        return !!value;
    }
    else if constexpr ( has_bool_op_view_v<T> )
    {
        return !!to_view(value);
    }
    else if constexpr ( has_exist_v<T> )
    {
        return exist(to_view(value));
    }
    else
    {
        return valid(to_view(value));
    }
}

template<class T>
using has_close_t = decltype( close(to_view(std::declval<T>())) );

template<class T>
constexpr bool has_close_v = is_detected_v<has_close_t, T>;

struct private_handle_t
{
    template <class T>
    friend class unique_handle;

    template <class T>
    friend class shared_handle;

private:
     constexpr private_handle_t() = default;
};

template<class T>
void close_handle(private_handle_t tag, const T& handle) noexcept
{
    if constexpr ( has_close_v<T> )
    {
        close(to_view(handle));
    }
    else
    {
        close(tag, to_view(handle));
    }
}

struct handle_construct_t {}; 

inline constexpr handle_construct_t handle_construct{};

template <class T>
class unique_handle
{
public:
    using handle_type = T;
    using view_type = view_t<handle_type>;

    constexpr unique_handle() noexcept
        : handle_{ handle_type{} } 
    {}

    constexpr unique_handle(nullptr_t) noexcept
        : unique_handle{}
    {}

    template<class... Args>
    constexpr unique_handle(handle_construct_t, Args&&... args) noexcept
        : handle_{ std::forward<Args>(args)... }
    {}

    constexpr unique_handle(unique_handle&& right) noexcept
        : handle_{ std::exchange(right.handle_, handle_type{}) }
    {}

    constexpr unique_handle(const unique_handle&) noexcept = delete;

    ~unique_handle() noexcept
    {
        close_handle(private_handle_t{}, handle_);
    }

    constexpr unique_handle& operator=(unique_handle&& right) noexcept
    {
        swap(right);
        return *this;
    }

    constexpr unique_handle& operator=(const unique_handle&) noexcept = delete;

    constexpr unique_handle& operator=(std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    constexpr void swap(unique_handle& right) noexcept
    {
        std::swap(handle_, right.handle_);
    }

    constexpr const handle_type* operator ->() const noexcept
    {
        return &handle_;
    }

    constexpr handle_type* operator ->() noexcept
    {
        return &handle_;
    }

    explicit constexpr operator bool() const noexcept
    {
        return to_bool(handle_);
    }

    constexpr operator view_type () const noexcept
    {
        return to_view(handle_);
    }

    void reset() noexcept
    {
        unique_handle temp{ std::move(*this) };
        (void) temp;
    }

    template<class T>
    friend class shared_handle;

private:
    handle_type handle_;
};

template <class T, class... Types>
constexpr unique_handle<T> make_unique_handle(Types&&... args) noexcept
{
    return 
    {
        handle_construct,
        std::forward<Types>(args)... 
    };
}

template<class T>
class shared_handle
{
private:
    using self = shared_handle;

public:
    using handle_type = T;
    using view_type = view_t<handle_type>;

    constexpr shared_handle() noexcept
        : shared_handle{ handle_construct, handle_type{} }
    {}

    constexpr shared_handle(nullptr_t) noexcept
        : shared_handle{}
    {}

    template<class... Args>
    constexpr shared_handle(handle_construct_t, Args&&... args) noexcept
        : handle_{ std::forward<Args>(args)... }
        , copies_{ this, this }
    {}

    constexpr shared_handle(const shared_handle& right) noexcept
        : handle_{ right.handle_ }
        , copies_{ push(&self::copies_, this, const_cast<shared_handle*>( &right )) }
    {}

    constexpr shared_handle(unique_handle<handle_type>&& right) noexcept
        : shared_handle{ handle_construct, std::exchange(right.handle_, handle_type{}) }
    {}

    ~shared_handle() noexcept
    {
        deattach_or_close();
    }

    shared_handle& operator = (const shared_handle& right) noexcept
    {
        if ( this != &right )
        {
            deattach_or_close();
            handle_ = right.handle_;
            copies_ = push(&self::copies_, this, const_cast<shared_handle*>( &right ));
        }

        return *this;
    }

    shared_handle& operator = (nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    constexpr const handle_type* operator ->() const noexcept
    {
        return &handle_;
    }

    explicit constexpr operator bool() const noexcept
    {
        return to_bool(handle_);
    }

    constexpr operator view_type () const noexcept
    {
        return to_view(handle_);
    }

    void reset() noexcept
    {
        deattach_or_close();
        handle_ = handle_type{};
    }

private:
    void deattach_or_close() const noexcept
    {
        if ( copies_.next != this )
        {
            pop(&self::copies_, copies_);
        }
        else
        {
            close_handle(private_handle_t{}, handle_);
        }
    }

private:
    handle_type handle_;
    intrusive_list_node<shared_handle> copies_;
};

template <class T, class... Args>
constexpr shared_handle<T> make_shared_handle(Args&&... args) noexcept
{
    return 
    {
        handle_construct,
        std::forward<Args>(args)...
    };
}


