#include "private_path.h"

#include <sys/stat.h>

#include <core/small_vector.h>

namespace file
{
    namespace
    {
        constexpr size_t max_path{ 256_uz };
        constexpr int permissions{ S_IRWXU | S_IRWXG | S_IRWXO };
        constexpr path_char_t path_sep{ '/' };


        template<class T>
        T* _find_n(T* data, size_t size, std::type_identity_t<T> ch) noexcept
        {
            static_assert(1_uz == sizeof(T));
            return static_cast<T*>(memchr(data, ch, size));
        }

        template<class T>
        T* _find(T* data, std::add_pointer_t<std::add_const_t<T>> end_data, std::type_identity_t<T> ch) noexcept
        {
            return _find_n(data, narrow_cast<size_t>(end_data - data), ch);
        }
    }

    void create_file_directories(path_string_view path) noexcept
    {
        const auto first_psep = _find_n(path.data(), path.size(), path_sep);
        if (nullptr == first_psep)
        {
            return;
        }

        small_vector<path_char_t, max_path> mut_path{};
        mut_path.reserve(path.size());
        const auto size = std::min(path.size(), mut_path.capacity());
        const auto first_sep_pos = narrow_cast<size_t>(first_psep - path.data());
        if (D_UNLIKELY(size <= first_sep_pos))
        {
            return;
        }

        memcpy(mut_path.data(), path.data(), size);

        {
            auto it = mut_path.data() + first_sep_pos;
            const auto end_it = std::as_const(mut_path).data() + size;

            do
            {
                *it = '\0';
                mkdir(std::as_const(mut_path).data(), permissions);
                *it = path_sep;

                ++it;
                const auto psep = _find(it, end_it, path_sep);
                it = psep;
            } while (it);
        }
    }
}