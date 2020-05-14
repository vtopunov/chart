#include <platform/windows/event_matching.h>

using namespace os_windows;

struct event_visitor
{
    event_style style{ event_style::null };

    event_result_t operator () (const event& e) noexcept
    {
        D_ASSERT(style == event_style::null);
        style = event_style::user;
        return 0;
    }

    event_result_t operator () (const timer_event& e) noexcept
    {
        return set_event<event_style::timer>(e);
    }

    event_result_t operator () (const mouse_move_event& e) noexcept
    {
        return set_event<event_style::mouse_move>(e);
    }

    template<event_style specialization_style>
    event_result_t set_event(const specialized_event<specialization_style>& e) noexcept
    {
        D_ASSERT(style == event_style::null && e.style() == specialization_style);
        style = specialization_style;
        return 0L;
    }
};

struct event_visitor_without_on_event
{
    event_style style{ event_style::null };

    event_result_t operator () (const timer_event& e) noexcept
    {
        return set_event<event_style::timer>(e);
    }

    event_result_t operator () (const mouse_move_event& e) noexcept
    {
        return set_event<event_style::mouse_move>(e);
    }

    template<event_style specialization_style>
    event_result_t set_event(const specialized_event<specialization_style>& e) noexcept
    {
        D_ASSERT(style == event_style::null && e.style() == specialization_style);
        style = specialization_style;
        return 0L;
    }
};

constexpr event make_dummy_event(event_style style) noexcept
{
    return { null_window, 0u, 0L, style };
}

void test_event_matching() noexcept
{
    {
        event_visitor visitor;
        auto matching = event_match(std::ref(visitor));

        auto check_matching = [&matching, &visitor](event_style style)
        {
            visitor.style = event_style::null;
            matching(make_dummy_event(style));
            D_ASSERT(visitor.style == style);
        };

        auto check_matching_not_found = [&matching, &visitor](event_style style)
        {
            visitor.style = event_style::null;
            matching(make_dummy_event(style));
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
            matching(make_dummy_event(style));
            D_ASSERT(visitor.style == style);
        };

        auto check_matching_not_found = [&matching, &visitor](event_style style)
        {
            visitor.style = event_style::null;
            matching(make_dummy_event(style));
            D_ASSERT(visitor.style == event_style::null);
        };

        check_matching_not_found(event_style::user);
        check_matching(event_style::timer);
        check_matching(event_style::mouse_move);
        check_matching_not_found(event_style::close);
    }
}