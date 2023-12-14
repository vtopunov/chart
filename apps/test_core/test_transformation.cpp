#include <core/transformation.h>


void test_transformation() noexcept
{
    constexpr vec2 window
    {
        point2d{ 0.0, 200.0 },
        point2d{ 300.0, 0.0 }
    };

    constexpr vec2 chart
    {
        point2d{ -10.0, -1.0 },
        point2d{ 10.0, 1.0 }
    };

    constexpr auto chart_to_window = make_transformation(chart, window);
    constexpr auto window_to_chart = make_transformation(window, chart);


    constexpr auto w_c = chart_to_window(point2d{ 0.0, 0.0 });
    static_assert(w_c == point2d{ 150.0, 100.0 });
    constexpr auto c_c = window_to_chart(w_c);
    static_assert(c_c == point2d{ 0.0, 0.0 });

    constexpr auto w_c_ht = chart_to_window(point2d{ 0.0, 0.5 });
    static_assert(w_c_ht == point2d{ 150.0, 50.0 });
    constexpr auto c_c_ht = window_to_chart(w_c_ht);
    static_assert(c_c_ht == point2d{ 0.0, 0.5 });

    constexpr auto w_c_hb = chart_to_window(point2d{ 0.0, -0.5 });
    static_assert(w_c_hb == point2d{ 150.0, 150.0 });
    constexpr auto c_c_hb = window_to_chart(w_c_hb);
    static_assert(c_c_hb == point2d{ 0.0, -0.5 });

    constexpr auto w_hl_ht = chart_to_window(point2d{ -5.0, 0.5 });
    static_assert(w_hl_ht == point2d{ 75.0, 50.0 });
    constexpr auto c_hl_ht = window_to_chart(w_hl_ht);
    static_assert(c_hl_ht == point2d{ -5.0, 0.5 });
    
    static_assert(to_size2d(w_c - w_hl_ht) == chart_to_window(to_size2d(c_c - c_hl_ht)));
    static_assert(window_to_chart(to_size2d(w_c - w_hl_ht)) == to_size2d(c_c - c_hl_ht));
    
    D_ASSERT(!errno);
}