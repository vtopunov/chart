#include <widget/window_configation.h>

#include <debug/debug.h>

#include <ui/app.h>

#ifdef D_OS_ANDROID
#include <ui/window.h>
#endif

#include <widget/window.h>


namespace widget
{
    namespace private_detail_window_configation
    {
        namespace
        {
            bool configure(widget::window& window, size_t cfgs) noexcept
            {
                constexpr auto last_configure_mask = configure_mask_v<n_configuration - 1u>;
                constexpr auto configure_maxmask = last_configure_mask | (last_configure_mask - 1u);
                static_assert(configure_maxmask < to_underlying(badcfg));

                if (cfgs <= configure_maxmask) [[likely]]
                {
                    for (const auto configure : configurations)
                    {
                        if (cfgs & 1u)
                        {
                            if (!configure(window)) [[unlikely]]
                            {
                                break;
                            }
                        }

                        cfgs >>= 1u;
                    }
                }

                return !cfgs;
            }
        }

        bool configure_gray_texture_mix_color_shdr(widget::window& w) noexcept
        {
            return w.shaders.gray_texture_mix_color.initialize(w.viewport);
        }

        bool configure_colored_rectangle_shdr(widget::window& w) noexcept
        {
            return w.shaders.colored_rectangle.initialize(w.viewport);
        }

        bool configure_pix8_temp_buffer(widget::window& w) noexcept
        {
            const auto require_size_bytes 
                = pix8space{ w.viewport }.size_bytes();

            if (!w.temp_buffer.try_reserve(require_size_bytes)) [[unlikely]]
            {
                e_debug("out of memory temp buffer: require {} bytes", require_size_bytes);
                return false;
            }

            return true;
        }
    }

    bool configure(widget::window& window, window_configation cfgs) noexcept
    {
        return private_detail_window_configation::configure(window, to_underlying(cfgs));
    }
}