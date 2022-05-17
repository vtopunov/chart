#pragma once

#include <core/value_type.h>

namespace private_detail_string_char
{
    template<class C>
    struct string_char_type0
    {
        using method_type = std::conditional_t<
            std::is_pointer_v<C>,
            std::remove_cvref<std::remove_pointer_t<C>>,
            value_type_type<C>
        >;

        using type = typename method_type::type;
    };

    template<class C>
    struct string_char_type
    {
        using type = typename string_char_type0<std::decay_t<C>>::type;
    };

    template<class C>
    using string_char_t = typename string_char_type<C>::type;
}

using private_detail_string_char::string_char_type;
using private_detail_string_char::string_char_t;