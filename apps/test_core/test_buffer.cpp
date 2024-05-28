#include <array>

#include <core/buffer.h>
#include <core/buffer_view.h>


void test_buffer() noexcept
{
    static_assert(1_uz == byte_buffer::element_size);
    static_assert(1_uz == sizeof(decl_value_type_t<byte_buffer>));
    static_assert(std::is_same_v<decl_value_type_t<byte_buffer>, byte_buffer::value_type>);
    static_assert(std::is_same_v<decl_view_type_t<byte_buffer>, byte_buffer::view_type>);
    static_assert(std::is_same_v<decl_null_type_t<byte_buffer>, byte_buffer::null_type>);
    static_assert(std::is_same_v<decl_null_type_t<byte_buffer>, nullmem_t>);
    static_assert(!std::is_copy_constructible_v<byte_buffer>);
    static_assert(!std::is_copy_assignable_v<byte_buffer>);
    static_assert(std::is_move_constructible_v<byte_buffer>);
    static_assert(std::is_move_assignable_v<byte_buffer>);


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
    D_ASSERT(std::addressof(b.back()) == (b.data() + size - 1u));

    static_assert(std::is_same_v<decltype(std::as_const(b).back()), const type_t&>);
    D_ASSERT(std::addressof(std::as_const(b).back()) == (b.data() + size - 1u));

    static_assert(std::is_same_v<decltype(b.cback()), const type_t&>);
    D_ASSERT(std::addressof(b.cback()) == (b.data() + size - 1u));

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

    {
        {
            using span_t = span<type_t>;
            using cspan_t = span<const type_t>;
            static_assert(!std::is_same_v<span_t, cspan_t>);
            const span_t sp{ b };
            const cspan_t csp{ sp };
            const auto bsp = b.as_span();
            const auto cbcsp = std::as_const(b).as_span();
            static_assert(std::is_same_v<decltype(sp), decltype(bsp)>);
            static_assert(std::is_same_v<decltype(csp), decltype(cbcsp)>);
            D_ASSERT(sp == bsp);
            D_ASSERT(csp == cbcsp);
        }

        {
            using mbyte_t = std::array<std::byte, sizeof(type_t)>;
            static_assert(!std::is_same_v<type_t, mbyte_t>);
            static_assert(sizeof(type_t) == sizeof(mbyte_t));
            using mbspan_t = span<mbyte_t>;
            using cmbspan_t = span<const mbyte_t>;
            const mbspan_t mbsp{ static_cast<mbyte_t*>(b.void_data()), b.size() };
            const cmbspan_t cmbsp{ mbsp };
            const auto bmbsp = b.as_span<mbyte_t>();
            const auto bcmbsp = b.as_span<const mbyte_t>();
            const auto cbmbsp = std::as_const(b).as_span<mbyte_t>();
            const auto cbcmbsp = std::as_const(b).as_span<const mbyte_t>();

            static_assert(std::is_same_v<decltype(mbsp), decltype(bmbsp)>);
            static_assert(std::is_same_v<decltype(cmbsp), decltype(cbmbsp)>);
            static_assert(std::is_same_v<decltype(cmbsp), decltype(cbcmbsp)>);
            static_assert(std::is_same_v<decltype(cmbsp), decltype(bcmbsp)>);
            D_ASSERT(mbsp == bmbsp);
            D_ASSERT(cmbsp == bcmbsp);
            D_ASSERT(cmbsp == cbmbsp);
            D_ASSERT(cmbsp == cbcmbsp);
        }

        {
            using mbyte_t = std::array<std::byte, sizeof(type_t) / 2u>;
            static_assert(!std::is_same_v<type_t, mbyte_t>);
            static_assert(1u < sizeof(mbyte_t));
            static_assert(sizeof(type_t) == sizeof(mbyte_t) * 2u);

            using mbspan_t = span<mbyte_t>;
            using cmbspan_t = span<const mbyte_t>;
            const mbspan_t mbsp{ static_cast<mbyte_t*>(b.void_data()), 2u * b.size() };
            const cmbspan_t cmbsp{ mbsp };
            const auto bmbsp = b.as_span<mbyte_t>();
            const auto bcmbsp = b.as_span<const mbyte_t>();
            const auto cbmbsp = std::as_const(b).as_span<mbyte_t>();
            const auto cbcmbsp = std::as_const(b).as_span<const mbyte_t>();

            static_assert(std::is_same_v<decltype(mbsp), decltype(bmbsp)>);
            static_assert(std::is_same_v<decltype(cmbsp), decltype(cbmbsp)>);
            static_assert(std::is_same_v<decltype(cmbsp), decltype(cbcmbsp)>);
            static_assert(std::is_same_v<decltype(cmbsp), decltype(bcmbsp)>);
            D_ASSERT(mbsp == bmbsp);
            D_ASSERT(cmbsp == bcmbsp);
            D_ASSERT(cmbsp == cbmbsp);
            D_ASSERT(cmbsp == cbcmbsp);
        }

        {
            constexpr auto size_bytes = size_mul<sizeof(type_t)>(size);
            using mbyte_t = std::array<std::byte, 2u * sizeof(type_t)>;
            static_assert(!std::is_same_v<type_t, mbyte_t>);
            static_assert(size_bytes > sizeof(mbyte_t));
            static_assert(!(size_bytes % sizeof(mbyte_t)));

            using mbspan_t = span<mbyte_t>;
            using cmbspan_t = span<const mbyte_t>;
            const mbspan_t mbsp{ static_cast<mbyte_t*>(b.void_data()), b.size() / 2u };
            const cmbspan_t cmbsp{ mbsp };
            const auto bmbsp = b.as_span<mbyte_t>();
            const auto bcmbsp = b.as_span<const mbyte_t>();
            const auto cbmbsp = std::as_const(b).as_span<mbyte_t>();
            const auto cbcmbsp = std::as_const(b).as_span<const mbyte_t>();

            static_assert(std::is_same_v<decltype(mbsp), decltype(bmbsp)>);
            static_assert(std::is_same_v<decltype(cmbsp), decltype(cbmbsp)>);
            static_assert(std::is_same_v<decltype(cmbsp), decltype(cbcmbsp)>);
            static_assert(std::is_same_v<decltype(cmbsp), decltype(bcmbsp)>);
            D_ASSERT(mbsp == bmbsp);
            D_ASSERT(cmbsp == bcmbsp);
            D_ASSERT(cmbsp == cbmbsp);
            D_ASSERT(cmbsp == cbcmbsp);
            D_ASSERT(size_bytes == b.size_bytes());
            D_ASSERT(size_bytes == ::size_bytes(mbsp));
            D_ASSERT(size_bytes == ::size_bytes(cmbsp));
        }
    }

    {
        using bytes_t = span<std::byte>;
        using cbytes_t = span<const std::byte>;

        const bytes_t bsp{ static_cast<std::byte*>(b.void_data()), b.size_bytes() };
        const cbytes_t cbsp{ bsp };
        const auto bbsp = b.as_bytes();
        const auto cbbsp = std::as_const(b).as_bytes();
        static_assert(std::is_same_v<decltype(bsp), decltype(bbsp)>);
        static_assert(std::is_same_v<decltype(cbsp), decltype(cbbsp)>);
        D_ASSERT(bsp == bbsp);
        D_ASSERT(cbsp == cbbsp);
    }
}
