#include <platform/windows/event_handler.h>

using namespace os_windows;

void test_event_handler() noexcept
{
    D_ASSERT( !"not implemented" );
    
    constexpr auto window_handle = (HWND)0x1234;


    D_ASSERT(!errno);
}