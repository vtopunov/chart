#include <filesystem>
#include <string_view>
#include <iostream>

#include <core/small_vector.h>

namespace
{
    struct chars_sequence
    {
        char first;
        char last;

        constexpr bool contains(const char value) const noexcept
        {
            return value >= first && value <= last;
        }
    };

    struct locale
    {
        chars_sequence upper_abc;
        chars_sequence lower_abc;
        char slash;
        char backslash;

        constexpr bool is_lower(const char value) const noexcept
        {
            return lower_abc.contains(value);
        }

        constexpr bool is_upper(const char value) const noexcept
        {
            return upper_abc.contains(value);
        }

        constexpr bool is_slash(const char value) const noexcept
        {
            return value == slash
                || value == backslash;
        }

        constexpr char lower2upper(const char value) const noexcept
        {
            return upper_abc.first + ( value - lower_abc.first );
        }

        constexpr char upper2lower(const char value) const noexcept
        {
            return lower_abc.first + ( value - upper_abc.first );
        }
    };

    constexpr locale ascii
    {
        .upper_abc = { 'A', 'Z' },
        .lower_abc = { 'a', 'z' },
        .slash = '/',
        .backslash = '\\'
    };

    constexpr char preferred_path_separator = ascii.backslash;
    constexpr char alternative_path_separator = ascii.slash;

    constexpr char unipathchar(char value) noexcept
    {
        if ( value == alternative_path_separator )
        {
            value = preferred_path_separator;
        }
        else if ( ascii.is_upper(value) )
        {
            value = ascii.upper2lower(value);
        }

        return value;
    }

    constexpr std::string_view unipath(std::span<char> path) noexcept
    {
        if ( path.size() && ascii.is_slash(path.back()) )
        {
            path = path.first(path.size() - 1u);
        }

        for ( auto& ch : path )
        {
            ch = unipathchar(ch);
        }

        return { std::data(path), std::size(path) };
    }

    constexpr bool is_div_char(const char value) noexcept
    {
        return value == ';'
            || value == '<'
            || value == '>'
            || value == '|'
            || value == '\n'
            || value == '\''
            || value == '\"';
    }

    using cstrspan = std::span<const char>;

    constexpr bool is_valid_cd(cstrspan chars) noexcept
    {
        bool has_slash{ false };

        for ( const auto ch : chars )
        {
            if ( is_div_char(ch) )
            {
                return false;
            }

            if ( !has_slash )
            {
                has_slash = ascii.is_slash(ch);
            }
        }

        return has_slash && !ascii.is_slash(chars.back());
    }

    bool convert_to_relpath(std::string_view current, std::istream& in, std::ostream& out)
    {
        using traits = std::char_traits<char>;

        constexpr size_t detect_size{ _MAX_DRIVE + 1u };

        if ( std::size(current) <= detect_size || !is_valid_cd(current) )
        {
            return false;
        }

        small_vector<char, _MAX_PATH> buffer;

        bool need_search_div{ false };

        auto out_write = [&out] (cstrspan data)
        {
            out.write(data.data(), data.size());
        };

        for ( ;;)
        {
            const auto int_ch = in.get();
            if ( traits::eq_int_type(int_ch, traits::eof()) )
            {
                if ( buffer.size() == current.size() )
                {
                    out.put('.');
                }
                else
                {
                    out_write(buffer);
                }
                break;
            }

            const auto ch = traits::to_char_type(int_ch);
            if ( need_search_div )
            {
                D_ASSERT(!buffer.size());
                out.put(ch);
                need_search_div = !is_div_char(ch);
                continue;
            }

            const auto uni_ch = unipathchar(ch);

            auto pos = buffer.size();

            if ( pos < current.size() && uni_ch == current[pos] )
            {
                if ( !buffer.try_emplace_back(ch) )
                {
                    return false;
                }

                continue;
            }

            const auto is_div = is_div_char(ch);

            need_search_div = !is_div;

            if ( pos < detect_size )
            {
                out_write(buffer);
                buffer.clear();
                out.put(ch);
                continue;
            }

            const auto buffer_is_break = is_div || ascii.is_slash(ch);

            if ( pos == current.size() && buffer_is_break )
            {
                buffer.clear();
                if ( is_div )
                {
                    out.put('.');
                    out.put(ch);
                }
                continue;
            }

            if ( !buffer_is_break || !ascii.is_slash(current[pos]) )
            {
                do
                {
                    if ( ascii.is_slash(current[--pos]) )
                    {
                        break;
                    }
                }
                while ( pos );
            }

            constexpr char up[] = { '.', '.' };

            out_write(up);

            for ( auto up_pos = pos + 1; up_pos < current.size(); ++up_pos )
            {
                if ( ascii.is_slash(current[up_pos]) )
                {
                    out.put(preferred_path_separator);
                    out_write(up);
                }
            }

            out_write(cstrspan{ buffer }.subspan(pos));
            buffer.clear();
            out.put(ch);
        }

        return true;
    }
}

int main()
{
    constexpr int failed = -1;
    constexpr int successed = 0;

    std::ios_base::sync_with_stdio(false);

    const auto current_path = std::filesystem::current_path();
    if ( !current_path.is_absolute() )
    {
        return failed;
    }

    auto current_path_string = current_path.string();
    if ( !convert_to_relpath(unipath(current_path_string), std::cin, std::cout) )
    {
        return failed;
    }

    return successed;
}

