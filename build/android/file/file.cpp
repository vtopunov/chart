#include <file/file.h>

#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include <android/asset_manager.h>

#include <core/clamp_cast.h>
#include <core/narrow.h>

#include <common/asset_manager.h>

#include <file/file_io.h>

#include "private/private_path.h"
#include "private/private_file.h"


namespace file
{
    namespace
    {
        static_assert(STDIN_FILENO == to_underlying(stdin_fd.fd));
        static_assert(STDOUT_FILENO == to_underlying(stdout_fd.fd));
        static_assert(STDERR_FILENO == to_underlying(stderr_fd.fd));

        enum class open_flags
        {
            ro = O_RDONLY,
            wo = O_WRONLY,
            rw = O_RDWR,
            create = O_CREAT,
            exclusive = O_EXCL,
            truncate = O_TRUNC,
            append = O_APPEND
        };

        constexpr int permissions{ S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH };

        [[nodiscard]]
        constexpr open_flags operator | (open_flags left, open_flags right) noexcept
        {
            return e_or(left, right);
        }

        template<class... Args>
        [[nodiscard]] file_resource_descriptor_t _open(path_zstring_view path, Args... args) noexcept
        {
            return underlying_cast<file_resource_descriptor_t>(::open(path.c_str(), underlying_cast<int>(args)...));
        }

        struct asset_deleter
        {
            void operator () (AAsset* asset) const noexcept
            {
                if (asset)
                {
                    AAsset_close(asset);
                }
            }
        };

        using asset_t = unique_resource<AAsset*, asset_deleter>;

        [[nodiscard]]
        asset_t asset_open(AAssetManager* am, path_zstring_view path) noexcept
        {
            return
            {
                resource_construct,
                AAssetManager_open(am, path.c_str(), AASSET_MODE_BUFFER)
            };
        }

        class unpacking_asset_tmp
        {
            static constexpr path_zstring_view temp_path{ "unpacking_asset.tmp" };

        public:
            unpacking_asset_tmp(std::mutex& guard) noexcept
                : unpacking_lock_{ guard }
            {}

            D_DISABLE_COPY_MOVE(unpacking_asset_tmp);

            [[nodiscard]]
            bool unpack_from_buffer(const void* buffer, size_t len) noexcept
            {
                D_ASSERT(buffer);
                D_ASSERT(len);

                const auto temp_file = open_tmp();
                if (D_UNLIKELY(!temp_file))
                {
                    return false;
                }

                const auto w_len = file::write(temp_file, buffer, len);
                return w_len == len;
            }

            void move_to(path_zstring_view path) noexcept
            {
                D_ASSERT(!is_null_or_empty(path.c_str()));

                create_file_directories(path.c_str());

                autoremove_is_enabled_ = !!::rename(temp_path.c_str(), path.c_str());
            }

            ~unpacking_asset_tmp() noexcept
            {
                if (D_UNLIKELY(autoremove_is_enabled_))
                {
                    remove_tmp();
                }
            }

        private:
            [[nodiscard]]
            static wo_file try_open_tmp() noexcept
            {
                return
                {
                    resource_construct,
                    _open(temp_path, open_flags::create | open_flags::exclusive | open_flags::wo, permissions)
                };
            }

            [[nodiscard]]
            static wo_file open_tmp() noexcept
            {
                auto temp_file = try_open_tmp();
                if (D_UNLIKELY(!temp_file))
                {
                    remove_tmp();
                    temp_file = try_open_tmp();
                }

                return temp_file;
            }

            static void remove_tmp() noexcept
            {
                ::remove(temp_path.c_str());
            }

        private:
            bool autoremove_is_enabled_{ true };
            std::lock_guard<std::mutex> unpacking_lock_;
        };

        template<class... Args>
        file_resource_descriptor_t open_file_or_asset(path_zstring_view path, Args... args) noexcept
        {
            constexpr auto invalid = file_resource_descriptor_t::invalid;

            auto result = _open(path, args...);
            if (D_LIKELY(invalid != result))
            {
                return result;
            }

            if (D_UNLIKELY(is_null_or_empty(path.c_str())))
            {
                return invalid;
            }

            const auto am = common::asset_manager();
            if (D_UNLIKELY(!am))
            {
                return invalid;
            }

            const auto asset = asset_open(am, path);
            if (!asset)
            {
                return invalid;
            }

            const auto asset_buffer = AAsset_getBuffer(asset);
            if (D_UNLIKELY(!asset_buffer))
            {
                return invalid;
            }

            const auto asset_len = safe_numeric_cast<size_t>(clamp_to_unsigned(AAsset_getLength(asset)));
            if (D_UNLIKELY(!asset_len))
            {
                return invalid;
            }

            {
                static std::mutex unpack_mutex{};
                unpacking_asset_tmp temp_asset{ unpack_mutex };

                if (D_UNLIKELY(!temp_asset.unpack_from_buffer(asset_buffer, asset_len)))
                {
                    return invalid;
                }

                temp_asset.move_to(path);

                result = _open(path, args...);
            }

            return result;
        }


        file_resource_descriptor_t ro_open_file_or_asset(path_zstring_view path) noexcept
        {
            return open_file_or_asset(path, open_flags::ro);
        }

        file_resource_descriptor_t w_open_file_or_asset(path_zstring_view path, open_flags flags) noexcept
        {
            return open_file_or_asset(path, open_flags::create | flags, permissions);
        }

        constexpr open_flags mode_to_flags(w_open_mode mode) noexcept
        {
            switch (mode)
            {
            case w_open_mode::truncate:
                return open_flags::truncate;
            case w_open_mode::append:
                return open_flags::append;
            default:
                break;
            }

            return {};
        }
    }

    void file_resource_deleter::operator()(file_resource file) const noexcept
    {
        if (invalidfile != file)
        {
            ::close(to_underlying(file.fd));
        }
    }

    ro_file ro_open(path_zstring_view path) noexcept
    {
        return
        {
            resource_construct,
            ro_open_file_or_asset(path)
        };
    }

    wo_file wo_open(path_zstring_view path, w_open_mode mode) noexcept
    {
        return
        {
            resource_construct,
            w_open_file_or_asset(path, open_flags::wo | mode_to_flags(mode))
        };
    }

    rw_file rw_open(path_zstring_view path, w_open_mode mode) noexcept
    {
        return
        {
            resource_construct,
            w_open_file_or_asset(path, open_flags::rw | mode_to_flags(mode))
        };
    }

    uint64_t size(file_resource file) noexcept
    {
        struct stat data { .st_size{} };
        const auto is_success = !::fstat(file_resource_to_native(file), &data);
        return  is_success ? narrow_cast<uint64_t>(data.st_size) : 0ull;;
    }
}