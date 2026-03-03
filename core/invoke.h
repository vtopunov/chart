#pragma once

#include <core/utility.h>
#include <core/null.h>


template<class Fn, class... Args>
constexpr auto invoke(Fn&& fn, Args&&... args) noexcept -> decltype(std::forward<Fn>(fn)(std::forward<Args>(args)...))
{
    return std::forward<Fn>(fn)(std::forward<Args>(args)...);
}

template<class Fn, class... Args>
using decl_invoke_decl_result_t = decltype(::invoke(std::declval<Fn>(), std::declval<Args>()...));

template<class Fn, class... Args>
using decl_const_invoke_decl_result_t = decltype(::invoke(std::as_const(std::declval<Fn>()), std::declval<Args>()...));

template<class Fn, class... Args>
using is_invocable_decl = is_detected<decl_invoke_decl_result_t, Fn, Args...>;

template<class Fn, class... Args>
using is_const_invocable_decl = is_detected<decl_const_invoke_decl_result_t, Fn, Args...>;

template<class Fn, class... Args>
constexpr bool is_invocable_decl_v = is_invocable_decl<Fn, Args...>::value;

template<class Fn, class... Args>
constexpr bool is_const_invocable_decl_v = is_const_invocable_decl<Fn, Args...>::value;


struct invalid_invoke_t
{
    static constexpr struct {} tag{};
    constexpr explicit invalid_invoke_t(decltype(tag)) noexcept {}
};

constexpr invalid_invoke_t invalid_invoke{ invalid_invoke_t::tag };

struct no_invocable_t : invalid_invoke_t
{
    static constexpr struct {} tag{};
    constexpr explicit no_invocable_t(decltype(tag)) noexcept
        : invalid_invoke_t{ invalid_invoke }
    {}
};

constexpr no_invocable_t no_invocable{ no_invocable_t::tag };

struct no_convertible_t : invalid_invoke_t
{
    static constexpr struct {} tag{};
    constexpr explicit no_convertible_t(decltype(tag)) noexcept
        : invalid_invoke_t{ invalid_invoke }
    {}
};

constexpr no_convertible_t no_convertible{ no_convertible_t::tag };


template<class Fn, class... Args>
constexpr decltype(auto) invoke_if_exist(Fn&& fn, Args&&... args) noexcept
{
    if constexpr (is_invocable_decl_v<Fn, Args...>)
    {
        return ::invoke(std::forward<Fn>(fn), std::forward<Args>(args)...);
    }
    else
    {
        if constexpr (is_const_invocable_decl_v<Fn, Args...>)
        {
            return ::invoke(std::as_const(std::forward<Fn>(fn)), std::forward<Args>(args)...);
        }
        else
        {
            if constexpr (!std::is_lvalue_reference_v<Fn>)
            {
                return ::invoke_if_exist(::as_lref(fn), std::forward<Args>(args)...);
            }
            else
            {
                return no_invocable;
            }
        }
    }
}

template<class Fn, class... Args>
using invoke_if_exist_result_t = decltype(::invoke_if_exist(std::declval<Fn>(), std::declval<Args>()...));

template<class R>
using is_invalid_invoke_result = std::is_base_of<::invalid_invoke_t, R>;

template<class R>
constexpr bool is_invalid_invoke_result_v = ::is_invalid_invoke_result<R>::value;

template<class Fn, class... Args>
using is_invocable = std::negation<::is_invalid_invoke_result<::invoke_if_exist_result_t<Fn, Args...>>>;

template<class Fn, class... Args>
constexpr bool is_invocable_v = ::is_invocable<Fn, Args...>::value;


struct make_no_invocable
{
    constexpr no_invocable_t operator () () const noexcept
    {
        return no_invocable;
    }
};

struct make_no_convertible
{
    constexpr no_convertible_t operator () () const noexcept
    {
        return no_convertible;
    }
};

struct make_no_convertible_for
{
    constexpr no_convertible_t operator () (no_overload) const noexcept
    {
        return no_convertible;
    }
};

template
<
    class R,
    class DefaultNoInvocable = make_no_invocable,
    class DefaultNoConvertibleVoid = make_no_convertible,
    class DefaultNoConvertible = make_no_convertible_for
>
struct invoke_if_exist_r_t
{
    D_NO_UNIQUE_ADDRESS DefaultNoInvocable default_no_invocable;
    D_NO_UNIQUE_ADDRESS DefaultNoConvertibleVoid default_no_convertible_void;
    D_NO_UNIQUE_ADDRESS DefaultNoConvertible default_no_convertible;

    template<class Fn, class... Args>
    constexpr decltype(auto) operator () (Fn&& fn, Args&&... args) const noexcept
    {
        using invoke_R = ::invoke_if_exist_result_t<Fn, Args...>;

        if constexpr (is_invalid_invoke_result_v<invoke_R>)
        {
            return default_no_invocable();
        }
        else
        {
            if constexpr (std::is_void_v<R>)
            {
                ::invoke_if_exist(std::forward<Fn>(fn), std::forward<Args>(args)...);
            }
            else
            {
                if constexpr (std::is_constructible_v<R, invoke_R>)
                {
                    return R(::invoke_if_exist(std::forward<Fn>(fn), std::forward<Args>(args)...));
                }
                else
                {
                    if constexpr (std::is_void_v<invoke_R>)
                    {
                        ::invoke_if_exist(std::forward<Fn>(fn), std::forward<Args>(args)...);
                        return default_no_convertible_void();
                    }
                    else
                    {
                        return default_no_convertible(::invoke_if_exist(std::forward<Fn>(fn), std::forward<Args>(args)...));
                    }
                }
            }
        }
    }
};

template<class R>
constexpr invoke_if_exist_r_t<R> invoke_if_exist_r{};

template<class R, class MakeNull = instance_null_t<null_t<R> > >
constexpr invoke_if_exist_r_t<R, MakeNull, MakeNull> invoke_if_exist_r_or_null{};

template<class Fn, class R, class... Args>
using invoke_r_result_t = decltype(invoke_if_exist_r<R>(std::declval<Fn>(), std::declval<Args>()...));

template<class Fn, class R, class... Args>
using is_invocable_r = std::negation<is_invalid_invoke_result<invoke_r_result_t<Fn, R, Args...>>>;

template<class Fn, class R, class... Args>
constexpr bool is_invocable_r_v = is_invocable_r<Fn, R, Args...>::value;