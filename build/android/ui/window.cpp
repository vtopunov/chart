#include <ui/window.h>

#include <android/native_window.h>

#include <core/clamp_cast.h>


namespace ui
{
    namespace
    {
        template<class T>
        constexpr pxside_t to_px(T value) noexcept
        {
            return narrow<pxside_t>(clamp_to_unsigned(value));
        }
    }

    pxsize2d sizes(window_handle_t window) noexcept
    {
        static_assert(std::is_unsigned_v<pxside_t>);
        
        pxsize2d result{ to_px(ANativeWindow_getWidth(window)), 0_px };

        if (result.width()) [[likely]]
        {
            result = result.with_height(to_px(ANativeWindow_getHeight(window)));
        }

        return result;
    }
}
