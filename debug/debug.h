#pragma once

#include <core/fmt.h>
#include <core/utility.h>

#include <os/fwd.h>


D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_enum_is_unscoped__prefer_enum_class)
D_WARNING_DISABLE_MSVC(W_potential_comparison_of_a_constant_with_another_constant)

namespace private_detail_debug
{
    enum log_priority
    {
        LOG_UNKNOWN,
        LOG_DEFAULT,
        LOG_VERBOSE,
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR,
        LOG_FATAL,
        LOG_SILENT
    };

#if defined(D_OS_WINDOWS)
    constexpr char priority_tag(log_priority p) noexcept
    {
        constexpr char priority_tags[]{ 'U', 'D', 'V', 'D', 'I', 'W', 'E', 'F', 'S' };
        constexpr auto n_tags = std::size(priority_tags);

        static_assert(!log_priority::LOG_UNKNOWN);
        static_assert(log_priority::LOG_SILENT == (n_tags - 1u));
        const auto tag
            = (p >= log_priority::LOG_UNKNOWN && p <= log_priority::LOG_SILENT)
            ? priority_tags[p] : '\0';
        D_ASSERT(tag);

        return tag;
    }

    template<class CharT>
    struct debug_overhead
    {
        static constexpr CharT tagend[]{ ':', ' ' };
        static constexpr CharT endl[]{ '\n', '\0' };
        static constexpr auto n_prefix = std::size(tagend) + 1u;
        static constexpr auto n_suffix = std::size(endl);
        static constexpr auto n_overhead = n_prefix + n_suffix;

        template<size_t NBuf>
        static constexpr void test_buffer([[maybe_unused]] CharT(&buf)[NBuf]) noexcept
        {
            static_assert(n_prefix < NBuf);
            static_assert(n_suffix < NBuf);
            static_assert(n_overhead < NBuf);
        }

        static constexpr void prefix_write(CharT* out, log_priority priority) noexcept
        {
            *out = priority_tag(priority);
            std::copy(std::cbegin(tagend), std::cend(tagend), ++out);
        }

        static constexpr void suffix_write(CharT* out) noexcept
        {
            std::copy(std::cbegin(endl), std::cend(endl), out);
        }
    };

    void output_debug_string(const char* string) noexcept;

    void output_debug_string(const wchar_t* string) noexcept;

    template<class CharT>
    void priority_output_debug_string(log_priority, const CharT* string) noexcept
    {
        output_debug_string(string);
    }

#elif defined(D_OS_ANDROID)
    void priority_output_debug_string(log_priority priority, const char* string) noexcept;

    template<class CharT>
    struct debug_overhead
    {
        static constexpr size_t n_prefix{ 0u };
        static constexpr size_t n_suffix{ 1u };
        static constexpr auto n_overhead = n_prefix + n_suffix;

        template<size_t NBuf>
        static constexpr void test_buffer(CharT(&buf)[NBuf]) noexcept
        {
            static_assert(n_prefix < NBuf);
            static_assert(n_suffix < NBuf);
            static_assert(n_overhead < NBuf);
        }

        static constexpr void prefix_write(CharT*, log_priority) noexcept
        {}

        static constexpr void suffix_write(CharT* out) noexcept
        {
            *out = '\0';
        }
    };
#endif

    template<class FormatString, class... Args>
    void priority_debug(log_priority priority, const FormatString& format_string, const Args&... args) noexcept
    {
        using format_char_t = string_char_t<FormatString>;

        constexpr debug_overhead<format_char_t> overhead{};

        if constexpr (overhead.n_prefix || (overhead.n_suffix > 1u) || sizeof...(Args))
        {
            using format_string_view = std::basic_string_view<format_char_t>;

            constexpr size_t n_buf{ 512u };
            format_char_t buffer[n_buf];

            overhead.test_buffer(buffer);
            overhead.prefix_write(buffer, priority);

            const auto result = fmt::format_to_n
            (
                buffer + overhead.n_prefix,
                n_buf - overhead.n_overhead,
                static_cast<format_string_view>(format_string),
                args...
            );

            overhead.suffix_write(result.out);

            priority_output_debug_string(priority, buffer);
        }
        else
        {
            priority_output_debug_string(priority, format_string);
        }
    }

    template<class FormatString, class... Args>
    void debug(const FormatString& format_string, const Args&... args) noexcept
    {
        priority_debug(LOG_DEBUG, format_string, args...);
    }

    template<class FormatString, class... Args>
    void w_debug(const FormatString& format_string, const Args&... args) noexcept
    {
        priority_debug(LOG_WARN, format_string, args...);
    }

    template<class FormatString, class... Args>
    void e_debug(const FormatString& format_string, const Args&... args) noexcept
    {
        priority_debug(LOG_ERROR, format_string, args...);
    }

    template<class FormatString, class... Args>
    void fatal_debug(const FormatString& format_string, const Args&... args) noexcept
    {
        priority_debug(LOG_FATAL, format_string, args...);
        D_ASSERT(!"fatal");
    }
}
D_WARNING_POP


using private_detail_debug::debug;
using private_detail_debug::w_debug;
using private_detail_debug::e_debug;
using private_detail_debug::fatal_debug;