#pragma once

#include <iterator>

#include <widget/window_fwd.h>


namespace widget
{
    class window_configuration
    {
    public:
        using cfgbits_t = size_t;

        constexpr explicit window_configuration(widget::window& w) noexcept
            : window_{ w }
        {}

        class builder
        {
        public:
            constexpr explicit builder(cfgbits_t& cfgs) noexcept
                : cfgs_{ cfgs }
            {}

            constexpr const builder& gray_texture_mix_color_shdr() const noexcept
            {
                cfgs_ |= configure_fn_mask_v<configure_gray_texture_mix_color_shdr>;
                return *this;
            }

            constexpr const builder& colored_rectangle_shdr() const noexcept
            {
                cfgs_ |= configure_fn_mask_v<configure_colored_rectangle_shdr>;
                return *this;
            }

            constexpr const builder& pix8_temp_buffer() const noexcept
            {
                cfgs_ |= configure_fn_mask_v<configure_pix8_temp_buffer>;
                return *this;
            }

        private:
            cfgbits_t& cfgs_;
        };

        constexpr const builder build() noexcept
        {
            return builder{ cfgs_ };
        }

        constexpr const window& window() const noexcept
        {
            return window_;
        }

        bool configure() noexcept;

    private:
        static bool configure_gray_texture_mix_color_shdr(widget::window&) noexcept;
        static bool configure_colored_rectangle_shdr(widget::window&) noexcept;
        static bool configure_pix8_temp_buffer(widget::window&) noexcept;

    private:
        using configure_fn_t = bool (*)(widget::window&);

        static constexpr configure_fn_t configurations[]
        {
            configure_gray_texture_mix_color_shdr,
            configure_colored_rectangle_shdr,
            configure_pix8_temp_buffer
        };

        static constexpr auto n_configuration = std::size(configurations);
        static_assert(std::is_unsigned_v<cfgbits_t>);
        static_assert(n_configuration <= sizeof(cfgbits_t));

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

            static constexpr auto mask = cfgbits_t(1u) << index;
        };

        template<configure_fn_t CfgFn>
        static constexpr auto configure_fn_mask_v = configure_fn_traits<CfgFn>::mask;

    private:
        widget::window& window_;
        cfgbits_t cfgs_{0u};
    };
}

using widget::window_configuration;
