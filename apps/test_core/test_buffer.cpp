#include <array>
#include <random>

#include <core/buffer.h>
#include <core/view.h>

namespace
{
    template<size_t Size0, size_t Size1>
    void test_interpret() noexcept
    {
        enum class byte0 : uint8_t {};
        enum class byte1 : uint8_t {};
        static_assert(1_uz == sizeof(byte0));
        static_assert(1_uz == sizeof(byte1));

        using mbyte0_t = std::array<byte0, Size0>;
        using mbyte_t = std::array<byte1, Size1>;
        using buffer_type = buffer<mbyte0_t>;
        static_assert(std::is_same_v<decl_value_type_t<buffer_type>, typename buffer_type::value_type>);
        static_assert(std::is_same_v<decl_value_type_t<buffer_type>, mbyte0_t>);

        std::random_device entropy{};
        std::mt19937_64 random_engine_64{ entropy() };
        buffer_type b{ std::uniform_int_distribution<size_t>{ 3_uz * Size0, 7_uz * Size0 }(random_engine_64) };

        {
            const auto end = static_cast<const std::byte*>(b.cvoid_data()) + size_bytes(b);
            for (size_t i = 0; i < 3; ++i )
            {
                zero_memory(b);
                for (auto it = static_cast<std::byte*>(b.void_data()); it != end; )
                {
                    const auto nbytes = narrow<size_t>(end - it);
                    const auto random_value = random_engine_64();
                    const auto nwbytes = std::min(sizeof(random_value), nbytes);

                    {
                        constexpr std::remove_reference_t<decltype(random_value)> zeros{};
                        D_ASSERT(!memcmp(it, std::addressof(zeros), nwbytes));
                    }

                    memcpy(it, std::addressof(random_value), nwbytes);
                    it += nwbytes;
                }
            }
        }

        using mbview_t = basic_buffer_view<mbyte_t>;
        using cmbview_t = basic_buffer_view<const mbyte_t>;
        D_ASSERT(sizeof(mbyte0_t) * b.size() == size_bytes(b));
        const mbview_t sp{ memory_construct, b.void_data(), size_bytes(b) };
        const cmbview_t csp{ sp };

        {
            const auto bsp = interpret<mbyte_t>(view(b));
            static_assert(std::is_same_v<decltype(sp), decltype(bsp)>);
            D_ASSERT(sp == bsp);
        }

        {
            const auto bsp = interpret<mbyte_t>(view(std::as_const(b)));
            static_assert(std::is_same_v<decltype(sp), decltype(bsp)>);
            D_ASSERT(sp == bsp);
        }

        {
            const auto bsp = interpret<mbyte_t>(cview(b));
            static_assert(std::is_same_v<decltype(csp), decltype(bsp)>);
            D_ASSERT(csp == bsp);
        }

        {
            const auto bsp = interpret<const mbyte_t>(view(b));
            static_assert(std::is_same_v<decltype(csp), decltype(bsp)>);
            D_ASSERT(csp == bsp);
        }

        {
            const auto bsp = interpret<mbyte_t>(view(b).as_const());
            static_assert(std::is_same_v<decltype(csp), decltype(bsp)>);
            D_ASSERT(csp == bsp);
        }

        {
            const auto bsp = interpret<const mbyte_t>(cview(std::as_const(b)).as_const());
            static_assert(std::is_same_v<decltype(csp), decltype(bsp)>);
            D_ASSERT(csp == bsp);
        }
    }
}

void test_buffer() noexcept
{
    static_assert(1_uz == byte_buffer::element_size);
    static_assert(1_uz == sizeof(decl_value_type_t<byte_buffer>));
    static_assert(std::is_same_v<decl_value_type_t<byte_buffer>, byte_buffer::value_type>);
    static_assert(std::is_same_v<decl_view_type_t<byte_buffer>, byte_buffer::view_type>);
    static_assert(std::is_same_v<decl_null_type_t<byte_buffer>, byte_buffer::null_type>);
    static_assert(std::is_same_v<decl_null_type_t<byte_buffer>, nullmem_t>);
    static_assert(std::is_same_v<size_t, byte_buffer::size_type>);
    static_assert(std::is_nothrow_default_constructible_v<byte_buffer>);
    static_assert(std::is_nothrow_constructible_v<byte_buffer, size_t>);
    static_assert(std::is_nothrow_move_constructible_v<byte_buffer>);
    static_assert(std::is_nothrow_move_assignable_v<byte_buffer>);
    static_assert(!std::is_copy_constructible_v<byte_buffer>);
    static_assert(!std::is_copy_assignable_v<byte_buffer>);


    using type_t = int32_t;
    constexpr auto size = 10_uz;
    buffer<type_t> b{ size };

    {
        type_t value_gen{ 0 };
        for (auto& value : b)
        {
            value = ++value_gen;
        }
    }

    D_ASSERT(size <= b.size());
    D_ASSERT(b.size() * sizeof(type_t) == size_bytes(b));

    static_assert(std::is_same_v<decltype(b.void_data()), void*>);
    static_assert(std::is_same_v<decltype(b.cvoid_data()), const void*>);
    D_ASSERT(b.cvoid_data() == b.void_data());

    static_assert(std::is_same_v<decltype(std::as_const(b).void_data()), void*>);
    D_ASSERT(std::as_const(b).void_data() == b.void_data());

    static_assert(std::is_same_v<decltype(b.data()), type_t*>);
    D_ASSERT(b.data() == b.void_data());

    static_assert(std::is_same_v<decltype(std::as_const(b).data()), type_t*>);
    static_assert(std::is_same_v<decltype(b.cdata()), const type_t*>);
    D_ASSERT(std::as_const(b).data() == b.void_data());
    D_ASSERT(b.cdata() == b.void_data());

    static_assert(std::is_same_v<decltype(b.front()), type_t&>);
    D_ASSERT(std::addressof(b.front()) == b.data());

    static_assert(std::is_same_v<decltype(std::as_const(b).front()), type_t&>);
    static_assert(std::is_same_v<decltype(b.cfront()), const type_t&>);
    D_ASSERT(std::addressof(std::as_const(b).front()) == b.data());
    D_ASSERT(std::addressof(b.cfront()) == b.data());

    static_assert(std::is_same_v<decltype(b.cfront()), const type_t&>);
    D_ASSERT(std::addressof(b.cfront()) == b.data());

    static_assert(std::is_same_v<decltype(b.back()), type_t&>);
    D_ASSERT(std::addressof(b.back()) == (b.data() + b.size() - 1u));

    static_assert(std::is_same_v<decltype(std::as_const(b).back()), type_t&>);
    static_assert(std::is_same_v<decltype(b.cback()), const type_t&>);
    D_ASSERT(std::addressof(std::as_const(b).back()) == (b.data() + b.size() - 1u));
    D_ASSERT(std::addressof(b.cback()) == (b.data() + b.size() - 1u));

    static_assert(std::is_same_v<decltype(b.cback()), const type_t&>);
    D_ASSERT(std::addressof(b.cback()) == (b.data() + b.size() - 1u));

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
        const auto b_size = b.size();
        D_ASSERT(data);
        buffer<type_t> new_b{ std::move(b) };
        D_ASSERT(data == new_b.data());
        D_ASSERT(b_size == new_b.size());
        D_ASSERT(!b.data());
        D_ASSERT(!b.size());
        b = std::move(new_b);
        D_ASSERT(data == b.data());
        D_ASSERT(b_size == b.size());
        D_ASSERT(!new_b.data());
        D_ASSERT(!new_b.size());
    }

    {
        byte_buffer_view bv{ b };
        D_ASSERT(bv.data());
        D_ASSERT(bv.void_data() == b.void_data());
        D_ASSERT(bv.void_data() == b.cvoid_data());
        D_ASSERT(bv.cvoid_data() == b.cvoid_data());
        D_ASSERT(b.data() == static_cast<const type_t*>(b.cvoid_data()));
        D_ASSERT(bv.data() == static_cast<const std::byte*>(b.cvoid_data()));
        D_ASSERT(b.cdata() == static_cast<const type_t*>(b.cvoid_data()));
        D_ASSERT(bv.cdata() == static_cast<const std::byte*>(b.cvoid_data()));

        D_ASSERT(bv.size());
        D_ASSERT(bv.size() == size_bytes(b));
        bv = {};
        D_ASSERT(!bv.data());
        D_ASSERT(!bv.size());
        bv = b;
        D_ASSERT(bv.data());
        D_ASSERT(bv.cvoid_data() == b.void_data());
        D_ASSERT(bv.size());
        D_ASSERT(bv.size() == size_bytes(b));
    }

    {
        const_byte_buffer_view bv{ b };
        D_ASSERT(bv.data());
        D_ASSERT(bv.cvoid_data() == b.cvoid_data());
        D_ASSERT(bv.size());
        D_ASSERT(bv.size() == size_bytes(b));
        bv = {};
        D_ASSERT(!bv.data());
        D_ASSERT(!bv.size());
        bv = b;
        D_ASSERT(bv.data());
        D_ASSERT(bv.cvoid_data() == b.cvoid_data());
        D_ASSERT(bv.size());
        D_ASSERT(bv.size() == size_bytes(b));
    }


    {
        using span_t = basic_buffer_view<type_t>;
        using cspan_t = basic_buffer_view<const type_t>;
        static_assert(!std::is_same_v<span_t, cspan_t>);
        const span_t sp{ b };
        const cspan_t csp{ sp };
        const auto bsp = view(b);
        const auto cbsp = view(std::as_const(b));
        const auto bcsp = cview(b);
        const auto cbcsp = cview(std::as_const(b));
        static_assert(std::is_same_v<decltype(sp), decltype(bsp)>);
        static_assert(std::is_same_v<decltype(sp), decltype(cbsp)>);
        static_assert(std::is_same_v<decltype(csp), decltype(bcsp)>);
        static_assert(std::is_same_v<decltype(csp), decltype(cbcsp)>);
        D_ASSERT(sp == bsp);
        D_ASSERT(sp == cbsp);
        D_ASSERT(csp == bcsp);
        D_ASSERT(csp == cbcsp);
    }

    test_interpret<4_uz, 4_uz>();
    test_interpret<8_uz, 4_uz>();
    test_interpret<4_uz, 8_uz>();
    test_interpret<4_uz, 1_uz>();
    test_interpret<1_uz, 4_uz>();
    test_interpret<5_uz, 11_uz>();
    test_interpret<11_uz, 5_uz>();
}
