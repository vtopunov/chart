#pragma once

#include <array>

#include <platform/windows/event.h>

namespace os_windows
{
    namespace fill_array_detail
    {
        template <typename T, std::size_t...Is>
        constexpr std::array<T, sizeof...( Is )>
            fill_array( T value, std::index_sequence<Is...> ) noexcept
        {
            return { {( static_cast<void>( Is ), value )...} };
        }
    }

    template <std::size_t N, typename T>
    constexpr std::array<T, N> fill_array( T value ) noexcept
    {
        return fill_array_detail::fill_array( value, std::make_index_sequence<N>() );
    }

    template<class function_type>
    struct event_matching
    {
    public:
        template<class Fn>
        explicit event_matching(Fn&& function) noexcept
            : function_{ std::forward<Fn>(function) }
        {}

        LRESULT operator () (const event& e) noexcept
        {
            constexpr auto event_mapping = [](event_type type) noexcept
            {
                static constexpr auto map_size = underlying_cast<size_t>(event_type::out_of_os);

                static constexpr auto on_event_pointer = &event_matching::on_event;

                static constexpr auto map = []() noexcept
                {
                    auto map = fill_array<map_size>(on_event_pointer);

                    constexpr auto set_special_event = [&map]<event_type type>(event_type_constant<type> constant) noexcept
                    {
                        constexpr auto type_value = constant.value;
                        map[underlying_cast<size_t>(type_value)] = &event_matching::on_special_event<type_value>;
                    };

                    set_special_event(event_type_constant<event_type::close>());
                    set_special_event(event_type_constant<event_type::timer>());
                    set_special_event(event_type_constant<event_type::mouse_move>());

                    return map;
                }();

                const auto index = underlying_cast<size_t>(type);
                return (index < map_size) ? map[index] : on_event_pointer;
            };

            const auto on_special_event = event_mapping(e.type());
            return (this->*on_special_event) (e);
        }

    private:
        LRESULT on_event( const event& e ) noexcept
        {
            return match( e );
        }

        template<event_type special_type>
        LRESULT on_special_event( const event& e ) noexcept
        {
            return match( e.as<special_type>() );
        }

        template<class special_event>
        auto match( const special_event& e ) noexcept -> decltype( this->function_( e ) )
        {
            return function_( e );
        }

        struct event_view
        {
            const event& e_;
            constexpr event_view( const event& e ) noexcept
                : e_( e )
            {}
        };

        LRESULT match( const event_view e ) const noexcept
        {
            return default_event_handler( e.e_ );
        }

        function_type function_;
    };

    template<class function_type>
    event_matching<std::decay_t<function_type>> event_match(function_type&& function) noexcept
    {
        return { std::forward<function_type>(function) };
    }
}

