#pragma once

#include <core/vec.h>
#include <core/numerical_range.h>
#include <core/point.h>
#include <core/polynom.h>

template< class To, class From>
constexpr auto lerp( vec<From> from, vec<To> to ) noexcept
{
    assert( from._0 != from._1 );

    const auto scaling = ( to._1 - to._0 ) / ( from._1 - from._0 );
    const auto offset = ( to._0 * from._1 - to._1 * from._0 ) / ( from._1 - from._0 );

    return make_polynom<To, From>( offset, scaling );
}

template< class To, class From>
constexpr auto lerp( numerical_range<From> from, numerical_range<To> to ) noexcept
{
    return lerp(from.bounds, to.bounds);
}

template<class T>
constexpr polynom<T> lerp( point<T> p0, point<T> p1 ) noexcept
{
    return lerp( vec<T>{ p0.x(), p1.x() }, vec<T>{ p0.y(), p1.y() } );
}