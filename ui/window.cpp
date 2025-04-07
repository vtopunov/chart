#include "window.h"

#include <os/os.h>

#include <ui/event_processors_storage.h>
#include <ui/app.h>


namespace ui
{
    namespace
    {
        using gdi_rect_t = RECT;

        constexpr dword_t parent_window_style{ WS_OVERLAPPEDWINDOW };
        constexpr dword_t child_window_style{ WS_VISIBLE | WS_CHILD };

        [[nodiscard]]
        window_set& windows_global() noexcept
        {
            static window_set set{};
            return set;
        }

        struct by_parent
        {
            window_handle_t parent;
        };

        [[nodiscard]]
        constexpr bool operator < (const window_dependency& left, const by_parent& right) noexcept
        {
            return left.parent < right.parent;
        }

        [[nodiscard]]
        constexpr bool operator < (const by_parent& left, const window_dependency& right) noexcept
        {
            return left.parent < right.parent;
        }

        [[nodiscard]]
        constexpr siblings_window childrens(const window_set& c, window_handle_t parent) noexcept
        {
            const auto first = c.cbegin();

            return
            {
                narrow<size_t>(std::lower_bound(first, c.cend(), by_parent{ parent }) - first),
                std::addressof(c),
                parent
            };
        }

        [[nodiscard]]
        constexpr siblings_window roots(const window_set& c) noexcept
        {
            return { 0u, std::addressof(c), nullptr };
        }

        [[nodiscard]]
        constexpr gdi_rect_t pxsizes_to_gdi(pxsizes sizes) noexcept
        {
            return
            {
                .right{ narrow<decltype(gdi_rect_t::right)>(sizes.width()) },
                .bottom{ narrow<decltype(gdi_rect_t::bottom)>(sizes.height()) }
            };
        }

        [[nodiscard]]
        constexpr pxsizes gdi_to_pxsizes(const gdi_rect_t& rect) noexcept
        {
            static_assert(std::is_unsigned_v<npx_t>);

            constexpr auto side_length = [] (auto p0, auto p1) noexcept
            {
                D_ASSERT(p1 >= p0);
                return narrow<npx_t>(p1 - p0);
            };

            pxsizes result{ side_length(rect.left, rect.right), 0_npx };

            if (result.width()) [[likely]]
            {
                result = result.with_height(side_length(rect.top, rect.bottom));
            }

            return result;
        }

        [[nodiscard]]
        constexpr pxrectangle gdi_to_pxrectangle(const gdi_rect_t& rect) noexcept
        {
            return
            {
                .position{ md_narrow<pxpoint>(rect.left, rect.top) },
                .sizes{ gdi_to_pxsizes(rect) }
            };
        }

        [[nodiscard]]
        gdi_rect_t gdi_geometry(window_handle_t window) noexcept
        {
            gdi_rect_t rect{};
            D_ASSERT_OR_UNUSED(GetClientRect(window, &rect));
            return rect;
        }

        bool close(window_set& windows, window_handle_t window) noexcept
        {
            static window_handle_t in_process_of_destruction{ nullptr };

            if (window != in_process_of_destruction) [[likely]]
            {
                event_processors_global().close_window(window);

                while (const auto children_opt = childrens(windows, window))
                {
                    close(windows, *children_opt);
                }

                if (const auto it = std::find(windows.cbegin(), windows.cend(), window); it != windows.cend())
                {
                    class destruction_locker
                    {
                    public:
                        explicit destruction_locker(const window_dependency& window) noexcept
                            : window_type_holder_{ window.type }
                        {
                            in_process_of_destruction = window.current;
                        }

                        D_DISABLE_COPYMOVE_CA(destruction_locker);

                        ~destruction_locker() noexcept
                        {
                            in_process_of_destruction = nullptr;
                        }

                    private:
                        shared_type_window window_type_holder_;
                    };

                    const destruction_locker lock{ *it };

                    windows.erase(it);

                    if (!windows.size())
                    {
                        quit();
                    }

                    const auto ok = !!DestroyWindow(window);
                    D_ASSERT(ok);
                    return ok;
                }
            }

            return false;
        }

#if D_IS_DEBUG
        [[nodiscard]]
        pxsizes display_resolution() noexcept
        {
            DEVMODEW dev{};
            EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dev);
            return md_narrow<pxsizes>(dev.dmPelsWidth, dev.dmPelsHeight);
        }

        [[nodiscard]]
        inline bool is_maximum_resolution(pxsizes sizes) noexcept
        {
            const auto resolution = ui::display_resolution();
            return sizes.width() >= resolution.width()
                && sizes.height() >= resolution.height();
        }

#endif 
    }

    pxrectangle geometry(window_handle_t window) noexcept
    {
        return gdi_to_pxrectangle(gdi_geometry(window));
    }

    pxsizes sizes(window_handle_t window) noexcept
    {
        return gdi_to_pxsizes(gdi_geometry(window));
    }

    pxsizes adjust_sizes(pxsizes sizes) noexcept
    {
        auto rect = pxsizes_to_gdi(sizes);
        D_ASSERT_OR_UNUSED(AdjustWindowRect(std::addressof(rect), parent_window_style, FALSE));
        return gdi_to_pxsizes(rect);
    }

    bool geometry(window_handle_t window, pxrectangle rc) noexcept
    {
        return !!SetWindowPos
        (
            window, nullptr,
            rc.x0(), rc.y0(),
            rc.width(), rc.height(),
            SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOSENDCHANGING
        );
    }

    pxsizes desktop_sizes() noexcept
    {
        const auto sizes = ::sizes(ui::geometry(::GetDesktopWindow()));
        D_ASSERT(is_maximum_resolution(sizes) || !"Resolution is not high dpi. Add <dpiAware>true</dpiAware> in manifest.");
        return sizes;
    }

    bool close(window_handle_t window) noexcept
    {
        return window && close(windows_global(), window);
    }

    bool window_text(window_handle_t window, wzstring_view text) noexcept
    {
        return !!SetWindowTextW(window, text.c_str());
    }

    siblings_window childrens(window_handle_t window) noexcept
    {
        return childrens(windows_global(), window);
    }

    siblings_window roots() noexcept
    {
        return roots(windows_global());
    }

    bool show(window_handle_t window, int cmd) noexcept
    {
        static_assert(std::is_same_v<int, std::underlying_type_t<show_command>>);
        static_assert(SW_HIDE == to_underlying(show_command::hide));
        static_assert(SW_NORMAL == to_underlying(show_command::normal));
        static_assert(SW_SHOWMINIMIZED == to_underlying(show_command::minimazed));
        static_assert(SW_SHOWMAXIMIZED == to_underlying(show_command::maximazed));
        static_assert(SW_SHOWNOACTIVATE == to_underlying(show_command::inactive));
        static_assert(SW_SHOW == to_underlying(show_command::show));
        static_assert(SW_RESTORE == to_underlying(show_command::restore));

        return !!ShowWindow(window, cmd);
    }

    window create_window(const window_parameters& params) noexcept
    {
        constexpr auto px_to_native = [] (npx_t px) noexcept
        {
            using namespace private_detail_window_constants;
            static_assert(std::is_same_v<decltype(CW_USEDEFAULT), native_npx_t>);
            static_assert(CW_USEDEFAULT == cw_usedefault);
            return (px == px_usedefault) ? cw_usedefault : narrow<native_npx_t>(px);
        };

        window result{};

        prepare(params);
        if (params.cached_type) [[likely]]
        {
            {
                const auto style = (params.parent) ? child_window_style : parent_window_style;

                result = window
                {
                    CreateWindowExW
                    (
                        0u,
                        params.cached_type.r().handle,
                        params.title.c_str(),
                        style,
                        px_to_native(params.geometry.x()),
                        px_to_native(params.geometry.y()),
                        px_to_native(params.geometry.width()),
                        px_to_native(params.geometry.height()),
                        params.parent,
                        nullptr,
                        params.cached_type.r().module,
                        nullptr
                    )
                };
            }

            if (result) [[likely]]
            {
                auto& window_set = windows_global();

                const auto ok = !!window_set.try_emplace
                (
                    std::upper_bound(window_set.cbegin(), window_set.cend(), by_parent{ params.parent }),
                    view(result),
                    params.parent,
                    params.cached_type
                );

                D_ASSERT(ok);
                if (!ok) [[unlikely]]
                {
                    result.reset();
                }
            }
        }

        return result;
    }
}