#include "window.h"

namespace os_windows
{
    extern void unregister_all_procedures( HWND window_handle ) noexcept;

    namespace
    {
        void destroy_window( HWND handle ) noexcept
        {
            unregister_all_procedures( handle );

            const auto result = DestroyWindow( handle );
            result; assert( result );
        }
    }

    bool window::show( int cmd ) const noexcept
    {
        assert( is_valid() );
        return to_bool( ShowWindow( handle_, cmd ) );
    }

    bool window::update() const noexcept
    {
        assert( is_valid() );
        return to_bool( UpdateWindow( handle_ ) );
    }

    void window::close() noexcept
    {
        if ( is_valid() )
        {
            destroy_window( release_window_handle() );
        }
    }

    safe_window create_window( window_info info ) noexcept
    {
        if ( !info.type_ )
        {
            info.type_ = register_window_type( {} );
        }

        const HWND window_handle = ( info.type_ ) ?
            CreateWindowExW(
                0,
                info.type_->name_id(),
                info.title_.c_str(),
                info.style_,
                info.x_,
                info.y_,
                info.width_,
                info.height_,
                nullptr,
                nullptr,
                info.type_->module_address(),
                nullptr
            ) : nullptr;

        return 
        {
            window
            {
                std::move( info.type_ ),
                window_handle
            }
        };

        /*{
            assert( !event_handlers_map().contains( window_handle ) );

            if ( window_handle && info.event_handler_ )
            {
                const auto item = event_handlers_map().force_insert( window_handle, std::move( info.event_handler_ ) );
                item->value( event{ window_handle, event_type::event_handler_registered, 0, 0 } );
            }
        }

        return created_window;*/
    }
}