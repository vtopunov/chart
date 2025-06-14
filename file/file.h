#pragma once

#include <core/resource.h>

#include <file/path.h>


namespace file
{
    enum class w_open_mode
    {
        rewrite,
        truncate,
        append
    };

    namespace private_detail_file_descriptor
    {
        constexpr auto file_descriptor_is_integral_v = std::is_integral_v<os::file_descriptor_t>;

        struct s_file_descriptor
        {};

        using file_descriptor_int_t = std::conditional_t<file_descriptor_is_integral_v, os::file_descriptor_t, int>;

        enum class e_file_descriptor : file_descriptor_int_t
        {
            invalid = -1,
            stdin_fileno = 0,
            stdout_fileno = 1,
            stderr_fileno = 2
        };

        using file_resource_descriptor_t = std::conditional_t<file_descriptor_is_integral_v, e_file_descriptor, s_file_descriptor*>;
    }

    using private_detail_file_descriptor::file_resource_descriptor_t;

    struct file_resource
    {
        struct null_type
        {
            template<class N>
            constexpr operator N () const noexcept
            {
                static_assert(std::is_base_of_v<file_resource, N>);

                if constexpr (private_detail_file_descriptor::file_descriptor_is_integral_v)
                {
                    return { file_resource_descriptor_t::invalid };
                }
                else
                {
                    return { file_resource_descriptor_t(-1) };
                }
            }
        };

        struct deleter_type
        {
            void operator () (file_resource resource) const noexcept;
        };

        file_resource_descriptor_t fd;

        [[nodiscard]]
        constexpr bool operator == (const file_resource&) const noexcept = default;

        [[nodiscard]]
        constexpr bool operator != (const file_resource&) const noexcept = default;
    };

    using invalidfile_t = null_t<file_resource>;
    static_assert(std::is_same_v<invalidfile_t, file_resource::null_type>);
    constexpr invalidfile_t invalidfile{};

    struct ro_file_resource : file_resource
    {};

    struct wo_file_resource : file_resource
    {};

    struct rw_file_resource : file_resource
    {
        [[nodiscard]]
        constexpr operator ro_file_resource() const noexcept
        {
            return ro_file_resource{ fd };
        }

        [[nodiscard]]
        constexpr operator wo_file_resource() const noexcept
        {
            return wo_file_resource{ fd };
        }
    };

    static_assert(sizeof(file_resource) == sizeof(os::file_descriptor_t));
    static_assert(sizeof(ro_file_resource) == sizeof(os::file_descriptor_t));
    static_assert(sizeof(wo_file_resource) == sizeof(os::file_descriptor_t));
    static_assert(sizeof(rw_file_resource) == sizeof(os::file_descriptor_t));


#ifdef D_OS_WINDOWS
    struct stdin_file_resource
    {
        operator ro_file_resource() const noexcept;
    };

    constexpr stdin_file_resource stdin_fd{};

    struct stdout_file_resource
    {
        operator wo_file_resource() const noexcept;
    };

    constexpr stdout_file_resource stdout_fd{};

    struct stderr_file_resource
    {
        operator wo_file_resource() const noexcept;
    };

    constexpr stderr_file_resource stderr_fd{};

#else
    constexpr ro_file_resource stdin_fd{ file_resource_descriptor_t::stdin_fileno };

    constexpr wo_file_resource stdout_fd{ file_resource_descriptor_t::stdout_fileno };

    constexpr wo_file_resource stderr_fd{ file_resource_descriptor_t::stderr_fileno };

#endif

    struct ro_file : unique_resource<ro_file_resource>
    {
        using unique_resource::unique_resource;

        [[nodiscard]]
        constexpr operator file_resource() const noexcept
        {
            return static_cast<file_resource>(r());
        }
    };

    struct wo_file : unique_resource<wo_file_resource>
    {
        using unique_resource::unique_resource;

        [[nodiscard]]
        constexpr operator file_resource() const noexcept
        {
            return static_cast<file_resource>(r());
        }
    };

    struct rw_file : unique_resource<rw_file_resource>
    {
        using unique_resource::unique_resource;

        [[nodiscard]]
        constexpr operator file_resource() const noexcept
        {
            return static_cast<file_resource>(r());
        }

        [[nodiscard]]
        constexpr operator ro_file_resource() const noexcept
        {
            return static_cast<ro_file_resource>(r());
        }

        [[nodiscard]]
        constexpr operator wo_file_resource() const noexcept
        {
            return static_cast<wo_file_resource>(r());
        }
    };


    [[nodiscard]]
    ro_file ro_open(path_zstring_view path) noexcept;

    [[nodiscard]]
    wo_file wo_open(path_zstring_view path, w_open_mode mode) noexcept;

    [[nodiscard]]
    rw_file rw_open(path_zstring_view path, w_open_mode mode) noexcept;

    [[nodiscard]]
    uint64_t size(file_resource file) noexcept;
}