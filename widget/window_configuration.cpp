#include <widget/window_configuration.h>

#include <debug/debug.h>

#include <ui/app.h>

#include <widget/window.h>


namespace widget
{
    namespace
    {
        bool shaders_error_report_if_not(bool conditional) noexcept
        {
            if (D_UNLIKELY(!conditional)) D_ATTRIB_UNLIKELY
            {
                e_debug("shaders error: {}", glGetError());
            }
            return conditional;
        }
    }

    bool widget::window_configuration::configure() noexcept
    {
        for (const auto configure : configurations)
        {
            if (cfgs_ & 1)
            {
                if (D_UNLIKELY(!configure(window_))) D_ATTRIB_UNLIKELY
                {
                    return false;
                }
            }
            cfgs_ >>= 1;
        }

        return true;
    }

    bool window_configuration::configure_gray_texture_mix_color_shdr(widget::window& w) noexcept
    {
        return shaders_error_report_if_not(w.shaders.gray_texture_mix_color.initialize(sizes(w)));
    }

    bool window_configuration::configure_colored_rectangle_shdr(widget::window& w) noexcept
    {
        return shaders_error_report_if_not(w.shaders.colored_rectangle.initialize(sizes(w)));
    }

    bool window_configuration::configure_pix8_temp_buffer(widget::window& w) noexcept
    {
        const auto require_size_bytes
            = pix8space{ sizes(w) }
            .size_bytes();

        if (D_UNLIKELY(!w.temp_buffer.try_reserve(require_size_bytes))) D_ATTRIB_UNLIKELY
        {
            e_debug("out of memory temp buffer: require {} bytes", require_size_bytes);
            return false;
        }

        return true;
    }
}