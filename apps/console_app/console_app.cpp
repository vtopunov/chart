#include <iterator>
#include <optional>
#include <tuple>

#include <core/buffer.h>
#include <core/buffer_view.h>
#include <core/tuple_algorithm.h>

#include <ui/event_loop.h>


struct temp_buffer_t : buffer_t
{
    constexpr operator buffer_view() const noexcept
    {
        return as_mutable(*this);
    }
};

template<size_t id>
struct shader
{
    constexpr shader() noexcept = default;
    D_DISABLE_COPY(shader);
};

struct content_sizes_cache : pxsize2d
{
    void operator () (const ui::size_event& e) noexcept
    {
        static_cast<pxsize2d&>(*this) = e.sizes();
    }
};

template<class T>
constexpr auto is_nothrow_copiable_v = std::conjunction_v
<
    std::is_nothrow_copy_constructible<T>,
    std::is_nothrow_copy_assignable<T>
>;

template<class T>
using cref_wrap_if_need_t = std::conditional_t
<
    is_nothrow_copiable_v<T>, T,
    std::reference_wrapper<std::add_const_t<T>>
>;

template<class T>
using cref_if_need_t = std::conditional_t
<
    is_nothrow_copiable_v<T>, T,
    std::add_lvalue_reference_t<std::add_const_t<T>>
>;

template<class T>
using unview_t = std::conditional_t
<
    is_buffer_view_v<T>,
    temp_buffer_t,
    T
>;

template<class... Args>
class common_draw_context
{
public:
    using tuple_type = tuple_sizeof_optimization_t<std::tuple<Args...>>;

    template<class T>
    constexpr decltype(auto) cget() const noexcept
    {
        return std::get<unview_t<T>>(tuple_);
    }

    template<class T>
    constexpr decltype(auto) get() const noexcept
    {
        return cget<T>();
    }

    template<class T>
    constexpr decltype(auto) get() noexcept
    {
        return as_mutable(cget<T>());
    }

private:
    tuple_type tuple_{};
};


template<class Tuple>
struct make_common_draw_context_type;

template<class... Types>
struct make_common_draw_context_type<std::tuple<Types...>>
{
    using type = common_draw_context<unview_t<Types>...>;
};

template<class Tuple>
using make_common_draw_context_t = typename make_common_draw_context_type<Tuple>::type;


template<class... Args>
class redraw_event
{
public:
    using tuple_type = std::tuple<Args...>;
    using cref_wrap_tuple_type = std::tuple<cref_wrap_if_need_t<Args>...>;

    template<class... ContextArgs>
    constexpr redraw_event(const common_draw_context<ContextArgs...>& context) noexcept
        : redraw_event{ context, std::index_sequence_for<Args...>() }
    {}

    template<class T>
    constexpr cref_if_need_t<T> get() const noexcept
    {
        return std::get<cref_wrap_if_need_t<T>>(tuple_);
    }

private:
    template<class... ContextArgs, size_t... Indices>
    constexpr redraw_event(const common_draw_context<ContextArgs...>& context, std::index_sequence<Indices...>) noexcept
        : tuple_{ context.get<std::tuple_element_t<Indices, tuple_type>>()...}
    {}

private:
    cref_wrap_tuple_type tuple_{};
};

struct widget0
{
    using redraw_event_type = redraw_event<shader<0>, shader<1>, temp_buffer_t>;

    template<class Fn>
    decltype(auto) apply(Fn fn) noexcept
    {
        return fn();
    }
};

struct widget1
{
    using redraw_event_type = redraw_event<shader<1>, shader<2>, buffer_view, content_sizes_cache>;

    template<class Fn>
    decltype(auto) apply(Fn fn) noexcept
    {
        return fn();
    }
};

template<class Widget>
using decl_redraw_event_type_t = typename Widget::redraw_event_type;

template<class Widget>
using redraw_event_type_t = detected_or_t<redraw_event<>, decl_redraw_event_type_t, Widget>;

template<class Widget>
using redraw_event_tuple_type_t = typename redraw_event_type_t<Widget>::tuple_type;

template<class Widget>
using draw_context_tuple_t = transform_tuple_t<unview_t, redraw_event_tuple_type_t<Widget>>;


struct main_widget
{
    widget0 w0_0;
    widget0 w0_1;
    widget1 w1;

    template<class Fn>
    decltype(auto) apply(Fn fn) noexcept
    {
        return fn(w0_0, w0_1, w1);
    }
};

template<class W>
using siblings_widget_tuple_t = decltype(std::declval<W>().apply([] (auto... types) noexcept { return std::make_tuple(types...); }));

namespace private_detail_widget_tuple
{
    template<class P, class S>
    struct siblings_loop_type;

    template<class P>
    struct siblings_loop_type<P, std::tuple<>>
    {
        using type = std::tuple<P>;
    };

    template<class P, class... S>
    struct siblings_loop_type<P, std::tuple<S...>>
    {
        using type = tuple_push_front_t<P, tuple_cat_t<typename siblings_loop_type<S, siblings_widget_tuple_t<S>>::type...>>;
    };
}

template<class W>
struct widget_tuple_type
{
    using type = typename private_detail_widget_tuple::siblings_loop_type<W, siblings_widget_tuple_t<W>>::type;
};

template<class Widget>
using widget_tuple_t = typename widget_tuple_type<Widget>::type;


template<class TupleWidgets>
struct tuple_common_draw_context_type;

template<>
struct tuple_common_draw_context_type<std::tuple<>>
{
    using type = std::tuple<>;
};

template<class T0, class... Types>
struct tuple_common_draw_context_type<std::tuple<T0, Types...>>
{
    using type = tuple_unique_push_back_tuple_t<
        draw_context_tuple_t<T0>,
        typename tuple_common_draw_context_type<std::tuple<Types...>>::type
    >;
};

template<class TupleWidgets>
using tuple_common_draw_context_t = typename tuple_common_draw_context_type<TupleWidgets>::type;

template<class Widget>
using widget_common_draw_context_t = make_common_draw_context_t<tuple_common_draw_context_t<widget_tuple_t<Widget>>>;


int main() noexcept
{
    common_draw_context<shader<0>, shader<1>, shader<2>, temp_buffer_t> dc{};
    redraw_event<shader<0>, shader<1>, buffer_view> e{ dc };

    dc.get<temp_buffer_t>().reserve(100);

    auto& e_sh0 = e.get<shader<0>>();
    auto& e_sh1 = e.get<shader<1>>();
    auto bv = e.get<buffer_view>();

    widget_tuple_t<main_widget> wtuple{};
    widget_common_draw_context_t<main_widget> wdc{};

    return static_cast<int>(bv.size());
}