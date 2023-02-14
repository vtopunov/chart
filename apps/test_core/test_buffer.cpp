#include <array>

#include <core/buffer.h>
#include <core/buffer_view.h>

void test_buffer() noexcept
{
    using type_t = int32_t;
    constexpr auto size = 10_uz;
    buffer<type_t> b{ buffer_construct, size };
    
    {
        type_t value_gen{ 0 };
        for (auto& value : b)
        {
            value = ++value_gen;
        }
    }

    D_ASSERT(size == b.size());
    D_ASSERT(size * sizeof(type_t) == b.size_bytes());
    
    static_assert(std::is_same_v<decltype(b.void_data()), void*>);
    static_assert(std::is_same_v<decltype(b.cvoid_data()), const void*>);
    D_ASSERT(b.cvoid_data() == b.void_data());

    static_assert(std::is_same_v<decltype(std::as_const(b).void_data()), const void*>);
    D_ASSERT(std::as_const(b).void_data() == b.void_data());
    
    static_assert(std::is_same_v<decltype(b.data()), type_t*>);
    D_ASSERT(b.data() == b.void_data());

    static_assert(std::is_same_v<decltype(std::as_const(b).data()), const type_t*>);
    D_ASSERT(std::as_const(b).data() == b.void_data());

    static_assert(std::is_same_v<decltype(b.front()), type_t&>);
    D_ASSERT(std::addressof(b.front()) == b.data());

    static_assert(std::is_same_v<decltype(std::as_const(b).front()), const type_t&>);
    D_ASSERT(std::addressof(std::as_const(b).front()) == b.data());

    static_assert(std::is_same_v<decltype(b.cfront()), const type_t&>);
    D_ASSERT(std::addressof(b.cfront()) == b.data());

    static_assert(std::is_same_v<decltype(b.back()), type_t&>);
    D_ASSERT(std::addressof(b.back()) == (b.data() + size - 1));

    static_assert(std::is_same_v<decltype(std::as_const(b).back()), const type_t&>);
    D_ASSERT(std::addressof(std::as_const(b).back()) == (b.data() + size - 1));

    static_assert(std::is_same_v<decltype(b.cback()), const type_t&>);
    D_ASSERT(std::addressof(b.cback()) == (b.data() + size - 1));

    D_ASSERT(b.front() == 1);
    D_ASSERT(b.back() == b.size());

    {
        constexpr type_t c{ 123 };
        b.front() = c;
        D_ASSERT(c == b.front());
    }

    {
        constexpr type_t c{ 321 };
        b.front() = c;
        D_ASSERT(c == b.front());
    }

    {
        constexpr type_t c{ 231 };
        constexpr auto pos = size >> 1;
        D_ASSERT((pos + 1) == b[pos]);
        b[pos] = c;
        D_ASSERT(c == b[pos]);
        D_ASSERT(c == std::as_const(b)[pos]);
        D_ASSERT(c == b.value(pos));
        D_ASSERT(c == std::as_const(b).value(pos));
        D_ASSERT(c == b.cvalue(pos));
    }

    {
        const auto data = b.data();
        D_ASSERT(data);
        buffer<type_t> new_b{ std::move(b) };
        D_ASSERT(data == new_b.data());
        D_ASSERT(size == new_b.size());
        D_ASSERT(!b.data());
        D_ASSERT(!b.size());
        b = std::move(new_b);
    }

    {
        buffer_view bv{ b };
        D_ASSERT(bv.data());
        D_ASSERT(bv.data() == b.data());
        D_ASSERT(bv.size());
        D_ASSERT(bv.size() == b.size_bytes());
        bv = {};
        D_ASSERT(!bv.data());
        D_ASSERT(!bv.size());
        bv = b;
        D_ASSERT(bv.data());
        D_ASSERT(bv.data() == b.data());
        D_ASSERT(bv.size());
        D_ASSERT(bv.size() == b.size_bytes());
    }

    {
        const_buffer_view bv{ b };
        D_ASSERT(bv.data());
        D_ASSERT(bv.data() == b.data());
        D_ASSERT(bv.size());
        D_ASSERT(bv.size() == b.size_bytes());
        bv = {};
        D_ASSERT(!bv.data());
        D_ASSERT(!bv.size());
        bv = b;
        D_ASSERT(bv.data());
        D_ASSERT(bv.data() == b.data());
        D_ASSERT(bv.size());
        D_ASSERT(bv.size() == b.size_bytes());
    }
}
