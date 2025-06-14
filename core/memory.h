#pragma once

#include <memory>

#include <core/span.h>


template<class T>
[[nodiscard]] constexpr const T* u_nextmem(const T* mem, size_t size) noexcept
{
    D_ASSERT_OR_ASSUME(is_safe_narrowing_conversion<ptrdiff_t>(size));
    const auto result = mem + size;
    D_ASSERT_OR_ASSUME(result >= mem);
    return result;
}

template<class T>
[[nodiscard]] constexpr const T* u_prevmem(const T* mem, size_t size) noexcept
{
    const auto result = mem - narrow<ptrdiff_t>(size);
    D_ASSERT_OR_ASSUME(result <= mem);
    return result;
}

template<class T>
[[nodiscard]] constexpr bool is_newmem(const T* newmem, const T* oldmem, size_t size) noexcept
{
    return (newmem >= u_nextmem(oldmem, size))
        || (oldmem >= u_nextmem(newmem, size));
}

template<class T>
[[nodiscard]] constexpr bool is_newmem(const T* newmem, const T* oldmem, const T* oldmem_end) noexcept
{
    return (newmem >= oldmem_end)
        || (oldmem >= u_nextmem(newmem, u_distance(oldmem, oldmem_end)));
}

template<class T, size_t Extent>
T* copynew(span<const T, Extent> sp, T* out) noexcept
{
    D_ASSERT_OR_ASSUME(is_newmem(out, ::cdata(sp), std::size(sp)));
    return std::uninitialized_copy_n(::cdata(sp), std::size(sp), out);
}

template<class T, size_t Extent>
T* movenew(span<T, Extent> sp, T* out) noexcept
{
    D_ASSERT_OR_ASSUME(is_newmem(out, ::cdata(sp), std::size(sp)));
    return std::uninitialized_move_n(std::data(sp), std::size(sp), out).second;
}

template<class T>
T* movenew(T* first, T* last, T* out) noexcept
{
    D_ASSERT_OR_ASSUME(is_newmem(out, first, last));
    return std::uninitialized_move(first, last, out);
}