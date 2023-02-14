#include "window.h"

#include <algorithm>

#include <os/os.h>

#include <ui/event_processors_container.h>

namespace ui
{
    namespace
    {
        [[nodiscard]]
        window_container& window_container_global() noexcept
        {
            static window_container map;
            return map;
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
        constexpr window_childrens childrens(const window_container& c, window_handle_t parent) noexcept
        {
            const auto first = c.cbegin();

            return
            {
                narrow_cast<size_t>(std::lower_bound(first, c.cend(), by_parent{ parent }) - first),
                std::addressof(c),
                parent
            };
        }

        [[nodiscard]]
        constexpr window_roots roots(const window_container& c) noexcept
        {
            size_t position{ 0 };
            for (const auto& value : c)
            {
                if (value.is_root())
                {
                    break;
                }

                ++position;
            }

            return { position, std::addressof(c) };
        }

        [[nodiscard]] constexpr pxsize2d sizes(const RECT& rect) noexcept
        {
            constexpr auto side_length = [] (auto p0, auto p1) noexcept
            {
                D_ASSERT(p1 >= p0);
                return narrow_cast<pxside_t>(p1 - p0);
            };

            return
            {
                side_length(rect.left, rect.right),
                side_length(rect.top, rect.bottom)
            };
        }

        [[nodiscard]]
        constexpr pxrectangle make_rectangle(const RECT& rect) noexcept
        {
            return
            {
                .position{ narrow2d_cast<pxpoint2d>(rect.left, rect.top) },
                .sizes{ sizes(rect) }
            };
        }

        [[nodiscard]]
        RECT gdi_geometry(window_handle_t window) noexcept
        {
            RECT rect{ 0, 0, 0, 0 };
            D_ASSERT_OR_UNUSED(GetClientRect(window, &rect));
            return rect;
        }

        bool close(window_container& window_set, window_handle_t window) noexcept
        {
            static window_handle_t in_process_of_destruction{ nullptr };

            if (window != in_process_of_destruction)
            {
                event_processors_global().erase(window);

                while (const auto children_opt = childrens(window_set, window))
                {
                    close(window_set, *children_opt);
                }

                if (const auto it = std::find(window_set.cbegin(), window_set.cend(), window); it != window_set.cend())
                {
                    class destruction_locker
                    {
                    public:
                        explicit destruction_locker(const window_dependency& window) noexcept
                            : window_type_holder_{ window.type }
                        {
                            in_process_of_destruction = window.current;
                        }

                        D_DISABLE_COPY_MOVE(destruction_locker);

                        ~destruction_locker() noexcept
                        {
                            in_process_of_destruction = nullptr;
                        }

                    private:
                        shared_type_window window_type_holder_;
                    };

                    const destruction_locker lock{ *it };

                    window_set.erase(it);

                    if (!window_set.size())
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
    }

    pxrectangle geometry(window_handle_t window) noexcept
    {
        return make_rectangle(gdi_geometry(window));
    }

    pxsize2d sizes(window_handle_t window) noexcept
    {
        return sizes(gdi_geometry(window));
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

    pxsize2d desktop_sizes() noexcept
    {
        return sizes(geometry(::GetDesktopWindow()));
    }

    bool close(window_handle_t window) noexcept
    {
        return window && close(window_container_global(), window);
    }

    window_childrens childrens(window_handle_t window) noexcept
    {
        return childrens(window_container_global(), window);
    }

    window_roots roots() noexcept
    {
        return roots(window_container_global());
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

    window window_builder::build() const noexcept
    {
        constexpr auto px_to_native = [] (pxside_t px) noexcept
        {
            using namespace private_detail_window;
            static_assert(std::is_same_v<decltype(CW_USEDEFAULT), native_px_t>);
            static_assert(CW_USEDEFAULT == cw_usedefault);
            return (px == px_usedefault) ? cw_usedefault : narrow_cast<native_px_t>(px);
        };

        constexpr auto select_window_style = [] (bool has_parent) noexcept
        {
            constexpr dword_t main_window_style{ WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX };
            constexpr dword_t child_window_style{ WS_VISIBLE | WS_CHILD };
            return (has_parent) ? child_window_style : main_window_style;
        };

        window result;

        if (!cached_type_)
        {
            cached_type_ = type_builder_.build();
        }

        if (cached_type_)
        {
            result = window
            {
                resource_construct,
                CreateWindowExW
                (
                    0,
                    cached_type_.r().name_id,
                    title_.c_str(),
                    select_window_style(!!parent_),
                    px_to_native(geometry_.x()),
                    px_to_native(geometry_.y()),
                    px_to_native(geometry_.width()),
                    px_to_native(geometry_.height()),
                    parent_,
                    nullptr,
                    cached_type_.r().module,
                    nullptr
                )
            };

            if (result)
            {
                auto& window_set = window_container_global();

                const auto ok = !!window_set.try_emplace
                (
                    std::upper_bound(window_set.cbegin(), window_set.cend(), by_parent{ parent_ }),
                    result.r(),
                    parent_,
                    cached_type_
                );

                D_ASSERT(ok);

                if (!ok)
                {
                    result.reset();
                }
            }
        }

        return result;
    }
}