#include <core/small_vector.h>

#include <file/file_io.h>
#include <file/file_mmap.h>

namespace
{
    struct chars_sequence
    {
        char first;
        char last;

        [[nodiscard]]
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

        [[nodiscard]]
        constexpr bool is_upper(const char value) const noexcept
        {
            return upper_abc.contains(value);
        }

        [[nodiscard]]
        constexpr bool is_slash(const char value) const noexcept
        {
            return value == slash
                || value == backslash;
        }

        [[nodiscard]]
        constexpr char upper2lower(const char value) const noexcept
        {
            return lower_abc.first + (value - upper_abc.first);
        }
    };

    constexpr locale ascii
    {
        .upper_abc{ 'A', 'Z' },
        .lower_abc{ 'a', 'z' },
        .slash{ '/' },
        .backslash{ '\\' }
    };

    struct path_spec
    {
        char preferred_sep;
        char alternative_sep;
        bool case_insensitive;

        [[nodiscard]]
        constexpr char unisep(char ch) const noexcept
        {
            return (ch == alternative_sep) ? preferred_sep : ch;
        }

        [[nodiscard]]
        constexpr char unicase(char ch) const noexcept
        {
            return (case_insensitive && ascii.is_upper(ch)) ? ascii.upper2lower(ch) : ch;
        }

        [[nodiscard]]
        constexpr char unichar(char ch) const noexcept
        {
            return unicase(unisep(ch));
        }

        [[nodiscard]]
        constexpr bool unieq(char c0, char c1) const noexcept
        {
            return unichar(c0) == unichar(c1);
        }
    };

    constexpr path_spec ms_path_spec
    {
        .preferred_sep{ ascii.backslash },
        .alternative_sep{ ascii.slash },
        .case_insensitive{ true }
    };

    constexpr void remove_endslash(std::string& s) noexcept
    {
        while (s.size() && ascii.is_slash(s.back()))
        {
            s.pop_back();
        }
    }

    [[nodiscard]]
    constexpr bool is_div_char(const char value) noexcept
    {
        return value == ';'
            || value == '<'
            || value == '>'
            || value == '|'
            || value == '\n'
            || value == '\''
            || value == '\"'
            || value == '*';
    }

    [[nodiscard]]
    constexpr bool is_valid_cd(std::string_view chars) noexcept
    {
        bool has_slash{ false };

        for (const auto ch : chars)
        {
            if (is_div_char(ch))
            {
                return false;
            }

            if (!has_slash)
            {
                has_slash = ascii.is_slash(ch);
            }
        }

        return has_slash && !ascii.is_slash(chars.back());
    }

    class wo_buffered_file_resource
    {
    public:
        static constexpr size_t buffer_size{ 1024_uz };

D_WARNING_PUSH
D_WARNING_DISABLE_MSVC(W_variable_is_uninitialized) // buffer_
        constexpr explicit wo_buffered_file_resource(file::wo_file_resource out) noexcept
            : out_{ out }
            , size_{ 0_uz }
        {}
D_WARNING_POP

        void put(char ch) noexcept
        {
            *_memory_for(1_uz) = ch;
        }

        void write(const_buffer_view data) noexcept
        {
            if (data.size() > buffer_size)
            {
                _flush_all();
                _direct_write(data.data(), data.size());
                return;
            }

            memcpy(_memory_for(data.size()), data.data(), data.size());
        }

        ~wo_buffered_file_resource() noexcept
        {
            _flush_all();
        }


    private:
        [[nodiscard]]
        char* _memory_for(size_t size) noexcept
        {
            _flush_for(size);

            const auto mem = buffer_ + size_;
            size_ += size;
            return mem;
        }

        void _flush_all() noexcept
        {
            _flush_for(buffer_size);
        }

        void _flush_for(size_t size) noexcept
        {
            if (size > (buffer_size - size_))
            {
                _direct_write(buffer_, std::exchange(size_, 0_uz));
            }
        }

        void _direct_write(const void* data, size_t size) const noexcept
        {
            file::write(out_, data, size);
        }

    private:
        char buffer_[buffer_size];
        file::wo_file_resource out_;
        size_t size_;
    };

    [[nodiscard]]
    constexpr size_t find_first_endslash(std::string_view str, size_t pos) noexcept
    {
        pos = std::min(pos, str.size());

        while (pos && !ascii.is_slash(str[pos]))
        {
            --pos;
        }

        return pos;
    }

    void write_relpath(wo_buffered_file_resource& out, std::string_view current, size_t pos) noexcept
    {
        constexpr char sep_up[]{ ms_path_spec.preferred_sep, '.', '.' };
        constexpr std::span sep_up_sp{ sep_up };
        constexpr auto up_sp = sep_up_sp.last<2_uz>();

        out.write(up_sp);

        for (auto up_pos = pos + 1_uz; up_pos < current.size(); ++up_pos)
        {
            if (ascii.is_slash(current[up_pos]))
            {
                out.write(sep_up_sp);
            }
        }
    }

    void write_tail(wo_buffered_file_resource& out, std::span<const char> buffer, size_t pos) noexcept
    {
        const auto tail = buffer.subspan(pos);
        out.write(tail);
    }

    [[nodiscard]]
    bool convert_to_relpath(std::string_view path, std::string_view in, file::wo_file_resource out_res) noexcept
    {
        constexpr size_t detect_size{ _MAX_DRIVE + 1_uz };

        if (std::size(path) <= detect_size)
        {
            return false;
        }

        if (!is_valid_cd(path))
        {
            return false;
        }

        wo_buffered_file_resource out{ out_res };
        small_vector<char, _MAX_PATH> relpathbuffer;
        bool need_search_div{ false };

        for (const auto ch : in)
        {
            if (need_search_div)
            {
                D_ASSERT(!relpathbuffer.size());
                out.put(ch);
                need_search_div = !is_div_char(ch);
                continue;
            }

            auto pos = relpathbuffer.size();

            if (pos < path.size() && ms_path_spec.unieq(ch, path[pos]))
            {
                if (!relpathbuffer.try_emplace_back(ch))
                {
                    return false;
                }

                continue;
            }

            const auto is_div = is_div_char(ch);

            need_search_div = !is_div;

            if (pos < detect_size)
            {
                out.write(relpathbuffer);
                out.put(ch);

                relpathbuffer.clear();
                continue;
            }

            {
                const auto relpath_is_break = is_div || ascii.is_slash(ch);

                if (relpath_is_break)
                {
                    if (pos == path.size())
                    {
                        if (is_div)
                        {
                            out.put('.');
                            out.put(ch);
                        }

                        relpathbuffer.clear();
                        continue;
                    }
                }
                else
                {
                    pos = find_first_endslash(path, pos);
                }
            }

            write_relpath(out, path, pos);
            write_tail(out, relpathbuffer, pos);
            out.put(ch);

            relpathbuffer.clear();
        }

        if (relpathbuffer.size() == path.size())
        {
            out.put('.');
        }
        else
        {
            out.write(relpathbuffer);
        }

        return true;
    }
}

int wmain(int argc, wchar_t* argv[], wchar_t**)
{
    constexpr int failed = -1;
    constexpr int successed = 0;

    if (argc <= 1)
    {
        return failed;
    }

    const std::filesystem::path path{ argv[1] };

    const auto map = file::mmap(path.c_str());
    if (!map)
    {
        return failed;
    }

    auto cd = absolute(path).parent_path().generic_string();

    remove_endslash(cd);

    if (cd.empty())
    {
        return failed;
    }

    if (!convert_to_relpath(cd, view(map).as_str<char>(), file::out()))
    {
        return failed;
    }

    return successed;
}

