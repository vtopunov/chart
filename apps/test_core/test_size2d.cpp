
#include<core/size2d.h>


namespace
{
    namespace private_detail_test_sizes
    {
        using size2d_t = size2d<int>;

        static constexpr size2d_t testconst_sizes_o{ 1, 2 };
        static constexpr size2d_t testconst_sizes_mem_fn{ 3, 4 };
        static constexpr size2d_t testconst_sizes_mem{ 5, 6 };

        constexpr struct sizes_o : size2d_t
        {} sizes_o_v{ testconst_sizes_o };

        constexpr struct sizes_mem_fn
        {
            [[nodiscard]] constexpr sizes_o sizes() const noexcept
            {
                return { testconst_sizes_mem_fn };
            }
        } sizes_mem_fn_v{};

        constexpr struct sizes_mem
        {
            sizes_o sizes;
        } sizes_mem_v{ testconst_sizes_mem };


        constexpr struct sizes_mem_fn_o : sizes_o
        {
            [[nodiscard]] constexpr sizes_o sizes() const noexcept
            {
                return { testconst_sizes_mem_fn };
            }
        } sizes_mem_fn_o_v{ sizes_o_v };

        constexpr struct sizes_mem_o : sizes_o
        {
            sizes_o sizes;
        } sizes_mem_o_v{ .sizes{ testconst_sizes_mem } };

        constexpr struct sizes_mem_view
        {
            using view_type = sizes_mem;

            constexpr operator view_type () const noexcept
            {
                return sizes_mem_v;
            }
        } sizes_mem_view_v{};

        constexpr struct sizes_mem_view_o : sizes_o
        {
            using view_type = sizes_mem;

            constexpr operator view_type () const noexcept
            {
                return sizes_mem_v;
            }
        } sizes_mem_view_o_v{ sizes_o_v };

        constexpr struct sizes_mem_fn_res_mem_view
        {
            using view_type = sizes_mem;
            using resource_type = sizes_mem_fn;

            constexpr operator view_type () const noexcept
            {
                return sizes_mem_v;
            }

            constexpr operator resource_type () const noexcept
            {
                return sizes_mem_fn_v;
            }
        } sizes_mem_fn_res_mem_view_v{};


        constexpr struct sizes_res_mem_view
        {
            using view_type = sizes_mem;
            struct resource_type {};

            constexpr operator view_type () const noexcept
            {
                return sizes_mem_v;
            }

            constexpr operator resource_type () const noexcept
            {
                return {};
            }
        } sizes_res_mem_view_v{};
    }

    void test_sizes() noexcept
    {
        using namespace private_detail_test_sizes;

        static_assert(testconst_sizes_o == sizes(sizes_o_v));
        static_assert(testconst_sizes_mem_fn == sizes(sizes_mem_fn_v));
        static_assert(testconst_sizes_mem == sizes(sizes_mem_v));

        static_assert(testconst_sizes_mem_fn == sizes(sizes_mem_fn_o_v));
        static_assert(testconst_sizes_mem == sizes(sizes_mem_o_v));

        static_assert(testconst_sizes_mem == sizes(sizes_mem_view_v));
        static_assert(testconst_sizes_o == sizes(sizes_mem_view_o_v));
        static_assert(testconst_sizes_mem_fn == sizes(sizes_mem_fn_res_mem_view_v));
        static_assert(testconst_sizes_mem == sizes(sizes_res_mem_view_v));

        D_ASSERT(!errno);
    }
}

void test_size2d() noexcept
{
    constexpr vec2 v{ 3, 4 };
    constexpr size2d sz{ v };
    static_assert(std::is_same_v<decltype(sz), const size2d<int>>);
    static_assert(std::is_trivial_v<size2d<int>> && std::is_standard_layout_v<size2d<int>>);
    static_assert(sz.width() == 3);
    static_assert(sz.height() == 4);

    constexpr vec2 uv{ 3u, 4u };
    constexpr size2d usz{ uv };
    static_assert(std::is_same_v<decltype(usz), const size2d<unsigned>>);

    static_assert(md_narrow<size2d<int>>(usz) == sz);
    static_assert(md_narrow<vec2<int>>(usz) == v);
    static_assert(md_narrow<size2d<unsigned>>(sz) == usz);
    static_assert(md_narrow<vec2<unsigned>>(sz) == uv);

    constexpr size2d csz{ '\5', '\6' };
    static_assert(std::is_same_v<decltype(csz), const size2d<char>>);
    static_assert(csz.width() == '\5');
    static_assert(csz.height() == '\6');

    test_sizes();
}