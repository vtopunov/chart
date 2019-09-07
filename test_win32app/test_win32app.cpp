// test_win32app.cpp : Defines the entry point for the application.
//

#include <platform/windows/window.h>
#include <platform/windows/event_handler.h>

using namespace os_windows;

namespace
{
    void output_mouse_move( const mouse_move_event& e ) noexcept
    {
        wchar_t outbuf[128];
        if ( swprintf_s( outbuf, L"mouse move: %d %d\n", e.x(), e.y() ) > 0 )
        {
            OutputDebugStringW( outbuf );
        }
    }

    int output_error_code() noexcept
    {
        const auto error_code = GetLastError();

        {
            wchar_t outbuf[128];
            if ( swprintf_s( outbuf, L"error code: %lu", error_code ) > 0 )
            {
                OutputDebugStringW( outbuf );
            }
        }

        return static_cast<int>( error_code );
    }

    void output_windows_class_name( const wchar_t* name ) noexcept
    {
        wchar_t outbuf[128];
        if ( swprintf_s( outbuf, L"windows class name: %ls", name ) > 0 )
        {
            OutputDebugStringW( outbuf );
        }
    }
}

int APIENTRY wWinMain( _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    const auto window_type =
        register_window_type(
            window_type_info{}
            .module_address( hInstance )  
            .name( L"test_win32wnd" )
        );

    if ( !window_type )
    {
        return output_error_code();
    }

    const auto window =
        create_window(
            window_info{}
            .type( window_type )
            .title( L"test_win32app" )
        );

    if ( !window )
    {
        return output_error_code();
    }

    window->show( nCmdShow );
    window->update();

    const auto handler_lock = register_event_handler( window, [] ( event e )
    {
        if ( const auto & mouse_move = e.as<event_type::mouse_move>() )
        {
            output_mouse_move( mouse_move );
        }

        return default_event_handler( e );
    } );

    MSG msg{};

    while ( GetMessageW( &msg, nullptr, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessageW( &msg );
    }

    return static_cast<int>( msg.wParam );
}

