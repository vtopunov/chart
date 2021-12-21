#include "window.h"

#include <ui/event_processors_container.h>

namespace ui
{
    namespace
    {
        void break_event_loop() noexcept
        {
            event_processors_global().reset();
            PostQuitMessage(0);
        }

        [[nodiscard]]
        window_container& window_container_global() noexcept
        {
            static window_container map;
            return map;
        }

        struct parent_construct_t
        {};

        struct parent_selector
        {
            constexpr parent_selector(window_resource parent, parent_construct_t) noexcept
                : parent{ parent }
            {}

            constexpr parent_selector(const window_dependency& window) noexcept
                : parent{ window.parent }
            {}

            [[nodiscard]]
            constexpr auto operator <=> (const parent_selector&) const noexcept = default;

            window_resource parent;
        };

        [[nodiscard]]
        constexpr parent_selector as_parent(window_resource parent) noexcept
        {
            return { parent, parent_construct_t{} };
        }

        [[nodiscard]]
        constexpr window_childrens childrens(const window_container& c, window_resource parent) noexcept
        {
            const auto first = c.cbegin();

            return
            {
                narrow_cast<size_t>(std::lower_bound(first, c.cend(), as_parent(parent)) - first),
                &c,
                parent
            };
        }

        [[nodiscard]]
        constexpr px::rect_t make_rect_from_gdi(const RECT& rect) noexcept
        {
            return
            {
                narrow2d_cast<px::point2d_t>(rect.left, rect.top),
                narrow2d_cast<px::point2d_t>(rect.right, rect.bottom)
            };
        }

        [[nodiscard]]
        window_resource desktop_window() noexcept
        {
            return { ::GetDesktopWindow() };
        }

        [[nodiscard]]
        constexpr DWORD select_window_style(window_resource parent) noexcept
        {
            constexpr DWORD main_window_style{ WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX };
            constexpr DWORD child_window_style{ WS_VISIBLE | WS_CHILD };
            return (parent) ? child_window_style : main_window_style;
        }

        bool destroy_window(window_container& window_set, window_resource window) noexcept
        {
            if (const auto it = std::find(window_set.cbegin(), window_set.cend(), window); it != window_set.cend())
            {
                const auto lock_type = it->type;

                window_set.erase(it);

                const bool ok = !!DestroyWindow(window.handle);
                D_ASSERT(ok);
                return ok;
            }

            return false;
        }

        bool close(window_container& window_set, window_resource window) noexcept
        {
            event_processors_global().erase(window);

            while (const auto children_opt = childrens(window_set, window))
            {
                close(window_set, *children_opt);
            }

            return destroy_window(window_set, window);
        }

        bool destroy_window_tree(window_container& window_set, window_resource window) noexcept
        {
            while (const auto children_opt = childrens(window_set, window))
            {
                destroy_window_tree(window_set, *children_opt);
            }

            return destroy_window(window_set, window);
        }
    }

    px::rect_t rect(window_resource window) noexcept
    {
        RECT rect{ 0, 0, 0, 0 };
        D_ASSERT_WITH_SIDE_EFFECTS(GetClientRect(window.handle, &rect));
        return make_rect_from_gdi(rect);
    }

    bool rect(window_resource window, px::rect_t rc) noexcept
    {
        return !!SetWindowPos
        (
            window.handle, nullptr,
            rc.x0(), rc.y0(),
            rc.width(), rc.height(),
            SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOSENDCHANGING
        );
    }

    px::size2d_t desktop_sizes() noexcept
    {
        return rect(desktop_window()).sizes();
    }

    px::size2d_t display_resolution() noexcept
    {
        DEVMODEW dev{};
        EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dev);
        return narrow2d_cast<px::size2d_t>(dev.dmPelsWidth, dev.dmPelsHeight);
    }

    bool show(window_resource window, int cmd) noexcept
    {
        return !!ShowWindow(window.handle, cmd);
    }

    bool update(window_resource window) noexcept
    {
        return !!UpdateWindow(window.handle);
    }

    bool close(window_resource window) noexcept
    {
        if (window)
        {
            struct collector
            {
                constexpr collector() noexcept = default;

                D_DISABLE_COPY_MOVE(collector);

                ~collector() noexcept
                {
                    if (!window_container_global().size())
                    {
                        break_event_loop();
                    }
                }
            };

            [[maybe_unused]]
            const collector temp;

            return close(window_container_global(), window);
        }

        return false;
    }

    void quit() noexcept
    {
        auto& g_window_set = window_container_global();
        if (g_window_set.size())
        {
            window_container window_set{ attach_construct, g_window_set };

            break_event_loop();

            do
            {
                destroy_window_tree(window_set, window_set.cfront());
            }
            while (window_set.size());
        }
    }

    window_childrens childrens(window_resource window) noexcept
    {
        return childrens(window_container_global(), window);
    }

    window_t window_factory::create() noexcept
    {
        window_t result;

        if (!type_)
        {
            type_ = window_type_factory{}.create();
        }

        if (type_)
        {
            const auto style = (style_.has_value()) ? *style_ : select_window_style(parent_);

            result = window_t
            {
                resource_construct,
                CreateWindowExW
                (
                    0,
                    type_.r().name_id,
                    title_.c_str(),
                    style,
                    px_to_native(position_.x()),
                    px_to_native(position_.y()),
                    px_to_native(sizes_.width()),
                    px_to_native(sizes_.height()),
                    parent_.handle,
                    nullptr,
                    type_.r().module_instance,
                    nullptr
                )
            };

            if (result)
            {
                auto& window_set = window_container_global();

                const auto ok = !!window_set.try_emplace
                (
                    std::upper_bound
                    (
                        window_set.cbegin(),
                        window_set.cend(),
                        as_parent(parent_)
                    ),
                    result.r(),
                    parent_,
                    type_
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