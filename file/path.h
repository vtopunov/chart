#pragma once

#include <string_view>
#include <string>
#include <filesystem>

#include <core/zstring_view.h>

#include <os/fwd.h>


namespace file
{
    using path = std::filesystem::path;
    using path_char_t = typename path::value_type;
    using path_zstring_view = basic_zstring_view<path_char_t>;
    using path_string_view = std::basic_string_view<path_char_t>;
    using path_string = std::basic_string<path_char_t>;

    namespace private_detail_path
    {
        template<class T>
        struct string_literal
        {
            using value_type = std::remove_const_t<T>;
            using const_pointer = const value_type*;
            using zstring_view_type = basic_zstring_view<value_type>;
            using string_view_type = std::basic_string_view<value_type>;
            using string_type = std::basic_string<value_type>;
            

            const_pointer string_literal;
            size_t size_literal;

            constexpr operator const_pointer()  const noexcept
            {
                return string_literal;
            }

            constexpr operator zstring_view_type() const noexcept
            {
                return string_literal;
            }

            constexpr operator string_view_type() const noexcept
            {
                return { string_literal, size_literal };
            }

            operator string_type() const noexcept
            {
                return { string_literal, size_literal };
            }

            operator std::filesystem::path() const noexcept
            {
                return string_type{ string_literal, size_literal };
            }
        };
    }

    using path_string_literal_t = private_detail_path::string_literal<path_char_t>;

    namespace literals
    {
        [[nodiscard]]
        constexpr path_string_literal_t operator"" _path(const path_char_t* string, size_t size) noexcept
        {
            return { string, size };
        }
    }
}

using namespace file::literals;


#ifdef D_OS_WINDOWS
#define _PATH(x)  L##x##_path
#else
#define _PATH(x)  x##_path
#endif