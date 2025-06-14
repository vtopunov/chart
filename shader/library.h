#pragma once

#include <core/fwd.h>

#if D_IS_DEBUG
#include <bitset>
#endif

#include <shader/minembed.h>


namespace private_detail_shader_library
{
    class program_base
    {
    public:
        void use() const noexcept
        {
            gl::use(program_);
        }

        [[nodiscard]] constexpr bool in_use() const noexcept
        {
            return program_ && (program_ == gl::current_program());
        }

        [[nodiscard]] constexpr gl::program_resource program() const noexcept
        {
            return program_;
        }

    protected:
        [[nodiscard]] bool _build(gl::vertex_source_view vs, gl::fragment_source_view fs) noexcept
        {
            D_ASSERT(!program_);
            program_ = gl::create_program(vs, fs);
            return !!program_;
        }

    private:
        gl::program program_{};
    };

    template<size_t CountOfLocations>
    class program_locations : public program_base
    {
    public:
        constexpr program_locations() noexcept
        {
            locations_.fill(gl::uniform_location::invalid);
        }

        [[nodiscard]] constexpr bool debug_all_in_use() const noexcept
        {
            return in_use() && D_DEBUG_OR(debug_location_in_use_.all(), true);
        }

        [[nodiscard]] constexpr gl::uniform_location uniform_location(size_t index) const noexcept
        {
            D_ASSERT(in_use());
            D_ASSERT_OR_ASSUME(index < std::size(locations_));
            D_ASSERT_OR_ASSUME(gl::uniform_location::invalid != locations_[index]);
            D_ONLY_DEBUG(debug_location_in_use_.set(index));
            return locations_[index];
        }

    protected:
        static constexpr size_t _count_of_locations{ CountOfLocations };
        static_assert(0_uz < _count_of_locations);

        struct write_location_ref
        {
            mutable gl::uniform_location* p_current;

#if D_IS_DEBUG
            mutable size_t debug_index_{ 0_uz };
#endif

            constexpr void operator () (gl::uniform_location loc) const noexcept
            {
                D_ASSERT(debug_index_ < _count_of_locations);
                D_ASSERT(gl::uniform_location::invalid == *p_current);
                *p_current = loc;
                ++p_current;
                D_ONLY_DEBUG(++debug_index_);
            }
        };

        [[nodiscard]] constexpr write_location_ref _write_location_ref() noexcept
        {
            return { locations_.data() };
        }

    private:
        std::array<gl::uniform_location, _count_of_locations> locations_;

#if D_IS_DEBUG
        mutable std::bitset<_count_of_locations> debug_location_in_use_{};
#endif
    };

    template<>
    class program_locations<0_uz> : public program_base
    {
    public:
        constexpr program_locations() noexcept = default;

        [[nodiscard]] constexpr bool debug_all_in_use() const noexcept
        {
            return in_use();
        }
    };

    template<class ExportsPack>
    class program_exports_base : public program_locations<shader_common::count_of_locations_v<ExportsPack> >
    {
    public:
        using exports_pack_type = ExportsPack;
        static_assert(0_uz < ttypes_size_v<exports_pack_type>);

    protected:
        [[nodiscard]] bool _load(gl::vertex_source_view vs, gl::fragment_source_view fs) noexcept
        {
            if (this->_build(vs, fs)) [[likely]]
            {
                this->use();
                _load_impl();
                return true;
            }

            return false;
        }

    private:
        constexpr void _load_impl() noexcept
        {
            using unique_export_variables_t = ttypes_unique_t<shader_common::export_pack_variables_t<ExportsPack> >;
            constexpr auto has_indexed_location_uniform = ttypes_has_v<shader_common::has_indexed_location, unique_export_variables_t>;
            constexpr auto has_sampler = ttypes_has_v<shader_common::is_sampler, unique_export_variables_t>;
            constexpr auto has_non_uniform = ttypes_has_v<shader_common::is_not_uniform, unique_export_variables_t>;

            if constexpr (has_indexed_location_uniform)
            {
                shader_common::for_each_export<ExportsPack>([program_handle = this->program(), write_location = this->_write_location_ref()] <class T> (const T & u) noexcept
                {
                    if constexpr (shader_common::has_indexed_location_v<T>)
                    {
                        write_location(u.location(program_handle));
                    }
                });
            }

            if constexpr (has_sampler)
            {
                struct mutable_simpler_couter
                {
                    mutable GLint count;
                };

                shader_common::for_each_export<ExportsPack>([program_handle = this->program(), simpler_counter = mutable_simpler_couter{ 0 }] <class T> (const T & u) noexcept
                {
                    if constexpr (shader_common::is_sampler_v<T>)
                    {
                        u.store(u.location(program_handle), simpler_counter.count);
                        ++simpler_counter.count;
                    }
                });
            }

            if constexpr (has_non_uniform)
            {
                shader_common::for_each_export<ExportsPack>([program_handle = this->program()] <class T> (const T & u) noexcept
                {
                    if constexpr (shader_common::is_not_uniform_v<T>)
                    {
                        u.bind(program_handle);
                    }
                });
            }
        }
    };

    template<class... Interfaces>
    struct root_interface : Interfaces...
    {};

    template<class ExportsPack, class Interfaces>
    struct program_exports :
        program_exports_base<ExportsPack>,
        ttypes_repack_t<Interfaces, root_interface>
    {};

    template<class... Shaders>
    using exports_t = ttypes_unique_t<types_cat_t<shader_common::decl_exports_t<Shaders>...>>;

    namespace private_detail_interfaces
    {
        template<class Lib, class Exports, class = void>
        struct interfaces_helper;

        template<class Lib, class... Exports>
        struct interfaces_helper<Lib, ttypes<Exports...>, std::void_t<shader_common::decl_interface_t<Exports, Lib>...> >
        {
            using type = ttypes_unique_insert_back_t<ttypes<>, shader_common::decl_interface_t<Exports, Lib>...>;
        };

        template<class Lib, class Exports>
        using interfaces_t = typename interfaces_helper<Lib, Exports>::type;
    }

    using private_detail_interfaces::interfaces_t;

    template<class Derived, class Exports>
    using program_exports_t = program_exports<Exports, interfaces_t<Derived, Exports> >;

    template<class VS, class FS>
    struct shader_library : program_exports_t<
        shader_library<VS, FS>,
        exports_t<VS, FS>
    >
    {
        static_assert(is_unqualified_class_v<VS>);
        static_assert(is_unqualified_class_v<FS>);

        using vertex_shader_type = VS;
        using fragment_shader_type = FS;

        static_assert(gl::shader_type::vertex == vertex_shader_type::source.type_id);
        static_assert(gl::shader_type::fragment == fragment_shader_type::source.type_id);

        [[nodiscard]] bool load() noexcept
        {
            return this->_load(vertex_shader_type::source, fragment_shader_type::source);
        }
    };
}

using private_detail_shader_library::shader_library;

namespace shader_embed
{
    using default_texture = shader_library<vert::positioned_texture, frag::default_texture>;
    using luminance_texture = shader_library<vert::positioned_texture, frag::luminance_texture>;
    using colored_rectangle = shader_library<vert::positioned_rectangle, frag::default_color>;
}