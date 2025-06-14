#include <ui/window.h>

#include <android/native_window.h>
#include <entry_point/android_native_app_glue.h>
#include <common/app.h>

namespace ui
{
    namespace
    {
        template<class T>
        constexpr npx_t to_npx(T value) noexcept
        {
            return narrow<npx_t>(clamp_to_unsigned(value));
        }

        window_handle_t startup_request() noexcept
        {
            if (const auto app = common::app()) [[likely]]
            {
                constexpr int max_timeout_ms = 3000;

                while (!(app->destroyRequested))
                {
                    if (app->window)
                    {
                        return app->window;
                    }

                    int events{};
                    android_poll_source* source{ nullptr };

                    const auto ident = ALooper_pollOnce(max_timeout_ms, nullptr, &events, (void**)&source);
                    if (ident >= 0)
                    {
                        if (source && source->process)
                        {
                            source->process(app, source);
                        }
                    }
                    else
                    {
                        if (ALOOPER_POLL_TIMEOUT == ident || ALOOPER_POLL_ERROR == ident)
                        {
                            break;
                        }
                    }
                }
            }

            return nullptr;
        }
    }

    pxsizes sizes(window_handle_t window) noexcept
    {
        static_assert(std::is_unsigned_v<npx_t>);

        pxsizes result{ to_npx(ANativeWindow_getWidth(window)), 0_npx };

        if (result.width()) [[likely]]
        {
            result = result.with_height(to_npx(ANativeWindow_getHeight(window)));
        }

        return result;
    }

    window create_window(const window_parameters& params) noexcept
    {
        window w{ .type{ params.type_builder.build() } };

        if(w.type) [[likely]]
        {
            w.handle = startup_request();
        }

        return w;
    }
}
