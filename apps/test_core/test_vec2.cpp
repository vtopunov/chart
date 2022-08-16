#include <core/vec2.h>

#include <vector>

namespace
{
    template<class T>
    void test_view0(span<const T, 2_uz> view, const vec2<T>& vec) noexcept
    {
        static_assert( std::is_same_v<decltype(view), typename std::decay_t<decltype(vec)>::view_type> );
        static_assert(std::is_same_v<typename vec2<T>::value_type, T>);
        static_assert(std::is_same_v<decltype(vec2<T>::_0), T>);
        static_assert(vec2<T>{}.size() == 2_uz);

        D_ASSERT(view.data() == std::addressof(vec._0));
    }

    template<class T>
    void test_view(const vec2<T>& vec) noexcept
    {
        test_view0<T>(vec, vec);
    }
}

void test_vec2() noexcept
{
    {
        constexpr vec2 v0{ 1, 1 };
        constexpr vec2 v1{ 1, 2 };
        constexpr vec2 v2{ 2, 1 };
        constexpr vec2 v3{ 2, 2 };
        constexpr vec2 v4{ 1, 0 };
        constexpr vec2 v5{ 3, 1 };

        static_assert(std::is_same_v<std::remove_const_t<decltype(v0)>, vec2<int>>);
        static_assert(std::is_same_v<std::remove_const_t<decltype(v0._0)>, int>);
        
        static_assert(v0.data() == std::addressof(v0._0));         
        test_view(v0);
        
        static_assert(v1.data() == std::addressof(v1._0));
        test_view(v1);

        static_assert(std::is_trivial_v<vec2<int>> && std::is_standard_layout_v<vec2<int>>);

        static_assert(1 == v0._0);
        static_assert(1 == v0._1);
        static_assert(1 == get<0>(v0));
        static_assert(1 == get<1>(v0));
        static_assert(v0 == vec2<int>{ 1, 1 });
        static_assert(!(v0 != vec2<int>{ 1, 1 }));
        static_assert(v0 != v1);
        static_assert(!(v0 == v1));

        static_assert(1 == v1._0);
        static_assert(2 == v1._1);
        static_assert(1 == get<0>(v1));
        static_assert(2 == get<1>(v1));
        static_assert(v1 == vec2<int>{ 1, 2 });
        static_assert(!(v1 != vec2<int>{ 1, 2 }));
        static_assert(v1 != v2);
        static_assert(!(v1 == v2));

        static_assert(2 == v2._0);
        static_assert(1 == v2._1);
        static_assert(2 == get<0>(v2));
        static_assert(1 == get<1>(v2));
        static_assert(v2 == vec2<int>{ 2, 1 });
        static_assert(!(v2 != vec2<int>{ 2, 1 }));
        static_assert(v2 != v0);
        static_assert(!(v2 == v0));

        static_assert(2 * v0 == v0 * 2);
        static_assert(2 * v0 == v3);
        static_assert(v3 / 2 == v0);
        static_assert(v2 + v4 == v5);
        static_assert(v4 + v2 == v5);
        static_assert(v5 - v2 == v4);
        static_assert(v5 - v4 == v2);
        static_assert(-(-v5) == v5);
        static_assert(-(-(-v5)) == -v5);
        static_assert(-(v4 + (-v5)) == v2);
    }

    {
        constexpr auto vec_size = narrow2d_cast<vec2<size_t>>(vec2{ 3, 4 });
        static_assert(std::is_same_v<std::remove_const_t<decltype(vec_size)>, vec2<size_t>>);
        static_assert(std::is_same_v<std::remove_const_t<decltype(vec_size._0)>, size_t>);
        static_assert(vec_size._0 == 3_uz);
        static_assert(vec_size._1 == 4_uz);
        test_view(vec_size);

        constexpr auto vec_ptrdiff = narrow2d_cast<vec2<ptrdiff_t>>(vec_size);
        static_assert(std::is_same_v<std::remove_const_t<decltype(vec_ptrdiff)>, vec2<ptrdiff_t>>);
        static_assert(std::is_same_v<std::remove_const_t<decltype(vec_ptrdiff._0)>, ptrdiff_t>);
        static_assert(vec_ptrdiff._0 == 3);
        static_assert(vec_ptrdiff._1 == 4);
        static_assert(narrow2d_cast<vec2<size_t>>(vec_ptrdiff) == vec_size);
        test_view(vec_ptrdiff);

        const auto stdvec = narrow2d_cast<std::vector<int>>(vec_ptrdiff);
        D_ASSERT(stdvec.size() == 2_uz);
        D_ASSERT(stdvec[0] == 3);
        D_ASSERT(stdvec[1] == 4);
    }

 
    D_ASSERT( !errno );
}