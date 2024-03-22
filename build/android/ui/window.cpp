#include <ui/window.h>

#include <android/native_window.h>
#include <entry_point/android_native_app_glue.h>

namespace ui
{
    namespace
    {
        template<class T>
        constexpr pxsize_t to_npx(T value) noexcept
        {
            return narrow<pxsize_t>(clamp_to_unsigned(value));
        }

        window_handle_t startup_request(module_handle_t module) noexcept
        {
            if (module) [[likely]]
            {
                constexpr int max_timeout_ms = 3000;

                while (!(module->destroyRequested))
                {
                    if (module->window)
                    {
                        return module->window;
                    }

                    int events{};
                    android_poll_source* source{ nullptr };

                    const auto ident = ALooper_pollOnce(max_timeout_ms, nullptr, &events, (void**)&source);
                    if (ident >= 0)
                    {
                        if (source && source->process)
                        {
                            source->process(module, source);
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

    pxsize2d sizes(window_handle_t window) noexcept
    {
        static_assert(std::is_unsigned_v<pxsize_t>);

        pxsize2d result{ to_npx(ANativeWindow_getWidth(window)), 0_npx };

        if (result.width()) [[likely]]
        {
            result = result.with_height(to_npx(ANativeWindow_getHeight(window)));
        }

        return result;
    }

    window create_window(const window_parameters& params) noexcept
    {
        prepare(params);

        return 
        {
            params.cached_type,
            startup_request(params.cached_type.r().module)
        };
    }
}
