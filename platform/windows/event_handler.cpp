#include "event_handler.h"

#include <core/small_flat_map.h>
#include <core/underlying_cast.h>

namespace os_windows
{
    namespace
    {
        struct procedure_type
        {
            event_handler_type callback;
            size_t id;

            LRESULT operator () ( event e ) const noexcept
            {
                return callback( e );
            }
        };

        using procedures_map_t = small_flat_map<HWND, procedure_type, 4>;

        procedures_map_t& procedures_map() noexcept
        {
            static procedures_map_t map;
            return map;
        }

        span<const procedures_map_t::value_type> other_procedures( const HWND window_handle, procedures_map_t::const_iterator current, const procedures_map_t& map ) noexcept
        {
            const auto next = std::next( current );
            return { next, map.count( next, window_handle ) };
        }

        constexpr size_t generate_id( span<const procedures_map_t::value_type> other_procedures ) noexcept
        {
            size_t id = 0;
            for ( const auto& item : other_procedures )
            {
                id = std::max( id, item.value.id + 1_z );
            }
            return id;
        }

        size_t register_procedure( HWND window_handle, event_handler_type event_handler ) noexcept
        {
            auto& map = procedures_map();
            const auto position = map.force_insert( window_handle, procedure_type{ std::move( event_handler ), invaid_procedure_id } );
            const auto id = generate_id( other_procedures( window_handle, position, map ) );
            position->value.id = id;
            return id;
        }

        void unregister_procedure( HWND window_handle, size_t id ) noexcept
        {
            auto& map = procedures_map();
            for ( auto& item : map.items( window_handle ) )
            {
                if ( item.value.id == id )
                {
                    map.erase( &item );
                }
            }
        }
    }

    LRESULT CALLBACK window_procedure( HWND window_handle, UINT message, WPARAM word_parameter, LPARAM long_parameter ) noexcept
    {
        const event e { window_handle, underlying_cast<event_type>( message ), word_parameter, long_parameter };

        for ( const auto& item : std::as_const( procedures_map() ).items( window_handle ) )
        {
            return item.value( e );
        }

        return default_event_handler( e );
    }

    LRESULT default_event_handler( event e ) noexcept
    {
        return DefWindowProcW( e.window_handle_, to_underlying( e.type_ ), e.word_parameter_, e.long_parameter_ );
    }

    void event_handler_handle::close() noexcept
    {
        if ( is_valid() )
        {
            unregister_procedure( window_handle_, release_procedure_id() );
        }
    }

    safe_event_handler_handle register_event_handler( window_view window, event_handler_type event_handler ) noexcept
    {
        assert( window );
        assert( event_handler );

        const auto id = register_procedure( window.handle, procedure_type{ std::move( event_handler ), invaid_procedure_id } );

        return
        {
            event_handler_handle
            {
                window.handle,
                id
            }
        };
    }

    void unregister_all_procedures( HWND window_handle ) noexcept
    {
        procedures_map().erase( window_handle );
    }
}
