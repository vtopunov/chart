#include <ui/window.h>

#include <android/native_window.h>


namespace ui
{
    namespace
    {
        template<class T>
        constexpr pxsize_t to_npx(T value) noexcept
        {
            return narrow<pxsize_t>(clamp_to_unsigned(value));
        }
    }

    pxsize2d sizes(window_handle_t window) noexcept
    {
        static_assert(std::is_unsigned_v<pxsize_t>);
        
        pxsize2d result{ to_npx(ANativeWindow_getWidth(window)), 0_npx };

        if (result.width()) [[likely]]
        {
            result = result.with_height(to_npx(ANativeWindow_getHeight(window)));
        }

        return result;
    }
}
