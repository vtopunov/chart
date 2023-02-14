#include <cstdio>

namespace
{
	enum class sel
	{
        idle,
        predraw,
        draw_to_cache,
        draw
    };

    void predraw() noexcept
    {
        printf("%s %p\n", __FUNCTION__, predraw);
    }

    void draw_to_cache() noexcept
    {
        printf("%s %p\n", __FUNCTION__, draw_to_cache);
    }


    void draw() noexcept
    {
        printf("%s %p\n", __FUNCTION__, draw);
    }


    void test(sel s) noexcept
    {
        printf("\n%s %p %d\n", __FUNCTION__, test, static_cast<int>(s));

        switch (s)
        {
            case sel::predraw:
                predraw();
                [[fallthrough]];
            case sel::draw_to_cache:
                draw_to_cache();
                [[fallthrough]];
            case sel::draw:
                draw();
                [[fallthrough]];
        }
    }
}

int main()
{
    test(sel::predraw);
    test(sel::draw_to_cache);
    test(sel::draw);

    return 0;;
}