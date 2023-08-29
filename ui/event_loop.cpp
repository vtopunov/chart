#include "event_loop.h"

#include <os/os.h>

#include <ui/window.h>
#include <ui/event_processors_storage.h>

namespace ui
{
    namespace private_detail_event_loop
    {
        void message_wait_for(milliseconds timeout) noexcept
        {
            static_assert(INFINITE == infinite.count());
            
            MsgWaitForMultipleObjectsEx
            (
                0u,
                nullptr,
                narrow<dword_t>(std::min(infinite, timeout).count()),
                QS_ALLEVENTS,
                MWMO_ALERTABLE
            );
        }

        void process_message(const os::message_t* msg) noexcept
        {
            static_assert(msg_storage_size <= sizeof(MSG));
            static_assert(pm_remove == PM_REMOVE);

            TranslateMessage(msg);
            DispatchMessageW(msg);
        }
        
        event_style e_style(const os::message_t* msg) noexcept
        {
            return underlying_cast<event_style>(msg->message);
        }

        word_parameter_t word_parameter(const os::message_t* msg) noexcept
        {
            return msg->wParam;
        }
        
        void sizes_initialization() noexcept
        {
            constexpr auto make_size_event = [] (window_handle_t w, pxsize2d sizes) noexcept
            {
                return size_event{ w, event_style::size, 0u, MAKELPARAM(sizes.width(), sizes.height()) };
            };

            for (const auto& window_dep : roots())
            {
                const auto window = window_dep.current;
                const auto size_e = make_size_event(window, sizes(window));

                for (const auto& processor : event_processors_global().lock())
                {
                    if (window == processor.window && processor(size_e))
                    {
                        break;
                    }
                }
            }
        }
    }
}