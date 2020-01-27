#include <platform/windows/event_matching.h>

using namespace os_windows;

struct event_visitor
{
    event_style style{ event_style::null };

    LRESULT operator () (const event& e) noexcept
    {
        D_ASSERT(style == event_style::null);
        style = event_style::user;
        return 0;
    }

    LRESULT operator () (const timer_event& e) noexcept
    {
        return set_event<event_style::timer>(e);
    }

    LRESULT operator () (const mouse_move_event& e) noexcept
    {
        return set_event<event_style::mouse_move>(e);
    }

    template<event_style specialization_style>
    LRESULT set_event(const specialized_event<specialization_style>& e) noexcept
    {
        D_ASSERT(style == event_style::null && e.style() == specialization_style);
        style = specialization_style;
        return 0;
    }
};

struct event_visitor_without_on_event
{
    event_style style{ event_style::null };

    LRESULT operator () (const timer_event& e) noexcept
    {
        return set_event<event_style::timer>(e);
    }

    LRESULT operator () (const mouse_move_event& e) noexcept
    {
        return set_event<event_style::mouse_move>(e);
    }

    template<event_style specialization_style>
    LRESULT set_event(const specialized_event<specialization_style>& e) noexcept
    {
        D_ASSERT(style == event_style::null && e.style() == specialization_style);
        style = specialization_style;
        return 0;
    }
};

void test_event_matching() noexcept
{
    {
        event_visitor visitor;
        auto matching = event_match(std::ref(visitor));

        auto check_matching = [&matching, &visitor](event_style style)
        {
            visitor.style = event_style::null;
            matching(event{ 0, style, 0, 0 });
            D_ASSERT(visitor.style == style);
        };

        auto check_matching_not_found = [&matching, &visitor](event_style style)
        {
            visitor.style = event_style::null;
            matching(event{ 0, style, 0, 0 });
            D_ASSERT(visitor.style == event_style::user);
        };

        check_matching(event_style::user);
        check_matching(event_style::timer);
        check_matching(event_style::mouse_move);
        check_matching_not_found(event_style::close);
    }

    {
        event_visitor_without_on_event visitor;
        auto matching = event_match(std::ref(visitor));

        auto check_matching = [&matching, &visitor](event_style style)
        {
            visitor.style = event_style::null;
            matching(event{ 0, style, 0, 0 });
            D_ASSERT(visitor.style == style);
        };

        auto check_matching_not_found = [&matching, &visitor](event_style style)
        {
            visitor.style = event_style::null;
            matching(event{ 0, style, 0, 0 });
            D_ASSERT(visitor.style == event_style::null);
        };

        check_matching_not_found(event_style::user);
        check_matching(event_style::timer);
        check_matching(event_style::mouse_move);
        check_matching_not_found(event_style::close);
    }
}