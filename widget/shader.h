#pragma once

#include <shader/library.h>

#include <widget/event.h>


namespace widget
{
    template<class T>
    using decl_store_viewport_t = decltype(std::declval<const T&>().viewport(px::no_sizes));


    template<class VS, class FS>
    class widget_shader_library : public shader_library<VS, FS>
    {
        using base_type = shader_library<VS, FS>;

    public:
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

        constexpr noapply_t apply(no_overload) const noexcept
        {
            return noapply;
        }
    };
}