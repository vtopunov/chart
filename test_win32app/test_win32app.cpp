// test_win32app.cpp : Defines the entry point for the application.
//

#include <platform/windows/window.h>
#include <platform/windows/event_handler.h>
#include <platform/windows/event_matching.h>

using namespace os_windows;

namespace
{
    int output_error_code() noexcept
    {
        const auto error_code = GetLastError();

        {
            wchar_t outbuf[128];
            swprintf_s( outbuf, L"error code: %lu", error_code );
            OutputDebugStringW( outbuf );
        }

        return ( error_code ) ? static_cast<int>( error_code ) : -1;
    }

    void output_windows_class_name( const wchar_t* name ) noexcept
    {
        wchar_t outbuf[128];
        if ( swprintf_s( outbuf, L"windows class name: %ls", name ) > 0 )
        {
            OutputDebugStringW( outbuf );
        }
    }

    constexpr struct
    {
        LRESULT operator () ( const mouse_move_event& e ) const noexcept
        {
            wchar_t outbuf[128];
            if ( swprintf_s( outbuf, L"mouse move: %d %d\n", e.x(), e.y() ) > 0 )
            {
                OutputDebugStringW( outbuf );
            }
            return 0;
        }
    } event_handlers;
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

    const auto event_dispatcher = register_event_handler( window, event_match(event_handlers));

    //SetTimer( window->native_handle(), 0, 5000, [] ( HWND, UINT, UINT_PTR, DWORD ) { PostQuitMessage( 0 ) } );

    MSG msg{};

    while ( GetMessageW( &msg, nullptr, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessageW( &msg );
    }

    return static_cast<int>( msg.wParam );
}

