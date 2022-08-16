#pragma once


namespace ordered_overload
{
    struct _3
    {};

    struct _2 : _3
    {};

    struct _1 : _2
    {};

    struct _0 : _1
    {};

    template<class T>
    using _overload = const T* const;

    template<class T>
    using _order = _overload<T>;

    template<class T>
    constexpr _overload<T> _overload_v{ nullptr };

    constexpr _order<_0> _start{ nullptr };
}

struct no_overloaded
{
    template<class T>
    constexpr no_overloaded(const T&) noexcept
    {}
};

template<class T>
struct no_overload_for
{
    constexpr no_overload_for(const T&) noexcept
    {}
};