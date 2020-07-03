#include "event_timer.h"

#include <core/handle.h>

#include <platform/windows/event.h>
#include <platform/windows/window.h>
#include <platform/windows/event_matching.h>
#include <platform/windows/event_processors_container.h>

using namespace std::chrono_literals;

namespace os_windows
{
    namespace event_timer
    {
        bool close(timer_view timer) noexcept
        {
            if (valid(timer))
            {
                const auto ok = !!KillTimer(timer.window_handle, timer.id);
                D_ASSERT(ok);
                return ok;
            }

            return false;
        }

        safe_timer create_timer(window_handle_t window_handle, size_t id, timer_duration_t interval) noexcept
        {
            using elapse_t = UINT;
            static_assert( std::is_unsigned_v<elapse_t> );

            const auto elapse = interval.count();

            const auto arguments_is_valid = is_safe_narrowing_conversion<elapse_t>(elapse);

            const auto timer_id = ( arguments_is_valid ) ? SetTimer
            (
                window_handle,
                id,
                narrow_cast<elapse_t>( elapse ),
                nullptr
            ) : id;

            const auto result_is_valid = arguments_is_valid && timer_id;

            const auto result_interval = ( result_is_valid ) ? interval : invalid_timer_duration;

            return
            {
                handle_construct,
                window_handle,
                timer_id,
                result_interval
            };
        }

        safe_timer create_timer(window_view window, timer_duration_t interval) noexcept
        {
            D_ASSERT(exist(window));
            return create_timer(window.handle, window.id, interval);
        }

        safe_timer set_timer_interval(safe_timer timer, timer_duration_t interval) noexcept
        {
            bool sucessed = false;

            if (interval > invalid_timer_duration && timer && timer->interval != interval)
            {
                if (auto new_timer = create_timer(timer->window_handle, timer->id, interval))
                {
                    if (new_timer->id == timer->id)
                    {
                        timer->interval = std::exchange(new_timer->interval, invalid_timer_duration);
                        sucessed = true;
                    }
                }
            }

            if (!sucessed)
            {
                const auto timer_view = to_view(timer);
                timer->interval = invalid_timer_duration;
                close(timer_view);
            }

            return timer;
        }

        namespace
        {
            struct timer_processor
            {
                safe_timer timer;
                timer_callback_t callback{nullptr};
                timer_duration_t interval{invalid_timer_duration};
                size_t processor_id{0u};

                std::optional<event_result_t> operator () (const timer_event& e) noexcept;

                void remove_event_processor_for(window_view window) noexcept
                {
                    if (const auto id = std::exchange(processor_id, 0u))
                    {
                        close(event_processor_view{window, id});
                    }
                }
            };

#pragma warning(push)
#pragma warning(disable : 26436) // non-virtual destructor
            class timer_controller_impl final : public timer_controller
            {
            public:
                timer_controller_impl(timer_processor& processor, const timer_event& e) noexcept
                    : processor_{processor}
                    , interval_{processor.interval}
                    , window_{e.window()}
                {}

                ~timer_controller_impl() noexcept
                {
                    if (finised_)
                    {
                        if (auto callback = std::exchange(callback_, nullptr))
                        {
                            processor_.callback = std::move(callback);
                        }

                        processor_.timer = set_timer_interval(std::move(processor_.timer), interval_);
                    }
                    else
                    {
                        callback_ = nullptr;
                        processor_.timer = nullptr;
                    }

                    if (!processor_.timer)
                    {
                        processor_.callback = nullptr;
                        processor_.remove_event_processor_for(window_);
                    }
                }

                void finish() noexcept
                {
                    finised_ = true;
                }

            private:
                void restart(timer_duration_t interval) noexcept final
                {
                    if (interval.count() > 0)
                    {
                        interval_ = interval;
                    }
                }

                timer_duration_t interval() const noexcept final
                {
                    return interval_;
                }

                window_view window() const noexcept final
                {
                    return window_;
                }

                void close() noexcept final
                {
                    callback_ = nullptr;
                    processor_.timer = nullptr;
                    processor_.remove_event_processor_for(window_);
                }

                void replace_callback(timer_callback_t new_callback) noexcept final
                {
                    callback_ = std::move(new_callback);
                }

                timer_processor& processor_;
                timer_duration_t interval_{invalid_timer_duration};
                timer_callback_t callback_{nullptr};
                window_view window_{null_window};
                bool finised_{false};
            };
#pragma warning(pop)

            std::optional<event_result_t> timer_processor::operator()(const timer_event& e) noexcept
            {
                D_ASSERT(e.window().handle == timer->window_handle);

                if (e.id() == timer->id)
                {
                    timer_controller_impl control{*this, e};

                    callback(control);

                    control.finish();

                    return 0L;
                }

                return std::nullopt;
            }


            struct timer_callback_factory
            {
                timer_callback_t callback;
                timer_duration_t interval;

                event_callback_t operator () (event_processor_view processor) noexcept
                {
                    auto timer = create_timer(processor.window.handle, processor.id, interval);

                    if (timer)
                    {
                        return event_match
                        (
                            timer_processor
                            {
                                std::move(timer),
                                std::move(callback),
                                interval,
                                processor.id
                            }
                        );
                    }

                    return nullptr;
                }
            };
        }

        safe_event_processor create_timer(window_view window, timer_duration_t interval, timer_callback_t callback) noexcept
        {
            return 
            {
                handle_construct,
                window,
                event_processors_container_global().insert(window, timer_callback_factory
                {
                    std::move(callback),
                    interval
                })
            };
        }
    }
}
