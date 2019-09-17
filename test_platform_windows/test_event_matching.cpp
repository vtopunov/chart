#include <platform/windows/event_matching.h>

using namespace os_windows;

struct event_visitor
{
    event_type type{ event_type::null };

    LRESULT operator () (const event& e) noexcept
    {
        assert(type == event_type::null);
        type = event_type::out_of_os;
        return 0;
    }

    LRESULT operator () (const timer_event& e) noexcept
    {
        return set_event<event_type::timer>(e);
    }

    LRESULT operator () (const mouse_move_event& e) noexcept
    {
        return set_event<event_type::mouse_move>(e);
    }

    template<event_type special_type>
    LRESULT set_event(const special_event<special_type>& e) noexcept
    {
        assert(type == event_type::null && e.type() == special_type);
        type = special_type;
        return 0;
    }
};

struct event_visitor_without_on_event
{
    event_type type{ event_type::null };

    LRESULT operator () (const timer_event& e) noexcept
    {
        return set_event<event_type::timer>(e);
    }

    LRESULT operator () (const mouse_move_event& e) noexcept
    {
        return set_event<event_type::mouse_move>(e);
    }

    template<event_type special_type>
    LRESULT set_event(const special_event<special_type>& e) noexcept
    {
        assert(type == event_type::null && e.type() == special_type);
        type = special_type;
        return 0;
    }
};

void test_event_matching() noexcept
{
    {
        event_visitor visitor;
        auto matching = event_match(std::ref(visitor));

        auto check_matching = [&matching, &visitor](event_type type)
        {
            visitor.type = event_type::null;
            matching(event{ 0, type, 0, 0 });
            assert(visitor.type == type);
        };

        auto check_matching_not_found = [&matching, &visitor](event_type type)
        {
            visitor.type = event_type::null;
            matching(event{ 0, type, 0, 0 });
            assert(visitor.type == event_type::out_of_os);
        };

        check_matching(event_type::out_of_os);
        check_matching(event_type::timer);
        check_matching(event_type::mouse_move);
        check_matching_not_found(event_type::close);
    }

    {
        event_visitor_without_on_event visitor;
        auto matching = event_match(std::ref(visitor));

        auto check_matching = [&matching, &visitor](event_type type)
        {
            visitor.type = event_type::null;
            matching(event{ 0, type, 0, 0 });
            assert(visitor.type == type);
        };

        auto check_matching_not_found = [&matching, &visitor](event_type type)
        {
            visitor.type = event_type::null;
            matching(event{ 0, type, 0, 0 });
            assert(visitor.type == event_type::null);
        };

        check_matching_not_found(event_type::out_of_os);
        check_matching(event_type::timer);
        check_matching(event_type::mouse_move);
        check_matching_not_found(event_type::close);
    }
}