#include <core/tuple_algorithm.h>
#include <core/ordered_overload.h>
#include <core/member_detector.h>

#include <widget/temp_buffer.h>
#include <widget/window.h>

#include <format>

using widget::pix8_temp_buffer;
using widget::window;
using widget::content_size2d;


namespace
{
    struct ev0 {};
    struct ev1 {};

    enum class e_res
    {};

    struct proc
    {
        void operator () (ev0) {}
        e_res operator () (ev1) { return {}; }
    };

    struct combo
    {
        template<class... Args>
        void add(Args...) {}
    };

}



int main() noexcept
{
    combo c;
    
    decltype(proc{}(ev1{})) res;

    c.add(proc{}(ev1{}));


    return 0;
}