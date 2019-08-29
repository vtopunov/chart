// test_win32app.cpp : Defines the entry point for the application.
//

#include <platform/windows/window.h>

constexpr WCHAR sz_name[] = L"sz_name";

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch ( message )
    {
        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;
        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}

/*
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
    HWND hWnd = CreateWindowW( sz_name, sz_name, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr );

    if ( !hWnd )
    {
        return FALSE;
    }

    ShowWindow( hWnd, nCmdShow );
    UpdateWindow( hWnd );

    return TRUE;
}*/

int APIENTRY wWinMain( _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    auto window = os_windows::create_window( {} );
    if ( window && window->show( nCmdShow ) && window->update() )
    {
        MSG msg{};

        while ( GetMessageW( &msg, nullptr, 0, 0 ) )
        {
            TranslateMessage( &msg );
            DispatchMessageW( &msg );
        }

        return static_cast<int>( msg.wParam );
    }
    
    const auto error_code = static_cast<int>( GetLastError() );
    return error_code;
}

