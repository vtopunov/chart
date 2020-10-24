#pragma once

#include <optional>
#include <variant>

#include <core/rect.h>
#include <core/small_vector.h>

#include <display/window_type.h>


namespace display
{   
    inline constexpr auto use_default_px = narrow_cast<pixel_t>( CW_USEDEFAULT );
    inline constexpr auto use_default_vec = fill_vec(use_default_px);

    constexpr void px_by_default(pixel_t& value, pixel_t new_px) noexcept
    {
        if (value == use_default_px)
        {
            value = new_px;
        }
    }

    constexpr void px_by_default(vec_t& value, vec_t new_vec) noexcept
    {
        px_by_default(value._0, new_vec._0);
        px_by_default(value._1, new_vec._1);
    }

    struct window_dependency
    {
        window_resource current;
        window_resource parent;
        window_type_t type;

        constexpr operator window_resource() const noexcept
        {
            return current;
        }
    };

    using window_container = small_vector<window_dependency, 6>;

    struct window_childrens
    {
        using container_type = window_container;
        using reference = container_type::const_reference;

        size_t position;
        const container_type* contaner;
        window_resource parent;

        [[nodiscard]]
        constexpr window_childrens begin() const noexcept
        {
            return *this;
        }

        [[nodiscard]]
        constexpr null_t<window_childrens> end() const noexcept
        {
            return {};
        }

        [[nodiscard]]
        constexpr explicit operator bool() const noexcept
        {
            return position < std::size(*contaner) && parent == value().parent;
        }

        constexpr window_childrens& operator++() noexcept
        {
            ++position;
            return *this;
        }

        [[nodiscard]]
        constexpr reference value() const noexcept
        {
#pragma warning(push)
#pragma warning(disable : 26446) // Prefer to use gsl::at() instead of unchecked subscript operator
            return (*contaner)[position];
#pragma warning(pop)
        }

        [[nodiscard]]
        constexpr reference operator*() const noexcept
        {
            return value();
        }
    };

    [[nodiscard]]
    window_childrens childrens(window_resource parent) noexcept;

    bool show(window_resource window, int cmd = SW_SHOW) noexcept;

    bool update(window_resource window) noexcept;

    bool close(window_resource window) noexcept;

    void quit() noexcept;

    [[nodiscard]]
    rect_t rect(window_resource window) noexcept;

    bool rect(window_resource window, rect_t rc) noexcept;

    [[nodiscard]]
    rect_t display_rect() noexcept;

    using window_t = unique_resource<window_resource>;

    class window_factory
    {
    public:
        window_factory& type(unique_window_type_t type) noexcept
        {
            type_ = std::move(type);
            return *this;
        }

        window_factory& title(std::wstring title) noexcept
        {
            title_ = std::move(title);
            return *this;
        }

        constexpr window_factory& parent(window_resource window) noexcept
        {
            parent_ = window;
            return *this;
        }

        constexpr window_factory& position(vec_t position) noexcept
        {
            position_ = position;
            return *this;
        }


        constexpr window_factory& position(pixel_t x, pixel_t y) noexcept
        {
            return position(vec_t{ x, y });
        }

        constexpr window_factory& position_by_default(vec_t position) noexcept
        {
            px_by_default(position_, position);
            return *this;
        }

        constexpr window_factory& sizes(vec_t sizes) noexcept
        {
            sizes_ = sizes;
            return *this;
        }

        constexpr window_factory& sizes(pixel_t width, pixel_t height) noexcept
        {
            return sizes(vec_t{ width, height });
        }

        constexpr window_factory& sizes_by_default(vec_t sizes) noexcept
        {
            px_by_default(sizes_, sizes);
            return *this;
        }

        constexpr window_factory& rect(const rect_t& rc) noexcept
        {
            return position(rc.v00()).sizes(rc.sizes());
        }

        constexpr window_factory& rect_by_default(const rect_t& rc) noexcept
        {
            return position_by_default(rc.v00()).sizes_by_default(rc.sizes());
        }

        [[nodiscard]]
        window_t create() noexcept;

    private:
        window_type_t type_;
        std::wstring title_;
        window_resource parent_{ nullwindow };
        std::optional<DWORD> style_;
        vec_t position_{ use_default_vec };
        vec_t sizes_{ use_default_vec };
    };
}