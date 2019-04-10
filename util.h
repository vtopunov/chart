#pragma once

#include <cstddef>
#include <cassert>

template<class Target, class Source>
constexpr Target narrow_cast(Source v) noexcept
{
#pragma warning(push)
#pragma warning(disable : 26472) //  Don't use a static_cast for arithmetic conversions. Use brace initialization, narrow_cast or narrow
    auto r = static_cast<Target>(v);
    assert(static_cast<Source>(r) == v);
    return r;
#pragma warning(pop)
}

constexpr std::size_t operator "" _z(unsigned long long n)
{
    return n;
}

#define DISABLE_COPY(Class) \
    Class(const Class&) = delete;\
    Class &operator=(const Class&) = delete;

#define DISABLE_MOVE(Class) \
    Class(Class&&) = delete; \
    Class &operator=(Class&&) = delete;

#define DISABLE_COPY_MOVE(Class) \
    DISABLE_COPY(Class) \
    DISABLE_MOVE(Class)
