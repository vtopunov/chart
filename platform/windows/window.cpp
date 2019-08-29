#include "window.h"

#include <core/static_map.h>

namespace os_windows
{
    namespace
    {
        constexpr bool to_std_bool( BOOL value ) noexcept
        {
            return value != FALSE;
        }

        using window_map_t = static_map<HWND, const window*, 2>;

        window_map_t& window_map() noexcept
        {
            static window_map_t map;
            return map;
        }

        void destroy_window( not_null<HWND> window_handle, bool is_event_handler ) noexcept
        {
            assert( is_event_handler || !window_map().contains( window_handle ) );

            if ( is_event_handler )
            {
                window_map().unsafe_erase( window_handle );
            }

            const auto result = to_std_bool( DestroyWindow( window_handle ) );
            result; assert( result );
        }
    }

    LRESULT CALLBACK window_procedure( HWND window_handle, UINT message, WPARAM word_parameter, LPARAM long_parameter ) noexcept
    {
        const event e { window_handle, message, word_parameter, long_parameter };

        if ( const auto item = window_map().item( window_handle ) )
        {
            return item->value->process_event( e );
        }

        return default_event_handler( e );
    }

    void window::construct_weak( const void& ) noexcept
    {
        assert( !window_map().contains( window_handle_ ) );

        if ( is_event_handler() )
        {
            window_map().unsafe_insert( window_handle_, this );
        }
    }

    void window::replace_weak( const void&, const safe_handle<window>& new_window ) noexcept
    {
        assert( is_event_handler() == window_map().contains( window_handle_ ) );

        if ( is_event_handler() )
        {
            const auto item = window_map().unsafe_lower_bound( window_handle_ );
            if ( item->value == this )
            {
                item->value = &( new_window.get() );
            }
        }
    }

    bool window::show( int cmd ) const noexcept
    {
        assert( is_valid() );
        return to_std_bool( ShowWindow( window_handle_, cmd ) );
    }

    bool window::update() const noexcept
    {
        assert( is_valid() );
        return to_std_bool( UpdateWindow( window_handle_ ) );
    }

    void window::close(const void&) noexcept
    {
        if ( is_valid() )
        {
            destroy_window( release_window_handle(), release_event_handler() );
        }
    }

    safe_window create_window( window_info info ) noexcept
    {
        if ( !info.type_ )
        {
            info.type_ = register_window_type( {} );
            if ( !info.type_ )
            {
                return {};
            }
        }

        const auto window_handle = CreateWindowExW(
            0,
            info.type_->name().as_string_id(),
            info.title_.c_str(),
            info.style_,
            info.x_,
            info.y_,
            info.width_,
            info.height_,
            nullptr,
            nullptr,
            info.type_->module_address(),
            nullptr );

        if ( !window_handle )
        {
            return {};
        }

        return window
        {
            std::move( info.type_ ),
            std::move( info.event_handler_ ),
            window_handle
        };


    }
}