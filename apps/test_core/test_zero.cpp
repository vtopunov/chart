#include <core/zero.h>

#include <cerrno>

#include <string>
#include <vector>

#include <core/rational.h>

using namespace rational_literals;


namespace
{
    template<class To, class From>
    constexpr To bit_cast(const From& from) noexcept
    {
        To to;
        std::memcpy(std::addressof(to), std::addressof(from), sizeof(to));
        return to;
    }

    struct test_zero_op
    {
        bool res{ false };
        size_t count{ 0 };

        bool operator () () noexcept
        {
            ++count;
            return res;
        }
    };

    struct test_zero_eq
    {
        inline static test_zero_op eq{};

        test_zero_eq() noexcept
        {}

        [[nodiscard]] bool operator == (const test_zero_eq&) const noexcept
        {
            return eq();
        }
    };

    struct test_zero_neq
    {
        inline static test_zero_op neq{};

        test_zero_neq() noexcept
        {}

        [[nodiscard]] bool operator != (const test_zero_neq&) const noexcept
        {
            return neq();
        }
    };

    struct test_zero_bool_op
    {
        inline static test_zero_op b_op{};

        operator bool() const noexcept
        {
            return b_op();
        }
    };

    struct test_zero_eq_neq
    {
        inline static test_zero_op eq{};
        inline static test_zero_op neq{};

        test_zero_eq_neq() noexcept
        {}

        [[nodiscard]] bool operator == (const test_zero_eq_neq&) const noexcept
        {
            return eq();
        }

        [[nodiscard]] bool operator != (const test_zero_eq_neq&) const noexcept
        {
            return neq();
        }
    };

    struct test_zero_bool_op_eq
    {
        inline static test_zero_op eq{};
        inline static test_zero_op b_op{};

        [[nodiscard]] bool operator == (const test_zero_eq_neq&) const noexcept
        {
            return eq();
        }

        operator bool() const noexcept
        {
            return b_op();
        }
    };

    struct test_zero_bool_op_neq
    {
        inline static test_zero_op neq{};
        inline static test_zero_op b_op{};

        [[nodiscard]] bool operator != (const test_zero_eq_neq&) const noexcept
        {
            return neq();
        }

        operator bool() const noexcept
        {
            return b_op();
        }
    };
}

void test_zero() noexcept
{
    {
        {
            test_zero_eq eq;
            D_ASSERT(!is_eqz(eq));
            D_ASSERT(is_neqz(eq));

            eq.eq.res = true;
            D_ASSERT(is_eqz(eq));
            D_ASSERT(!is_neqz(eq));
            D_ASSERT(4u == eq.eq.count);
        }

        {
            test_zero_neq neq;
            D_ASSERT(is_eqz(neq));
            D_ASSERT(!is_neqz(neq));

            neq.neq.res = true;
            D_ASSERT(!is_eqz(neq));
            D_ASSERT(is_neqz(neq));
            D_ASSERT(4u == neq.neq.count);
        }

        {
            test_zero_bool_op b_op;
            D_ASSERT(is_eqz(b_op));
            D_ASSERT(!is_neqz(b_op));

            b_op.b_op.res = true;
            D_ASSERT(!is_eqz(b_op));
            D_ASSERT(is_neqz(b_op));
            D_ASSERT(4u == b_op.b_op.count);
        }

        {
            test_zero_eq_neq v;
            D_ASSERT(!is_eqz(v));
            D_ASSERT((1u == v.eq.count) && (0u == v.neq.count));
            D_ASSERT(!is_neqz(v));
            D_ASSERT((1u == v.eq.count) && (1u == v.neq.count));

            v.eq.res = true;
            D_ASSERT(is_eqz(v));
            D_ASSERT((2u == v.eq.count) && (1u == v.neq.count));
            D_ASSERT(!is_neqz(v));
            D_ASSERT((2u == v.eq.count) && (2u == v.neq.count));

            v.neq.res = true;
            D_ASSERT(is_eqz(v));
            D_ASSERT((3u == v.eq.count) && (2u == v.neq.count));
            D_ASSERT(is_neqz(v));
            D_ASSERT((3u == v.eq.count) && (3u == v.neq.count));

            v.eq.res = false;
            D_ASSERT(!is_eqz(v));
            D_ASSERT((4u == v.eq.count) && (3u == v.neq.count));
            D_ASSERT(is_neqz(v));
            D_ASSERT((4u == v.eq.count) && (4u == v.neq.count));
        }

        {
            test_zero_bool_op_eq v;
            D_ASSERT(is_eqz(v));
            D_ASSERT(!is_neqz(v));

            v.eq.res = true;
            D_ASSERT(is_eqz(v));
            D_ASSERT(!is_neqz(v));

            v.b_op.res = true;
            D_ASSERT(!is_eqz(v));
            D_ASSERT(is_neqz(v));

            v.eq.res = false;
            D_ASSERT(!is_eqz(v));
            D_ASSERT(is_neqz(v));
            D_ASSERT(0u == v.eq.count);
            D_ASSERT(8u == v.b_op.count);
        }

        {
            test_zero_bool_op_neq v;
            D_ASSERT(is_eqz(v));
            D_ASSERT(!is_neqz(v));

            v.neq.res = true;
            D_ASSERT(is_eqz(v));
            D_ASSERT(!is_neqz(v));

            v.b_op.res = true;
            D_ASSERT(!is_eqz(v));
            D_ASSERT(is_neqz(v));

            v.neq.res = false;
            D_ASSERT(!is_eqz(v));
            D_ASSERT(is_neqz(v));
            D_ASSERT(0u == v.neq.count);
            D_ASSERT(8u == v.b_op.count);
        }

        {
            const std::string zs{}, nzs{ '0' };
            const std::vector<char> zv, nzv{ '0' };

            D_ASSERT(zs == zero_v<std::string>);
            D_ASSERT(zero_v<std::string> == zs);
            D_ASSERT(zv == zero_v<std::vector<char>>);
            D_ASSERT(zero_v<std::vector<char>> == zv);
            D_ASSERT(zs == zero_v<>);
            D_ASSERT(zero_v<> == zs);
            D_ASSERT(zv == zero_v<>);
            D_ASSERT(zero_v<> == zv);
            D_ASSERT(nzs != zero_v<std::string>);
            D_ASSERT(zero_v<std::string> != nzs);
            D_ASSERT(nzv != zero_v<std::vector<char>>);
            D_ASSERT(zero_v<std::vector<char>> != nzv);
            D_ASSERT(nzs != zero_v<>);
            D_ASSERT(zero_v<> != nzs);
            D_ASSERT(nzv != zero_v<>);
            D_ASSERT(zero_v<> != nzv);
        }
    }

    static_assert(constexpr_abs(0) == 0);
    static_assert(constexpr_abs(0u) == 0u);
    static_assert(constexpr_abs(0_uz) == 0_uz);
    static_assert(constexpr_abs(0.0f) == 0.0f);

    D_ASSERT(bit_cast<uint32_t>(0.0f) != bit_cast<uint32_t>(-0.0f));
    D_ASSERT(bit_cast<uint32_t>(constexpr_abs(-0.0f)) == bit_cast<uint32_t>(-0.0f));
    D_ASSERT(bit_cast<uint32_t>(constexpr_abs(0.0f)) == bit_cast<uint32_t>(0.0f));

    D_ASSERT(bit_cast<uint64_t>(0.0) != bit_cast<uint64_t>(-0.0));
    D_ASSERT(bit_cast<uint64_t>(constexpr_abs(-0.0)) == bit_cast<uint64_t>(-0.0));
    D_ASSERT(bit_cast<uint64_t>(constexpr_abs(0.0)) == bit_cast<uint64_t>(0.0));

    static_assert(constexpr_abs(numeric_max_v<uint32_t>) == numeric_max_v<uint32_t>);
    static_assert(constexpr_abs(numeric_max_v<int32_t>) == numeric_max_v<int32_t>);
    static_assert(constexpr_abs(-numeric_max_v<int32_t>) == numeric_max_v<int32_t>);

    static_assert(0 == zero_v<>);
    static_assert(0.0 == zero_v<>);
    static_assert(0_r == zero_v<>);
    static_assert(0_ur == zero_v<>);

    using zrational_t = rational<ptrdiff_t>;
    static_assert(0 == static_cast<zrational_t>(zero_v<>).num);
    static_assert(1 == static_cast<zrational_t>(zero_v<>).den);
    static_assert(0 == static_cast<zrational_t>(zero_v<zrational_t>).num);
    static_assert(1 == static_cast<zrational_t>(zero_v<zrational_t>).den);

    D_ASSERT(!errno);
}