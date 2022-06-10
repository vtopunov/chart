#include "event_loop.h"

#include <os/os.h>

namespace ui
{
    namespace private_detail_event_loop
    {
        namespace
        {
            [[nodiscard]]
            constexpr int exit_status(const os::message_t* msg) noexcept
            {
                D_WARNING_PUSH;
                D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
                return static_cast<int>(msg->wParam);
                D_WARNING_POP;
            }
        }

        void sleep_or_reñeive_message(milliseconds_t timeout) noexcept
        {
            static_assert(infinite.count() == INFINITE);

            MsgWaitForMultipleObjectsEx
            (
                0u,
                nullptr,
                narrow_cast<dword_t>(std::min(infinite, timeout).count()),
                QS_ALLEVENTS,
                MWMO_ALERTABLE
            );
        }

        std::optional<int> process_message(const os::message_t* msg) noexcept
        {
            static_assert(msg_storage_size <= sizeof(MSG));
            static_assert(pm_remove == PM_REMOVE);

            TranslateMessage(msg);
            DispatchMessageW(msg);

            if (WM_QUIT == msg->message) [[unlikely]]
            {
                return exit_status(msg);
            }

            return std::nullopt;
        }
    }
}