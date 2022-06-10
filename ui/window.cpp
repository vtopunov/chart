#include "window.h"

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
                narrow_cast<size_t>(std::lower_bound(first, c.cend(), by_parent{parent}) - first),
                &c,
                parent
            };
        }

        [[nodiscard]]
        constexpr px::rect make_rect_from_gdi(const RECT& rect) noexcept
        {
            return
            {
                narrow2d_cast<px::point2d>(rect.left, rect.top),
                narrow2d_cast<px::point2d>(rect.right, rect.bottom)
            };
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

    px::rect geometry(window_handle_t window) noexcept
    {
        RECT rect{ 0, 0, 0, 0 };
        D_ASSERT_WITH_SIDE_EFFECTS(GetClientRect(window, &rect));
        return make_rect_from_gdi(rect);
    }

    bool geometry(window_handle_t window, px::rect rc) noexcept
    {
        return !!SetWindowPos
        (
            window, nullptr,
            rc.x0(), rc.y0(),
            rc.width(), rc.height(),
            SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOSENDCHANGING
        );
    }

    bool close(window_handle_t window) noexcept
    {
        return window && close(window_container_global(), window);
    }

    window_childrens childrens(window_handle_t window) noexcept
    {
        return childrens(window_container_global(), window);
    }

    window window_builder::build() const noexcept
    {
        constexpr dword_t main_window_style{ WS_OVERLAPPED | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX };
        constexpr dword_t child_window_style{ WS_VISIBLE | WS_CHILD };

        window result;

        if (!cached_type_)
        {
            cached_type_ = type_window_builder{}
                .module(module_)
                .build();
        }

        if (cached_type_)
        {
            const auto style = (parent_) ? child_window_style : main_window_style;

            result = window
            {
                resource_construct,
                CreateWindowExW
                (
                    0,
                    cached_type_.r().name_id,
                    title_.c_str(),
                    style,
                    px_to_native(position_.x()),
                    px_to_native(position_.y()),
                    px_to_native(sizes_.width()),
                    px_to_native(sizes_.height()),
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
                    std::upper_bound
                    (
                        window_set.cbegin(),
                        window_set.cend(),
                        by_parent{ parent_ }
                    ),
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