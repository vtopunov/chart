#pragma once

#include <core/vec.h>
#include <core/num_range.h>
#include <core/point.h>
#include <core/polynom.h>

template< class To, class From>
constexpr auto lerp( const vec<From>& from, const vec<To>& to ) noexcept
{
    D_ASSERT( from._0 != from._1 );

    const auto scaling = ( to._1 - to._0 ) / ( from._1 - from._0 );
    const auto offset = ( to._0 * from._1 - to._1 * from._0 ) / ( from._1 - from._0 );

    return make_polynom<To, From>( offset, scaling );
}

template< class To, class From>
constexpr auto lerp( const num_range<From>& from, const num_range<To>& to ) noexcept
{
    return lerp(from.bounds, to.bounds);
}

template<class T>
constexpr polynom<T> lerp( const point<T>& p0, const point<T>& p1 ) noexcept
{
    return lerp( make_vec( p0.x(), p1.x() ), make_vec( p0.y(), p1.y() ) );
}