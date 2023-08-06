#pragma once

#include <iterator>

#include <core/utility.h>

#include <widget/window_fwd.h>


namespace widget
{
    class widget_initializer
    {
    public:
        D_DISABLE_COPY_MOVE(widget_initializer);

        constexpr widget_initializer(const widget::window& window) noexcept
            : window_{ window }
        {}

        class configuration_accumulator
        {
            using configure_fn_t = bool (*)(widget::window&);
            using cfgbits_t = size_t;

        public:
            constexpr configuration_accumulator() noexcept = default;
            D_DISABLE_COPY_MOVE(configuration_accumulator);

            class configuration_set
            {
            public:
                constexpr explicit configuration_set(const configuration_accumulator& cfgs) noexcept
                    : cfgs_{ cfgs.cfgs_ }
                {}

                class const_iterator
                {
                public:
                    constexpr explicit const_iterator(const configuration_set& cfgs) noexcept
                        : fn_cfgs_{ data_configurations }
                        , cfgs_{ cfgs.cfgs_ }
                    {}

                    class configure_fn_wrapper
                    {
                    public:
                        constexpr explicit configure_fn_wrapper(const const_iterator& it) noexcept
                            : fn_{ (it.cfgs_ & 1) ? *(it.fn_cfgs_) : nullptr }
                        {}

                        [[nodiscard]]
                        bool operator () (widget::window& window) const noexcept
                        {
                            return !fn_ || fn_(window);
                        }

                    private:
                        configure_fn_t fn_;
                    };

                    [[nodiscard]]
                    constexpr configure_fn_wrapper operator*() const noexcept
                    {
                        return configure_fn_wrapper{ *this };
                    }

                    constexpr const_iterator& operator++() noexcept
                    {
                        ++fn_cfgs_;
                        cfgs_ >>= 1;
                        return *this;
                    }

                    struct end_t {};

                    [[nodiscard]]
                    constexpr bool operator != (end_t) const noexcept
                    {
                        return end_data_configurations != fn_cfgs_;
                    }

                private:
                    const configure_fn_t* fn_cfgs_;
                    cfgbits_t cfgs_;
                };

                [[nodiscard]]
                constexpr const_iterator begin() const noexcept
                {
                    return const_iterator{ *this };
                }

                [[nodiscard]]
                constexpr typename const_iterator::end_t end() const noexcept
                {
                    return {};
                }

            private:
                cfgbits_t cfgs_;
            };

            constexpr operator configuration_set () const noexcept
            {
                return configuration_set{ *this };
            }

            constexpr configuration_accumulator& gray_texture_mix_color_shdr() noexcept
            {
                cfgs_ |= configure_fn_mask_v<configure_gray_texture_mix_color_shdr>;
                return *this;
            }

            constexpr configuration_accumulator& colored_rectangle_shdr() noexcept
            {
                cfgs_ |= configure_fn_mask_v<configure_colored_rectangle_shdr>;
                return *this;
            }

            constexpr configuration_accumulator& pix8_temp_buffer() noexcept
            {
                cfgs_ |= configure_fn_mask_v<configure_pix8_temp_buffer>;
                return *this;
            }

        private:
            static bool configure_gray_texture_mix_color_shdr(widget::window&) noexcept;
            static bool configure_colored_rectangle_shdr(widget::window&) noexcept;
            static bool configure_pix8_temp_buffer(widget::window&) noexcept;

        private:
            static constexpr configure_fn_t configurations[]
            {
                configure_gray_texture_mix_color_shdr,
                configure_colored_rectangle_shdr,
                configure_pix8_temp_buffer
            };

            static constexpr auto data_configurations = std::data(configurations);
            static constexpr auto n_configuration = std::size(configurations);
            static constexpr auto end_data_configurations = data_configurations + n_configuration;
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
            cfgbits_t cfgs_{ 0u };
        };

        [[nodiscard]]
        constexpr configuration_accumulator& cfg() noexcept
        {
            return cfgs_;
        }

        [[nodiscard]]
        constexpr const widget::window& window() const noexcept
        {
            return window_;
        }

    private:
        const widget::window& window_;
        configuration_accumulator cfgs_{};
    };

    using configuration_set = widget_initializer::configuration_accumulator::configuration_set;

    inline bool configure(widget::window& window, const configuration_set& cfgs) noexcept
    {
        for (const auto configure : cfgs)
        {
            if (!configure(window)) [[unlikely]]
            {
                return false;
            }
        }

        return true;
    }
}

using widget::widget_initializer;
