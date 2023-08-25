#pragma once

#include <iterator>

#include <widget/fwd.h>


namespace widget
{
    namespace private_detail_window_configation
    {
        bool configure_gray_texture_mix_color_shdr(widget::window&) noexcept;
        bool configure_colored_rectangle_shdr(widget::window&) noexcept;
        bool configure_pix8_temp_buffer(widget::window&) noexcept;

        using configure_fn_t = bool (*)(widget::window&);

        constexpr configure_fn_t configurations[]
        {
            configure_gray_texture_mix_color_shdr,
            configure_colored_rectangle_shdr,
            configure_pix8_temp_buffer
        };

        static constexpr auto n_configuration = std::size(configurations);
        static_assert(0u < n_configuration);
        static_assert(n_configuration < to_underlying(window_configation::badcfg));

        template<size_t index>
        constexpr auto configure_mask_v = size_t{ 1u } << index;

        template<configure_fn_t CfgFn>
        struct configure_fn_traits
        {
            static constexpr size_t index = [] () noexcept
            {
                size_t i{ 0 };
                for (; i != n_configuration; ++i)
                {
                    if (configurations[i] == CfgFn)
                    {
                        break;
                    }
                }
                return i;
            }();

            static_assert(index < n_configuration);

            static constexpr auto mask = configure_mask_v<index>;
        };

        template<configure_fn_t CfgFn>
        constexpr auto configure_fn_mask_v = configure_fn_traits<CfgFn>::mask;
        static_assert(to_underlying(nocfg) < configure_fn_mask_v<configurations[0]>);

        template<configure_fn_t CfgFn>
        constexpr auto e_configure_fn_mask_v = underlying_cast<window_configation>(configure_fn_mask_v<CfgFn>);

        namespace enable_window_configation_constants
        {
            constexpr auto enable_gray_texture_mix_color_shdr = e_configure_fn_mask_v<configure_gray_texture_mix_color_shdr>;
            constexpr auto enable_colored_rectangle_shdr = e_configure_fn_mask_v<configure_colored_rectangle_shdr>;
            constexpr auto enable_pix8_temp_buffer = e_configure_fn_mask_v<configure_pix8_temp_buffer>;
        }
    }

    using namespace private_detail_window_configation::enable_window_configation_constants;

    bool configure(widget::window& window, window_configation cfgs) noexcept;
}

using namespace widget::private_detail_window_configation::enable_window_configation_constants;
