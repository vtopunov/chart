#include "event.h"

namespace os_windows
{
    LRESULT default_event_handler( event e ) noexcept
    {
        return DefWindowProcW( e.window_handle_, e.type_, e.word_parameter_, e.long_parameter_ );
    }
}
