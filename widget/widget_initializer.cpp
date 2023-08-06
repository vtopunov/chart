#include <widget/widget_initializer.h>

#include <debug/debug.h>

#include <ui/app.h>

#include <widget/window.h>


namespace widget
{
    namespace
    {
        bool shaders_error_report_if_not(bool conditional) noexcept
        {
            if (!conditional) [[unlikely]]
            {
                e_debug("shaders error: {}", glGetError());
            }
            return conditional;
        }
    }

    bool widget_initializer::configuration_accumulator::configure_gray_texture_mix_color_shdr(widget::window& w) noexcept
    {
        return shaders_error_report_if_not(w.shaders.gray_texture_mix_color.initialize(sizes(w)));
    }

    bool widget_initializer::configuration_accumulator::configure_colored_rectangle_shdr(widget::window& w) noexcept
    {
        return shaders_error_report_if_not(w.shaders.colored_rectangle.initialize(sizes(w)));
    }

    bool widget_initializer::configuration_accumulator::configure_pix8_temp_buffer(widget::window& w) noexcept
    {
        const auto require_size_bytes
            = pix8space{ sizes(w) }
            .size_bytes();

        if (!w.temp_buffer.try_reserve(require_size_bytes)) [[unlikely]]
        {
            e_debug("out of memory temp buffer: require {} bytes", require_size_bytes);
            return false;
        }

        return true;
    }
}