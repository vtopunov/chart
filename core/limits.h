#pragma once

#include <limits>

#include <core/type_traits.h>


namespace private_detail_numeric_limits
{
    template<class T>
    struct numeric_max_factory
    {
        [[nodiscard]]
        static constexpr T create() noexcept
        {
            return std::numeric_limits<T>::max();
        }
    };

    template<class T>
    struct numeric_min_factory
    {
        [[nodiscard]]
        static constexpr T create() noexcept
        {
            return std::numeric_limits<T>::min();
        }
    };

    template<class T>
    struct numeric_eps_factory
    {
        [[nodiscard]]
        static constexpr T create() noexcept
        {
            return std::numeric_limits<T>::epsilon();
        }
    };

    template<class T>
    struct numeric_lowest_factory
    {
        [[nodiscard]]
        static constexpr T create() noexcept
        {
            return std::numeric_limits<T>::lowest();
        }
    };

    template<class T>
    struct numeric_nan_factory
    {
        [[nodiscard]]
        static constexpr T create() noexcept
        {
            return std::numeric_limits<T>::quiet_NaN();
        }
    };

    template<class T>
    struct numeric_inf_factory
    {
        [[nodiscard]]
        static constexpr T create() noexcept
        {
            return std::numeric_limits<T>::infinity();
        }
    };

    template<template<class> class Gen, class T>
    struct numeric_munis
    {
        [[nodiscard]]
        static constexpr T create() noexcept
        {
            return -(Gen<T>::create());
        }
    };

    template<class T>
    struct numeric_lowest_inf_factory : numeric_munis<numeric_inf_factory, T>
    {};

    template<class T>
    struct numeric_lowest_eps_factory : numeric_munis<numeric_eps_factory, T>
    {};
}

template<template<class> class Gen, class T>
struct numeric_deduction_factory : Gen<T>
{};

template<template<class> class C>
struct numeric_deduction_factory<C, void>
{
    struct deduction_value_type
    {
        template<class T, std::enable_if_t<std::negation_v<std::is_reference<T>>, int> = 0>
        constexpr operator T () const noexcept
        {
            return C<T>::create();
        }
    };

    [[nodiscard]]
    static constexpr deduction_value_type create() noexcept
    {
        return {};
    }
};


template<class T = void>
constexpr auto numeric_max_v = numeric_deduction_factory<private_detail_numeric_limits::numeric_max_factory, T>::create();

template<class T = void>
constexpr auto numeric_min_v = numeric_deduction_factory<private_detail_numeric_limits::numeric_min_factory, T>::create();

template<class T = void>
constexpr auto numeric_lowest_v = numeric_deduction_factory<private_detail_numeric_limits::numeric_lowest_factory, T>::create();

template<class T = void>
constexpr auto numeric_eps_v = numeric_deduction_factory<private_detail_numeric_limits::numeric_eps_factory, T>::create();

template<class T = void>
constexpr auto numeric_lowest_eps_v = numeric_deduction_factory<private_detail_numeric_limits::numeric_lowest_eps_factory, T>::create();

template<class T = void>
constexpr auto numeric_nan_v = numeric_deduction_factory<private_detail_numeric_limits::numeric_nan_factory, T>::create();

template<class T = void>
constexpr auto numeric_inf_v = numeric_deduction_factory<private_detail_numeric_limits::numeric_inf_factory, T>::create();

template<class T = void>
constexpr auto numeric_lowest_inf_v = numeric_deduction_factory<private_detail_numeric_limits::numeric_lowest_inf_factory, T>::create();

template<class T>
constexpr auto numeric_digits_v = std::numeric_limits<T>::digits;


template<class T>
using tr_numeric_digits = std::integral_constant<
    std::decay_t<decltype(numeric_digits_v<T>)>, 
    numeric_digits_v<T>
>;