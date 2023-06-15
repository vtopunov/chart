#pragma once

#include <ui/event_loop.h>

#if defined(D_OS_WINDOWS)
#include <ui/window.h>
#endif

#include <egl_ui/egl_window_builder.h>
#include <egl_ui/painting_owner.h>


namespace ui
{
#ifdef D_OS_WINDOWS
    template<>
    class event_binder<egl_ui::egl_window_resource> 
    {
    public:
        constexpr event_binder(const egl_ui::egl_window_resource& egl) noexcept
            : app_event_source_{ app_window(egl) }
            , input_event_source_{ render_window(egl) }
        {}

        template<class EventTarget>
        [[nodiscard]] std::array<event_processor, 2u> bind(EventTarget& target) noexcept
        {
            auto target_ref = std::ref(target);

            return
            {
                create_event_processor
                (
                    app_event_source_,
                    event_match
                    {
                        function_filter
                        <
                            decltype(target_ref),
                            const size_event&
                        >
                        { target_ref }
                    }
                ),
                create_event_processor
                (
                    input_event_source_,
                    event_match{ target_ref }
                )
            };
        }
    
    private:
        window_handle_t app_event_source_;
        window_handle_t input_event_source_;
    };

#endif
}