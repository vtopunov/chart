#include <ui/manipulator.cpp>


namespace
{
    using namespace ui::manipulator;

    struct user_motion
    {
        vpoint_cache v0;
        vpoint_cache v1;
    };


    [[nodiscard]]
    constexpr bool test_new_manipulation_apply(user_motion um, vpoint_cache& v) noexcept
    {
        const auto cv1 = um.v1;
        const auto result = new_manipulation(um.v0, um.v1);
        D_ASSERT(cv1 == um.v0);
        D_ASSERT(cv1 == um.v1);

        const auto new_v = result.transformation_as(v);
        const auto is_new = (new_v != v);

        if (is_new)
        {
            v = new_v;
        }

        return is_new;
    }
}

template<class T>
[[nodiscard]] std::enable_if_t<std::is_floating_point_v<T>, bool> md_test_eq(const T& a, const T& b) noexcept
{
    return std::nextafter(a, std::numeric_limits<T>::lowest()) <= b
        && std::nextafter(a, std::numeric_limits<T>::max()) >= b;
}

template<class T>
[[nodiscard]] auto md_test_eq(const T& a, const T& b) noexcept -> decltype(md_test_eq(as_vec2(a)._0, b._0))
{
    return md_test_eq(a._0, b._0)
        && md_test_eq(a._1, b._1);
}


void test_manipulator() noexcept
{
    D_ASSERT(md_test_eq(1.0, 1.0));
    D_ASSERT(!md_test_eq(1.0, 1.1));
    D_ASSERT(!md_test_eq(1.0, 0.9));

    const user_motion um0
    {
        { {65,512}, {412,207} },
        { {65,531}, {412,199} }
    };

    const user_motion um1
    {
        um0.v1,
        { {412,191}, {65,542} }
    };

    {
        auto new_v0 = um0.v0;
        D_ASSERT(test_new_manipulation_apply(um0, new_v0));
        D_ASSERT(!md_test_eq(um0.v0, new_v0));
        D_ASSERT(md_test_eq(um0.v1, new_v0));
    }

    {
        auto new_v0 = um1.v0;
        D_ASSERT(test_new_manipulation_apply(um1, new_v0));
        D_ASSERT(!md_test_eq(um1.v0, new_v0));
        D_ASSERT(md_test_eq(inverse(um1.v1), new_v0));
    }

    {
        auto um0_x = um0;
        um0_x.v0._0.ref_x() = um0_x.v0._1.ref_x() =
            um0_x.v1._0.ref_x() = um0_x.v1._1.ref_x() = 300;

        auto new_v0 = um0_x.v0;
        D_ASSERT(test_new_manipulation_apply(um0_x, new_v0));
        D_ASSERT(!md_test_eq(um0_x.v0, new_v0));
        D_ASSERT(md_test_eq(um0_x.v1, new_v0));
    }

    {
        auto um1_x = um1;
        um1_x.v0._0.ref_x() = um1_x.v0._1.ref_x() =
            um1_x.v1._0.ref_x() = um1_x.v1._1.ref_x() = 300;

        auto new_v0 = um1_x.v0;
        D_ASSERT(test_new_manipulation_apply(um1_x, new_v0));
        D_ASSERT(!md_test_eq(um1_x.v0, new_v0));
        D_ASSERT(md_test_eq(inverse(um1_x.v1), new_v0));
    }

    {
        auto um0_y = um0;
        um0_y.v0._0.ref_y() = um0_y.v0._1.ref_y() =
            um0_y.v1._0.ref_y() = um0_y.v1._1.ref_y() = 300;

        auto new_v0 = um0_y.v0;
        D_ASSERT(!test_new_manipulation_apply(um0_y, new_v0));
        D_ASSERT(md_test_eq(um0_y.v1, new_v0));
    }

    {
        auto um1_y = um1;
        um1_y.v0._0.ref_y() = um1_y.v0._1.ref_y() =
            um1_y.v1._0.ref_y() = um1_y.v1._1.ref_y() = 300;

        auto new_v0 = um1_y.v0;
        D_ASSERT(!test_new_manipulation_apply(um1_y, new_v0));
        D_ASSERT(md_test_eq(inverse(um1_y.v1), new_v0));
    }
}