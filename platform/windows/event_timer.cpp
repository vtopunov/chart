#include "event_timer.h"

#include <platform/windows/event.h>
#include <platform/windows/event_matching.h>

namespace os_windows
{

    namespace
    {
        class event_timer;

        bool kill_timer(const event_timer& timer) noexcept;

        class event_timer
        {
        public:
            constexpr event_timer() noexcept = default;

            constexpr event_timer(HWND window_handle, UINT_PTR id) noexcept
                : window_handle_{ window_handle }
                , id_{ id }
            {}

            constexpr bool is_valid() const noexcept
            {
                return id_ != 0u;
            }

            constexpr UINT_PTR id() const noexcept
            {
                return id_;
            }

            constexpr HWND window_handle() const noexcept
            {
                return window_handle_;
            }

            bool close() noexcept
            {
                if (const auto self = std::exchange(*this, {}); self.id())
                {
                    const auto ok = kill_timer(self);
                    D_ASSERT(ok);
                    return ok;
                }
                return false;
            }

        private:
            HWND window_handle_{ nullptr };
            UINT_PTR id_{ 0u };
        };

        bool kill_timer(const event_timer& timer) noexcept
        {
            return KillTimer(timer.window_handle(), timer.id()) != FALSE;
        }

        using safe_event_timer = shared_handle<event_timer>;

        safe_event_timer create_timer(HWND window_handle, size_t id, std::chrono::milliseconds interval) noexcept
        {
            return event_timer
            {
                window_handle,
                SetTimer
                (
                    window_handle,
                    narrow_cast<UINT_PTR>(id),
                    narrow_cast<UINT>(interval.count()),
                    nullptr
                )
            };
        }

#pragma warning(push)
#pragma warning(disable : 26436) // non-virtual destructor 
        class timer_controller_impl final : public timer_controller
        {
        public:
            timer_controller_impl
            (
                safe_event_timer timer,
                event_timer_callback_t callback,
                event_handler_view handler,
                std::chrono::milliseconds recent_interval
            ) noexcept
                : timer_{std::move(timer)}
                , callback_{std::move(callback)}
                , new_callback_{nullptr}
                , handler_{handler}
                , recent_interval_{recent_interval}
            {}

            event_result operator () (const timer_event& timer_event) noexcept
            {
                D_ASSERT(timer_event.window().handle == timer_->window_handle());

                if (timer_event.id() == timer_->id())
                {
                    const struct collector
                    {
                        timer_controller_impl& self;
                        ~collector() noexcept
                        {
                            if (self.timer_)
                            {
                                if (self.new_callback_)
                                {
                                    self.callback_ = std::exchange(self.new_callback_, nullptr);
                                }
                            }
                            else
                            {
                                self.new_callback_ = nullptr;
                                self.callback_ = nullptr;
                                self.close();
                            }
                        }
                    } collect{ *this };

                    callback_(*this);

                    return accept_event_result;
                }

                return ignore_event_result;
            }

        private:
            bool restart(std::chrono::milliseconds interval) noexcept final
            {
                const auto saved_recent_interval = std::exchange(recent_interval_, std::chrono::milliseconds{ 0 });
                timer_.force_close_all_copies();
                timer_ = create_timer(handler_.window.handle, handler_.id, interval);
                const auto ok = timer_->is_valid();
                recent_interval_ = (ok) ? interval : saved_recent_interval;
                return ok;
            }

            std::chrono::milliseconds interval() const noexcept final
            {
                return recent_interval_;
            }

            window_view window() const noexcept final
            {
                return handler_.window;
            }

            bool close() noexcept final
            {
                recent_interval_ = std::chrono::milliseconds{ 0 };
                timer_.force_close_all_copies();
                new_callback_ = nullptr;
                return unregister_event_handler(std::exchange(handler_, null_event_handler_view));
            }

            void replace_callback(event_timer_callback_t new_callback) noexcept final
            {
                new_callback_ = std::move(new_callback);
            }

        private:
            safe_event_timer timer_;
            event_timer_callback_t callback_;
            event_timer_callback_t new_callback_;
            event_handler_view handler_;
            std::chrono::milliseconds recent_interval_;
        };
#pragma warning(pop)

        struct timer_factory
        {
            event_timer_callback_t callback;
            std::chrono::milliseconds interval;

            event_callback_function operator () (event_handler_view handler) noexcept
            {
                auto timer = create_timer(handler.window.handle, handler.id, interval);

                if (timer && narrow_cast<size_t>(timer->id()) == handler.id)
                {
                    return event_match
                    (
                        timer_controller_impl
                        {
                            std::move(timer),
                            std::move(callback),
                            handler,
                            interval
                        }
                    );
                }

                return nullptr;
            }
        };
    }

    safe_event_handler register_timer(window_view window, std::chrono::milliseconds interval, event_timer_callback_t callback) noexcept
    {
        return register_event_handler_factory(window, timer_factory{ std::move(callback), interval });
    }
}
