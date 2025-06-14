#pragma once

#include <shader/library.h>

#include <widget/event.h>


namespace widget
{
    template<class T>
    using decl_store_viewport_t = decltype(std::declval<const T&>().viewport(no_sizes));


    template<class VS, class FS>
    class widget_shader_library : public shader_library<VS, FS>
    {
        using base_type = shader_library<VS, FS>;

    public:
        constexpr widget_shader_library() noexcept = default;

        D_DISABLE_COPYMOVE_CA(widget_shader_library);

        [[nodiscard]] bool operator()(basic_initialization_event<>) noexcept
        {
            return this->load();
        }

        void operator()(viewport_event<> e) const noexcept
        {
            if constexpr (is_detected_v<decl_store_viewport_t, base_type>)
            {
                this->use();
                this->viewport(e.viewport());
            }
        }

        constexpr dummy apply(no_overload) const noexcept
        {
            return dummy_v;
        }
    };
}