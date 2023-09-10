#pragma once

#include <functional>

#include <ui/event.h>

#include <egl_ui/viewport_size2d.h>


namespace widget
{
    namespace helpers
    {
        template<class T>
        constexpr auto is_nothrow_copiable_v = std::conjunction_v<
            std::is_nothrow_copy_constructible<T>,
            std::is_nothrow_copy_assignable<T>
        >;

        template<class T>
        using cref_wrap_if_need_t = std::conditional_t<
            is_nothrow_copiable_v<T>, T,
            std::reference_wrapper<std::add_const_t<T>>
        >;

        template<class T>
        using cref_if_need_t = std::conditional_t<
            is_nothrow_copiable_v<T>, T,
            std::add_lvalue_reference_t<std::add_const_t<T>>
        >;
    }

    template<class... Args>
    class redraw_event
    {
    public:
        using tuple_type = std::tuple<Args...>;
        using cref_wrap_tuple_type = std::tuple<helpers::cref_wrap_if_need_t<Args>...>;

        template<class... IniArgs>
        constexpr explicit redraw_event(const IniArgs&... args) noexcept
            : tuple_{ args... }
        {}

        template<class T>
        constexpr helpers::cref_if_need_t<T> get() const noexcept
        {
            return std::get<helpers::cref_wrap_if_need_t<T>>(tuple_);
        }

    private:
        cref_wrap_tuple_type tuple_{};
    };
}